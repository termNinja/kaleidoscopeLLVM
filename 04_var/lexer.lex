%option noyywrap
%option nounput
%option noinput
%{
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include "ast.hpp"
#include "parser.tab.hpp"
%}

%%
def 		return def_token;
extern 		return extern_token;
if 			return if_token;
else 		return else_token;
then 		return then_token;
for 		return for_token;
in 			return in_token;
var 		return var_token;
[#].* { }
end { return end_token; }
[0-9]+(\.[0-9]+)? { yylval.num = atof(yytext); return num_token; }
[a-zA-Z][a-zA-Z0-9]* { yylval.str = new std::string(yytext); return id_token; }
[:=+<()>;(),*-] return *yytext;
[\t\n ] {}
. {
	std::cerr << "Lexical error. Unrecognized character: '" << *yytext << "'" << std::endl;
	exit(EXIT_FAILURE);
}
%%
