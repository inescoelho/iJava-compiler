#include "codeGeneration.h"
#include "symbol_table.h"
#include "structures.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAXSTR 15

extern class_element* symtab;

int create_symbol_table(is_start_program* root)
{
	int result=0;
	is_field_method_decl* field_method_decl;
	if (root==NULL)
		return;
	
	for (field_method_decl = root->field_method_decl; field_method_decl!=NULL; field_method_decl= field_method_decl->next)
	{
		switch (field_method_decl->desc)
		{
			case desc_VARDECL:
				result = insertVarDecl(field_method_decl->data.var_decl);
				break;
				
			case desc_METHODDECL:
				result = insertMethodDecl(field_method_decl->data.method_decl);
				break;

		}
		if (result==1) /* error: var or method name already exists */
			return 1;
	}
	
	return 0;
}

int insertMethodElementFromDecl(is_method_decl* method_decl, class_element* method)
{
	is_method_params* aux;
	is_var_decl_list* aux2;
	is_var_decl* aux3;
	is_type type;
	for (aux=method_decl->formal_params; aux!=NULL; aux=aux->next)
	{
		if (insert_method_el(is_PARAMETER, aux->param_name, aux->type, method)==NULL)
		{
			printf("Symbol %s already defined\n", aux->param_name);
			return 1;
		}
	}
	if (method_decl->method_body==NULL) /* method body is empty */
		return 0;
		
	for (aux2=method_decl->method_body->var_decl_list; aux2!=NULL; aux2=aux2->next)
	{
		if (aux2->var_decl!=NULL)
			type = aux2->var_decl->type;
		for (aux3=aux2->var_decl; aux3!=NULL; aux3=aux3->next)
		{
			if (insert_method_el(is_VAR, aux3->var_name, type, method) == NULL)
			{
				printf("Symbol %s already defined\n", aux3->var_name);
				return 1; /* method var already exists */
			}
		}
	}
	
	return 0;
}

int insertMethodDecl(is_method_decl* method_decl)
{
	int result;
	class_element* method=insert_class_el(is_METHOD, method_decl->method_name, method_decl->type);
	if (method==NULL)/* class var already exists */
	{
		printf("Symbol %s already defined\n", method_decl->method_name);
		result = 1; 
	}
	else
	{
		/* inserts local variables/return values/parameters in the respective method  */
		result = insertMethodElementFromDecl(method_decl, method);	/*args: method_decl from tree; method: pointer to symbol table */
	}
	return result;
}

int insertVarDecl(is_var_decl* var_decl)
{
	if(var_decl==NULL) return 0;
	is_type type = var_decl->type;
	
	for (; var_decl!=NULL; var_decl=var_decl->next)
	{
		if (insert_class_el(is_VAR, var_decl->var_name, type) == NULL)
		{
			printf("Symbol %s already defined\n", var_decl->var_name);
			return 1; /* class var already exists */
		}
	}
	return 0;	/* class var inserted successfully */
}

/* Inserts method element (local) */
method_element* insert_method_el(symbol_description desc, char* var_name, is_type type, class_element* targetMethod)
{
	method_element* newMethodSymbol;
	method_element* auxMethodEl;
	method_element* prevMethodEl;
	
	if (targetMethod->method_element_list == NULL) /* method has no symbols yet */
	{
		newMethodSymbol=(method_element*) malloc(sizeof(method_element));
		targetMethod->method_element_list = newMethodSymbol;
	}
	else /* method already has symbols */
	{
		for (auxMethodEl=targetMethod->method_element_list; auxMethodEl!=NULL; prevMethodEl=auxMethodEl, auxMethodEl=auxMethodEl->next)
		{
			if(strcmp(auxMethodEl->name, var_name)==0) /* symbol (arguments or local variable) with same name already exists */
				return NULL;
		}
		newMethodSymbol=(method_element*) malloc(sizeof(method_element));
		prevMethodEl->next = newMethodSymbol;
	}
	newMethodSymbol->desc=desc;
	newMethodSymbol->name=var_name;
	newMethodSymbol->type=type;
	newMethodSymbol->next=NULL;
	return newMethodSymbol;
}

class_element* insert_class_el(symbol_description desc, char* name, is_type type)
{
	class_element* newClassSymbol;
	class_element* aux;
	class_element* previous;
	
	if (symtab!=NULL) /* If symbol table already has symbols */
	{
		for(aux=symtab; aux!=NULL; previous=aux, aux=aux->next)
			if(strcmp(aux->name, name)==0)
				return NULL; /* element already exists */
		
		newClassSymbol=(class_element*) malloc(sizeof(class_element));
		previous->next = newClassSymbol;
	}
	else /* symbol table has no symbols */
	{
		newClassSymbol=(class_element*) malloc(sizeof(class_element));
		symtab = newClassSymbol;
	}
	newClassSymbol->desc=desc;
	newClassSymbol->name=name; /*VER Sera' necessario fazer strcpy(newClassSymbol->name, name); e por char[32] name; na struct? */
	newClassSymbol->type=type;
	newClassSymbol->next=NULL;
	newClassSymbol->method_element_list=NULL;
	return newClassSymbol;
}

is_type getVarTypeFromSymbolTable(char* var_name, char* method_name)
{
	is_type result=is_INCOMPATIBLE;
	class_element* aux;
	class_element* targetMethod;
	method_element* methodEl_aux;

	for(aux=symtab; aux!=NULL; aux=aux->next)
	{
		if(strcmp(aux->name, var_name)==0 && aux->desc == is_VAR)
			result = aux->type; /* found global variable - to return IF no local variable is found */
		if ( strcmp(aux->name, method_name)==0 && aux->desc == is_METHOD )
			targetMethod = aux;
	}
	
	assert(targetMethod!=NULL);
	
	for(methodEl_aux=targetMethod->method_element_list; methodEl_aux!=NULL; methodEl_aux=methodEl_aux->next)
			if(strcmp(methodEl_aux->name, var_name)==0)
				result = methodEl_aux->type; /* local variable found - return instead of global possibly found earlier */
				
	return result;
}

/* search class elements (var or method) by name
 * returns method/var pointer */
class_element* search_class_el(char *str)
{
	class_element* aux;

	for(aux=symtab; aux!=NULL; aux=aux->next)
		if(strcmp(aux->name, str)==0)
			return aux;

	return NULL;
}

/* search method elements (vars) by name */
method_element* search_method_el(char* methodName, char* varName)
{
	method_element* aux;
	class_element* pmethod = search_class_el(methodName);
	
	if (pmethod!=NULL) /* if method with name 'methodName' exists */
	{
		for(aux=pmethod->method_element_list; aux!=NULL; aux=aux->next)
			if(strcmp(aux->name, varName)==0)
				return aux;
	}
	return NULL;
}

void show_table(char* programName)
{
	class_element* c_element;
	method_element* m_element;
	char varTypeStr[MAXSTR] = "";
	
	printf("===== Class %s Symbol Table =====\n", programName);
	
	for( c_element = symtab; c_element!=NULL; c_element=c_element->next )
	{
		switch (c_element->desc)
		{
			case is_METHOD:
				printf("%s\tmethod\n", c_element->name);
				break;
				
			case is_VAR:
				getVarType(c_element->type, varTypeStr);
				printf("%s\t%s\n", c_element->name, varTypeStr);
				break;
		}
	}
	
	for( c_element = symtab; c_element!=NULL; c_element=c_element->next )
	{
		if (c_element->desc == is_METHOD)
		{
			printf("\n===== Method %s Symbol Table =====\n", c_element->name);
			getVarType(c_element->type, varTypeStr);
			printf("return\t%s\n", varTypeStr);
			printMethodElement(c_element->method_element_list);
		}
	}
}

void printMethodElement(method_element* m_element)
{
	char varTypeStr[MAXSTR] = "";
	for ( ; m_element!=NULL; m_element=m_element->next)
	{
		getVarType(m_element->type, varTypeStr);
		switch(m_element->desc)
		{
			case is_PARAMETER:
				printf("%s\t%s\tparam\n", m_element->name, varTypeStr);
				break;
					
			case is_VAR:
				printf("%s\t%s\n", m_element->name, varTypeStr);
				break;
		}
	}
}

void getVarType(is_type type, char* returnStr)
{
	switch (type)
	{
		case is_INT:
			strcpy(returnStr,"int");
			break;
		
		case is_INTARRAY:
			strcpy(returnStr,"int[]");
			break;
			
		case is_BOOL:
			strcpy(returnStr,"boolean");
			break;
		
		case is_BOOLARRAY:
			strcpy(returnStr,"boolean[]");
			break;
			
		case is_STRINGARRAY:
			strcpy(returnStr,"String[]");
			break;
			
		case is_VOID:
			strcpy(returnStr,"void");
			break;
	}
}

/* functions used in codeGeneration.c */
/* ********************************** */


/* returns variable from symbol table */
expression_element* getExpressionElementFromSymbolTable(char* var_name, char* method_name)
{
	expression_element* expression_el;
	expression_el = (expression_element*) malloc(sizeof(expression_element));
	
	class_element* aux;
	class_element* targetMethod;
	method_element* methodEl_aux;

	for(aux=symtab; aux!=NULL; aux=aux->next)
	{
		if(strcmp(aux->name, var_name)==0 && aux->desc == is_VAR)
		{
			/* found global variable - to return IF no local variable is found */
			expression_el->desc = is_GLOBAL;
			expression_el->register_index = aux->reg;
			expression_el->type = aux->type;
			expression_el->name = var_name;
		}
		if ( strcmp(aux->name, method_name)==0 && aux->desc == is_METHOD )
			targetMethod = aux;
	}
	
	assert(targetMethod!=NULL);
	
	for(methodEl_aux=targetMethod->method_element_list; methodEl_aux!=NULL; methodEl_aux=methodEl_aux->next)
		if(strcmp(methodEl_aux->name, var_name)==0)
		{
			/* local variable found - return instead of global possibly found earlier */
			expression_el->desc = methodEl_aux->desc;
			expression_el->register_index = methodEl_aux->reg;
			expression_el->type = methodEl_aux->type;
			expression_el->name = var_name;
		}
	return expression_el;
}






