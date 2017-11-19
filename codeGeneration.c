#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGeneration.h"
#include "structures.h"
#include "symbol_table.h"
#include <assert.h>

#define MAXSTR 100
#define MAXTEXT 300

int registerIndex;
int structureIndex;

/*initiate code generation*/
void generateCode(is_start_program* root, class_element* symtab)
{
	class_element* aux_variable;
	is_field_method_decl* aux_method;

	/*initiate struture index - global to the intire program*/
	structureIndex = 0;

	/*generate structures declaration*/
	printf("%%IntArray = type {i32, i32*}\n");
	printf("%%BooleanArray = type {i32, i1*}\n");
	printf("\n");

	/*generate global variables declaration*/
	for (aux_variable = symtab; aux_variable != NULL; aux_variable=aux_variable->next) 
	{
		if (aux_variable->desc == is_VAR) 
		{
			generate_global_variable(aux_variable);
		}
	}

	/*generate standard declarations - print, atoi, malloc*/
	generate_declarations();

	/*generate methods*/
	for (aux_method=root->field_method_decl; aux_method != NULL; aux_method=aux_method->next) 
	{
		if (aux_method->desc == desc_METHODDECL) 
		{
			printf("\n");
			aux_variable = search_class_el(aux_method->data.method_decl->method_name);
			generate_method(aux_method->data.method_decl, aux_variable);
		}
	}
}

/*generate global variables declaration*/
void generate_global_variable(class_element* var_decl)
{
	switch(var_decl->type)
	{
		case (is_INT):
			printf("@%s = common global i32 0\n", var_decl->name);
			break;
		case (is_BOOL):
			printf("@%s = common global i1 0\n", var_decl->name);
			break;
		case (is_INTARRAY):
			printf("@%s = common global %%IntArray* null\n", var_decl->name);
			break;
		case (is_BOOLARRAY):
			printf("@%s = common global %%BooleanArray* null\n", var_decl->name);
			break;
	}

	var_decl->reg = -1;
}

/*generate methods*/
void generate_method(is_method_decl* method_decl, class_element* method_symbols)
{
	char varTypeStr[MAXSTR] = "";
	is_statement_list* statement_list_aux;
	int num_tabs = 1;

	/*print method declaration*/
	getSize(method_symbols->type, varTypeStr);
	
	/*method definition - main must retorn an int value*/
	if (strcmp(method_symbols->name, "main")==0)
		printf("define i32 @%s(", method_symbols->name);
	else
		printf("define %s @%s(", varTypeStr, method_symbols->name);

	/*initiate register number - used locally inside a method*/
	registerIndex = 1; /*address 1 is used for return value*/

	/*generate method parameters*/
	if (method_symbols->method_element_list != NULL)
	{
		generate_method_params(method_symbols->method_element_list);
	}
	printf(")\n");

	/*initiate method body*/
	printf("{\n");

	/*allocate space for formal parameters*/
	allocate_method_params(method_symbols->type, method_symbols->method_element_list);

	/*generate statement list*/
	if (method_decl->method_body != NULL)
	{
		for (statement_list_aux = method_decl->method_body->statement_list; 
				statement_list_aux!= NULL; statement_list_aux = statement_list_aux->next)
		{
			generate_statement(statement_list_aux->statement, method_symbols, num_tabs);
			if (statement_list_aux->statement->desc == desc_IFELSE || statement_list_aux->statement->desc == desc_WHILE)
				num_tabs++;
			if (statement_list_aux->next == NULL && statement_list_aux->statement->desc != desc_RETURN && statement_list_aux->statement->desc != desc_RETURN_E)
				printf("\tbr label %%return.%s\n", method_symbols->name);
		}
	}

	/*insert label return*/
	generate_return_label(method_symbols);

	/*terminate method body*/
	printf("}\n");
}

void generate_return_label(class_element* method_symbols)
{
	int current_register_index;

	printf("\n\treturn.%s:\n", method_symbols->name);
	if (method_symbols->type == is_VOID)
	{
		if (strcmp(method_symbols->name, "main")==0)
			printf("\t\tret i32 0\n");
		else
			printf("\t\tret void\n");
	}
	else
	{
		switch(method_symbols->type)
		{
			case (is_INT):
				current_register_index = generateRegisterIndex();
				printf("\t\t%%reg.%d = load i32* %%reg.1\n", current_register_index);
				printf("\t\tret i32 %%reg.%d\n", current_register_index);
				break;
			case (is_BOOL):
				current_register_index = generateRegisterIndex();
				printf("\t\t%%reg.%d = load i1* %%reg.1\n", current_register_index);
				printf("\t\tret i1 %%reg.%d\n", current_register_index);
				break;
			case (is_INTARRAY):
				current_register_index = generateRegisterIndex();
				printf("\t\t%%reg.%d = load %%IntArray** %%reg.1\n", current_register_index);
				printf("\t\tret %%IntArray* %%reg.%d\n", current_register_index);
				break;
			case (is_BOOLARRAY):
				current_register_index = generateRegisterIndex();
				printf("\t\t%%reg.%d = load %%BooleanArray** %%reg.1\n", current_register_index);
				printf("\t\tret %%BooleanArray* %%reg.%d\n", current_register_index);
				break;
		}
	}
}

void generate_method_params (method_element* method_el)
{
	char varTypeStr[MAXSTR] = "";
	method_element* aux;

	if(method_el->desc == is_PARAMETER)
	{
		getSize(method_el->type, varTypeStr);

		/*spread stringarray in length and array*/
		if (method_el->type == is_STRINGARRAY)
		{
			generateRegisterIndex();
			printf("i32 %%%s.length, ", method_el->name);
		}

		printf("%s %%%s", varTypeStr, method_el->name);
		method_el->reg = generateRegisterIndex();
		method_el = method_el->next;
	}

	for (aux=method_el; aux != NULL; aux=aux->next) 
	{
		if(aux->desc == is_PARAMETER)
		{
			getSize(aux->type, varTypeStr);
			printf(", %s %%%s", varTypeStr, aux->name);
			aux->reg = generateRegisterIndex();
		}
		else
			break;
	}
}

void allocate_method_params (is_type return_type, method_element* method_el)
{
	char varTypeStr[MAXSTR] = "";
	method_element* aux;

	/*create return value*/
	switch(return_type)
	{
		case (is_INT):
			printf("\t%%reg.1 = alloca i32\n");
			printf("\tstore i32 0, i32* %%reg.1\n");
			break;
		case (is_BOOL):
			printf("\t%%reg.1 = alloca i1\n");
			printf("\tstore i1 0, i1* %%reg.1\n");
			break;
		case (is_INTARRAY):
			printf("\t%%reg.1 = alloca %%IntArray*\n");
			printf("\tstore %%IntArray* null, %%IntArray** %%reg.1\n");
			break;
		case (is_BOOLARRAY):
			printf("\t%%reg.1 = alloca %%BooleanArray*\n");
			printf("\tstore %%BooleanArray* null, %%BooleanArray** %%reg.1\n");
			break;
	}

	for (aux=method_el; aux != NULL; aux=aux->next) 
	{
		/*create parameters*/
		if(aux->desc == is_PARAMETER)
		{
			if (method_el->type == is_STRINGARRAY)
			{
				/*allocate string array length on register number 2*/
				printf("\t%%reg.2 = alloca i32\n");
				printf("\tstore i32 %%%s.length, i32* %%reg.2\n", method_el->name);				
			}
			getSize(aux->type, varTypeStr); /* parameter -> load value to register */
			printf("\t%%reg.%d = alloca %s\n", aux->reg, varTypeStr);
			printf("\tstore %s %%%s, %s* %%reg.%d\n", varTypeStr, aux->name, varTypeStr, aux->reg);
		}
		else
		{
			aux->reg = -1; /* local variable */
			getSize(aux->type, varTypeStr);
			printf("\t%%%s = alloca %s\n", aux->name, varTypeStr);
		}
	}
}

void generate_statement(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el = NULL;
	is_type type;

	switch(statement->desc)
	{	
		case(desc_COMPSTAT):
			generate_statement_list(statement->statement_list, method_symbols, num_tabs);
			break;
			
		case(desc_IFELSE):
			generate_statement_ifelse(statement, method_symbols, num_tabs);
			break;
			
		case(desc_WHILE):
			generate_statement_while(statement, method_symbols, num_tabs);
			break;

		case(desc_PRINT):
			generate_statement_print(statement, method_symbols, num_tabs);
			break;

		case(desc_ASSIGN):
			generate_statement_assign(statement, method_symbols, num_tabs);
			break;

		case(desc_ASSIGNARRAY):
			generate_statement_assign_array(statement, method_symbols, num_tabs);
			break;
			
		case(desc_RETURN_E):
			generate_statement_return(statement, method_symbols, num_tabs);
			break;
			
		case(desc_RETURN):
			print_tabulation(num_tabs);
			printf("\tbr label %%return.%s\n", method_symbols->name);
			break;
	}

	if (expression_el != NULL)
		free(expression_el);
}

void generate_statement_list(is_statement_list* statement_list, class_element* method_symbols, int num_tabs)
{
	while(statement_list != NULL)
	{
		generate_statement(statement_list->statement, method_symbols, num_tabs);
		if (statement_list->statement->desc == desc_IFELSE || statement_list->statement->desc == desc_WHILE)
			num_tabs++;
		statement_list = statement_list->next;
	}
}

void generate_statement_ifelse(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	int labelIndex = generateStructureIndex();
	int num_tabs_in_stat;
	is_statement_list* statement_list;
	char varType[MAXSTR];
	char varTypeStr[MAXSTR];
	char register_expression[MAXSTR] ;
	int current_register_index;

	print_tabulation(num_tabs);
	printf("br label %%entry.%d\n\n", labelIndex);

	print_tabulation(num_tabs);
	printf("entry.%d:\n", labelIndex);
	expression_el = generate_expression(statement->exp1, method_symbols, num_tabs+1);

	/*check if we need to load variable to register*/
	switch(expression_el->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(register_expression, "%d", expression_el->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(register_expression, "%%reg.%d", expression_el->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_el->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_el->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el->type, varTypeStr);
		print_tabulation(num_tabs+1);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(register_expression, "%%reg.%d", current_register_index);
	}

	print_tabulation(num_tabs+1);
	printf("br i1 %s, label %%then.%d, label %%else.%d\n\n", register_expression, labelIndex, labelIndex);

	print_tabulation(num_tabs);
	printf("then.%d:\n", labelIndex);
	generate_statement(statement->statement1, method_symbols, num_tabs+1);
	if (statement->statement1 != NULL)
	{
		num_tabs_in_stat = num_tabs+1;
		for (statement_list = statement->statement1->statement_list; statement_list!= NULL; statement_list = statement_list->next)
		{
			if (statement_list->statement->desc == desc_IFELSE || statement_list->statement->desc == desc_WHILE)
				num_tabs_in_stat++;
		}
	}
	print_tabulation(num_tabs_in_stat);
	printf("br label %%ifcont.%d\n\n", labelIndex);

	print_tabulation(num_tabs);
	printf("else.%d:\n", labelIndex);
	if (statement->statement2 != NULL)
	{
		generate_statement(statement->statement2, method_symbols, num_tabs+1);
		num_tabs_in_stat = num_tabs+1;
		for (statement_list = statement->statement2->statement_list; statement_list!= NULL; statement_list = statement_list->next)
		{
			if (statement_list->statement->desc == desc_IFELSE || statement_list->statement->desc == desc_WHILE)
				num_tabs_in_stat++;
		}
	}
	print_tabulation(num_tabs_in_stat);
	printf("br label %%ifcont.%d\n\n", labelIndex);

	print_tabulation(num_tabs);
	printf("ifcont.%d:\n", labelIndex);

	free (expression_el);
	return;
}

void generate_statement_while(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	int labelIndex = generateStructureIndex();
	int num_tabs_in_stat;
	is_statement_list* statement_list;
	char varType[MAXSTR];
	char varTypeStr[MAXSTR];
	char register_expression[MAXSTR] ;
	int current_register_index;

	print_tabulation(num_tabs);
	printf("br label %%entry.%d\n\n", labelIndex);

	print_tabulation(num_tabs);
	printf("entry.%d:\n", labelIndex);
	expression_el = generate_expression(statement->exp1, method_symbols, num_tabs+1);

	/*check if we need to load variable to register*/
	switch(expression_el->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(register_expression, "%d", expression_el->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(register_expression, "%%reg.%d", expression_el->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_el->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_el->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el->type, varTypeStr);
		print_tabulation(num_tabs+1);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(register_expression, "%%reg.%d", current_register_index);
	}

	print_tabulation(num_tabs+1);
	printf("br i1 %s, label %%then.%d, label %%ifcont.%d\n\n", register_expression, labelIndex, labelIndex);


	print_tabulation(num_tabs);
	printf("then.%d:\n", labelIndex);
	generate_statement(statement->statement1, method_symbols, num_tabs+1);
	num_tabs_in_stat = num_tabs+1;
	for (statement_list = statement->statement1->statement_list; statement_list!= NULL; statement_list = statement_list->next)
	{
		if (statement_list->statement->desc == desc_IFELSE || statement_list->statement->desc == desc_WHILE)
			num_tabs_in_stat++;
	}
	print_tabulation(num_tabs_in_stat);
	printf("br label %%entry.%d\n\n", labelIndex);

	print_tabulation(num_tabs);
	printf("ifcont.%d:\n", labelIndex);

	free (expression_el);
	return;
}

void generate_statement_print(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	char varType[MAXSTR]="";
	char register_expression[MAXSTR]="";
	char varTypeStr[MAXSTR] = "";
	int current_register_index;
	int labelIndex;

	expression_el = generate_expression(statement->exp1, method_symbols, num_tabs);

	/*check if we need to load variable to register*/
	switch(expression_el->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(register_expression, "%%reg.%d", expression_el->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_el->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_el->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(register_expression, "%%reg.%d", current_register_index);
	}

	/*check variables type*/
	getSize(expression_el->type, varTypeStr);
	/*generate new register*/
	current_register_index = generateRegisterIndex();

	/*print element*/
	if (expression_el->desc == is_LITERAL) /*is a literal, so we can print it directly*/
	{
		if (expression_el->type == is_INTLIT)
		{
			print_tabulation(num_tabs);
			printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %d)\n", 
		current_register_index, expression_el->value);
		}
		else /*expression is a boolean*/
		{
			if(expression_el->value == 0) /*value is false*/
			{
				print_tabulation(num_tabs);
				printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str2, i32 0, i32 0))\n", 
					current_register_index);
			}
			else /*value is true*/
			{
				print_tabulation(num_tabs);
				printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str1, i32 0, i32 0))\n", 
					current_register_index);
			}
		}
	}
	else /*use register to print variable value*/
	{
		if (expression_el->type == is_INT || expression_el->type == is_INTLIT)
		{
			print_tabulation(num_tabs);
			printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str, i32 0, i32 0), i32 %s)\n", 
		current_register_index, register_expression);
		}
		else /*expression is a boolean*/
		{
			labelIndex = generateStructureIndex();

			print_tabulation(num_tabs);
			printf("%%ifcond.%d = icmp eq i1 %s, 0\n", labelIndex, register_expression);
			print_tabulation(num_tabs);
			printf("br i1 %%ifcond.%d, label %%then.%d, label %%else.%d\n\n", labelIndex, labelIndex, labelIndex);

			print_tabulation(num_tabs);
			printf("then.%d:\n", labelIndex);
			print_tabulation(num_tabs+1);
			printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str2, i32 0, i32 0))\n", 
				current_register_index);
			print_tabulation(num_tabs+1);
			printf("br label %%ifcont.%d\n\n", labelIndex);

			current_register_index = generateRegisterIndex();
			print_tabulation(num_tabs);
			printf("else.%d:\n", labelIndex);
			print_tabulation(num_tabs+1);
			printf("%%reg.%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([6 x i8]* @.str1, i32 0, i32 0))\n", 
				current_register_index);
			print_tabulation(num_tabs+1);
			printf("br label %%ifcont.%d\n\n", labelIndex);

			print_tabulation(num_tabs);
			printf("ifcont.%d:\n", labelIndex);
		}
	}

	/*free memory*/
	free(expression_el);
}

void generate_statement_assign(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_id;
	expression_element* expression_el;
	char idType[MAXSTR]="";
	char varType[MAXSTR]="";
	char auxStr[MAXSTR]="";
	char varTypeStr[MAXSTR] = "";
	int current_register_index;

	expression_id = getExpressionElementFromSymbolTable(statement->id, method_symbols->name);
	expression_el = generate_expression(statement->exp1, method_symbols, num_tabs);

	/*check the ID type*/
	switch(expression_id->desc)
	{
		case (is_PARAMETER):
			sprintf(idType, "%%reg.%d", expression_id->register_index);
			break;
		case (is_GLOBAL):
			sprintf(idType, "@%s", statement->id);
			break;
		case (is_VAR):
			sprintf(idType, "%%%s", statement->id);
			break;
	}

	/*load variable to register if needed*/
	switch(expression_el->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(auxStr, "%d", expression_el->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(auxStr, "%%reg.%d", expression_el->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_el->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_el->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(auxStr, "%%reg.%d", current_register_index);
	}

	/*check variables type*/
	getSize(expression_el->type, varTypeStr);
	print_tabulation(num_tabs);
	printf("store %s %s, %s* %s\n", varTypeStr, auxStr, varTypeStr, idType);

	/*free memory*/
	free(expression_el);
	free(expression_id);
}

void generate_statement_assign_array(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	char idType[MAXSTR]="";
	char indexType[MAXSTR]="";
	char assignType[MAXSTR]="";
	char arrayType[MAXSTR]="";
	char varType[MAXSTR]="";	
	char varTypeStr[MAXSTR] = "";
	char resultType[MAXSTR] = "";
	int current_register_index;
	int current_register_index2;
	int current_register_index3;
	int current_register_index4;
	expression_element* expression_id; /* array ID */
	expression_element* expression_el1; /* expression to be assigned */
	expression_element* expression_el2; /* array index */

	expression_id = getExpressionElementFromSymbolTable(statement->id, method_symbols->name); 	/* array ID */
	expression_el1 = generate_expression(statement->exp1, method_symbols, num_tabs); 			/* expression to be assigned */
	expression_el2 = generate_expression(statement->exp2, method_symbols, num_tabs);			/* array index */

	/*check the ID type, and load var if needed */
	switch(expression_id->desc)
	{
		case (is_PARAMETER):
			current_register_index = generateRegisterIndex();
			getSize(expression_id->type, resultType);
			/*printf(">>>>>parameter %s\n", resultType);*/
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* %%reg.%d\n", current_register_index, resultType, expression_id->register_index);
			sprintf(idType, "%%reg.%d", current_register_index);
			break;
		case (is_GLOBAL):
			current_register_index = generateRegisterIndex();
			getSize(expression_id->type, resultType);
			/*printf(">>>>>global %s\n", resultType);*/
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* @%s\n", current_register_index, resultType, statement->id);
			sprintf(idType, "%%reg.%d", current_register_index);
			break;
		case (is_VAR):
			current_register_index = generateRegisterIndex();
			getSize(expression_id->type, resultType);
			/*printf(">>>>>var %s\n", resultType);*/
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* %%%s\n", current_register_index, resultType, statement->id);
			sprintf(idType, "%%reg.%d", current_register_index);
			break;
		case (is_RESULT):
			/*printf(">>>>>result %s\n", resultType);*/
			sprintf(idType, "%%reg.%d", expression_id->register_index);
			break;
	}

	/*check array index*/
	switch(expression_el2->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(indexType, "%d", expression_el2->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(indexType, "%%reg.%d", expression_el2->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s",  expression_el2->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s",  expression_el2->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el2->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el2->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexType, "%%reg.%d", current_register_index);
	}

	/*check if expression to be assigned is already loaded on a register; otherwise load it*/
	switch(expression_el1->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(assignType, "%d", expression_el1->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(assignType, "%%reg.%d", expression_el1->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s",  expression_el1->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s",  expression_el1->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el1->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el1->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(assignType, "%%reg.%d", current_register_index);
	}

	/*check variables type*/
	current_register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %s %s, i32 0, i32 1\n", 
		current_register_index2, resultType, idType );				/* load pointer to Array */
	current_register_index3 = generateRegisterIndex();
	
	if (expression_id->type == is_INTARRAY) /*check array type*/
		sprintf(arrayType, "i32");
	else /*expression_el1->type == is_BOOLARRAY*/
		sprintf(arrayType, "i1");
	
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %s** %%reg.%d\n", current_register_index3, arrayType, current_register_index2);  /* load pointer to array */
	current_register_index4 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %s* %%reg.%d, i32 %s\n", 
		current_register_index4, arrayType, current_register_index3, indexType ); /*get pointer to array index*/
	
	print_tabulation(num_tabs);
	printf("store %s %s, %s* %%reg.%d\n", arrayType, assignType, arrayType, current_register_index4);

	/*free memory*/
	free(expression_id);
	free(expression_el1);
	free(expression_el2);
}

void generate_statement_return(is_statement* statement, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	char varType[MAXSTR]="";
	char auxStr[MAXSTR]="";
	char varTypeStr[MAXSTR] = "";
	int current_register_index;

	expression_el = generate_expression(statement->exp1, method_symbols, num_tabs);

	/*load variable to register if needed*/
	switch(expression_el->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(auxStr, "%d", expression_el->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(auxStr, "%%reg.%d", expression_el->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_el->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_el->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_el->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_el->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(auxStr, "%%reg.%d", current_register_index);
	}

	/*check variables type*/
	getSize(expression_el->type, varTypeStr);
	print_tabulation(num_tabs);
	printf("store %s %s, %s* %%reg.1\n", varTypeStr, auxStr, varTypeStr);
	print_tabulation(num_tabs);
	printf("br label %%return.%s\n", method_symbols->name);

	/*free memory*/
	free(expression_el);
}

expression_element* generate_expression(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	char* method_name = method_symbols->name;

	switch(expression->disc_type)
	{
		case (d_id):
			expression_el = getExpressionElementFromSymbolTable(expression->data.id, method_name);
			break;
		case (d_number):
			expression_el = (expression_element*) malloc(sizeof(expression_element));
			expression_el->desc = is_LITERAL;
			expression_el->register_index = -1;
			expression_el->type = is_INTLIT;
			
			/*convert string to decimal value*/
			int value;
			if ( (expression->data.number)[0]=='0') /* possible hexa or octal */
			{
				if (expression->data.number[1] == '\0') /* decimal with value 0 */
				{
					value = 0;
				}
				else if ( (expression->data.number)[1]=='x') /*if hexa*/
				{
					sscanf(expression->data.number, "%x", &value);
				}
				else /* else => number is octal */
				{
					sscanf(expression->data.number, "%o", &value);
				}
			}
			else /* decimal */
			{
				sscanf(expression->data.number, "%d", &value);
			}
			
			expression_el->value = value;
			break;
			
		case (d_bool):
			expression_el = (expression_element*) malloc(sizeof(expression_element));
			expression_el->desc = is_LITERAL;
			expression_el->register_index = -1;
			expression_el->type = is_BOOLLIT;
			expression_el->value = expression->data.bool;
			break;
		case (d_unary):
			expression_el = generate_expression(expression->data.exp1, method_symbols, num_tabs);
			break;
		case (d_infix):
			expression_el = generate_infix_expression(expression, method_symbols, num_tabs);
			break;
		case (d_NEW_INT_OS_E_CS):
			expression_el =  generate_expression_int_array_initialization(expression, method_symbols, num_tabs);
			break;
		case (d_NEW_BOOL_OS_E_CS):
			expression_el =  generate_expression_bool_array_initialization(expression, method_symbols, num_tabs);
			break;
		case (d_NOT_E):
			expression_el = generate_expression_not(expression->data.exp1, method_symbols, num_tabs);
			break;
		case (d_PLUS_E):
			expression_el = generate_expression_plus(expression->data.exp1, method_symbols, num_tabs);
			break;
		case (d_MINUS_E):
			expression_el = generate_expression_minus(expression->data.exp1, method_symbols, num_tabs);
			break;
		case (d_E_DOTLENGTH):
			expression_el = generate_expression_length(expression->data.exp1, method_symbols, num_tabs);
			break;
		case (d_E2_OS_E_CS):
			expression_el = generate_expression_load_array(expression, method_symbols, num_tabs);
			break;
		case (d_PARSEINT):
			expression_el = generate_expression_parseint(expression, method_symbols, num_tabs);
			break;
		case (d_ID_OC_ARGS_CC):
			expression_el = generate_expression_call(expression, method_symbols, num_tabs);
			break;	
	}
	
	return expression_el;
}

expression_element* generate_infix_and_or_expression(is_expression* expr, class_element* method_symbols, int num_tabs)
{
	expression_element* expression_el;
	is_expression* expr1 = expr->data.exp1;
	is_oper oper = expr->oper;
	is_expression* expr2 = expr->exp2;
	int labelIndex = generateStructureIndex();
	char aux1Str[MAXSTR]="";
	char aux2Str[MAXSTR]="";
	int current_register_index;
	char varTypeStr[MAXSTR] = "";
	char varType[MAXSTR]="";
	expression_element* aux1;
	expression_element* aux2;
	aux1 = generate_expression(expr1, method_symbols, num_tabs);

	/*load aux 1 to register, if needed*/
	switch(aux1->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(aux1Str, "%d", aux1->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(aux1Str, "%%reg.%d", aux1->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux1->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux1->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux1->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux1->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(aux1Str, "%%reg.%d", current_register_index);
	}

	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = icmp ne i1 %s, 0\n", current_register_index, aux1Str);

	/*AND or OR short-circuit*/
	if (oper == is_AND)
	{
		print_tabulation(num_tabs);
		printf("br i1 %%reg.%d, label %%then.%d, label %%else.%d\n\n", current_register_index, labelIndex, labelIndex);
	}
	else
	{
		print_tabulation(num_tabs);
		printf("br i1 %%reg.%d, label %%else.%d, label %%then.%d\n\n", current_register_index, labelIndex, labelIndex);
	}

	print_tabulation(num_tabs);
	printf("then.%d:\n", labelIndex);

	aux2 = generate_expression(expr2, method_symbols, num_tabs);
	print_tabulation(num_tabs);
	printf("br label %%end.%d\n", labelIndex);
	print_tabulation(num_tabs);
	printf("end.%d:\n", labelIndex);
	/*load aux 2 to register, if needed*/
	switch(aux2->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(aux2Str, "%d", aux2->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(aux2Str, "%%reg.%d", aux2->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux2->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux2->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux2->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux2->type, varTypeStr);
		print_tabulation(num_tabs+1);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(aux2Str, "%%reg.%d", current_register_index);
	}

	print_tabulation(num_tabs+1);
	printf("%%result.%d = icmp ne i1 %s, 0\n", labelIndex, aux2Str);
	print_tabulation(num_tabs+1);
	printf("br label %%finally.%d\n", labelIndex);

	print_tabulation(num_tabs);
	printf("else.%d:\n", labelIndex);
	print_tabulation(num_tabs+1);
	printf("br label %%finally.%d\n", labelIndex);

	print_tabulation(num_tabs);
	printf("finally.%d:\n", labelIndex);

	/*AND or OR short-circuit*/
	current_register_index = generateRegisterIndex();
	if (oper == is_AND)
	{
		print_tabulation(num_tabs+1);
		printf("%%reg.%d = phi i1 [ %%result.%d, %%end.%d ], [ false, %%else.%d ]\n", current_register_index, labelIndex, labelIndex, labelIndex);
	}
	else
	{
		print_tabulation(num_tabs+1);
		printf("%%reg.%d = phi i1 [ %%result.%d, %%end.%d ], [ true, %%else.%d ]\n", current_register_index, labelIndex, labelIndex, labelIndex);
	}

	/*generate return value*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = is_BOOL;
	expression_el->register_index = current_register_index;

	return expression_el;
}

expression_element* generate_infix_expression(is_expression* expr, class_element* method_symbols, int num_tabs)
{
	is_expression* expr1 = expr->data.exp1;
	is_oper oper = expr->oper;
	is_expression* expr2 = expr->exp2;
	is_type resultType;

	char* operStr;
	char aux1Str[MAXSTR]="";
	char aux2Str[MAXSTR]="";
	int current_register_index;
	char varTypeStr[MAXSTR] = "";
	char varType[MAXSTR]="";
	expression_element* expression_el;
	
	expression_element* aux1;
	expression_element* aux2;
	
	if (oper == is_AND || oper == is_OR)
	{
		expression_el = generate_infix_and_or_expression(expr, method_symbols, num_tabs);
		return expression_el;
	}

	aux1 = generate_expression(expr1, method_symbols, num_tabs);
	aux2 = generate_expression(expr2, method_symbols, num_tabs);	

	switch(oper)
	{
		case (is_EQUAL):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp eq i1";
			else if (aux1->type==is_INT || aux1->type==is_INTLIT)
				operStr = "icmp eq i32";
			else if (aux1->type==is_INTARRAY)
				operStr = "icmp eq %IntArray*";
			else if (aux1->type==is_BOOLARRAY)
				operStr = "icmp eq %BooleanArray*";
			else /*string array*/
				operStr = "icmp eq i8**";
			break;
		case (is_NOTEQUAL):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp ne i1";
			else if (aux1->type==is_INT || aux1->type==is_INTLIT)
				operStr = "icmp ne i32";
			else if (aux1->type==is_INTARRAY)
				operStr = "icmp ne %IntArray*";
			else if (aux1->type==is_BOOLARRAY)
				operStr = "icmp ne %BooleanArray*";
			else /*string array*/
				operStr = "icmp ne i8**";
			break;
		case (is_LT):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp slt i1";
			else
				operStr = "icmp slt i32";
			break;
		case (is_GT):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp sgt i1";
			else
				operStr = "icmp sgt i32";
			break;
		case (is_LTE):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp sle i1";
			else
				operStr = "icmp sle i32";
			break;
		case (is_GTE):
			resultType = is_BOOL;
			if (aux1->type==is_BOOL || aux1->type==is_BOOLLIT)
				operStr = "icmp sge i1";
			else
				operStr = "icmp sge i32";
			break;
		case (is_ADD):
			resultType = is_INT;
			operStr = "add i32";
			break;
		case (is_SUB):
			resultType = is_INT;
			operStr = "sub i32";
			break;
		case (is_MULTIPLY):
			resultType = is_INT;
			operStr = "mul i32";
			break;
		case (is_DIVIDE):
			resultType = is_INT;
			operStr = "sdiv i32";
			break;
		case (is_REM):
			resultType = is_INT;
			operStr = "srem i32";
			break;
	}
	
	/*load variables to register if needed*/

	/*check aux 1*/
	switch(aux1->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(aux1Str, "%d", aux1->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(aux1Str, "%%reg.%d", aux1->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux1->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux1->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux1->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux1->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(aux1Str, "%%reg.%d", current_register_index);
	}

	/*check aux 2*/
	switch(aux2->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(aux2Str, "%d", aux2->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(aux2Str, "%%reg.%d", aux2->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux2->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux2->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux2->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux2->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(aux2Str, "%%reg.%d", current_register_index);
	}

	/*check variables type*/
	getSize(aux1->type, varTypeStr);
	
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = %s %s, %s\n", current_register_index, operStr, aux1Str, aux2Str);

	/*release memory*/
	free(aux1);
	free(aux2);
	
	/*generate return value*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = resultType;
	expression_el->register_index = current_register_index;

	return expression_el;
}

expression_element* generate_expression_int_array_initialization(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int current_register_index, register_index1, register_index2, array_index;
	char varType[MAXSTR];
	char varTypeStr[MAXSTR];
	char indexStr[MAXSTR];
	expression_element* aux;
	expression_element* expression_el;
	
	/*create new strut*/
	array_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = alloca %%IntArray*\n", array_index);
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = call noalias i8* @malloc(i32 8)\n", register_index1);
	/*cast strut to intarray strut*/
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = bitcast i8* %%reg.%d to %%IntArray*\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store %%IntArray* %%reg.%d, %%IntArray** %%reg.%d\n", register_index2, array_index);

	/*check array length*/
	aux = generate_expression(expression->data.exp1, method_symbols, num_tabs);
	/*load variable to register if needed*/
	switch(aux->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(indexStr, "%d", aux->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(indexStr, "%%reg.%d", aux->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexStr, "%%reg.%d", current_register_index);
	}

	/*allocate space for new array*/
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = call noalias i8* @calloc(i32 %s, i32 4)\n", register_index2, indexStr);
	/*cast array to int array*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = bitcast i8* %%reg.%d to i32*\n", current_register_index, register_index2);

	/*save array on struct*/
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%IntArray** %%reg.%d\n", register_index1, array_index);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %%IntArray* %%reg.%d, i32 0, i32 1\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store i32* %%reg.%d, i32** %%reg.%d\n", current_register_index, register_index2);

	/*save array size on struct*/
	if (strcmp(varType,"") != 0) /*size is not a literal value*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexStr, "%%reg.%d", current_register_index);
	}
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%IntArray** %%reg.%d\n", register_index1, array_index);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %%IntArray* %%reg.%d, i32 0, i32 0\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store i32 %s, i32* %%reg.%d\n", indexStr, register_index2);

	/*load new struct*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%IntArray** %%reg.%d\n", current_register_index, array_index);

	/*generate expression element to return, containing new array*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = is_INTARRAY;
	expression_el->register_index = current_register_index;

	/*release memory*/
	free(aux);

	return expression_el;
}

expression_element* generate_expression_bool_array_initialization(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int current_register_index, register_index1, register_index2, array_index;
	char varType[MAXSTR];
	char varTypeStr[MAXSTR];
	char indexStr[MAXSTR];
	expression_element* aux;
	expression_element* expression_el;
	
	/*create new strut*/
	array_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = alloca %%BooleanArray*\n", array_index);
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = call noalias i8* @malloc(i32 8)\n", register_index1);
	/*cast strut to intarray strut*/
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = bitcast i8* %%reg.%d to %%BooleanArray*\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store %%BooleanArray* %%reg.%d, %%BooleanArray** %%reg.%d\n", register_index2, array_index);

	/*check array length*/
	aux = generate_expression(expression->data.exp1, method_symbols, num_tabs);
	/*load variable to register if needed*/
	switch(aux->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(indexStr, "%d", aux->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(indexStr, "%%reg.%d", aux->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", aux->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", aux->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  aux->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexStr, "%%reg.%d", current_register_index);
	}

	/*allocate space for new array*/
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = call noalias i8* @calloc(i32 %s, i32 1)\n", register_index2, indexStr);
	/*cast array to int array*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = bitcast i8* %%reg.%d to i1*\n", current_register_index, register_index2);

	/*save array on struct*/
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%BooleanArray** %%reg.%d\n", register_index1, array_index);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %%BooleanArray* %%reg.%d, i32 0, i32 1\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store i1* %%reg.%d, i1** %%reg.%d\n", current_register_index, register_index2);

	/*save array size on struct*/
	if (strcmp(varType,"") != 0) /*size is not a literal value*/
	{
		current_register_index = generateRegisterIndex();
		getSize(aux->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexStr, "%%reg.%d", current_register_index);
	}
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%BooleanArray** %%reg.%d\n", register_index1, array_index);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %%BooleanArray* %%reg.%d, i32 0, i32 0\n", register_index2, register_index1);
	print_tabulation(num_tabs);
	printf("store i32 %s, i32* %%reg.%d\n", indexStr, register_index2);

	/*load new struct*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %%BooleanArray** %%reg.%d\n", current_register_index, array_index);

	/*generate expression element to return, containing new array*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = is_BOOLARRAY;
	expression_el->register_index = current_register_index;

	/*release memory*/
	free(aux);

	return expression_el;
}

expression_element* generate_expression_not(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int register_index_loaded_variable, register_index1, register_index2;;
	expression_element* expression_el;
	expression_element* aux;
	int labelIndex;
	int current_register_index;

	aux = generate_expression(expression, method_symbols, num_tabs);

	if(aux->desc == is_LITERAL)
	{
		/*generate expression element to return, containing new array*/
		expression_el = (expression_element*) malloc(sizeof(expression_element));
		expression_el->desc = is_LITERAL;
		expression_el->type = is_BOOLLIT;
		expression_el->register_index=-1;

		if(aux->value == 0)
			expression_el->value = 1;
		else
			expression_el->value = 0;
	}
	else
	{
		switch(aux->desc)
		{
			case is_GLOBAL:
				register_index_loaded_variable = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i1* @%s\n", register_index_loaded_variable, aux->name);
				break;
			case is_VAR:
				register_index_loaded_variable = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i1* %%%s\n", register_index_loaded_variable, aux->name);
				break;
			case is_PARAMETER:
				register_index_loaded_variable = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i1* %%reg.%d\n", register_index_loaded_variable, aux->register_index);
				break;	
			case is_RESULT:
				register_index_loaded_variable = aux->register_index;
				break;
		}

		register_index1 = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = icmp ne i1 %%reg.%d, 0\n", register_index1, register_index_loaded_variable);
		
		register_index2 = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = xor i1 %%reg.%d, 1\n", register_index2, register_index1);

		/*generate expression element to return, containing new array*/
		expression_el = (expression_element*) malloc(sizeof(expression_element));
		expression_el->desc = is_RESULT;
		expression_el->type = is_BOOL;
		expression_el->register_index=register_index2;
	}

	free(aux);

	return expression_el;
}

expression_element* generate_expression_plus(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	
	int register_index;
	expression_element* expression_el;

	expression_el = generate_expression(expression, method_symbols, num_tabs);

	if(expression_el->desc != is_LITERAL)
	{
		switch(expression_el->desc)
		{
			case is_GLOBAL:
				register_index = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* @%s\n", register_index, expression_el->name);
				break;
			case is_VAR:
				register_index = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* %%%s\n", register_index, expression_el->name);
				break;
			case is_PARAMETER:
				register_index = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* %%reg.%d\n", register_index, expression_el->register_index);
				break;	
			case is_RESULT:
				register_index = expression_el->register_index;
				break;
		}

		expression_el->desc = is_RESULT;
		expression_el->type = is_INT;
		expression_el->register_index=register_index;
	}

	return expression_el;
}

expression_element* generate_expression_minus(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int register_index1, register_index2;
	expression_element* expression_el;

	expression_el = generate_expression(expression, method_symbols, num_tabs);

	if(expression_el->desc == is_LITERAL)
		expression_el->value = -expression_el->value;
	else
	{
		switch(expression_el->desc)
		{
			case is_GLOBAL:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* @%s\n", register_index1, expression_el->name);
				break;
			case is_VAR:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* %%%s\n", register_index1, expression_el->name);
				break;
			case is_PARAMETER:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load i32* %%reg.%d\n", register_index1, expression_el->register_index);
				break;	
			case is_RESULT:
				register_index1 = expression_el->register_index;
				break;
		}
		
		register_index2 = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = sub nsw i32 0, %%reg.%d\n", register_index2, register_index1);
		expression_el->desc = is_RESULT;
		expression_el->type = is_INT;
		expression_el->register_index=register_index2;
	}

	return expression_el;
}

expression_element* generate_expression_length(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int current_register_index, register_index1;
	char varTypeStr[MAXSTR] = "";
	expression_element* expression_el;
	expression_element* expression_el_result;

	expression_el = generate_expression(expression, method_symbols, num_tabs);

	if(expression_el->type == is_STRINGARRAY)
	{
		register_index1 = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = load i32* %%reg.2\n", register_index1);
		current_register_index = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = sub i32 %%reg.%d, 1\n", current_register_index, register_index1);

		/*generate expression element to return, containing new array*/
		expression_el_result = (expression_element*) malloc(sizeof(expression_element));
		expression_el_result->desc = is_RESULT;
		expression_el_result->type = is_INT;
		expression_el_result->register_index=current_register_index;
	}
	else
	{
		/*check Id type*/
		getSize(expression_el->type, varTypeStr);

		/*load ID to register, if needed*/
		switch(expression_el->desc)
		{
			case is_GLOBAL:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load %s* @%s\n", register_index1, varTypeStr, expression_el->name);
				break;
			case is_VAR:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load %s* %%%s\n", register_index1, varTypeStr, expression_el->name);
				break;
			case is_PARAMETER:
				register_index1 = generateRegisterIndex();
				print_tabulation(num_tabs);
				printf("%%reg.%d = load %s* %%reg.%d\n", register_index1, varTypeStr, expression_el->register_index);
				break;	
			case is_RESULT:
				register_index1 = expression_el->register_index;
				break;
		}

		current_register_index = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = getelementptr inbounds %s %%reg.%d, i32 0, i32 0\n", current_register_index, varTypeStr, register_index1);
		register_index1 = generateRegisterIndex();
		print_tabulation(num_tabs);
		printf("%%reg.%d = load i32* %%reg.%d\n", register_index1, current_register_index);


		/*generate expression element to return, containing array length*/
		expression_el_result = (expression_element*) malloc(sizeof(expression_element));
		expression_el_result->desc = is_RESULT;
		expression_el_result->type = is_INT;
		expression_el_result->register_index=register_index1;
	}

	return expression_el_result;
}

expression_element* generate_expression_parseint(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	int current_register_index, register_aux, register_index1, register_index2;;
	char idType[MAXSTR]="";
	char indexType[MAXSTR]="";
	char varType[MAXSTR]="";
	char varTypeStr[MAXSTR] = "";
	expression_element* expression_el;
	expression_element* expression_ID; /*array ID*/
	expression_element* expression_index; /*array index*/

	expression_ID = getExpressionElementFromSymbolTable(expression->data.id, method_symbols->name);
	expression_index = generate_expression(expression->exp2, method_symbols, num_tabs);

	/*check index - load to register if needed*/
	switch(expression_index->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(indexType, "%d", expression_index->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(indexType, "%%reg.%d", expression_index->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_index->name);	
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_index->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_index->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_index->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexType, "%%reg.%d", current_register_index);
	}
	/*since the first arg on String[] is the name of the program, we add 1 to the index value*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = add i32 1, %s\n", current_register_index, indexType);
	sprintf(indexType, "%%reg.%d", current_register_index);		

	/*check ID type*/
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load i8*** %%reg.%d\n", current_register_index, expression_ID->register_index);
	sprintf(idType, "%%reg.%d", current_register_index);

	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds i8** %s, i32 %s\n", register_index1, idType, indexType);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load i8** %%reg.%d\n", register_index2, register_index1);
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = call i32 @atoi(i8* %%reg.%d) nounwind readonly\n", current_register_index, register_index2);

	/*generate expression element to return*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = is_INT;
	expression_el->register_index = current_register_index;

	/*free memory*/
	free(expression_ID);
	free(expression_index);

	return expression_el;
}

expression_element* generate_expression_load_array(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	char indexType[MAXSTR]="";
	char varType[MAXSTR]="";
	char varTypeStr[MAXSTR] = "";
	char* resultType;
	int id_register, current_register_index, register_index1, register_index2;
	expression_element* expression_el;
	expression_element* expression_ID; /*array ID*/
	expression_element* expression_index; /*array index*/

	expression_ID = generate_expression(expression->data.exp1, method_symbols, num_tabs);
	expression_index = generate_expression(expression->exp2, method_symbols, num_tabs);

	/*check index*/
	switch(expression_index->desc)
	{
		case (is_LITERAL):
			sprintf(varType, "");
			sprintf(indexType, "%d", expression_index->value);
			break;
		case (is_RESULT):
			sprintf(varType, "");
			sprintf(indexType, "%%reg.%d", expression_index->register_index);
			break;
		case (is_GLOBAL):
			sprintf(varType, "@%s", expression_index->name);
			break;
		case (is_VAR):
			sprintf(varType, "%%%s", expression_index->name);
			break;
		case (is_PARAMETER):
			sprintf(varType, "%%reg.%d",  expression_index->register_index);
			break;
	}
	if (strcmp(varType,"") != 0) /*load variable to register*/
	{
		current_register_index = generateRegisterIndex();
		getSize(expression_index->type, varTypeStr);
		print_tabulation(num_tabs);
		printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
		sprintf(indexType, "%%reg.%d", current_register_index);
	}

	/*check Id type*/
	getSize(expression_ID->type, varTypeStr);
	if (expression_ID->type == is_INTARRAY)
		resultType = "i32";
	else
		resultType = "i1";
	/*load ID to register, if needed*/
	switch(expression_ID->desc)
	{
		case is_GLOBAL:
			id_register = generateRegisterIndex();
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* @%s\n", id_register, varTypeStr, expression_ID->name);
			break;
		case is_VAR:
			id_register = generateRegisterIndex();
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* %%%s\n", id_register, varTypeStr, expression_ID->name);
			break;
		case is_PARAMETER:
			id_register = generateRegisterIndex();
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* %%reg.%d\n", id_register, varTypeStr, expression_ID->register_index);
			break;	
		case is_RESULT:
			id_register = expression_el->register_index;
			break;
	}

	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %s %%reg.%d, i32 0, i32 1\n", current_register_index, varTypeStr, id_register);
	register_index1 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %s** %%reg.%d\n", register_index1, resultType, current_register_index);
	register_index2 = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = getelementptr inbounds %s* %%reg.%d, i32 %s\n", register_index2, resultType, register_index1, indexType);
	current_register_index = generateRegisterIndex();
	print_tabulation(num_tabs);
	printf("%%reg.%d = load %s* %%reg.%d\n", current_register_index, resultType, register_index2);

	/*generate expression element to return, containing array loaded*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	if(expression_ID->type == is_INTARRAY)
		expression_el->type = is_INT;
	else
		expression_el->type = is_BOOL;
	expression_el->register_index = current_register_index;

	/*free memory*/
	free(expression_ID);
	free(expression_index);

	return expression_el;
}

expression_element* generate_expression_call(is_expression* expression, class_element* method_symbols, int num_tabs)
{
	class_element* class_el;
	char varTypeStr[MAXSTR] = "";
	is_expression_list* expression_list;
	method_element* me_aux;
	expression_element* expression_el;
	expression_element* expression_real_parameter; 
	int current_register_index, return_address;
	char varType[MAXSTR]="";
	char auxStr[MAXSTR]="";
	char callText[MAXTEXT]="";
	char callText2[MAXTEXT]="";
	
	/*find class element for method being called*/
	class_el = search_class_el(expression->data.id); /*method being called*/
	getSize(class_el->type, varTypeStr); /*get return type*/
	sprintf(callText, "call %s @%s(", varTypeStr, class_el->name);

	/*get method elements*/
	me_aux=class_el->method_element_list;

	for (expression_list=expression->exp_list; expression_list != NULL; expression_list=expression_list->next) 
	{	
		expression_real_parameter = generate_expression(expression_list->expr, method_symbols, num_tabs);
		
		/*load any needed variable to register*/
		switch(expression_real_parameter->desc)
		{
			case (is_LITERAL):
				sprintf(varType, "");
				sprintf(auxStr, "%d", expression_real_parameter->value);
				break;
			case (is_RESULT):
				sprintf(varType, "");
				sprintf(auxStr, "%%reg.%d", expression_real_parameter->register_index);
				break;
			case (is_GLOBAL):
				sprintf(varType, "@%s", expression_real_parameter->name);
				break;
			case (is_VAR):
				sprintf(varType, "%%%s", expression_real_parameter->name);
				break;
			case (is_PARAMETER):
				sprintf(varType, "%%reg.%d",  expression_real_parameter->register_index);
				break;
		}
		if (strcmp(varType,"") != 0) /*load variable to register*/
		{
			current_register_index = generateRegisterIndex();
			getSize(expression_real_parameter->type, varTypeStr);
			print_tabulation(num_tabs);
			printf("%%reg.%d = load %s* %s\n", current_register_index, varTypeStr, varType);
			sprintf(auxStr, "%%reg.%d", current_register_index);
		}

		getSize(me_aux->type, varTypeStr);
		strcat(callText, varTypeStr);
		strcat(callText, " ");
		strcat(callText, auxStr);

		if (expression_list->next != NULL)
		{
			strcat(callText, ", ");
			me_aux=me_aux->next;
		}

		free(expression_real_parameter);
	}

	strcat(callText, ")\n");

	if (class_el->type == is_VOID)
	{
		return_address = -1;
		print_tabulation(num_tabs);
		printf("%s", callText);
	}
	else
	{
		return_address = generateRegisterIndex();
		sprintf(callText2, "%%reg.%d = ", return_address);
		strcat(callText2, callText);
		print_tabulation(num_tabs);
		printf("%s", callText2);
	}

	/*generate expression element to return, containing array loaded*/
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	expression_el->desc = is_RESULT;
	expression_el->type = class_el->type;
	expression_el->register_index = return_address;

	return expression_el;
}

/*generate standard declarations*/
void generate_declarations()
{
	printf("\n");
	printf("@.str = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\"\n");
	printf("@.str1 = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"\n");
	printf("@.str2 = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"\n");

	printf("\ndeclare i32 @printf(i8*, ...)\n");
	printf("\ndeclare noalias i8* @calloc(i32, i32) nounwind\n");
	printf("\ndeclare noalias i8* @malloc(i32) nounwind\n");
	printf("\ndeclare i32 @atoi(i8*) nounwind readonly\n");
}

/*return size as a string for each type of variable*/
void getSize(is_type type, char* returnStr)
{
	switch (type)
	{
		case is_INT:
		case is_INTLIT:
			strcpy(returnStr,"i32");
			break;
		
		case is_INTARRAY:
			strcpy(returnStr,"%IntArray*");
			break;
			
		case is_BOOL:
		case is_BOOLLIT:
			strcpy(returnStr,"i1");
			break;
		
		case is_BOOLARRAY:
			strcpy(returnStr,"%BooleanArray*");
			break;
			
		case is_STRINGARRAY:
			strcpy(returnStr,"i8**");
			break;
			
		case is_VOID:
			strcpy(returnStr,"void");
			break;
	}
}

/*returns the next available register index*/
int generateRegisterIndex()
{
	registerIndex++;
	return registerIndex;
}

/*returns the next available structure index*/
int generateStructureIndex()
{
	structureIndex++;
	return structureIndex;
}

/*returns present structure index*/
int getStructureIndex()
{
	return structureIndex;
}

void print_tabulation(int num)
{
	int i;
	for (i = 0; i < num; i++)
		printf("\t");
}
