#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbol_table.h"
#include "structures.h"
#include "semantics.h"

#define STRMAX 20

int check_program(is_start_program* root)
{
	int error = 0;
	is_field_method_decl* field_method_decl;
	is_statement_list* aux;

	if (root==NULL)
		return;

	for (field_method_decl = root->field_method_decl; field_method_decl!=NULL; field_method_decl= field_method_decl->next)
	{
		if (field_method_decl->desc == desc_METHODDECL)
		{
			assert(field_method_decl->data.method_decl!=NULL);
			if (field_method_decl->data.method_decl->method_body != NULL)
			{
				for (aux = field_method_decl->data.method_decl->method_body->statement_list; aux!= NULL; aux = aux->next)
				{
					error = check_statement(aux->statement, field_method_decl->data.method_decl->method_name);
					if (error == 1)
						return error;
				}
			}
		}
	}

	return error;
}

int check_statement_list(is_statement_list* statement_list, char* method_name)
{
	int error = 0;

	if (statement_list==NULL)
	{
		return 0;
	}
	
	while(statement_list != NULL)
	{
		error = check_statement(statement_list->statement, method_name);
		statement_list = statement_list->next;

		if (error == 1)
			break;
	}

	return error;
}


int check_statement(is_statement* statement, char* method_name)
{
	int error = 0;
	is_type type, type2;
	char typeStr[STRMAX];
	char typeStr2[STRMAX];
	class_element* class_el;
	method_element* method_el;

	if (statement == NULL)
		return 0;

	switch(statement->desc)
	{
		case(desc_COMPSTAT):
			error = check_statement_list(statement->statement_list, method_name);
			
			break;
			
		case(desc_IFELSE):
			type = check_expression(statement->exp1, method_name);	/* check expression condition */
			if (type==is_INCOMPATIBLE)
			{
				error=1;
			}
			else if (type!=is_BOOL)
			{
				getVarTypeToStr(type, typeStr);
				printf("Incompatible type in if statement (got %s, required boolean)\n", typeStr);
				error=1;
			}
			if(!error) error = check_statement(statement->statement1, method_name);	/* check 'if' branch */
			if(!error) error = check_statement(statement->statement2, method_name); /* check 'else' branch */
			break;
			
		case(desc_WHILE):
			type = check_expression(statement->exp1, method_name);
			
			if (type==is_INCOMPATIBLE)
			{
				error=1;
			}
			else if (type!=is_BOOL)
			{
				getVarTypeToStr(type, typeStr);
				printf("Incompatible type in while statement (got %s, required boolean)\n", typeStr);
				error=1;
			}
			if(!error) error = check_statement(statement->statement1, method_name);
			break;
			
		case(desc_PRINT):
			type = check_expression(statement->exp1, method_name);
			if (type==is_INCOMPATIBLE)
				error=1;
			else
			{
				if (type!=is_INT && type!=is_BOOL)
				{
					getVarTypeToStr(type, typeStr);
					printf("Incompatible type in System.out.println statement (got %s, required boolean or int)\n", typeStr);
					error=1;
				}
			}	
				
			break;
			
		case(desc_ASSIGN):
			type = check_expression(statement->exp1, method_name); /* "[statement->id] = [statement->exp1];" */
			if (type==is_INCOMPATIBLE)
			{
				error=1;
			}
			else
			{
				/*check if assigned expression is from the same type than id var*/
				type2 = getVarTypeFromSymbolTable(statement->id, method_name);
				if (type2==is_INCOMPATIBLE) /* symbol was not found */
				{
					printf("Cannot find symbol %s\n", statement->id);
					error = 1;
				}
				else if (type!=type2)
				{
					getVarTypeToStr(type, typeStr);
					getVarTypeToStr(type2, typeStr2);
					printf("Incompatible type in assignment to %s (got %s, required %s)\n", statement->id, typeStr, typeStr2);
					error=1;
				}
			}
			break;
			
		case(desc_ASSIGNARRAY):
			
			type = getVarTypeFromSymbolTable(statement->id, method_name);
			if (type == is_INCOMPATIBLE)
			{
				printf("Cannot find symbol %s\n", statement->id);
				error=1;
			}
			else
			{
				if (type == is_BOOLARRAY || type == is_INTARRAY) /* id valid */
				{
					type2 = check_expression(statement->exp2, method_name); /*check index expression*/
					if (type2 != is_INCOMPATIBLE)
					{
						if (type2!=is_INT) /* index expression invalid */
						{
							getVarTypeToStr(type, typeStr);
							getVarTypeToStr(type2, typeStr2);
							printf("Operator [ cannot be applied to types %s, %s\n", typeStr, typeStr2);
							error=1;
						}
						else /* index expression valid => check assign expression */
						{
							type2 = check_expression(statement->exp1, method_name); /* check expression to assign */
							if (type2==is_INCOMPATIBLE)
							{
								error=1;
							}
							else
							{
								if (  type==is_BOOLARRAY && type2!=is_BOOL  )
								{
									getVarTypeToStr(type, typeStr);
									getVarTypeToStr(type2, typeStr2);
									printf("Incompatible type in assignment to %s[] (got %s, required %s)\n", statement->id, typeStr2, "boolean");
									error=1;
								}
								else if ( type==is_INTARRAY && type2!=is_INT )
								{
									getVarTypeToStr(type, typeStr);
									getVarTypeToStr(type2, typeStr2);
									printf("Incompatible type in assignment to %s[] (got %s, required %s)\n", statement->id, typeStr2, "int");
									error=1;
								}
								else
									error=0;
							}
						}
					}
					else
					{
						error = 1;
					}
					
				}
				else
				{
					type2 = check_expression(statement->exp2, method_name); /*check index expression*/
					if (type2 != is_INCOMPATIBLE)
					{
						getVarTypeToStr(type, typeStr);
						getVarTypeToStr(type2, typeStr2);
						printf("Operator [ cannot be applied to types %s, %s\n", typeStr, typeStr2);
						error=1;
					}
					else
					{
						error=1;
					}
				}
			}
			break;
			
		case(desc_RETURN_E):
			type = check_expression(statement->exp1, method_name);
			class_el = search_class_el(method_name);
			assert(class_el!=NULL);
			if (type!=class_el->type)
			{
				getVarTypeToStr(type, typeStr);
				getVarTypeToStr(class_el->type, typeStr2);
				printf("Incompatible type in return statement (got %s, required %s)\n", typeStr, typeStr2);
				error=1;
			}
			break;
			
		case(desc_RETURN):

			class_el = search_class_el(method_name); /* get function */
			assert(class_el!=NULL);
			type=class_el->type; /* get return type */
			if (type!=is_VOID)
			{
				getVarTypeToStr(type, typeStr);
				printf("Incompatible type in return statement (got %s, required %s)\n", "void", typeStr);
				error=1;
			}
			else
			{
				error=0;
			}
			
			break;
	}

	return error;
}

/* checks if expression is semantically valid 
 * returns type is valid, is_INCOMPATIBLE if not
 * */
is_type check_expression(is_expression* expression, char* method_name)
{
	is_type result, resultAux;
	char typeStr[STRMAX];
	char type2Str[STRMAX];
	class_element* class_el; /* used while checking method args and return type */
	is_expression_list* expression_list;
	
	switch(expression->disc_type)
	{
		case (d_id):
			result = getVarTypeFromSymbolTable(expression->data.id, method_name);
			if (result == is_INCOMPATIBLE)
				printf("Cannot find symbol %s\n", expression->data.id);
			break;
		case (d_number):
			result = checkNumber(expression->data.number);
			break;
		case (d_bool):
			result = is_BOOL;
			break;
		case (d_unary):
			result = check_expression(expression->data.exp1, method_name);
			break;
		case (d_infix):
			result = check_infix_expression(expression->data.exp1, expression->oper, expression->exp2, method_name);
			break;
		case (d_NEW_INT_OS_E_CS): /* Indexing expression must be is_INT */
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INT) /* indexing expression type not valid */
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator new int cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else /* indexing expression is valid (INT), set new return type: IntArray */
				{
					result = is_INTARRAY;
				}
			}
			break;
			
		case (d_NEW_BOOL_OS_E_CS):
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INT) /* indexing expression type not valid */
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator new boolean cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else /* indexing expression is valid (INT), set new return type: BoolArray */
				{
					result = is_BOOLARRAY;
				}
			}
			break;
			
		case (d_NOT_E):
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_BOOL)
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator ! cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else
				{
					result = is_BOOL;
				}
			}
			break;
		case (d_PLUS_E):
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INT)
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator + cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else
				{
					result = is_INT;
				}
			}
			break;
			
		case (d_MINUS_E):
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INT)
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator - cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else
				{
					result = is_INT;
				}
			}
			break;
			
		case (d_E_DOTLENGTH):
			result = check_expression(expression->data.exp1, method_name);
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INTARRAY && result!=is_BOOLARRAY && result!=is_STRINGARRAY)
				{
					getVarTypeToStr(result, typeStr);
					printf("Operator .length cannot be applied to type %s\n", typeStr);
					result = is_INCOMPATIBLE;
				}
				else /* expression is valid (BoolArray or IntArray or StringArray), set return type to is_INT */
				{
					result = is_INT;
				}
			}
			break;
			
		case (d_E2_OS_E_CS):
			result = check_expression(expression->data.exp1, method_name); /* check var id (or expression) - must be IntArray or BoolArray*/
			if (result!=is_INCOMPATIBLE)
			{
				if (result!=is_INTARRAY && result!=is_BOOLARRAY)
				{
					resultAux = check_expression(expression->exp2, method_name); /* check indexing type */
					if (resultAux!=is_INCOMPATIBLE) /* if indexing type is incompatible, error message was already printed, skip */
					{	
						getVarTypeToStr(result, typeStr);
						getVarTypeToStr(resultAux, type2Str);
						printf("Operator [ cannot be applied to types %s, %s\n", typeStr, type2Str);
					}
					result = is_INCOMPATIBLE;
				}
				else /* var expression is valid (BoolArray or IntArray) -> check indexing type */
				{
					resultAux = check_expression(expression->exp2, method_name); /* check indexing type */
					
					if (resultAux!=is_INCOMPATIBLE)
					{
						if (resultAux!=is_INT)
						{
							getVarTypeToStr(result, typeStr);
							getVarTypeToStr(resultAux, type2Str);
							printf("Operator [ cannot be applied to types %s, %s\n", typeStr, type2Str);
							result = is_INCOMPATIBLE;
						}
						else /* indexing expression is valid (INT), set new return type to Int or Bool */
						{
							if (result == is_BOOLARRAY)
								result = is_BOOL;
							else if (result == is_INTARRAY)
								result = is_INT;
						}
					}
					else
					{
						result = is_INCOMPATIBLE;
					}
				}
			}

			break;
			
		case (d_PARSEINT):	/*Integer.parseInt só aplicavel a indexações de string[] */
			/* Formato: Integer.parseInt( "expression->data.id" [ "expression->exp2" ] );  */
			result = getVarTypeFromSymbolTable(expression->data.id, method_name); 
			if (result == is_INCOMPATIBLE)
				printf("Cannot find symbol %s\n", expression->data.id);
			else 
			{
				if(result != is_STRINGARRAY) /*argument must be a string[]*/
				{
					resultAux = check_expression(expression->exp2, method_name); /* check indexing type */
					if (resultAux == is_INCOMPATIBLE)
					{
						result = is_INCOMPATIBLE; /* indexing expression was not valid and error was already printed in previous call to check_expression() */
						break;
					}
					
					getVarTypeToStr(result, typeStr);
					getVarTypeToStr(resultAux, type2Str);
					printf("Operator Integer.parseInt cannot be applied to types %s, %s\n", typeStr, type2Str);
					result = is_INCOMPATIBLE;
					break;
				}
				
				/* var id is valid (String[]) => check indexing type */
				resultAux = check_expression(expression->exp2, method_name);
				if (resultAux!=is_INCOMPATIBLE)
				{
					if (resultAux!=is_INT) /* indexing expression was valid BUT not int => print error */
					{
						getVarTypeToStr(result, typeStr);
						getVarTypeToStr(resultAux, type2Str);
						printf("Operator Integer.parseInt cannot be applied to types %s, %s\n", typeStr, type2Str);
						result = is_INCOMPATIBLE;
						break;
					}
					else /* var id is String[] and index is int => valid */
					{
						result = is_INT;
					}
				}
				else /* indexing expression was not valid and error was already printed in previous call to check_expression() */
				{
					result = is_INCOMPATIBLE;
				}
			}

			break;
			
		case (d_ID_OC_ARGS_CC):
			result = checkMethodCall(expression, method_name);
			break;
	}
	
	return result;
}

is_type checkMethodCall(is_expression* expression, char* method_name)
{
	is_type result;
	char typeStr[STRMAX];
	char type2Str[STRMAX];
	class_element* class_el;
	method_element* method_el;
	is_expression_list* expression_list;
	int countArgs;

	class_el = search_class_el(expression->data.id); /*method being called*/
	expression_list = expression->exp_list; /*list containing real parameters from method call*/
	
	if (class_el==NULL || class_el->desc!=is_METHOD) /* method does not exist */
	{
		printf("Cannot find symbol %s\n", expression->data.id);
		result = is_INCOMPATIBLE;
	}
	else /* function found */
	{
		method_el = class_el->method_element_list; /*parameters and variables from method*/

		if (method_el==NULL || method_el->desc!=is_PARAMETER) /*method has no parameteres*/
		{
			if ( expression_list==NULL ) /* function has no args && function was called with no args ==> correct call */
				result = class_el->type;
			else /*incorrect call*/
			{
				result = check_expression(expression_list->expr, method_name);
				getVarTypeToStr(result, typeStr);
				printf("Incompatible type of argument %d in call to method %s (got %s, required %s)\n", 0, expression->data.id, typeStr, "void");
				result = is_INCOMPATIBLE;
			}
		}
		else /*method has parameteres*/
		{
			if (expression_list==NULL ) /*method has parameteres but it was called without them*/
			{
				getVarTypeToStr(method_el->type, typeStr);
				printf("Incompatible type of argument %d in call to method %s (got %s, required %s)\n", 0, expression->data.id, "void", typeStr);
				result = is_INCOMPATIBLE;
			}
			else
			{
				countArgs = 0; /*count the number of correct arguments on function call*/
				while(method_el!=NULL && method_el->desc==is_PARAMETER)
				{
					if(expression_list == NULL)
					{
						getVarTypeToStr(method_el->type, typeStr); /*get formal parameter type*/
						printf("Incompatible type of argument %d in call to method %s (got %s, required %s)\n", countArgs, expression->data.id, "void", typeStr);
						result = is_INCOMPATIBLE;
						break;
					}

					result = check_expression(expression_list->expr, method_name);
					if (result != is_INCOMPATIBLE)
					{
						getVarTypeToStr(result, type2Str); /*get real parameter type*/
						if (method_el->type!=result) /*check if both parameters are from the same type*/
						{
							getVarTypeToStr(method_el->type, typeStr); /*get formal parameter type*/
							printf("Incompatible type of argument %d in call to method %s (got %s, required %s)\n", countArgs, expression->data.id, type2Str, typeStr);
							result = is_INCOMPATIBLE;
							break;
						}
						else
						{
							method_el=method_el->next;
							expression_list=expression_list->next;
							countArgs ++;
						}
					}					
					else
					{
						result = is_INCOMPATIBLE;
						break;
					}
				}
				

				if (result != is_INCOMPATIBLE) 
				{
					if (expression_list!=NULL) /*function call was made using more arguments than the parameters needed*/
					{
						result = check_expression(expression_list->expr, method_name);
						getVarTypeToStr(result, typeStr);
						printf("Incompatible type of argument %d in call to method %s (got %s, required %s)\n", countArgs, expression->data.id, typeStr, "void");
						result = is_INCOMPATIBLE;
					}
					else /*the function call was done correctly*/
					{
						result = class_el->type;
					}
				}
			}
		}
	}
	return result;
}

is_type check_infix_expression(is_expression* exp1, is_oper oper, is_expression* exp2, char* method_name)
{
	is_type type1, type2;
	char type1Str[STRMAX];
	char type2Str[STRMAX];
	char operStr[STRMAX];
	type1 = check_expression(exp1, method_name);
	type2 = check_expression(exp2, method_name);
	
	if ( type1==is_INCOMPATIBLE || type2==is_INCOMPATIBLE )
		return is_INCOMPATIBLE;
	
	switch(oper)
	{
		case (is_AND):
		case (is_OR):
			if ( (type1==is_BOOL || type1==is_BOOLLIT) && (type2==is_BOOL || type2==is_BOOLLIT) )
				return is_BOOL;
			else
			{
				getVarTypeToStr(type1, type1Str);
				getVarTypeToStr(type2, type2Str);
				getOperTypeToStr(oper, operStr);
				printf("Operator %s cannot be applied to types %s, %s\n", operStr, type1Str, type2Str);
			}
			break;
		
		case (is_EQUAL):
		case (is_NOTEQUAL):
			if (type1==type2)
				return is_BOOL;
			else if ( (type1==is_BOOL || type1==is_BOOLLIT) 
				&& (type2==is_BOOL|| type2==is_BOOLLIT) )
				return is_BOOL;
			else if ( (type1==is_INT || type1==is_INTLIT) 
				&& (type2==is_INT|| type2==is_INTLIT) )
				return is_BOOL;
			else
			{
				getVarTypeToStr(type1, type1Str);
				getVarTypeToStr(type2, type2Str);
				getOperTypeToStr(oper, operStr);
				printf("Operator %s cannot be applied to types %s, %s\n", operStr, type1Str, type2Str);
			}
			break;
			
		case (is_LT):
		case (is_GT):
		case (is_LTE):
		case (is_GTE):
			if ( (type1==is_INT || type1==is_INTLIT) 
				&& (type2==is_INT|| type2==is_INTLIT) )
				return is_BOOL;
			else
			{
				getVarTypeToStr(type1, type1Str);
				getVarTypeToStr(type2, type2Str);
				getOperTypeToStr(oper, operStr);
				printf("Operator %s cannot be applied to types %s, %s\n", operStr, type1Str, type2Str);
			}
			break;

		case (is_ADD):
		case (is_SUB):
		case (is_MULTIPLY):
		case (is_DIVIDE):
		case (is_REM):
			if ( (type1==is_INT || type1==is_INTLIT) 
				&& (type2==is_INT|| type2==is_INTLIT) )
				return is_INT;
			else
			{
				getVarTypeToStr(type1, type1Str);
				getVarTypeToStr(type2, type2Str);
				getOperTypeToStr(oper, operStr);
				printf("Operator %s cannot be applied to types %s, %s\n", operStr, type1Str, type2Str);
			}
			break;
	}
	return is_INCOMPATIBLE;
}

void getOperTypeToStr(is_oper oper, char* returnStr)
{
	switch(oper)
	{
		case (is_AND):
			strcpy(returnStr,"&&");
			break;
		case (is_OR):
			strcpy(returnStr,"||");
			break;
		case (is_EQUAL):
			strcpy(returnStr,"==");
			break;
		case (is_NOTEQUAL):
			strcpy(returnStr,"!=");
			break;
		case (is_LT):
			strcpy(returnStr,"<");
			break;
		case (is_GT):
			strcpy(returnStr,">");
			break;
		case (is_LTE):
			strcpy(returnStr,"<=");
			break;
		case (is_GTE):
			strcpy(returnStr,">=");
			break;
		case (is_ADD):
			strcpy(returnStr,"+");
			break;
		case (is_SUB):
			strcpy(returnStr,"-");
			break;
		case (is_MULTIPLY):
			strcpy(returnStr,"*");
			break;
		case (is_DIVIDE):
			strcpy(returnStr,"/");
			break;
		case (is_REM):
			strcpy(returnStr,"%");
			break;
	}
}

void getVarTypeToStr(is_type type, char* returnStr)
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
			
		case is_INTLIT:
			strcpy(returnStr,"int");
			break;
			
		case is_BOOLLIT:
			strcpy(returnStr,"boolean");
			break;
		
		case is_INCOMPATIBLE: /* this should never be needed! */
			strcpy(returnStr,"<INCOMPATIBLE>");
			break;
	}
}

is_type checkNumber(char* number)
{
	char* number_aux;

	if (number[0] == '0')
	{
		if (number[1] != '\0' && number[1] != 'x' )
		{
			number_aux = number+2;
			while(*number_aux != '\0')
			{
				if (*number_aux>'7')
				{
					printf("Invalid literal %s\n", number);
					return is_INCOMPATIBLE;
				}
				number_aux+=1;
			}
		}
	}
	return is_INT;
}








