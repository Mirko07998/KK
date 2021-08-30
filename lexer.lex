%option noyywrap
%option nounput
%option noinput

%{
  
#include<iostream>
#include<cstdlib>
#include<string>
#include "ast.hpp"
using namespace std;
 
#include "parser.tab.hpp"

%}

%%
"print" { return print_token; }
"and" { return and_token; }
"or"  { return or_token;  }
"xor" { return xor_token; }
"not" { return not_token; }
"shl" { return shl_token; }
"shr" { return shr_token; }
"set" { return set_token; }
"to"  { return to_token; }
"call" { return call_token; }
"while" { return while_token; }
[0-9]+ {
    yylval.u = atoi(yytext);
    return num_token;
}
"0x"[0-9a-f]+ {
    sscanf(yytext,"%x",&(yylval.u));
    return num_token;
}
[a-z_]+ {
     yylval.s = new string(yytext);
     return id_token;
}
[();{},<+] {
    return *yytext;
}
[#].* {

}
[ \t\n] {
  
}
. {
  cerr<<"Leksicka greska: nepoznat karakter '"<<*yytext<<"'"<<endl;
  exit(EXIT_FAILURE);
}
%%

