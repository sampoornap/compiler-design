%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;
extern FILE *yyin;
extern int yylex();
extern char *yytext;
extern char linebuffer[500];
extern char* prevline;

// Struct for AST node
struct ASTNode {
    char* type;
    struct ASTNode* left;
    struct ASTNode* right;
    double value;
};

// Function to create AST node
struct ASTNode* create_ast_node(char* type, struct ASTNode* left, struct ASTNode* right, double value) {
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    node->value = value;
    return node;
}

// Symbol table entry
struct Node {
    char * name;
    char * type;
    double value;   
    struct Node *next;
};

// Symbol table
struct Node SymTab[47];


void addSym(char *n, char *t, double v) {
   
    int found = 0;
    int key = 0;
    char *temp = n;

    while (*temp != '\0') {
        int ascii = (int)(*temp);
        key += ascii;
        temp++;
    }
    key=key%101;

    if (SymTab[key].name == NULL) {
        SymTab[key].name = strdup(n);
        SymTab[key].type = strdup(t);
        SymTab[key].value = v;
        
    } else {
        struct Node *ptr = &SymTab[key];
        while (ptr->next != NULL) {
            if (strcmp(ptr->name, n) == 0) {
                found = 1;
                break;
            } else {
                ptr = ptr->next;
            }
        }

        if (found == 1) {
            ptr->value = v;
        } else {
            struct Node *Temp = malloc(sizeof(struct Node));
            if (Temp == NULL) {
                printf("Memory allocation failed\n");
                return;
            }
            Temp->name = strdup(n);
            Temp->type = strdup(t);
            Temp->value = v;
            ptr->next = Temp;
        }
    }
}

// Function to look up in symbol table
double lookup(char *name) {
    int key = 0;
    char *temp = name;

       while (*temp != '\0') {
        key += (int)(*temp);
        temp++;
    }
    key %= 47;

    // Search for the name in the symbol table
    struct Node *ptr = &SymTab[key];
    while (ptr != NULL) {
        if (ptr->name != NULL && strcmp(ptr->name, name) == 0) {
            return ptr->value; // Return value if name is found
        }
        ptr = ptr->next;
    }

    return 0; 
}

%}

%union {
    struct ASTNode* node;
    double value;
}

%token <value> NUMBER
%token <node> IDENTIFIER
%token INT FLOAT DOUBLE
%token FOR WHILE IF ELSE
%token EQ NE LE GE LT GT

%type <node> Function ArgumentList Argument Declaration Type IdentifierList CompoundStmt StmtList Stmt Expr LExpr Exp Term Factor Compare
%type <value> OptExpr

%start Function

%%

Function: Type IDENTIFIER '(' ArgumentList ')' CompoundStmt {
    $$ = create_ast_node("Function", create_ast_node($1, NULL, NULL, 0), $3, 0);
}

ArgumentList: Argument 
            | ArgumentList ',' Argument 
            | /* empty */ { $$ = NULL; }
            ;

Argument: Type IDENTIFIER {
    $$ = create_ast_node("Argument", create_ast_node($1, NULL, NULL, 0), $2, 0);
}

Declaration: Type IdentifierList ';' {
    $$ = create_ast_node("Declaration", create_ast_node($1, NULL, NULL, 0), $2, 0);
}

IdentifierList: IDENTIFIER ',' IdentifierList {
    $$ = create_ast_node("IdentifierList", $1, $3, 0);
}
              | IDENTIFIER {
                  $$ = create_ast_node("Identifier", $1, NULL, 0);
              }
              ;

CompoundStmt: '{' StmtList '}' {
    $$ = create_ast_node("CompoundStmt", $2, NULL, 0);
}

StmtList: StmtList Stmt {
    $$ = create_ast_node("StmtList", $1, $2, 0);
}
        | Stmt {
            $$ = create_ast_node("StmtList", $1, NULL, 0);
        }
        | /* empty */ {
            $$ = NULL;
        }
        ;

Stmt: ForStmt { $$ = create_ast_node("ForStmt", $1, NULL, 0); }
    | WhileStmt { $$ = create_ast_node("WhileStmt", $1, NULL, 0); }
    | IfStmt { $$ = create_ast_node("IfStmt", $1, NULL, 0); }
    | CompoundStmt { $$ = $1; }
    | DeclStmt { $$ = $1; }
    | Expr ';' { $$ = create_ast_node("ExprStmt", $1, NULL, 0); }
    | ';' { $$ = NULL; }
    ;

ForStmt: FOR '(' OptExpr ';' OptExpr ';' OptExpr ')' Stmt { 
    $$ = create_ast_node("ForStmt", create_ast_node("OptExpr", $3, NULL, 0), create_ast_node("OptExpr", $5, NULL, 0), 0); 
}
        | FOR '(' error OptExpr ';' OptExpr ';' OptExpr ')' Stmt { yyerrok; }
        | FOR '(' OptExpr ';' error ';' OptExpr ')' Stmt { yyerrok; }
        | FOR '(' OptExpr ';' OptExpr ';' error ')' Stmt { yyerrok; }
        ;

OptExpr: Expr { $$ = $1; }
       | /* empty */ { $$ = NULL; }
       ;

WhileStmt: WHILE '(' Expr ')' Stmt { 
    $$ = create_ast_node("WhileStmt", $3, $5, 0); 
}
         | WHILE '(' error ')' Stmt { yyerrok; }
         ;

IfStmt: IF '(' Expr ')' Stmt %prec LOWER_THAN_ELSE { 
    $$ = create_ast_node("IfStmt", $3, $5, 0); 
}
      | IF '(' Expr ')' Stmt ELSE Stmt { 
          $$ = create_ast_node("IfElseStmt", $3, $5, $7->left, 0); 
      }
      | IF '(' error ')' Stmt { yyerrok; }
      ;

Expr: IDENTIFIER '=' Expr {
    $$ = create_ast_node("AssignExpr", create_ast_node("Identifier", $1, NULL, 0), $3, 0);
}
    | LExpr {
        $$ = create_ast_node("LExprExpr", $1, NULL, 0);
    }
    ;

LExpr: LExpr Compare Exp { 
    $$ = create_ast_node($2, $1, $3, 0);
}
    | Exp { 
        $$ = $1; 
    }
    ;

Compare: EQ { $$ = "=="; }
    | NE { $$ = "!="; }
    | LE { $$ = "<="; }
    | GE { $$ = ">="; }
    | LT { $$ = "<"; }
    | GT { $$ = ">"; }
    ;
Exp: Exp '+' Term { 
    $$ = create_ast_node("+", $1, $3, 0); 
}
    | Exp '-' Term { 
        $$ = create_ast_node("-", $1, $3, 0); 
    }
    | Term { 
        $$ = $1; 
    }
    ;
Term: Term '*' Factor { 
    $$ = create_ast_node("*", $1, $3, 0); 
}
    | Term '/' Factor { 
        $$ = create_ast_node("/", $1, $3, 0); 
    }
    | Factor { 
        $$ = $1; 
    }
    ;

Factor: '(' Expr ')' { 
    $$ = $2; 
}
    | '(' error ')' { 
        yyerrok; 
    }
    | '-' Factor { 
        $$ = create_ast_node("-", NULL, $2, 0); 
    }
    | '+' Factor { 
        $$ = $2; 
    }
    | IDENTIFIER { 
        $$ = create_ast_node("Identifier", $1, NULL, lookup($1->name)); 
    }
    | NUMBER { 
        $$ = create_ast_node("Number", NULL, NULL, $1->value); 
    }
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
