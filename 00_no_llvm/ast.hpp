#ifndef AST_HPP
#define AST_HPP

#include <vector>
#include <string>

//#include "llvm/IR/IRBuilder.h"
//#include "llvm/IR/Module.h"
//#include "llvm/IR/Verifier.h"

class ExprAST {
public:
	virtual ~ExprAST() {}
	virtual std::string src_show() const = 0;
};

class NumberExprAST : public ExprAST {
public:
	NumberExprAST(double val)
		: m_val(val)
	{}
	std::string src_show() const;

private:
	double m_val;
};

class VariableExprAST : public ExprAST {
public:
	VariableExprAST(std::string name)
		: m_name(name)
	{}
	std::string src_show() const;

private:
	std::string m_name;
};

class BinaryExprAST : public ExprAST {
public:
	BinaryExprAST(char op, ExprAST *left, ExprAST *right)
		: m_op(op), m_left(left), m_right(right)
	{}
	std::string src_show() const;

private:
	BinaryExprAST(const BinaryExprAST&);
	BinaryExprAST& operator=(const BinaryExprAST&);
	char m_op;
	ExprAST *m_left, *m_right;
};

class CallExprAST : public ExprAST {
public:
	CallExprAST(std::string name, std::vector<ExprAST*> exps)
		: m_name(name), m_exps(exps)
	{}
	~CallExprAST() {
		for (auto e : m_exps) delete e;
	}
	std::string src_show() const;

private:
	CallExprAST(CallExprAST&);
	CallExprAST& operator=(const CallExprAST&);
	std::string m_name;
	std::vector<ExprAST*> m_exps;
};

class PrototypeAST {
public:
	PrototypeAST(std::string name, std::vector<std::string> args)
		: m_name(name), m_args(args)
	{}

	std::string name() const { return m_name; }
	std::string src_show() const;

private:
	std::string m_name;
	std::vector<std::string> m_args;
};

class FunctionAST {
public:
	FunctionAST(PrototypeAST proto, ExprAST* definition)
		: m_proto(proto), m_definition(definition)
	{}

	~FunctionAST() {
		delete m_definition;
	}
	std::string src_show() const;
	
private:
	FunctionAST(const FunctionAST&);
	FunctionAST& operator=(const FunctionAST&);
	PrototypeAST m_proto;
	ExprAST* m_definition;
};

#endif /* ifndef AST_HPP */
