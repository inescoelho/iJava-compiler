#ifndef _STRUCTURES_
#define _STRUCTURES_

typedef struct _a0 is_start_program;
typedef struct _a1 is_field_method_decl;
typedef struct _a2 is_var_decl;
typedef struct _a3 is_method_decl;
typedef struct _a4 is_method_params;
typedef struct _a5 is_method_body;
typedef struct _a6 is_expression_list;
typedef struct _a7 is_expression;
typedef struct _a9 is_statement;
typedef struct _a10 is_statement_list;
typedef struct _a11 is_var_decl_list;

typedef struct _t1 class_element;
typedef struct _t2 method_element;

typedef struct _p1 expression_element;

/*  ENUMS  */
typedef enum {
	is_AND, is_OR, is_LT, is_GT, is_EQUAL, is_NOTEQUAL, is_LTE, is_GTE,
	is_ADD, is_SUB, is_MULTIPLY, is_DIVIDE, is_REM, is_NOT, is_PLUS, is_MINUS,
	is_LENGTH, is_LOADARRAY, is_CALL, is_NEWINT, is_NEWBOOL, is_PARSEARGS, is_NULL0
} is_oper;

typedef enum {
	desc_METHODDECL, desc_VARDECL,			/* is_field_method_decl */
	desc_COMPSTAT, desc_IFELSE, desc_WHILE,		/* is_statement */
	desc_PRINT, desc_RETURN_E, desc_RETURN,					/*  ''  */
	desc_ASSIGN, desc_ASSIGNARRAY							/* 	''	*/
} data_description;

typedef enum {
	is_INT, is_BOOL, is_INTARRAY, is_BOOLARRAY, is_STRINGARRAY, is_VOID,
	is_ID, is_INTLIT, is_BOOLLIT, is_NULL, is_INCOMPATIBLE, is_STRING
} is_type;

typedef enum {
	d_infix, d_unary,  d_number, d_bool, d_id,
	d_E2_OS_E_CS, d_NEW_INT_OS_E_CS, d_NEW_BOOL_OS_E_CS, d_PLUS_E,
	d_MINUS_E, d_NOT_E, d_E_DOTLENGTH, d_PARSEINT, d_ID_OC_ARGS_CC
} disc_expression;

typedef enum {
	is_METHOD, is_VAR, is_PARAMETER, is_LITERAL, is_GLOBAL, is_RESULT
} symbol_description;


/*  STRUCTS  */
/* ********* */
struct _a7 {
	disc_expression disc_type;
	union{
		char* number;
		int bool;
		char* id;
		is_expression* exp1;
	} data;
	is_expression* exp2;
	is_expression_list* exp_list;
	is_oper oper;
}; /* is_expression */

struct _a6 {
	is_expression* expr;
	is_expression_list* next;
}; /* is_expression_list */

struct _a9{
	data_description desc;
	is_expression* exp1;
	is_statement* statement1;
	is_statement* statement2;
	is_statement_list* statement_list;
	char* id;
	is_expression* exp2;
}; /* is_statement */

struct _a10{
	is_statement* statement;
	is_statement_list* next;
}; /* is_statement_list */

struct _a5{
	
	is_var_decl_list* var_decl_list;
	is_statement_list* statement_list;
}; /* is_method_body */

struct _a4
{
	is_type type;
	char* param_name;
	is_method_params* next;
}; /* is_method_params */

struct _a3
{
	is_type type;
	char* method_name; /*ID*/
	is_method_params* formal_params;
	is_method_body* method_body;
}; /* is_method_decl */

struct _a2
{
	is_type type;
	char* var_name; /*ID*/
	is_var_decl* next;
}; /* is_var_decl */

struct _a11
{
	is_var_decl* var_decl;
	is_var_decl_list* next;
}; /* is_var_decl_list */

struct _a1
{
	data_description desc;
	union{
		is_var_decl* var_decl;
		is_method_decl* method_decl;
	}data;
	is_field_method_decl* next;
}; /* is_field_method_decl */

struct _a0
{
	char* program_id;
	is_field_method_decl* field_method_decl;
}; /* is_start_program */

/* ****SYMBOL TABLE**** */
struct _t1{
	symbol_description desc;/* method (is_METHOD) OR variable (is_VAR) */
	is_type type;			/* method return type OR variable type */
	char* name;				/* method/variable name */
	method_element* method_element_list;
	struct _t1 *next;
	int reg;				/* register number*/
}; /*class_element*/

struct _t2{
	symbol_description desc;/* variable (is_VAR) OR variable parameter (is_PARAMETER) */
	is_type type;			/* variable type */
	char* name;				/* variable name */
	struct _t2 *next;
	int reg;				/* register number*/
}; /*method_element*/

/* ****CODE GENERATION**** */
struct _p1{
	symbol_description desc;
	int register_index; /*value to print*/
	is_type type;
	int value;
	char* name;				/* variable name */
}; /*expression_element*/

#endif
