%{
#include <stdio.h>
#include <stdlib.h>

FILE *yyin; 
extern int yylval;

%}

Identifier [a-zA-Z]+[a-zA-Z0-9]*
Operator \+|\-|\*|\/|=
Digit [0-9]+(\.[0-9]+)?
String_literal \".*?\"|\'.*?\'
MultiComment "/*"(.|\n)*"*/"
SingleComment \/\/.*
IncorrectId [0-9]+[a-zA-Z]+[a-zA-Z0-9]*
%%

"while" {printf("Lexeme = while \t Token is WHILE \t Line No. = %d\n", yylineno);}
"for" {printf("Lexeme = for \t Token is FOR \t Line No. = %d\n", yylineno);}
"if" {printf("Lexeme = if \t Token is IF \t Line No. = %d\n", yylineno);}
"else" {printf("Lexeme = else \t Token is ELSE \t Line No. = %d\n", yylineno);}
"==" {printf("Lexeme = == \t Token is EQ \t Line No. = %d\n", yylineno);}
"!=" {printf("Lexeme = != \t Token is NE \t Line No. = %d\n", yylineno);}
"<=" {printf("Lexeme = <= \t Token is LE \t Line No. = %d\n", yylineno);}
">=" {printf("Lexeme = >= \t Token is GE \t Line No. = %d\n", yylineno);}
"<" {printf("Lexeme = <  \t Token is LT \t Line No. = %d\n", yylineno);}
">" {printf("Lexeme = > \t Token is GT \t Line No. = %d\n", yylineno);}
"int" {printf("Lexeme = int \t Token is INT \t Line No. = %d\n", yylineno);}
"float" {printf("Lexeme = float \t Token is FLOAT \t Line No. = %d\n", yylineno);}
"double" {printf("Lexeme = double \t Token is DOUBLE \t Line No. = %d\n", yylineno);}
{Identifier} {printf("Lexeme = %s \t Token is ID \t Line No. = %d\n", yytext, yylineno);}
{Operator} {printf("Lexeme = %s \t Token is ASSI \t Line No. = %d\n", yytext, yylineno);}
{Digit} {printf("Lexeme = %s \t Token is NUM \t Line No. = %d\n", yytext, yylineno);}
{String_literal} {printf("Lexeme = %s \t Token is STRING_LITERAL \t Line No. = %d\n", yytext, yylineno);}
{SingleComment} {/* Ignore single line comments */}
{MultiComment} {/* Ignore multi-line comments */}
"(" {printf("Lexeme = %s \t Token is LP \t Line No. = %d\n", yytext, yylineno); }
")" {printf("Lexeme = %s \t Token is RP \t Line No. = %d\n", yytext, yylineno); }
"{" {printf("Lexeme = %s \t Token is LC \t Line No. = %d\n", yytext, yylineno);}
"}" {printf("Lexeme = %s \t Token is RC \t Line No. = %d\n", yytext, yylineno); }
"," {printf("Lexeme = , \t Token is COMMA \t Line No. = %d\n", yylineno); }
";" {printf("Lexeme = ; \t Token is SEMI \t Line No. = %d\n", yylineno);}
[ \t]  {/* ignore spaces */}
\n {yylineno++;}  // Increment line number on new line
. {printf("Unrecognized character: %s at Line No. = %d\n", yytext, yylineno);}  // Catch all for unrecognized characters

%%

int yywrap() { return 1; }

int main() {
    yyin = fopen("input.c", "r");
    if (!yyin) {
        fprintf(stderr, "Error: Could not open input.c\n");
        exit(1);
    }

    yylex();  // Perform lexical analysis

    fclose(yyin);
    return 0;
}
