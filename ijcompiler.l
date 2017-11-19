
%X COMMENT
%{
#include <string.h>
#include "structures.h"
#include "y.tab.h"

int toReturn=1;
int toPrint=0;

int lineOfOpenComment;
int colOfOpenComment;
int currentLine=1;
int currentColumn=1;
%}
opencomment			"/*"
closecomment			"*/"
newline				"\n"
space				[ \t]+
id				[a-zA-Z_$][a-zA-Z0-9_$]*
intlit				"0x"[0-9a-fA-F]+|[0-9]+
reserved			"++"|"--"|"abstract"|"assert"|"break"|"byte"|"case"|"catch"|"char"|"const"|"continue"|"default"|"do"|"double"|"enum"|"extends"|"final"|"finally"|"float"|"for"|"goto"|"interface"|"implements"|"import"|"instanceof"|"long"|"native"|"package"|"private"|"protected"|"short"|"strictfp"|"super"|"switch"|"synchronized"|"this"|"throw"|"throws"|"transient"|"try"|"volatile"|"null"
%%
{opencomment}			{BEGIN COMMENT; lineOfOpenComment=currentLine; colOfOpenComment=currentColumn; currentColumn+=yyleng;}
<COMMENT>{closecomment}		{BEGIN 0; currentColumn+=yyleng;}
<COMMENT>.			{currentColumn+=yyleng;}
<COMMENT>{newline}		{currentLine++; currentColumn=1;}
<COMMENT><<EOF>>		{printf("Line %d, col %d: unterminated comment\n", lineOfOpenComment, colOfOpenComment); currentColumn+=yyleng; yyterminate();}
"true"				{if (toPrint) printf("TRUE\n"); currentColumn+=yyleng; if (toReturn) return TRUE;}
"false"                  	{if (toPrint) printf("FALSE\n"); currentColumn+=yyleng; if (toReturn) return FALSE;}
"int"				{if (toPrint) printf("INT\n"); currentColumn+=yyleng; if (toReturn) return INT;}
"boolean"			{if (toPrint) printf("BOOL\n"); currentColumn+=yyleng; if (toReturn) return BOOL;}
"new"				{if (toPrint) printf("NEW\n"); currentColumn+=yyleng; if (toReturn) return NEW;}
"if"				{if (toPrint) printf("IF\n"); currentColumn+=yyleng; if (toReturn) return IF;}
"else"				{if (toPrint) printf("ELSE\n"); currentColumn+=yyleng; if (toReturn) return ELSE;}
"while"				{if (toPrint) printf("WHILE\n"); currentColumn+=yyleng; if (toReturn) return WHILE;}
"System.out.println"		{if (toPrint) printf("PRINT\n"); currentColumn+=yyleng; if (toReturn) return PRINT;}
"Integer.parseInt"		{if (toPrint) printf("PARSEINT\n"); currentColumn+=yyleng; if (toReturn) return PARSEINT;}
"class"				{if (toPrint) printf("CLASS\n"); currentColumn+=yyleng; if (toReturn) return CLASS;}
"public"			{if (toPrint) printf("PUBLIC\n"); currentColumn+=yyleng; if (toReturn) return PUBLIC;}
"static"			{if (toPrint) printf("STATIC\n"); currentColumn+=yyleng; if (toReturn) return STATIC;}
"void"				{if (toPrint) printf("VOID\n"); currentColumn+=yyleng; if (toReturn) return VOID;}
"String"			{if (toPrint) printf("STRING\n"); currentColumn+=yyleng; if (toReturn) return STRING;}
".length"			{if (toPrint) printf("DOTLENGTH\n"); currentColumn+=yyleng; if (toReturn) return DOTLENGTH;}
"return"			{if (toPrint) printf("RETURN\n"); currentColumn+=yyleng; if (toReturn) return RETURN;}
"("				{if (toPrint) printf("OCURV\n"); currentColumn+=yyleng; if (toReturn) return OCURV;}
")"				{if (toPrint) printf("CCURV\n"); currentColumn+=yyleng; if (toReturn) return CCURV;}
"{"				{if (toPrint) printf("OBRACE\n"); currentColumn+=yyleng; if (toReturn) return OBRACE;}
"}"				{if (toPrint) printf("CBRACE\n"); currentColumn+=yyleng; if (toReturn) return CBRACE;}
"["				{if (toPrint) printf("OSQUARE\n"); currentColumn+=yyleng; if (toReturn) return OSQUARE;}
"]"				{if (toPrint) printf("CSQUARE\n"); currentColumn+=yyleng; if (toReturn) return CSQUARE;}
"&&"				{if (toPrint) printf("AND\n"); currentColumn+=yyleng; if (toReturn) return AND;}
"||"				{if (toPrint) printf("OR\n"); currentColumn+=yyleng; if (toReturn) return OR;}
"<"				{if (toPrint) printf("LT\n"); currentColumn+=yyleng; if (toReturn) return LT;}
">"				{if (toPrint) printf("GT\n"); currentColumn+=yyleng; if (toReturn) return GT;}
"=="				{if (toPrint) printf("EQUAL\n"); currentColumn+=yyleng; if (toReturn) return EQUAL;}
"!="				{if (toPrint) printf("NOTEQUAL\n"); currentColumn+=yyleng; if (toReturn) return NOTEQUAL;}
"<="				{if (toPrint) printf("LTE\n"); currentColumn+=yyleng; if (toReturn) return LTE;}			
">="				{if (toPrint) printf("GTE\n"); currentColumn+=yyleng; if (toReturn) return GTE;}
"+"				{if (toPrint) printf("PLUS\n"); currentColumn+=yyleng; if (toReturn) return PLUS;}
"-"				{if (toPrint) printf("MINUS\n"); currentColumn+=yyleng; if (toReturn) return MINUS;}
"*"				{if (toPrint) printf("MULTIPLY\n"); currentColumn+=yyleng; if (toReturn) return MULTIPLY;}
"/"				{if (toPrint) printf("DIVIDE\n"); currentColumn+=yyleng; if (toReturn) return DIVIDE;}
"%"				{if (toPrint) printf("REM\n"); currentColumn+=yyleng; if (toReturn) return REM;}
"!"				{if (toPrint) printf("NOT\n"); currentColumn+=yyleng; if (toReturn) return NOT;}
"="				{if (toPrint) printf("ASSIGN\n"); currentColumn+=yyleng; if (toReturn) return ASSIGN;}
";"				{if (toPrint) printf("SEMIC\n"); currentColumn+=yyleng; if (toReturn) return SEMIC;}
","				{if (toPrint) printf("COMMA\n"); currentColumn+=yyleng; if (toReturn) return COMMA;}
"//".*				{;}
{reserved}			{if (toPrint) printf("RESERVED(%s)\n", yytext); currentColumn+=yyleng; if (toReturn) return RESERVED;}
{intlit}			{if (toPrint) printf("INTLIT(%s)\n", yytext); currentColumn+=yyleng; yylval.str=(char*)strdup(yytext); if (toReturn) return INTLIT;}
{id}				{if (toPrint) printf("ID(%s)\n", yytext); currentColumn+=yyleng; yylval.str=(char*)strdup(yytext); if (toReturn) return ID;}
{newline}			{currentLine++; currentColumn=1;}
{space}				{currentColumn+=yyleng;}
.				{printf("Line %d, col %d: illegal character ('%s')\n", currentLine, currentColumn, yytext); currentColumn+=yyleng;}
<<EOF>>				{currentColumn+=yyleng; yyterminate();}
%%
/*
int main()
{
yylex();
return 0;
}
*/

int yywrap()
{
return 1;
}






