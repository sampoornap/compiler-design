%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;
extern FILE *yyin;
extern int yylex();

void yyerror(const char* msg);

typedef struct {
    char* name;
    double value;
} symbol;

symbol symtab[100];
int symcount = 0;

int lookup(char* name) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(symtab[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_symbol(char* name) {
    symtab[symcount].name = strdup(name);
    symtab[symcount].value = 0;
    symcount++;
    return symcount - 1;
}

%}

%union {
    double num;
    char* str;
}

%token <str> IDENTIFIER
%token <num> NUMBER
%token INT FLOAT DOUBLE
%token FOR WHILE IF ELSE
%token EQ NE LE GE LT GT


%left ELSE
%nonassoc LOWER_THAN_ELSE
%left '+' '-'
%left '*' '/'

%type <num> Expr LExpr Exp Term Factor Compare
%type <str> ArgumentList Argument Type IdentifierList CompoundStmt ForStmt IfStmt WhileStmt DeclStmt 
%type <num> StmtList Stmt OptExpr

%start Function

%%

Function: Type IDENTIFIER '(' ArgumentList ')' CompoundStmt
        | Type IDENTIFIER error ArgumentList ')' CompoundStmt { printf("Missing '('\n"); yyerrok; }
        | Type IDENTIFIER '(' ArgumentList error CompoundStmt { printf("Missing ')'\n"); yyerrok; }
        | error IDENTIFIER '(' ArgumentList ')' CompoundStmt { printf("Return type defaults to int\n"); yyerrok; }
        ;

ArgumentList: Argument 
            | ArgumentList ',' Argument 
            | ArgumentList error Argument { printf("Error: expected ',' or ')' before argument\n"); yyerrok; }
            | /* empty */ { $$ = 0;}
            ;

Argument: Type IDENTIFIER { $$ = 0; }
        ;

Type: INT { $$ = 0; }
    | FLOAT { $$ = 0; }
    | DOUBLE { $$ = 0; }
    ;

IdentifierList: IDENTIFIER ',' IdentifierList { $$ = 0; }
              | IDENTIFIER { $$ = 0; }
              ;

DeclStmt: Type IdentifierList ';' { $$ = 0; }
        ;

CompoundStmt: '{' StmtList '}' { $$ = 0; }
        ;

StmtList: StmtList Stmt { $$ = 0; }
        | /* empty */ { $$ = 0; }
        ;

Stmt: ForStmt { $$ = 0; }
    | WhileStmt { $$ = 0; }
    | IfStmt { $$ = 0; }
    | CompoundStmt { $$ = 0; }
    | DeclStmt { $$ = 0; }  
    | Expr ';' { $$ = $1; }
    | ';' { $$ = 0; }
    ;

ForStmt: FOR '(' OptExpr ';' OptExpr ';' OptExpr ')' Stmt { $$ = 0; }
        | FOR '(' error OptExpr ';' OptExpr ';' OptExpr ')' Stmt { printf("Error: missing expression before ';'\n"); yyerrok; }
        | FOR '(' OptExpr ';' error ';' OptExpr ')' Stmt { printf("Error: missing expression before ';'\n"); yyerrok; }
        | FOR '(' OptExpr ';' OptExpr ';' error ')' Stmt { printf("Error: missing expression before ')'\n"); yyerrok; }
        ;

OptExpr: Expr { $$ = $1; }
       | /* empty */ { $$ = 0; }
       ;

WhileStmt: WHILE '(' Expr ')' Stmt { $$ = 0; }
         | WHILE '(' error ')' Stmt { printf("Error: missing expression inside '()'\n"); yyerrok; }
         ;

IfStmt: IF '(' Expr ')' Stmt %prec LOWER_THAN_ELSE { $$ = 0; }
      | IF '(' Expr ')' Stmt ELSE Stmt { $$ = 0; }
      | IF '(' error ')' Stmt { printf("Error: missing expression inside '()'\n"); yyerrok; }
      ;

Expr: IDENTIFIER '=' Expr { int idx = lookup($1); if (idx == -1) idx = add_symbol($1); symtab[idx].value = $3; $$ = $3; }
    | LExpr { $$ = $1; }
    ;

LExpr: LExpr Compare Exp { if ($2 == EQ) $$ = $1 == $3; else if ($2 == NE) $$ = $1 != $3; else if ($2 == LE) $$ = $1 <= $3; else if ($2 == GE) $$ = $1 >= $3; else if ($2 == LT) $$ = $1 < $3; else if ($2 == GT) $$ = $1 > $3; }
    | Exp { $$ = $1; }
    ;

Compare: EQ { $$ = EQ; }
    | NE { $$ = NE; }
    | LE { $$ = LE; }
    | GE { $$ = GE; }
    | LT { $$ = LT; }
    | GT { $$ = GT; }
    ;
Exp: Exp '+' Term { $$ = $1 + $3; }
    | Exp '-' Term { $$ = $1 - $3; }
    | Term { $$ = $1; }
    ;
Term: Term '*' Factor { $$ = $1 * $3; }
    | Term '/' Factor { if ($3 == 0) { yyerror("Division by zero"); $$ = 0; } else { $$ = $1 / $3; } }
    | Factor { $$ = $1; }
    ;

Factor: '(' Expr ')' { $$ = $2; }
    | '(' error ')' { printf("Error: missing expression inside '()'\n"); yyerrok; }
    | '-' Factor { $$ = -$2; }
    | '+' Factor { $$ = $2; }
    | IDENTIFIER { int idx = lookup($1); if (idx == -1) { yyerror("Undefined variable"); $$ = 0; } else { $$ = symtab[idx].value; } }
    | NUMBER { $$ = $1; }
    ;

%%

void yyerror(const char* msg) {
    fprintf(stderr, "Error: %s on line %d\n", msg, yylineno);
}

int main() 
{
    FILE *fp;
    fp = fopen("input.c", "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return 1;
    }
    yyin = fp;
    
    int x = yyparse();
    fclose(fp);
    if (x == 0) {
        printf("Input Accepted\n");
    } else {
        printf("Input not Accepted\n");
    }
    return 0;
}







