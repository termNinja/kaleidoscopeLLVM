#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <string>
#include <iostream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetSelect.h"
#include "KaleidoscopeJIT.h"

#include <map>
#include <vector>

#define LLVM_FP(x) ConstantFP::get(TheContext, APFloat(x))
#define LLVM_DOUBLETY Type::getDoubleTy(TheContext)

using namespace llvm;

/// Writes a given msg to cerr and returns nullptr.
Value* logError(std::string errMsg);

/// Initializes the module and pass manager.
void InitializeModuleAndPassManager();

/// Returns the function if the function exists
/// either as a fully define function or as a prototype only.
Function* getFunction(std::string name);

/// Returns an address on stack for a variable called 'name'
/// inside the function called 'TheFunction'.
AllocaInst* CreateEntryBlockAlloca(Function* TheFunction, const std::string& name);

/// Represents an abstract node of the syntax tree (an expression)
class ExprAST {
public:
	virtual ~ExprAST() {}
	virtual Value* codegen() const = 0;
};

/// Represents an node that contains a constant. Ex '5.1'
class NumberExprAST : public ExprAST {
public:
	NumberExprAST(double val)
		: m_val(val)
	{}
	Value* codegen() const;

private:
	double m_val;
};

/// Represents a variable name. Ex. 'x'
class VariableExprAST : public ExprAST {
public:
	VariableExprAST(std::string name)
		: m_name(name)
	{}
	Value* codegen() const;
	std::string name() const { return m_name; }

private:
	std::string m_name;
};

/// Represents a binary operator. Ex. 'x + 2.11'
class BinaryExprAST : public ExprAST {
public:
	BinaryExprAST(char op, ExprAST *left, ExprAST *right)
		: m_op(op), m_left(left), m_right(right)
	{}
	Value* codegen() const;

private:
	BinaryExprAST(const BinaryExprAST&);
	BinaryExprAST& operator=(const BinaryExprAST&);
	char m_op;
	ExprAST *m_left, *m_right;
};

/// Represents a list of variable declarations and definitins.
/// Ex. 'var x, y = 3, z = 99 in f(x, y, z)'
class VarDefExprAST : public ExprAST {
public:
	VarDefExprAST(std::vector<std::pair<std::string, ExprAST*> > varDeclDefs, 
			ExprAST* innerExpr)
		: m_varDeclDefs(varDeclDefs), m_innerExpr(innerExpr)
	{}

	~VarDefExprAST() {
		for (auto& ex : m_varDeclDefs)
			delete ex.second;
		delete m_innerExpr;
	}

	Value* codegen() const;

private:
	VarDefExprAST(const VarDefExprAST&) = delete;
	VarDefExprAST& operator=(const VarDefExprAST&) = delete;

	std::vector<std::pair<std::string, ExprAST*> > m_varDeclDefs;
	ExprAST* m_innerExpr;
};

/// Represents an if-then-else statement (expression). Ex. 'if x > 2 then 1 else sin(x)'
class IfThenElseExprAST : public ExprAST {
public:
	IfThenElseExprAST(ExprAST* cond, ExprAST* thenExpr, ExprAST* elseExpr)
		: m_cond(cond), m_thenExpr(thenExpr), m_elseExpr(elseExpr)
	{}

	~IfThenElseExprAST() {
		delete m_cond;
		delete m_thenExpr;
		delete m_elseExpr;
	}
	Value* codegen() const;

private:
	IfThenElseExprAST(const IfThenElseExprAST&) = delete;
	IfThenElseExprAST& operator=(const IfThenElseExprAST&) = delete;

	ExprAST* m_cond;
	ExprAST* m_thenExpr;
	ExprAST* m_elseExpr;
};

/// Represents a for loop statement (expression). Ex. 'for i = 1, i < 10, 1.0 in sin(i)'
class ForExprAST : public ExprAST {
public:
	ForExprAST (std::string varName, ExprAST* init, ExprAST* cond, ExprAST* step, ExprAST* body)
		: m_varName(varName), m_init(init), m_cond(cond), m_step(step), m_body(body)
	{}
	~ForExprAST () {
		delete m_init;
		delete m_cond;
		delete m_step;
		delete m_body;
	}
	Value* codegen() const;

private:
	std::string m_varName;
	ExprAST* m_init;
	ExprAST* m_cond;
	ExprAST* m_step;
	ExprAST* m_body;
};

/// Represents a function call statement (expression). Ex. 'sin(3.14)'
class CallExprAST : public ExprAST {
public:
	CallExprAST(std::string name, std::vector<ExprAST*> exps)
		: m_name(name), m_exps(exps)
	{}
	~CallExprAST() {
		for (auto e : m_exps) delete e;
	}
	Value* codegen() const;

private:
	CallExprAST(CallExprAST&);
	CallExprAST& operator=(const CallExprAST&);
	std::string m_name;
	std::vector<ExprAST*> m_exps;
};

/// Represents a function prototype (declaration).
/// It can also be used to declare extern functions.
/// Ex. 'extern sin(x)'
class PrototypeAST {
public:
	PrototypeAST(std::string name, std::vector<std::string> args)
		: m_name(name), m_args(args)
	{}

	std::string name() const { return m_name; }
	Function* codegen() const;

private:
	std::string m_name;
	std::vector<std::string> m_args;
};

/// Represents a fully defined function with a prototype and definition.
/// Ex. 'def f(x) x*x'
class FunctionAST {
public:
	FunctionAST(PrototypeAST proto, ExprAST* definition)
		: m_proto(proto), m_definition(definition)
	{}

	~FunctionAST() {
		delete m_definition;
	}
	Function* codegen() const;

private:
	FunctionAST(const FunctionAST&);
	FunctionAST& operator=(const FunctionAST&);
	PrototypeAST m_proto;
	ExprAST* m_definition;
};

#endif /* ifndef AST_HPP */
