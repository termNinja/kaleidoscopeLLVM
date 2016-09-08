#include "ast.hpp"

#define INDENT "    "

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
