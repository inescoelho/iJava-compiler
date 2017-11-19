#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "structures.h"

int create_symbol_table(is_start_program* root);
void getVarType(is_type type, char* returnStr);
void printMethodElement(method_element* m_element);
void show_table(char* programName);
method_element* search_method_el(char* methodName, char* varName);
class_element* search_class_el(char *str);
class_element* insert_class_el(symbol_description desc, char* name, is_type type);
int insertMethodDecl(is_method_decl* method_decl);
method_element* insert_method_el(symbol_description desc, char* var_name, is_type type, class_element* targetMethod);
int insertMethodElementFromDecl(is_method_decl* method_decl, class_element* method);
expression_element* getExpressionElementFromSymbolTable(char* var_name, char* method_name);

#endif
