%{
#include <stdio.h>
#include <string.h>

/* Token definitions for the Kconfig parser engine */
#define T_MAINMENU  1
#define T_CONFIG    2
#define T_BOOL      3
#define T_TRISTATE  4
#define T_STRING    5
#define T_DEFAULT   6
#define T_HELP      7
#define T_PROMPT    8
#define T_IDENT     9
#define T_STR_LIT   10

char *yylval_string;
int line_num = 1;
%}

%option noyywrap

%%

[ \t]                   { /* Ignore whitespace */ }
\n                      { line_num++; }
"#"["."]* { /* Ignore single-line comments */ }

"mainmenu"              { return T_MAINMENU; }
"config"                { return T_CONFIG; }
"bool"                  { return T_BOOL; }
"tristate"              { return T_TRISTATE; }
"string"                { return T_STRING; }
"default"               { return T_DEFAULT; }
"help"                  { return T_HELP; }
"prompt"                { return T_PROMPT; }

[a-zA-Z_][a-zA-Z0-9_]* { 
                            yylval_string = strdup(yytext); 
                            return T_IDENT; 
                        }

\"[^\"\n]*\"            { 
                            /* Stripping quotes for literal values */
                            yylval_string = strndup(yytext + 1, strlen(yytext) - 2); 
                            return T_STR_LIT; 
                        }

.                       { printf("Kconfig Lexer Warning: Unknown token '%s' at line %d\n", yytext, line_num); }

%%