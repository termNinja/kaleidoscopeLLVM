#include "ast.hpp"

#define INDENT "    "

LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
std::unique_ptr<Module> TheModule;
std::map<std::string, Value*> NamedValues;

Value* logError(std::string errMsg) {
	std::cerr << errMsg << std::endl;
	return nullptr;
}

// ====----====----====----====----====----====----====----====----====----====
// CODEGEN
// ====----====----====----====----====----====----====----====----====----====
Value* NumberExprAST::codegen() const {
	return LLVM_FP(m_val);
}

Value* VariableExprAST::codegen() const {
	Value* v = NamedValues[m_name];
	if (! v) return logError("Unknown variable: '" + m_name + "'");
	return v;
}

Value* BinaryExprAST::codegen() const {
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
		case '<':
			// CreateFCmpULT returns a 1bit integer which we must covnert to a double
			left = Builder.CreateFCmpULT(left, right, "cmptmp");
			return Builder.CreateUIToFP(left, Type::getDoubleTy(TheContext), "booltmp");
		default:
			std::cout << "Unknown binary operator '" << m_op << "'" << std::endl;
			return nullptr;
	}

	return nullptr;
}

Value* CallExprAST::codegen() const {
	// We try to fetch the function
	Function* theFunction = TheModule->getFunction(m_name);
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
	return theFunction;
}

Function* FunctionAST::codegen() const {
	// We must take care here, we wish for the function NOT to have a body
	// here (only a declaration).
	// We check if some functions already exists with the same name
	Function* theFunction = TheModule->getFunction(m_proto.name());

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
	for (auto &argument : theFunction->args())
		NamedValues[argument.getName()] = &argument;
	
	// Finally, we try to generate the function body and function return value
	Value* functionBody = m_definition->codegen();
	if (functionBody == nullptr) {
		theFunction->eraseFromParent(); // we delete the function from the symtable
		return (Function*)logError("Failed generating code for function definition of '" + m_proto.name() + "'");
	} else {
		Builder.CreateRet(functionBody);
		verifyFunction(*theFunction);
		return theFunction;
	}
}

// ====----====----====----====----====----====----====----====----====----====
// Used to show parsed code like it looks in source code
// ====----====----====----====----====----====----====----====----====----====
std::string NumberExprAST::src_show() const {
	return std::to_string(this->m_val);
}

std::string VariableExprAST::src_show() const {
	return this->m_name;
}

std::string BinaryExprAST::src_show() const {
	return this->m_left->src_show() + " " + this->m_op + " " + this->m_right->src_show();
}

std::string CallExprAST::src_show() const {

	std::string res = this->m_name;
	res += " (";

	if (! m_exps.empty()) {
		unsigned i = 0;
		for (; i < this->m_exps.size()-1; ++i)
			res += m_exps[i]->src_show() + ", ";
		res += m_exps[i]->src_show();
	}

	res += ")";
	return res;
}

std::string PrototypeAST::src_show() const {
	std::string res = this->m_name;

	res += "(";
	if (! this->m_args.empty()) {
		unsigned i = 0;
		for (; i < m_args.size()-1; ++i)
			res += m_args[i] + ", ";
		res += m_args[i];
	}
	res += ")";
	return res;
}

std::string FunctionAST::src_show() const {
	std::string res =  m_proto.src_show() + "\n";
	res += INDENT + m_definition->src_show();
	return res;
}
