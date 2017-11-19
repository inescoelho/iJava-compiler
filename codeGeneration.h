#include "structures.h"
#include "symbol_table.h"

void generateCode(is_start_program* root, class_element* symtab);
void generate_global_variable(class_element* var_decl);
void generate_method(is_method_decl* method_decl, class_element* method_symbols);
void generate_method_params (method_element* method_el);
void allocate_method_params (is_type return_type, method_element* method_el);
void generate_return_label(class_element* method_symbols);

void generate_statement(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_list(is_statement_list* statement_list, class_element* method_symbols, int num_tabs);
void generate_statement_ifelse(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_while(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_print(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_assign(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_assign_array(is_statement* statement, class_element* method_symbols, int num_tabs);
void generate_statement_return(is_statement* statement, class_element* method_symbols, int num_tabs);

expression_element* generate_expression(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_infix_and_or_expression(is_expression* expr, class_element* method_symbols, int num_tabs);
expression_element* generate_infix_expression(is_expression* expr, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_int_array_initialization(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_bool_array_initialization(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_not(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_plus(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_minus(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_length(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_load_array(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_parseint(is_expression* expression, class_element* method_symbols, int num_tabs);
expression_element* generate_expression_call(is_expression* expression, class_element* method_symbols, int num_tabs);

void generate_declarations();
void getSize(is_type type, char* returnStr);
int generateRegisterIndex();
int generateStructureIndex();
void print_tabulation(int num);
