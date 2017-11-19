%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "structures.h"
#include "symbol_table.h"
#include "y.tab.h"
#include "functions.h"
#include "print.h"
#include "codeGeneration.h"

extern int currentLine, currentColumn, yyleng;
extern char* yytext;

int sintax_error=0;
is_start_program* root;
class_element* symtab=NULL;

%}

%union
{
	int bool;	/*0-false; 1-true*/
	char* str;
	is_start_program* 		start;
	is_field_method_decl*	field_method_decl;
	is_var_decl*			var_decl;
	is_var_decl_list*		var_decl_list;
	is_method_decl*			method_decl;
	is_method_body*			method_body;
	is_method_params*		method_params;
	is_statement_list*		statement_list;
	is_statement*			statement;
	is_expression*			expression;
	is_expression_list*		expression_list;
	is_type					type;
}

%token <str> ID
%token <str> TRUE
%token <str> FALSE
%token <str> INTLIT

%token INT BOOL NEW IF ELSE WHILE PRINT PARSEINT CLASS PUBLIC STATIC VOID STRING DOTLENGTH
%token RETURN OCURV CCURV OBRACE CBRACE OSQUARE CSQUARE AND OR LT GT EQUAL NOTEQUAL LTE GTE PLUS MINUS
%token MULTIPLY DIVIDE REM NOT ASSIGN SEMIC COMMA RESERVED MULTIDIMARRAY

%type <start> Program
%type <field_method_decl> FieldMethodDecl
%type <var_decl> FieldDecl
%type <var_decl> VarDecl
%type <var_decl> VarDecl2
%type <var_decl_list> VarDecl3
%type <type> Type
%type <method_decl> MethodDecl
%type <method_body> MethodBody
%type <method_params> FormalParams
%type <method_params> FormalParams2
%type <statement_list> StatementAux
%type <statement> Statement
%type <expression> Expr
%type <expression> Expr2
%type <expression_list> Args
%type <expression_list> Args2


%nonassoc CCURV
%nonassoc ELSE

%left OR
%left AND
%left EQUAL NOTEQUAL
%left GT LT GTE LTE
%left PLUS MINUS
%left MULTIPLY DIVIDE REM
%right NOT
%left DOTLENGTH OSQUARE

%%
Start:		Program 									{root = $1;}
	;

Program:	CLASS ID OBRACE	FieldMethodDecl CBRACE		{$$ = addProgram($2, $4);}
	;

FieldMethodDecl:										{$$ = NULL;}
	|	FieldDecl FieldMethodDecl 						{$$ = addFieldMethodDecl(desc_VARDECL, $1, $2);}
	|	MethodDecl  FieldMethodDecl						{$$ = addFieldMethodDecl(desc_METHODDECL, $1, $2);}
	;

FieldDecl: 	STATIC Type ID VarDecl2 SEMIC 				{$$ = addVarDecl($2, $3, $4);}
	;

VarDecl:	Type ID VarDecl2 SEMIC 						{$$ = addVarDecl($1, $2, $3);}
	;

VarDecl2:												{$$ = NULL;}
	|	COMMA ID VarDecl2 								{$$ = addVarDecl(is_NULL, $2, $3);}
	;

VarDecl3:												{$$ = NULL;}
	|	VarDecl VarDecl3								{$$ = addVarDeclList($1, $2);}
	;

Type:	INT												{$$ = is_INT;}
	|	BOOL											{$$ = is_BOOL;}
	|	INT OSQUARE CSQUARE								{$$ = is_INTARRAY;}
	|	BOOL OSQUARE CSQUARE							{$$ = is_BOOLARRAY;}
	;

MethodDecl: 	PUBLIC STATIC Type ID OCURV FormalParams CCURV OBRACE MethodBody CBRACE
														{$$ = addMethodDecl($3, $4, $6, $9);}
	|			PUBLIC STATIC VOID ID OCURV FormalParams CCURV OBRACE MethodBody CBRACE
														{$$ = addMethodDecl(is_VOID, $4, $6, $9);}
	|			PUBLIC STATIC Type ID OCURV CCURV OBRACE MethodBody CBRACE
														{$$ = addMethodDecl($3, $4, NULL, $8);}
	|			PUBLIC STATIC VOID ID OCURV CCURV OBRACE MethodBody CBRACE
														{$$ = addMethodDecl(is_VOID, $4, NULL, $8);}
	;

MethodBody: VarDecl3 StatementAux						{$$ = addMethodBody($1, $2);}
	;

FormalParams:	Type ID FormalParams2					{$$ = addMethodParams($1, $2, $3);}
	|			STRING OSQUARE CSQUARE ID 				{$$ = addMethodParams(is_STRINGARRAY, $4, NULL);}
	;

FormalParams2:											{$$ = NULL;}
	|	COMMA Type ID FormalParams2						{$$ = addMethodParams($2, $3, $4);}
	;

StatementAux: 											{$$ = NULL;}
	|	Statement StatementAux							{$$ = addStatementList($1, $2);}
	;

Statement:	OBRACE StatementAux CBRACE					{$$ = addStatement(desc_COMPSTAT, NULL, NULL, NULL, NULL, NULL, $2);}
	|	IF OCURV Expr CCURV Statement 					{$$ = addStatement(desc_IFELSE, $3, $5, NULL, NULL, NULL, NULL);}
	|	IF OCURV Expr CCURV Statement ELSE Statement 	{$$ = addStatement(desc_IFELSE, $3, $5, $7, NULL, NULL, NULL);}
	|	WHILE OCURV Expr CCURV Statement 				{$$ = addStatement(desc_WHILE, $3, $5, NULL, NULL, NULL, NULL);}
	|	PRINT OCURV Expr CCURV SEMIC 	 				{$$ = addStatement(desc_PRINT, $3, NULL, NULL, NULL, NULL, NULL);}
	|	ID ASSIGN Expr SEMIC 							{$$ = addStatement(desc_ASSIGN, $3, NULL, NULL, $1, NULL, NULL);}
	|	ID OSQUARE Expr CSQUARE ASSIGN Expr SEMIC 		{$$ = addStatement(desc_ASSIGNARRAY, $6, NULL, NULL, $1, $3, NULL);}
	|	RETURN Expr SEMIC 								{$$ = addStatement(desc_RETURN_E, $2, NULL, NULL, NULL, NULL, NULL);}
	|	RETURN SEMIC 									{$$ = addStatement(desc_RETURN, NULL, NULL, NULL, NULL, NULL, NULL);}
	;

Expr:	Expr AND Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_AND);}
	|	Expr OR Expr 									{$$ = addExpression(d_infix, 0, $1, $3, is_OR);}
	|	Expr LT Expr 									{$$ = addExpression(d_infix, 0, $1, $3, is_LT);}
	|	Expr GT Expr 									{$$ = addExpression(d_infix, 0, $1, $3, is_GT);}
	|	Expr EQUAL Expr 								{$$ = addExpression(d_infix, 0, $1, $3, is_EQUAL);}
	|	Expr NOTEQUAL Expr 								{$$ = addExpression(d_infix, 0, $1, $3, is_NOTEQUAL);}
	|	Expr LTE Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_LTE);}
	|	Expr GTE Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_GTE);}
	|	Expr PLUS Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_ADD);}
	|	Expr MINUS Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_SUB);}
	|	Expr MULTIPLY Expr								{$$ = addExpression(d_infix, 0, $1, $3, is_MULTIPLY);}
	|	Expr DIVIDE Expr								{$$ = addExpression(d_infix, 0, $1, $3, is_DIVIDE);}
	|	Expr REM Expr									{$$ = addExpression(d_infix, 0, $1, $3, is_REM);}
	|	Expr2 											{$$ = $1;}
	|	NEW INT OSQUARE Expr CSQUARE					{$$ = addExpression(d_NEW_INT_OS_E_CS, 0, $4, NULL, is_NULL0);}
	|	NEW BOOL OSQUARE Expr CSQUARE					{$$ = addExpression(d_NEW_BOOL_OS_E_CS, 0, $4, NULL, is_NULL0);}
	|	PLUS Expr %prec NOT 							{$$ = addExpression(d_PLUS_E, 0, $2, NULL, is_NULL0);}
	|	MINUS Expr %prec NOT							{$$ = addExpression(d_MINUS_E, 0, $2, NULL, is_NULL0);}
	|	NOT Expr 										{$$ = addExpression(d_NOT_E, 0, $2, NULL, is_NOT);}
	;

Expr2:	ID												{$$ = addExpression(d_id,  0, $1, NULL, is_NULL0);}	
	|	INTLIT											{$$ = addExpression(d_number, 0, $1, NULL, is_NULL0);}	
	|	TRUE											{$$ = addExpression(d_bool, 1, NULL, NULL, is_NULL0);}	
	|	FALSE											{$$ = addExpression(d_bool, 0, NULL, NULL, is_NULL0);}	
	|	Expr DOTLENGTH									{$$ = addExpression(d_E_DOTLENGTH, 0, $1, NULL, is_NULL0);}	
	|	PARSEINT OCURV ID OSQUARE Expr CSQUARE CCURV 	{$$ = addExpression(d_PARSEINT, 0, $3, $5, is_NULL0);}
	|	OCURV Expr CCURV								{$$ = addExpression(d_unary, 0, $2, NULL, is_NULL0);}
	|	ID OCURV Args CCURV								{$$ = addExpressionArgs(d_ID_OC_ARGS_CC, $1, $3);}		
	|	Expr2 OSQUARE Expr CSQUARE 						{$$ = addExpression(d_E2_OS_E_CS, 0, $1, $3, is_NULL0);}
	;

Args:													{$$ = NULL;}
	|	Expr Args2										{$$ = addExpressionList($1, $2);}	
	;
	
Args2:													{$$ = NULL;}
	|	COMMA Expr Args2								{$$ = addExpressionList($2, $3);}
	;

%%
int main( int argc, char *argv[] )
{	
	int error;
	int i;
	int print_ast=0;
	int print_table=0;
	int do_semantic_analysis=0;
	yyparse();
	
	if (sintax_error==1)
	{
		return 0;
	}

	for (i=1; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			print_ast=1;
		}

		if (strcmp(argv[i], "-s") == 0) {
			print_table=1;
		}
	}

	/*print ast*/
	if (print_ast==1)
		print_tree(root);

	/*create symbol table*/
	error=create_symbol_table(root);
	if (error == 1) /* error while creating symbol table */
		return 0;

	/*check semantics*/
	error=check_program(root);
	if (error == 1) /* error while checking semantics */
		return 0;
	
	if (print_table==1)
	{
		assert(root!=NULL);
		show_table(root->program_id);
	}

	if (print_ast==0 && print_table==0)
		generateCode(root, symtab);
	
	return 0;
}

int yyerror(char* s)
{
	int column = currentColumn-yyleng;
	int line = currentLine;
	printf ("Line %d, col %d: %s: %s\n", line, column, s, yytext);
	sintax_error=1;
	return 1;
}


















