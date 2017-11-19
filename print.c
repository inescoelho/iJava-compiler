#include "print.h"
#include "structures.h"
#include <stdlib.h>
#include <stdio.h>

void print_tabs(int num)
{
	int i;
	for (i = 0; i < num; i++)
		printf("  ");
}

/*print the type of parameters or function return*/
void print_type(int type)
{
	switch(type) {
		case (is_INT):
			printf("Int\n");
			break;
		case (is_BOOL):
			printf("Bool\n");
			break;
		case (is_INTARRAY):
			printf("IntArray\n");
			break;
		case (is_BOOLARRAY):
			printf("BoolArray\n");
			break;
		case (is_STRINGARRAY):
			printf("StringArray\n");
			break;
		case (is_VOID):
			printf("Void\n");
			break;
	}
}

void print_tree(is_start_program* root)
{
	is_field_method_decl* aux;
	int num_tabs = 1;

	if (root != NULL)
	{
		printf("Program\n");

		print_tabs(num_tabs);
		printf("Id(%s)\n", root->program_id);
	}

	for (aux=root->field_method_decl; aux != NULL; aux=aux->next) {

		print_tabs(num_tabs);

		if (aux->desc == desc_VARDECL) {
			printf("VarDecl\n");
			print_field_declaration(aux->data.var_decl, num_tabs+1);
		}

		else if (aux->desc == desc_METHODDECL) {
			printf("MethodDecl\n");
			print_method_declaration(aux->data.method_decl, num_tabs+1);
		}
	}
}

void print_field_declaration(is_var_decl* var_decl, int num_tabs)
{
	is_var_decl* aux;

	print_tabs(num_tabs);
	print_type(var_decl->type);

	for (aux=var_decl; aux != NULL; aux=aux->next) 
	{
		print_tabs(num_tabs);
		printf("Id(%s)\n", aux->var_name);
	}
}

void print_method_declaration(is_method_decl* method_decl, int num_tabs)
{
	print_tabs(num_tabs);
	print_type(method_decl->type);

	print_tabs(num_tabs);
	printf("Id(%s)\n", method_decl->method_name);

	print_tabs(num_tabs);
	printf("MethodParams\n");
	if (method_decl->formal_params != NULL)
	{
		print_Method_Params(method_decl->formal_params, num_tabs+1);
	}

	print_tabs(num_tabs);
	printf("MethodBody\n");
	if (method_decl->method_body != NULL && (method_decl->method_body-> var_decl_list!= NULL 
		|| method_decl->method_body->statement_list != NULL) )
	{
		print_Method_Body(method_decl->method_body, num_tabs+1);
	}
}

void print_Method_Params(is_method_params* method_params, int num_tabs)
{
	is_method_params* aux;

	for (aux=method_params; aux != NULL; aux=aux->next) 
	{
		print_tabs(num_tabs);
		printf("ParamDeclaration\n");	

		print_tabs(num_tabs+1);
		print_type(aux->type);

		print_tabs(num_tabs+1);
		printf("Id(%s)\n", aux->param_name);
	}
}

void print_Method_Body(is_method_body* method_body, int num_tabs)
{
	if (method_body->var_decl_list != NULL)
	{
		print_Var_Decl_List(method_body->var_decl_list, num_tabs);
	}

	if (method_body->statement_list != NULL)
	{
		print_statement_list(method_body->statement_list, num_tabs);
	}
}

void print_Var_Decl_List(is_var_decl_list* var_decl_list, int num_tabs)
{
	is_var_decl_list* aux;

	for (aux=var_decl_list; aux != NULL; aux=aux->next) 
	{
		print_tabs(num_tabs);
		printf("VarDecl\n");
		print_Var_Decl(aux->var_decl, num_tabs+1);
	}
}

void print_Var_Decl(is_var_decl* var_decl, int num_tabs)
{
	is_var_decl* aux;

	print_tabs(num_tabs);
	print_type(var_decl->type);

	for (aux=var_decl; aux != NULL; aux=aux->next) 
	{
		print_tabs(num_tabs);
		printf("Id(%s)\n", aux->var_name);
	}
}

void print_Expression(is_expression* expression, int num_tabs)
{
	switch(expression->disc_type)
	{
		case (d_id):
			print_tabs(num_tabs);
			printf("Id(%s)\n", expression->data.id);
			break;
		case (d_number):
			print_tabs(num_tabs);
			printf("IntLit(%s)\n", expression->data.number);
			break;
		case (d_bool):
			print_tabs(num_tabs);
			if (expression->data.bool == 0)
				printf("BoolLit(false)\n");
			else
				printf("BoolLit(true)\n");
			break;
		case (d_unary):
			print_Expression(expression->data.exp1, num_tabs);
			break;
		case (d_infix):
			print_type_infix_expression(expression->oper, num_tabs);
			print_Expression(expression->data.exp1, num_tabs+1);
			print_Expression(expression->exp2, num_tabs+1);
			break;
		case (d_NEW_INT_OS_E_CS):
			print_tabs(num_tabs);
			printf("NewInt\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_NEW_BOOL_OS_E_CS):
			print_tabs(num_tabs);
			printf("NewBool\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_NOT_E):
			print_tabs(num_tabs);
			printf("Not\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_PLUS_E):
			print_tabs(num_tabs);
			printf("Plus\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_MINUS_E):
			print_tabs(num_tabs);
			printf("Minus\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_E_DOTLENGTH):
			print_tabs(num_tabs);
			printf("Length\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			break;
		case (d_E2_OS_E_CS):
			print_tabs(num_tabs);
			printf("LoadArray\n");
			print_Expression(expression->data.exp1, num_tabs+1);
			print_Expression(expression->exp2, num_tabs+1);
			break;
		case (d_PARSEINT):
			print_tabs(num_tabs);
			printf("ParseArgs\n");
			print_tabs(num_tabs+1);
			printf("Id(%s)\n", expression->data.id);
			print_Expression(expression->exp2, num_tabs+1);
			break;
		case (d_ID_OC_ARGS_CC):
			print_tabs(num_tabs);
			printf("Call\n");
			print_tabs(num_tabs+1);
			printf("Id(%s)\n", expression->data.id);
			print_Expression_Args(expression->exp_list, num_tabs+1);
			break;
	}
}

void print_Expression_Args(is_expression_list* exp_list, int num_tabs)
{
	is_expression_list* aux;

	for (aux=exp_list; aux != NULL; aux=aux->next) 
	{
		print_Expression(aux->expr, num_tabs);
	}
}

void print_type_infix_expression(is_oper oper, int num_tabs)
{
	print_tabs(num_tabs);

	switch(oper)
	{
		case (is_AND):
			printf("And\n");
			break;
		case (is_OR):
			printf("Or\n");
			break;
		case (is_EQUAL):
			printf("Eq\n");
			break;
		case (is_NOTEQUAL):
			printf("Neq\n");
			break;
		case (is_LT):
			printf("Lt\n");
			break;
		case (is_GT):
			printf("Gt\n");
			break;
		case (is_LTE):
			printf("Leq\n");
			break;
		case (is_GTE):
			printf("Geq\n");
			break;
		case (is_ADD):
			printf("Add\n");
			break;
		case (is_SUB):
			printf("Sub\n");
			break;
		case (is_MULTIPLY):
			printf("Mul\n");
			break;
		case (is_DIVIDE):
			printf("Div\n");
			break;
		case (is_REM):
			printf("Mod\n");
			break;
	}
}

void print_statement_list(is_statement_list* statement_list, int num_tabs)
{
	if (statement_list==NULL)
	{
		return;
	}
	while(statement_list != NULL)
	{
		print_statement(statement_list->statement, num_tabs);
		statement_list = statement_list->next;
	}
}

void print_statement_compound(is_statement* statement, int num_tabs)
{
	is_statement_list* aux;

	if (statement == NULL)
	{
		print_tabs(num_tabs);
		printf("Null\n");
		return;
	}
	else
	{
		if (statement->desc != desc_COMPSTAT)
		{
			print_statement(statement, num_tabs);
			return;
		}
	}

	aux = statement->statement_list;
	if (aux!= NULL)
	{
		if (aux->next != NULL) 
		{
			print_tabs(num_tabs);
			printf("CompoundStat\n");
			while(aux != NULL)
			{
				print_statement(aux->statement, num_tabs+1);
				aux = aux->next;
			}
		}
		else
		{
			print_statement(aux->statement, num_tabs);
		}
	}
	else
	{
		print_tabs(num_tabs);
		printf("Null\n");
		return;
	}
}

void print_statement(is_statement* statement, int num_tabs)
{
	if (statement==NULL)
	{
		return;
	}
	switch(statement->desc)
	{
		case(desc_COMPSTAT):
			print_statement_list(statement->statement_list, num_tabs+1);
			break;
			
		case(desc_IFELSE):
			print_tabs(num_tabs);
			printf("IfElse\n");
			print_Expression(statement->exp1, num_tabs+1);	/* print condition */
			print_statement_compound(statement->statement1, num_tabs+1);	/* print 'if' branch */
			print_statement_compound(statement->statement2, num_tabs+1); /* print 'else' branch */
			break;
			
		case(desc_WHILE):
			print_tabs(num_tabs);
			printf("While\n");
			print_Expression(statement->exp1, num_tabs+1);
			print_statement_compound(statement->statement1, num_tabs+1);
			break;
			
		case(desc_PRINT):
			print_tabs(num_tabs);
			printf("Print\n");
			print_Expression(statement->exp1, num_tabs+1);
			break;
			
		case(desc_ASSIGN):
			print_tabs(num_tabs);
			printf("Store\n");
			print_tabs(num_tabs+1);
			printf("Id(%s)\n", statement->id); /* print var ID */
			print_Expression(statement->exp1, num_tabs+1); /* print assign expression */
			break;
			
		case(desc_ASSIGNARRAY):
			print_tabs(num_tabs);
			printf("StoreArray\n");
			print_tabs(num_tabs+1);
			printf("Id(%s)\n", statement->id); /* print var ID */
			print_Expression(statement->exp2, num_tabs+1); /* print index */
			print_Expression(statement->exp1, num_tabs+1); /* print assign expression */
			break;
			
		case(desc_RETURN_E):
			print_tabs(num_tabs);
			printf("Return\n");
			print_Expression(statement->exp1, num_tabs+1);
			break;
			
		case(desc_RETURN):
			print_tabs(num_tabs);
			printf("Return\n");
			break;
	}
}


