#include "structures.h"

void print_tabs(int num);
void print_tree(is_start_program* root);
void print_field_declaration(is_var_decl* aux, int num_tabs);
void print_method_declaration(is_method_decl* aux, int num_tabs);
void print_Method_Params(is_method_params* method_params, int num_tabs);
void print_Method_Body(is_method_body* method_body, int num_tabs);
void print_Var_Decl_List(is_var_decl_list* var_decl_list, int num_tabs);
void print_Var_Decl(is_var_decl* var_decl, int num_tabs);
void print_Statement_List(is_statement_list* statement_list, int num_tabs);
void print_Expression(is_expression* expression, int num_tabs);
void print_type_infix_expression(is_oper oper, int num_tabs);
void print_Expression_Args(is_expression_list* exp_list, int num_tabs);
void print_statement_list(is_statement_list* statement_list, int num_tabs);
void print_statement(is_statement* statement, int num_tabs);
void print_statement_compound(is_statement* statement, int num_tabs);
