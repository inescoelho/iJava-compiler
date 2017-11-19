#include "symbol_table.h"
#include "structures.h"

int check_program(is_start_program* root);
int check_statement_list(is_statement_list* statement_list, char* method_name);
int check_statement(is_statement* statement, char* method_name);
is_type check_expression(is_expression* expression, char* method_name);
is_type check_infix_expression(is_expression* exp1, is_oper oper, is_expression* exp2, char* method_name);
void getOperTypeToStr(is_oper oper, char* returnStr);
void getVarTypeToStr(is_type type, char* returnStr);
is_type checkMethodCall(is_expression* expression, char* method_name);
is_type checkNumber(char* number);

