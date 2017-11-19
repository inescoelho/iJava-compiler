#include "structures.h"

is_start_program* addProgram(char* id, is_field_method_decl* fieldMethodDecl);
is_field_method_decl* addFieldMethodDecl(data_description description, void* varORmethodDecl, is_field_method_decl* next);
is_var_decl* addVarDecl(is_type type, char* var_name, is_var_decl* next);
is_var_decl* addVartoVarDecl(char* var_name, is_var_decl* parent);
is_var_decl_list* addVarDeclList(is_var_decl* var_decl, is_var_decl_list* next);
is_method_decl* addMethodDecl(is_type type, char* method_name, is_method_params* formal_params, 
is_method_body* method_body);
is_method_body* addMethodBody(is_var_decl_list* var_decl_list, is_statement_list* statement_list);
is_method_params* addMethodParams(is_type type, char* param_name, is_method_params* next);
is_statement_list* addStatementList (is_statement* statement, is_statement_list* next);
is_statement* addStatement (data_description description, is_expression* exp1, is_statement* statement1, 
is_statement* statement2, char* id, is_expression* exp2, is_statement_list* list);
is_expression* addExpression(disc_expression type, int number, void* arg, is_expression* exp2, is_oper oper);
is_expression* addExpressionArgs(disc_expression type, char* id, is_expression_list* exp_list);
is_expression_list* addExpressionList(is_expression* expr, is_expression_list* next);
void clearTree (is_start_program* root);
