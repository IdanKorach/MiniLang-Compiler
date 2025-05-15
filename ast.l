%option yylineno

%%
#[^\n]*\n { /* Ignore comments */ }
\n        { /* Newlines counted by yylineno */ }

[0-9]+   {yylval = mknode(yytext, NULL, NULL); return NUM;}
[0-9]+\.[0-9]+ { yylval = mknode(yytext, NULL, NULL); return NUM; }

[0-9][a-zA-Z0-9_]+ { 
    yyerror("Invalid identifier: cannot start with a digit"); 
    yylval = mknode("invalid_id", NULL, NULL);
    return ERROR_TOKEN;
}

___[a-zA-Z0-9_]* { 
    yyerror("Invalid identifier: cannot start with more than 2 underscores"); 
    yylval = mknode("invalid_id", NULL, NULL);
    return ERROR_TOKEN; 
}

"if"     {yylval = mknode(yytext, NULL, NULL); return IF;}
"elif"   {yylval = mknode(yytext, NULL, NULL); return ELIF;}
"else"   {yylval = mknode(yytext, NULL, NULL); return ELSE;}
"while"  {yylval = mknode(yytext, NULL, NULL); return WHILE;}
"return" {yylval = mknode(yytext, NULL, NULL); return RETURN;}
"and"    {yylval = mknode(yytext, NULL, NULL); return AND;}
"or"     {yylval = mknode(yytext, NULL, NULL); return OR;}
"int"    {yylval = mknode(yytext, NULL, NULL); return INT;}
"float"  {yylval = mknode(yytext, NULL, NULL); return FLOAT;}
"string" {yylval = mknode(yytext, NULL, NULL); return STRING;}
"bool"   {yylval = mknode(yytext, NULL, NULL); return BOOL;}
"true"   {yylval = mknode(yytext, NULL, NULL); return TRUE;}
"false"  {yylval = mknode(yytext, NULL, NULL); return FALSE;}
"not"    {yylval = mknode(yytext, NULL, NULL); return NOT;}
"def"    {yylval = mknode(yytext, NULL, NULL); return DEF;}
"pass"   {yylval = mknode(yytext, NULL, NULL); return PASS;}
"is"     {yylval = mknode(yytext, NULL, NULL); return IS;}
"->"     {yylval = mknode(yytext, NULL, NULL); return ARROW;}
";"      {yylval = mknode(yytext, NULL, NULL); return SEMICOLON;}
","      {yylval = mknode(yytext, NULL, NULL); return COMMA;}
":"      {yylval = mknode(yytext, NULL, NULL); return COLON;}

 /* String literal handling - order matters! */

 /* Empty strings - handle these first */
\"\" {
    yylval = mknode("", NULL, NULL);
    return STRING_LITERAL;
}

\'\' {
    yylval = mknode("", NULL, NULL);
    return STRING_LITERAL;
}

 /* Valid double-quoted strings (including escape sequences) */
\"([^\"\\]|\\.)*\" {
    // Keep the quotes to distinguish from variable names
    yylval = mknode(yytext, NULL, NULL);  // Store with quotes
    return STRING_LITERAL;
}

 /* Valid single-quoted strings (including escape sequences) */
\'([^\'\\]|\\.)*\' {
    // Keep the quotes to distinguish from variable names
    yylval = mknode(yytext, NULL, NULL);  // Store with quotes
    return STRING_LITERAL;
}

 /* Unterminated strings (catch anything starting with quote that doesn't end properly) */
\"[^\"]* {
    yyerror("Unterminated string literal");
    yylval = mknode("invalid_string", NULL, NULL);
    return STRING_LITERAL;
}

\'[^\']* {
    yyerror("Unterminated string literal");
    yylval = mknode("invalid_string", NULL, NULL);
    return STRING_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval = mknode(yytext, NULL, NULL);
    return ID;
}

"+"      {yylval = mknode(yytext, NULL, NULL); return PLUS;}
"-"      {yylval = mknode(yytext, NULL, NULL); return MINUS;}
"*"      {yylval = mknode(yytext, NULL, NULL); return MUL;}
"/"      {yylval = mknode(yytext, NULL, NULL); return DIV;}
"=="     {yylval = mknode(yytext, NULL, NULL); return EQ;}
">"      {yylval = mknode(yytext, NULL, NULL); return GT;}
"<"      {yylval = mknode(yytext, NULL, NULL); return LT;}
">="     {yylval = mknode(yytext, NULL, NULL); return GE;}
"<="     {yylval = mknode(yytext, NULL, NULL); return LE;}
"!="     {yylval = mknode(yytext, NULL, NULL); return NE;}
"**"     {yylval = mknode(yytext, NULL, NULL); return POW;}
"%"      {yylval = mknode(yytext, NULL, NULL); return MOD;}

[ \t]    { /* Ignore tabs and spaces */ }
.        {yylval = mknode(yytext, NULL, NULL); return yytext[0];}
%%