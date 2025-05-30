%token NUM ID MINUS PLUS MUL DIV MOD EQ GT GE LT LE NE POW IF ELIF ELSE WHILE RETURN AND OR NOT
%token DEF PASS ARROW COMMA SEMICOLON FLOAT STRING_LITERAL TRUE FALSE
%token INT STRING BOOL COLON ERROR_TOKEN IS 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right NOT
%left OR
%left AND  
%left EQ GT GE LT LE NE IS
%left PLUS MINUS
%left MUL DIV MOD
%right POW

%{
    #include<stdio.h>
    #include<string.h>
    #include<stdlib.h>
    #include "semantic_analysis.h"
    #include "codegen.h"

    int yylex(void);
    int yyerror(const char* s);
    extern int yylineno;
    char *yytext;
    extern int semantic_errors;
    struct scope* global_scope;

    node *mknode(char *token, node *left, node *right);
    void printtree(node *tree, int tabs);
    int syntax_error = FALSE;
    int main_function_found = 0;
    
    #define YYSTYPE struct node*
%}

%%
s: program {
    if (main_function_found == 1) {
      printtree($1, 0); 
      printf("\n");
      
      // Perform semantic analysis
      global_scope = mkscope(NULL);
      semantic_analysis($1, global_scope);

      // Generate 3AC code
      if (semantic_errors == 0) {
          generate_3ac($1, global_scope);
      }
      
    } else if (main_function_found > 1) {
      printf("Error: Multiple '__main__' functions found.\n");
      return 1;
    } else {
      printf("Error: No '__main__' function found.\n");
      return 1;
    }
  }
  | error { yyerrok; }
  ;

program: function statement program {
            yyerror("Statement found after function without braces. Multiple statements require braces '{}'");
            YYABORT;
         }
       | function statement {
            yyerror("Statement found after function without braces. Multiple statements require braces '{}'");
            YYABORT;
         }
       | function program {$$ = mknode("", $1, $2);}
       | function          {$$ = $1;}
       ;

function: DEF ID '(' param_list ')' ARROW type COLON '{' statements '}' {
            $$ = mknode("function", $2, mknode("", mknode("", mknode("params", $4, NULL), mknode("return_type", $7, NULL)), $10));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' param_list ')' COLON '{' statements '}' {
            $$ = mknode("function", $2, mknode("", mknode("params", $4, NULL), $8));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' ')' ARROW type COLON '{' statements '}' {
            $$ = mknode("function", $2, mknode("", mknode("", mknode("params", NULL, NULL), mknode("return_type", $6, NULL)), $9));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' ')' COLON '{' statements '}' {
            $$ = mknode("function", $2, mknode("", mknode("params", NULL, NULL), $7));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' param_list ')' ARROW type COLON statement {
            $$ = mknode("function", $2, mknode("", mknode("", mknode("params", $4, NULL), mknode("return_type", $7, NULL)), $9));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' param_list ')' COLON statement {
            $$ = mknode("function", $2, mknode("", mknode("params", $4, NULL), $7));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' ')' ARROW type COLON statement {
            $$ = mknode("function", $2, mknode("", mknode("", mknode("params", NULL, NULL), mknode("return_type", $6, NULL)), $8));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        | DEF ID '(' ')' COLON statement {
            $$ = mknode("function", $2, mknode("", mknode("params", NULL, NULL), $6));
            if (strcmp($2->token, "__main__") == 0) main_function_found++;
          }
        
        | DEF ID '(' COLON error '{' { yyerror("Missing closing parenthesis"); YYABORT; }
        | DEF ID '(' error ')' { yyerror("Invalid parameter list"); YYABORT; } 
        | DEF ID '(' ')' error '{' { yyerror("Missing colon after function declaration"); YYABORT; }
        | DEF ID '(' param_list ')' error '{' { yyerror("Missing colon after function declaration"); YYABORT; }
        | DEF ID '(' param_list ')' ARROW type error '{' { yyerror("Missing colon after return type"); YYABORT; }
        | DEF error { yyerror("Invalid function declaration"); YYABORT; }
        ;

param_list: param {$$ = $1;}
          | param SEMICOLON param_list {$$ = mknode("", $1, $3);} 
          | param COMMA param_list {$$ = mknode("", $1, $3);} 
          | error { yyerror("Invalid parameter list"); YYABORT; }
          ;

param: type id_list {$$ = mknode($1->token, $2, NULL);} 
     | error { yyerror("Invalid parameter"); YYABORT; }
     ;

id_list: ID {$$ = $1;} 
       | ID COLON expr {$$ = mknode($1->token, $3, NULL);}
       | ID COMMA id_list {$$ = mknode("", $1, $3);} 
       | ID COLON expr COMMA id_list {node* with_default = mknode($1->token, $3, NULL);
           $$ = mknode("", with_default, $5);
         }
       | error { yyerror("Invalid parameter list"); YYABORT; }
       ;

statements: statement statements {$$ = mknode("", $1, $2);}
          | statement           {$$ = $1;}
          | '{' statements '}' {$$ = mknode("block", $2, NULL);}
          | error { yyerror("Invalid statement"); YYABORT; }
          ;

statement: declaration
         | assign
         | if_stmt
         | while_stmt
         | return_stmt
         | function_call
         | PASS SEMICOLON {$$ = mknode("pass", NULL, NULL);}
         | error SEMICOLON { yyerrok; $$ = mknode("ERROR", NULL, NULL); }
         ;

function_call: ID '(' arg_list ')' SEMICOLON {$$ = mknode("call", $1, $3);}
             | ID '(' ')' SEMICOLON {$$ = mknode("call", $1, NULL);}
             | ID '(' error SEMICOLON { yyerror("Invalid function call arguments"); yyerrok; $$ = mknode("call", $1, NULL); YYABORT;}
             | ID error SEMICOLON { yyerror("Malformed function call"); yyerrok; $$ = mknode("call", $1, NULL); }
             | ID '(' arg_list error { yyerror("Missing semicolon after function call"); YYABORT; }
             | ID '(' ')' error { yyerror("Missing semicolon after function call"); YYABORT; }
             ;

arg_list: expr {$$ = $1;}
        | expr COMMA arg_list {$$ = mknode("", $1, $3);}
        | error { yyerrok; $$ = mknode("ERROR", NULL, NULL); }
        ;

var_list: var_item {$$ = $1;}
        | var_item COMMA var_list {$$ = mknode("", $1, $3);}
        ;

var_item: ID {$$ = $1;}
        | ID '=' expr {$$ = mknode("init_var", $1, $3);}
        ;

declaration: type ID SEMICOLON {$$ = mknode("declare", $1, $2);}
           | type var_list SEMICOLON {$$ = mknode("declare", $1, $2);} 
           | type ID '=' expr SEMICOLON {$$ = mknode("init", mknode("declare", $1, $2), $4);}
           | type ID COLON expr SEMICOLON {$$ = mknode("init", mknode("declare", $1, $2), $4);}
           | type var_list '=' expr_list SEMICOLON {$$ = mknode("multi_init", mknode("declare", $1, $2), $4);}
           | type ERROR_TOKEN SEMICOLON { YYABORT; } 
           | type ERROR_TOKEN '=' expr SEMICOLON { YYABORT; } 
           | type error SEMICOLON { yyerror("Invalid variable name"); yyerrok; $$ = mknode("declare", $1, mknode("ERROR", NULL, NULL)); }
           | type ID error { yyerror("Missing semicolon in declaration"); YYABORT; }
           | type ID '=' error SEMICOLON { yyerror("Invalid expression in initialization"); yyerrok; $$ = mknode("init", mknode("declare", $1, $2), mknode("ERROR", NULL, NULL)); }
           | type ID '=' expr error { yyerror("Missing semicolon in initialization"); YYABORT; }
           | ID ID error { yyerror("Invalid data type"); YYABORT; }
           | ID ID '=' expr error { yyerror("Invalid data type"); YYABORT; }
           | ID ID '=' expr SEMICOLON { yyerror("Invalid data type"); YYABORT; }
           ;

type: INT {$$ = mknode("int", NULL, NULL);}
    | STRING {$$ = mknode("string", NULL, NULL);}
    | BOOL {$$ = mknode("bool", NULL, NULL);}
    | FLOAT {$$ = mknode("float", NULL, NULL);}
    | error { yyerror("Invalid data type"); YYABORT; }
    ;

lhs_list: ID {$$ = $1;}
        | ID COMMA lhs_list {$$ = mknode("", $1, $3);}
        ;

expr_list: expr {$$ = $1;}
         | expr COMMA expr_list {$$ = mknode("", $1, $3);}
         ;

assign: ID '=' expr SEMICOLON {$$ = mknode("assign", $1, $3);}
      | ID COLON expr SEMICOLON {$$ = mknode("assign", $1, $3);}
      | lhs_list '=' expr_list SEMICOLON {$$ = mknode("multi_assign", $1, $3);}
      | ERROR_TOKEN '=' expr SEMICOLON { YYABORT; } 
      | ID '=' error SEMICOLON { 
            yyerror("Invalid expression in assignment"); 
            syntax_error = TRUE;
            YYABORT; 
        }
      | ID error { yyerror("Invalid assignment operation"); YYABORT; }
      | ID '=' expr error { yyerror("Missing semicolon after assignment"); YYABORT; }
      ;

if_stmt: IF expr COLON '{' statements '}' %prec LOWER_THAN_ELSE {
            $$ = mknode("if", $2, $5);
         }
       | IF expr COLON '{' statements '}' elif_chain {
            $$ = mknode("if-elif", $2, mknode("", $5, $7));
         }
       | IF expr COLON '{' statements '}' elif_chain ELSE COLON '{' statements '}' {
            node* if_elif = mknode("if-elif", $2, mknode("", $5, $7));
            $$ = mknode("if-elif-else", if_elif, $11);
         }
       | IF expr COLON '{' statements '}' ELSE COLON '{' statements '}' {
            $$ = mknode("if-else", mknode("if", $2, $5), $10);
         }
       | IF expr COLON '{' statements '}' ELSE COLON statement {
            $$ = mknode("if-else", mknode("if", $2, $5), $9);
         }
       | IF expr COLON statement %prec LOWER_THAN_ELSE {
            $$ = mknode("if", $2, $4);
         }
       | IF expr COLON statement elif_chain {
            $$ = mknode("if-elif", $2, mknode("", $4, $5));
         }
       | IF expr COLON statement elif_chain ELSE COLON '{' statements '}' {
            node* if_elif = mknode("if-elif", $2, mknode("", $4, $5));
            $$ = mknode("if-elif-else", if_elif, $9);
         }
       | IF expr COLON statement elif_chain ELSE COLON statement {
            node* if_elif = mknode("if-elif", $2, mknode("", $4, $5));
            $$ = mknode("if-elif-else", if_elif, $8);
         }
       | IF expr COLON statement ELSE COLON '{' statements '}' {
            $$ = mknode("if-else", mknode("if", $2, $4), $8);
         }
       | IF expr COLON statement ELSE COLON statement {
            $$ = mknode("if-else", mknode("if", $2, $4), $7);
         }
       | IF error { yyerror("Invalid if statement"); YYABORT; }
       ;

elif_chain: ELIF expr COLON '{' statements '}' {
              $$ = mknode("elif", $2, $5);
           }
          | ELIF expr COLON '{' statements '}' elif_chain {
              $$ = mknode("", mknode("elif", $2, $5), $7);
           }
          | ELIF expr COLON statement {
              $$ = mknode("elif", $2, $4);
           }
          | ELIF expr COLON statement elif_chain {
              $$ = mknode("", mknode("elif", $2, $4), $5);
           }
          | ELIF error { yyerror("Invalid elif statement"); YYABORT; }
          ;

while_stmt: WHILE '(' expr ')' '{' statements '}' {
              $$ = mknode("while", $3, $6);
           }
          | WHILE expr COLON '{' statements '}' {
              $$ = mknode("while", $2, $5);
           }
          | WHILE '(' expr ')' COLON '{' statements '}' {
              $$ = mknode("while", $3, $7);
           }
          | WHILE expr COLON statement {
              $$ = mknode("while", $2, $4);
           }
          | WHILE '(' expr ')' COLON statement {
              $$ = mknode("while", $3, $6);
           }
          | WHILE '(' expr error '{' { yyerror("Missing closing parenthesis in while condition"); YYABORT; }
          | WHILE '(' error ')' { yyerror("Invalid condition in while loop"); YYABORT; }
          | WHILE error { yyerror("Invalid while statement"); YYABORT; }
          ;

return_stmt: RETURN expr SEMICOLON {
              $$ = mknode("return", $2, NULL);
            }
          | RETURN SEMICOLON {
              $$ = mknode("return", NULL, NULL);
            }
          | RETURN expr error { yyerror("Missing semicolon after return statement"); YYABORT; }
          | RETURN error { yyerror("Invalid return statement"); YYABORT; }
          ;

expr: expr PLUS expr {$$ = mknode("+", $1, $3);}
    | expr MINUS expr {$$ = mknode("-", $1, $3);}
    | expr MUL expr {$$ = mknode("*", $1, $3);}
    | expr DIV expr {$$ = mknode("/", $1, $3);}
    | expr MOD expr {$$ = mknode("%", $1, $3);}
    | expr EQ expr {$$ = mknode("==", $1, $3);}
    | expr IS expr {$$ = mknode("==", $1, $3);}
    | expr GT expr {$$ = mknode(">", $1, $3);}
    | expr GE expr {$$ = mknode(">=", $1, $3);}
    | expr LT expr {$$ = mknode("<", $1, $3);}
    | expr LE expr {$$ = mknode("<=", $1, $3);}
    | expr NE expr {$$ = mknode("!=", $1, $3);}
    | expr POW expr {$$ = mknode("**", $1, $3);}
    | expr AND expr {$$ = mknode("and", $1, $3);}
    | expr OR expr {$$ = mknode("or", $1, $3);}
    | NOT expr {$$ = mknode("not", NULL, $2);}
    | ID '[' expr ']' {$$ = mknode("index", $1, $3);}
    | ID '[' expr COLON expr ']' {$$ = mknode("slice", $1, mknode("", $3, $5));} 
    | ID '[' COLON expr ']' {$$ = mknode("slice", $1, mknode("", mknode("0", NULL, NULL), $4));}  
    | ID '[' expr COLON ']' {$$ = mknode("slice", $1, mknode("", $3, mknode("-1", NULL, NULL)));}  
    | ID '[' COLON ']' {$$ = mknode("slice", $1, mknode("", mknode("0", NULL, NULL), mknode("-1", NULL, NULL)));} 
    | ID '[' expr COLON expr COLON expr ']' {$$ = mknode("slice_step", $1, mknode("", mknode("", $3, $5), $7));}  
    | ID '[' COLON expr COLON expr ']' {$$ = mknode("slice_step", $1, mknode("", mknode("", mknode("0", NULL, NULL), $4), $6));}  
    | ID '[' expr COLON COLON expr ']' {$$ = mknode("slice_step", $1, mknode("", mknode("", $3, mknode("-1", NULL, NULL)), $6));} 
    | ID '[' COLON COLON expr ']' {$$ = mknode("slice_step", $1, mknode("", mknode("", mknode("0", NULL, NULL), mknode("-1", NULL, NULL)), $5));}   
    | '(' expr ')' {$$ = $2;}
    | '(' expr error { yyerror("Missing closing parenthesis in expression"); YYABORT; }
    | '(' error ')' { yyerror("Invalid expression in parentheses"); yyerrok; $$ = mknode("ERROR", NULL, NULL); YYABORT; }
    | ID '(' arg_list ')' {$$ = mknode("call", $1, $3);} 
    | ID '(' ')' {$$ = mknode("call", $1, NULL);}
    | ID '(' error ')' { yyerror("Invalid function arguments"); yyerrok; $$ = mknode("call", $1, mknode("ERROR", NULL, NULL)); YYABORT; }
    | ID {$$ = $1;}
    | ERROR_TOKEN { YYABORT; }
    | NUM {$$ = $1;}
    | STRING_LITERAL {$$ = $1;}
    | TRUE {$$ = $1;}
    | FALSE {$$ = $1;}
    | error { yyerror("Invalid expression"); yyerrok; syntax_error = TRUE; $$ = mknode("ERROR", NULL, NULL); YYABORT; }
    ;
%%
#include "lex.yy.c"
int main() { 
    int result = yyparse();
    if (result != 0) {  
        printf("Parsing failed!\n");
        return 1;
    }
    
    if (!main_function_found) {
        printf("Error: No '__main__' function found.\n");
        return 1;
    }
    
    return 0;
}

void printtree(node *tree, int tabs) {
    /* Check if tree is NULL before proceeding */
    if (!tree) return;
    
    if (strcmp(tree->token, "") != 0) {
        for (int i = 0; i < tabs; i++)
            printf("\t");
        
        /* Check if this is a leaf node (no children) */
        if (tree->left == NULL && tree->right == NULL) {
            /* Just print the token without parentheses for leaf nodes */
            printf("%s\n", tree->token);
        } else {
            /* Print opening parenthesis for non-leaf nodes */
            printf("(%s\n", tree->token);
            
            /* Print children with increased indentation */
            if (tree->left)
                printtree(tree->left, tabs + 1);
            if (tree->right)
                printtree(tree->right, tabs + 1);
            
            /* Print closing parenthesis for non-leaf nodes */
            for (int i = 0; i < tabs; i++)
                printf("\t");
            printf(")\n");
        }
    } else {
        /* For empty token nodes, just process children without adding parentheses */
        if (tree->left)
            printtree(tree->left, tabs);
        if (tree->right)
            printtree(tree->right, tabs);
    }
}

int yyerror(const char* s) {
    if (strcmp(s, "syntax error") == 0) {
        syntax_error = TRUE;
        return 0;
    }
    printf("Error at line %d: %s\n", yylineno, s);
    printf("Near token: %s\n", yytext);
    return 0;
}

node *mknode(char *token, node *left, node *right) {
    node *newnode = (node*)malloc(sizeof(node));
    char *newstr = (char*)malloc(strlen(token) + 1);
    strcpy(newstr, token);
    newnode -> left = left;
    newnode -> right = right;
    newnode -> token = newstr;
    return newnode;
}