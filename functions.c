#include <stdio.h>
#include <stdlib.h>
#include "structures.h"
#include "functions.h"


is_expression_list* addExpressionList(is_expression* expr, is_expression_list* next)
{
	is_expression_list* expressionList = (is_expression_list*) malloc(sizeof(is_expression_list));
	expressionList->expr = expr;
	expressionList->next = next;

	return expressionList;
}

is_expression* addExpressionArgs(disc_expression type, char* id, is_expression_list* exp_list)
{
	is_expression* expression = (is_expression*) malloc(sizeof(is_expression));
	expression->disc_type = type;
	expression->data.id = id;
	expression->exp2 = NULL;
	expression->oper = is_NULL;
	expression->exp_list = exp_list;

	return expression;
}

is_expression* addExpression(disc_expression type, int number, void* arg, is_expression* exp2, is_oper oper)
{
	is_expression* expression = (is_expression*) malloc(sizeof(is_expression));
	expression->disc_type = type;

	switch(type) /* possible values: d_infix, d_unary,  d_number, d_bool, d_id, other */
	{
		case(d_id):
			expression->data.id = (char*) arg;
			break;
		
		case(d_bool):
			expression->data.bool = number;
			break;

		case(d_number):
			expression->data.number = (char*) arg;
			break;

		case(d_PARSEINT):
			expression->data.id = (char*) arg;
			break;
		
		default:
			expression->data.exp1 = (is_expression*) arg;
			break;
	}
	
	expression->exp2 = exp2;
	expression->oper = oper;
	expression->exp_list = NULL;

	return expression;
}

/*Nos casos de assign, exp1 e' sempre o resultado
no caso de assignArray exp2 e' a indexacao*/
is_statement* addStatement (data_description description, is_expression* exp1, is_statement* statement1, 
	is_statement* statement2, char* id, is_expression* exp2, is_statement_list* list)
{
	/* possible values: desc_COMPSTAT, desc_IFELSE, desc_WHILE,
	desc_PRINT, desc_RETURN, desc_RETURN_E, desc_ASSIGN, desc_ASSIGNARRAY */

	if(list!=NULL && list->statement!=NULL && list->statement->desc == desc_COMPSTAT)
		return list->statement;

	is_statement* statement = (is_statement*) malloc(sizeof(is_statement));
	statement->desc = description;
	statement->exp1 = exp1;
	statement->statement1 = statement1;
	statement->statement2 = statement2;
	statement->statement_list = list;
	statement->id = id;
	statement->exp2 = exp2;

	return statement;
}

is_statement_list* addStatementList (is_statement* statement, is_statement_list* next)
{
	if (next!= NULL &&
		next->statement->desc == desc_COMPSTAT && statement->desc == desc_COMPSTAT)
		return next;

	is_statement_list* node = (is_statement_list*) malloc(sizeof(is_statement_list));
	node->statement = statement;
	node->next = next;

	return node;
}

is_method_body* addMethodBody(is_var_decl_list* var_decl_list, is_statement_list* statement_list)
{
	is_method_body* method_body = (is_method_body*) malloc(sizeof(is_method_body));
	method_body->var_decl_list=var_decl_list;
	method_body->statement_list=statement_list;
	
	return method_body;
}

is_method_params* addMethodParams(is_type type, char* param_name, is_method_params* next)
{
	is_method_params* method_params = (is_method_params*) malloc(sizeof(is_method_params));
	method_params->type=type;
	method_params->param_name=param_name;
	method_params->next=next;
	
	return method_params;
}

is_method_decl* addMethodDecl(is_type type, char* method_name, is_method_params* formal_params, 
	is_method_body* method_body)
{
	is_method_decl* method_decl = (is_method_decl*) malloc(sizeof(is_method_decl));
	method_decl->type=type;
	method_decl->method_name=method_name;
	method_decl->formal_params = formal_params;
	method_decl->method_body = method_body;
	
	return method_decl;
}

is_var_decl* addVarDecl(is_type type, char* var_name, is_var_decl* next)
{
	is_var_decl* var_decl = (is_var_decl*) malloc(sizeof(is_var_decl));
	var_decl->type=type;
	var_decl->var_name = var_name;
	var_decl->next = next;
	
	return var_decl;
}

is_var_decl_list* addVarDeclList(is_var_decl* var_decl, is_var_decl_list* next)
{
	is_var_decl_list* var_decl_list = (is_var_decl_list*) malloc(sizeof(is_var_decl_list));
	var_decl_list->var_decl = var_decl;
	var_decl_list->next = next;
	
	return var_decl_list;
}


is_field_method_decl* addFieldMethodDecl(data_description description, void* varORmethodDecl, is_field_method_decl* next)
{
	is_field_method_decl* decl = (is_field_method_decl*) malloc(sizeof(is_field_method_decl));
	decl->desc=description;
	decl->next=next;
	
	switch(description) /* possible values: desc_ID, desc_METHODDECL, desc_VARDECL */
	{
		case(desc_METHODDECL):
			decl->data.method_decl = (is_method_decl*) varORmethodDecl;
			break;
		
		case(desc_VARDECL):
			decl->data.var_decl = (is_var_decl*) varORmethodDecl;
			break;
	}
	
	return decl;
}

is_start_program* addProgram(char* id, is_field_method_decl* fieldMethodDecl)
{
	/* allocate memory for is_start_program_struct */
	is_start_program* program = (is_start_program*) malloc(sizeof(is_start_program)); 
	
	/* fill fields  */
	program->program_id = id;	/*VER esta atribuicao a char* funciona? ou temos de fazer malloc e copiar a string? */
								/*A Sandra usou assim*/
	program->field_method_decl = fieldMethodDecl;
	
	return program;
}

void clearTree (is_start_program* root)
{

}




