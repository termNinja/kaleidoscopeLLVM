#include "ast.hpp"

#define INDENT "    "

LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
std::unique_ptr<Module> TheModule;
std::map<std::string, AllocaInst*> NamedValues;
std::unique_ptr<legacy::FunctionPassManager> TheFPM;
std::unique_ptr<orc::KaleidoscopeJIT> TheJIT;
std::map<std::string, PrototypeAST> FunctionProtos;

Value* logError(std::string errMsg) {
	std::cerr << errMsg << std::endl;
	return nullptr;
}

Function* getFunction(std::string name) {
	// We search the given function inside our current module
	Function* f = TheModule->getFunction(name);
	if (f != nullptr) return f;

	// Can we codegen() from some existing prototype?
	auto searchRes = FunctionProtos.find(name);
	if (searchRes != FunctionProtos.end()) {
		return searchRes->second.codegen();
	}

	// Otherwise, we don't find the required function
	return nullptr;
}

AllocaInst* CreateEntryBlockAlloca(Function* TheFunction, const std::string& name) {
	IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
	return TmpB.CreateAlloca(LLVM_DOUBLETY, 0, name.c_str());
}

// ====----====----====----====----====----====----====----====----====----====
// CODEGEN
// ====----====----====----====----====----====----====----====----====----====
Value* NumberExprAST::codegen() const {
	return LLVM_FP(m_val);
}

Value* VariableExprAST::codegen() const {
	AllocaInst* varAddres = NamedValues[m_name];
	if (! varAddres) return logError("Unknown variable: '" + m_name + "'");
	return Builder.CreateLoad(varAddres);
	//return Builder.CreateLoad(varAddres, m_name.c_str());
}

Value* BinaryExprAST::codegen() const {
	if (m_op == '=') {
		Value* assignMeHomie = m_right->codegen();
		if (! assignMeHomie) return logError("Failed m_right->codegen() in BinaryExprAST::codegen()");
		VariableExprAST* varAST = dynamic_cast<VariableExprAST*>(m_left);
		if (varAST == nullptr) return logError("Bad left operand in assignment operator '=' in BinaryExprAST::codegen()");

		return Builder.CreateStore(assignMeHomie, NamedValues[varAST->name()]);
	}
	Value* left = m_left->codegen();
	Value* right = m_right->codegen();
	if (!left || !right) {
		if (!left) std::cerr << "Got nullptr from left operand in BinaryExprAST::codegen()" << std::endl;
		if (!right) std::cerr << "Got nullptr from right operand in BinaryExprAST::codegen()" << std::endl;
		return nullptr;
	}
	switch (m_op) {
		case '+': return Builder.CreateFAdd(left, right, "tmpadd");
		case '-': return Builder.CreateFSub(left, right, "tmpsub");
		case '*': return Builder.CreateFMul(left, right, "tmpmul");
		case ':': return right;
		case '<':
			// CreateFCmpULT returns a 1bit integer which we must covnert to a double
			left = Builder.CreateFCmpULT(left, right, "cmplt");
			return Builder.CreateUIToFP(left, Type::getDoubleTy(TheContext), "boollt");
		case '>':
			left = Builder.CreateFCmpUGT(left, right, "cmpgt");
			return Builder.CreateUIToFP(left, Type::getDoubleTy(TheContext), "boolgt");
		default:
			std::cout << "Unknown binary operator '" << m_op << "'" << std::endl;
			return nullptr;
	}

	return nullptr;
}

Value* VarDefExprAST::codegen() const {
	std::vector<AllocaInst*> oldVarAddresses;
	Function* TheFunction = Builder.GetInsertBlock()->getParent();

	// We iterate over a list of decl-assignments
	// Ex. i = 1, j, k = 2  <- <string, ExprAST*>
	for (auto &ass : m_varDeclDefs) {
		// We save the old variable address
		oldVarAddresses.push_back(NamedValues[ass.first]);

		// Calculate the value to assign to var (default is 0.0)
		Value* initVal = nullptr;
		if (ass.second == nullptr) {
			initVal = LLVM_FP(0.0);
		} else {
			initVal = ass.second->codegen();
			if (! initVal) return logError("Failed codegen() in VarDefExprAST::codegen()");
		}

		// Fetch a new addr for a given variable
		NamedValues[ass.first] = CreateEntryBlockAlloca(TheFunction, ass.first);
		// And store a value on it
		Builder.CreateStore(initVal, NamedValues[ass.first]);
	}

	// We execute the body expression
	Value* bodyExpr = m_innerExpr->codegen();
	if (! bodyExpr) return logError("Failed m_innerExpr->codegen() in VarDefExprAST::codegen()");

	// We restore old var address (and remove ones we've added and didn't exist before)
	unsigned i = 0;
	for (auto &ass : m_varDeclDefs) {
		if (ass.second == nullptr) NamedValues.erase(ass.first);
		else NamedValues[ass.first] = oldVarAddresses[i++];
	}
	return bodyExpr;
}

/// Info: Changed ifthenelse with phi nodes onto alloca ! ^_^
/// For the record, if-then is natural for the PHI instrunction, but I did it just for practice
/// If you prefer the PHI version, you have it commented below :)
Value* IfThenElseExprAST::codegen() const {
	Value* cond = m_cond->codegen();
	if (! cond) return logError("Failed m_cond->codegen() in IfThenElseExprAST::codegen()");
	Function* TheFunction = Builder.GetInsertBlock()->getParent();

	// Allocate memory for some random var and backup old value
	std::string ifthenVar = "ifthenvar";
	AllocaInst* ifThenAddr = CreateEntryBlockAlloca(TheFunction, ifthenVar);
	auto finder = NamedValues.find(ifthenVar);
	AllocaInst* oldVarAddr = (finder == NamedValues.end() ? nullptr : NamedValues[ifthenVar]);
	NamedValues[ifthenVar] = ifThenAddr;

	// Create required basic blocks
	BasicBlock* thenBB = BasicBlock::Create(TheContext, "then_if", TheFunction);
	BasicBlock* elseBB = BasicBlock::Create(TheContext, "else_if");
	BasicBlock* mergeBB = BasicBlock::Create(TheContext, "merge_if");

	cond = Builder.CreateFCmpONE(cond, LLVM_FP(0.0), "ifcond");
	Builder.CreateCondBr(cond, thenBB, elseBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLING THEN
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Builder.SetInsertPoint(thenBB);
	Value* thenVal = m_thenExpr->codegen();
	if (! thenVal) return logError("Failed m_thenExpr->codegen() in IfThenElseExprAST::codegen()");
	Builder.CreateStore(thenVal, ifThenAddr);
	Builder.CreateBr(mergeBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLING ELSE
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(elseBB);
	Builder.SetInsertPoint(elseBB);
	Value* elseVal = m_elseExpr->codegen();
	if (! elseVal) return logError("Failed m_elseExpr->codegen() in IfThenElseExprAST::codegen()");
	Builder.CreateStore(elseVal, ifThenAddr);
	Builder.CreateBr(mergeBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLING MERGE
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(mergeBB);
	Builder.SetInsertPoint(mergeBB);
	Value* retVal = Builder.CreateLoad(ifThenAddr, "ifload");

	// Restore old value in NamedValues
	if (oldVarAddr == nullptr) NamedValues.erase(ifthenVar);
	else NamedValues[ifthenVar] = oldVarAddr;

	return retVal;
//	Value* cond = m_cond->codegen();
//	if (! cond)
//		return logError("Got nullptr from m_cond->codegen() in IfThenElseExprAST");
//
//	// Convert to i1
//	cond = Builder.CreateFCmpONE(cond, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
//
//	// Get the function we're working in
//	Function *TheFunction = Builder.GetInsertBlock()->getParent();
//
//	// Create basic blocks and conditional jump
//	BasicBlock* thenBB = BasicBlock::Create(TheContext, "then", TheFunction);
//	BasicBlock* elseBB = BasicBlock::Create(TheContext, "else");
//	BasicBlock* mergeBB = BasicBlock::Create(TheContext, "merge");
//	Builder.CreateCondBr(cond, thenBB, elseBB);
//
//	// Handle then BB
//	Builder.SetInsertPoint(thenBB);
//	Value* thenVal = m_thenExpr->codegen();
//	if (! thenVal) return logError("nullptr from then block in IfThenElseExprAST");
//	Builder.CreateBr(mergeBB);
//	thenBB = Builder.GetInsertBlock();
//
//	// handle else BB
//	TheFunction->getBasicBlockList().push_back(elseBB); 	// ATTENTION NINJA! This is easily forgotten
//	Builder.SetInsertPoint(elseBB);
//	Value* elseVal = m_elseExpr->codegen();
//	if (! elseVal) return logError("nullptr from else block in IfThenElseExprAST");
//	Builder.CreateBr(mergeBB);
//	elseBB = Builder.GetInsertBlock();
//
//	// handle the merge
//	TheFunction->getBasicBlockList().push_back(mergeBB);
//	Builder.SetInsertPoint(mergeBB);
//
//	// Add the sexy PHI node!
//	PHINode* thePHI = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "phitmp");
//	thePHI->addIncoming(thenVal, thenBB);
//	thePHI->addIncoming(elseVal, elseBB);
//
//	return thePHI;
}

/// Official LLVM tutorial has actually implemented a do while loop.
/// My implementation works the way a C for loop should.
Value* ForExprAST::codegen() const {
	Value* startVal = m_init->codegen();
	if (! startVal) return logError("Faileed m_init->codegen() in ForExprAST::codegen()");

	// Save old variable stack address
	auto finder = NamedValues.find(m_varName);
	AllocaInst* oldVarAddr = (finder == NamedValues.end() ? nullptr : finder->second);

	Function* TheFunction = Builder.GetInsertBlock()->getParent();
	//BasicBlock* preLoopBB = Builder.GetInsertBlock();

	// Get ourselves an stack address for loop var
	AllocaInst* loopVarAddr = CreateEntryBlockAlloca(TheFunction, m_varName);
	// We store the initial value onto our loop variable
	Builder.CreateStore(startVal, loopVarAddr);
	// And remeber its addres in symtable (so other parts of syntree can access the var)
	NamedValues[m_varName] = loopVarAddr;

	// Get ourselves some basic blocks
	BasicBlock* entryBB = BasicBlock::Create(TheContext, "entryLoop", TheFunction);
	BasicBlock* loopBB = BasicBlock::Create(TheContext, "bodyLoop");
	BasicBlock* endBB = BasicBlock::Create(TheContext, "endLoop");

	Builder.CreateBr(entryBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP ENTRY
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Builder.SetInsertPoint(entryBB);
	Value* cond = m_cond->codegen();
	if (! cond) return logError("Failed m_cond->codegen() in ForExprAST::codegen()");
	cond = Builder.CreateFCmpONE(cond, LLVM_FP(0.0), "forcmp");
	Builder.CreateCondBr(cond, loopBB, endBB);
	entryBB = Builder.GetInsertBlock(); // NOTE: added

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP BODY
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(loopBB);
	Builder.SetInsertPoint(loopBB);

	Value* bodyVal = m_body->codegen();

	if (! bodyVal) return logError("Failed m_body->codegen() in ForExprAST::codegen()");

	Value* loopStep = nullptr;
	if (m_step == nullptr) loopStep = LLVM_FP(1.0);
	else {
		loopStep = m_step->codegen();
		if (! loopStep) return logError("Failed m_step->codegen() in ForExprAST::codegen()");
	}
	Value* loopVarVal = Builder.CreateLoad(loopVarAddr);
	Value* newVal = Builder.CreateFAdd(loopVarVal, loopStep);
	Builder.CreateStore(newVal, loopVarAddr);
	Builder.CreateBr(entryBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP END
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(endBB);
	Builder.SetInsertPoint(endBB);

	// Restore old var
	if (oldVarAddr == nullptr) NamedValues.erase(m_varName);
	else NamedValues[m_varName] = oldVarAddr;

	return LLVM_FP(0.0);
}

Value* WhileExprAST::codegen() const {
	Function* TheFunction = Builder.GetInsertBlock()->getParent();

	BasicBlock* entryBB = BasicBlock::Create(TheContext, "entry_while", TheFunction);
	BasicBlock* loopBB = BasicBlock::Create(TheContext, "loop_while");
	BasicBlock* endBB = BasicBlock::Create(TheContext, "end_while");

	// Jump into entry
	Builder.CreateBr(entryBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP ENTRY
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Builder.SetInsertPoint(entryBB);
	Value* condVal = m_cond->codegen();
	if (! condVal) return logError("Failed m_cond->codegen() in WhileExprAST::codegen()");
	condVal = Builder.CreateFCmpONE(condVal, LLVM_FP(0.0), "forcmp");
	Builder.CreateCondBr(condVal, loopBB, endBB);
	entryBB = Builder.GetInsertBlock();

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP BODY
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(loopBB);
	Builder.SetInsertPoint(loopBB);
	Value* bodyVal = m_body->codegen();
	if (! bodyVal) return logError("Failed m_body->codegen() in WhileExprAST::codegen()");
	Builder.CreateBr(entryBB);

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// HANDLE LOOP END
	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TheFunction->getBasicBlockList().push_back(endBB);
	Builder.SetInsertPoint(endBB);

	return LLVM_FP(0.0);
}

Value* CallExprAST::codegen() const {
	// We try to fetch the function
	Function* theFunction = getFunction(m_name);
	if (! theFunction) {
		std::cerr << "Failed finding function: '" << m_name << "'" << std::endl;
		return nullptr;
	}

	// We check function arguments
	if (m_exps.size() != theFunction->arg_size()) {
		std::cerr << "Function expects " << theFunction->arg_size() << " arguments.\n";
		std::cerr << m_exps.size() << " given." << std::endl;
		return nullptr;
	}

	// We create arguments
	std::vector<Value*> args;
	for (auto & arg : m_exps)
		args.push_back(arg->codegen());
	return Builder.CreateCall(theFunction, args, "calltmp");
}

Function* PrototypeAST::codegen() const {
	std::vector<Type*> protoParameters(m_args.size(), Type::getDoubleTy(TheContext));
	FunctionType* ftype = FunctionType::get(Type::getDoubleTy(TheContext), protoParameters, false);
	Function* theFunction = Function::Create(ftype, Function::ExternalLinkage, m_name, TheModule.get());

	// Set function names for args (not required but sexy)
	unsigned areSexy = 0;
	for (auto &argument : theFunction->args())
		argument.setName(m_args[areSexy++]); 		// my args are sexy no?

	FunctionProtos.insert(std::pair<std::string, PrototypeAST>(m_name, *this));
	return theFunction;
}

Function* FunctionAST::codegen() const {
	// We must take care here, we wish for the function NOT to have a body
	// here (only a declaration).
	// We check if some functions already exists with the same name
	Function* theFunction = getFunction(m_proto.name());

	// If function doesn't exist, we start by generating a declaration from it
	if (theFunction == nullptr) theFunction = m_proto.codegen();

	// If that failed, we're fuc...return nullptr
	if (theFunction == nullptr)
		return (Function*)logError("Failed generating declaration for function.");

	// Finally, what if function function actually exists and has a body?
	if (! theFunction->empty())
		return (Function*)logError("Function '" + m_proto.name() + "' can't be redefined.");

	// Now we give our function a basic block in which we shall dump it's definition
	BasicBlock* funBB = BasicBlock::Create(TheContext, "entry", theFunction);
	Builder.SetInsertPoint(funBB);

	// Now we set arguments into namedValues so function can use it
	NamedValues.clear();
	for (auto &argument : theFunction->args()) {
		AllocaInst* argAddr = CreateEntryBlockAlloca(theFunction, argument.getName());
		NamedValues[argument.getName()] = argAddr;
		Builder.CreateStore(&argument, argAddr);
	}

	// Finally, we try to generate the function body and function return value
	Value* functionBody = m_definition->codegen();
	if (functionBody == nullptr) {
		theFunction->eraseFromParent(); // we delete the function from the symtable
		return (Function*)logError("Failed generating code for function definition of '" + m_proto.name() + "'");
	} else {
		Builder.CreateRet(functionBody);
		verifyFunction(*theFunction); 		// verify the function
		TheFPM->run(*theFunction); 			// optimize this function
		return theFunction;
	}
}

void InitializeModuleAndPassManager() {
	TheModule = make_unique<Module>("mah module", TheContext);
	TheFPM = make_unique<legacy::FunctionPassManager>(TheModule.get());
	TheFPM->doInitialization();
}

