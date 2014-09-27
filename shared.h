/* shared.h
 *
 * Contains function definitions of all shared subroutines
 */

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define K 1024
#define MAX_ERROR_SIZE 1 * K
#define MAX_NEW_CHAR_SIZE 255
#define MAX_USRDEF_HASHKEYS 8
#define MAX_SYMTAB_HASHKEYS 26
#define PROGRAMNAME "opc"

#define PRIMITIVE_TYPE_NAME_INTEGER "integer"
#define PRIMITIVE_TYPE_NAME_BOOLEAN "boolean"
#define PRIMITIVE_TYPE_NAME_POINTER "pointer"
#define PRIMITIVE_TYPE_NAME_UNKNOWN "unknown"

#define BOOLEAN_VALUE_TRUE "True"
#define BOOLEAN_VALUE_FALSE "False"


/* Macro that checks for a malloc error */
#define CHECK_MEM_ERROR(name) {if (name == NULL) { \
				 printf("Memory allocation error\n"); \
				 exit(1); \
				} \
                              }

/* Macro that iterates to the end of a linked list whose next
 * element pointer is named "next"
 */
#define GOTO_END_OF_LIST(name) {while (name->next != NULL) \
                                  name = name->next; \
                               }

/* ================================================================
 * Datatype declarations
 * ================================================================
 */

extern int error_flag;

struct args_t {
  /** Print the user defined data types and symbol table */
  int verbose;  
  /** Exit after performing semantic analysis */
  int exit_after_sem;  
  /** Print the Intermediate Representation */
  int printIr;
  /** Print the Lower-level Intermediate Representation */
  int printLir;
  /** Apply constant folding */
  int optimize_cf;
  /** Apply redundant expression elimination */
  int optimize_ree;
  /** Apply value numbering */
  int optimize_vn;
  /** Apply global redundancy elimination */
  int optimize_gre;
};


struct identifier_list_t;
struct identifier_list_t {
  char *id; 
  struct identifier_list_t *next;
};

struct unsigned_number_t;
struct range_t{
  struct unsigned_number_t *min;
  struct unsigned_number_t *max;
};

struct array_type_t{
  struct range_t *r;
  struct type_denoter_t *td;
};

#define TYPE_DENOTER_T_ARRAY_TYPE 1
#define TYPE_DENOTER_T_CLASS_TYPE 2
#define TYPE_DENOTER_T_IDENTIFIER 3
struct type_denoter_t{
  int type; /* 1 - array_type
	     * 2 - class_type
	     * 3 - base_type (integer, real, boolean)
	     */
  char *name;
  union {
    struct array_type_t *at;
    struct class_list_t *cl;
    char *id;
  }data;
};

/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the variable_declaration_list
 * ----------------------------------------------------------------
 */
struct variable_declaration_t {
  struct identifier_list_t *il;
  struct type_denoter_t *tden;
  int line_number;
};

struct variable_declaration_list_t;
struct variable_declaration_list_t {
  struct variable_declaration_t *vd;
  struct variable_declaration_list_t *next;  
};

/* ---------------------------------------------------------------- */


/* ----------------------------------------------------------------
 * Everything required for the func_declaration_list
 * ----------------------------------------------------------------
 */
struct formal_parameter_section_t{
  struct identifier_list_t *il;
  char *id;
  int is_var;
};

struct formal_parameter_section_list_t{
  struct formal_parameter_section_t *fps;
  struct formal_parameter_section_list_t *next;
};

struct function_heading_t{
  char *id;
  char *res; 
  struct formal_parameter_section_list_t *fpsl; /* == formal_parameter_list */
};

struct function_declaration_t {
  struct function_heading_t *fh;
  struct function_block_t *fb;
  int line_number;
};

struct func_declaration_list_t;
struct func_declaration_list_t{
  struct function_declaration_t *fd;
  struct func_declaration_list_t *next;
};
/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the statement_sequence
 * ----------------------------------------------------------------
 */
struct expression_t;
struct actual_parameter_t{
  struct expression_t *e1;
  struct expression_t *e2;
  struct expression_t *e3;
};

struct actual_parameter_list_t;
struct actual_parameter_list_t{
  struct actual_parameter_t *ap;
  struct actual_parameter_list_t *next;
};

struct function_designator_t{
  char *id;
  struct actual_parameter_list_t *apl;
};

/* This is a temporary data structure used to hold the value and type of
   an expression. It is included (inherited) by a bunch of other data
   structures */
struct expression_data_t{
  float val;
  char *type;
};

struct unsigned_number_t{
  int ui;
  struct expression_data_t *expr;
};

struct variable_access_t;
#define PRIMARY_T_VARIABLE_ACCESS 1
#define PRIMARY_T_UNSIGNED_CONSTANT 2
#define PRIMARY_T_FUNCTION_DESIGNATOR 3
#define PRIMARY_T_EXPRESSION 4
#define PRIMARY_T_PRIMARY 5
struct primary_t{
  int type; /* 1 - variable_access
	     * 2 - unsigned_constant
	     * 3 - function_designator
	     * 4 - expression
	     * 5 - primary
	     */
  union{
    struct variable_access_t *va; 
    struct unsigned_number_t *un; /* == unsigned_constant */
    struct function_designator_t *fd;
    struct expression_t *e;
    struct primary_data_t{
      int not;
      struct primary_t *next;
    }p;
  }data;
  struct expression_data_t *expr;
};

#define FACTOR_T_SIGNFACTOR 1
#define FACTOR_T_PRIMARY 2
struct factor_t{
  int type; /* 1 - sign/factor
	     * 2 - primary
	     */
  union {
    struct factor_data_t{
      int *sign;
      struct factor_t *next;
    }f;
    struct primary_t *p;
  }data;
  struct expression_data_t *expr;
};

struct term_t;
struct term_t{
  struct factor_t *f;
  int mulop;
  struct expression_data_t *expr;
  struct term_t *next;
};

struct simple_expression_t;
struct simple_expression_t{
  struct term_t *t;
  int addop;
  struct expression_data_t *expr;
  struct simple_expression_t *next;
};

struct expression_t{
  struct simple_expression_t *se1;
  int relop;
  struct simple_expression_t *se2;
  struct expression_data_t *expr;
};

struct index_expression_list_t;
struct index_expression_list_t{
  struct expression_t *e;
  struct index_expression_list_t *next;
  struct expression_data_t *expr;
};

struct indexed_variable_t{
  struct variable_access_t *va;
  struct index_expression_list_t *iel;
  struct expression_data_t *expr;
};

struct attribute_designator_t{
  struct variable_access_t *va;
  char *id;
};

struct method_designator_t{
  struct variable_access_t *va;
  struct function_designator_t *fd;
};

#define VARIABLE_ACCESS_T_IDENTIFIER 1
#define VARIABLE_ACCESS_T_INDEXED_VARIABLE 2
#define VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR 3
#define VARIABLE_ACCESS_T_METHOD_DESIGNATOR 4
struct variable_access_t{
  int type; /* 1 - identifier
	     * 2 - indexed_variable
	     * 3 - attribute_designator
	     * 4 - method_designator
	     */
  union{
    char *id;
    struct indexed_variable_t *iv;
    struct attribute_designator_t *ad;
    struct method_designator_t *md;
  }data;
  char *recordname;          /* This is a temporary field used to collect
				a verbose description of the data type
				that is validated */
  struct expression_data_t *expr;
};

struct object_instantiation_t{
  char *id;
  struct actual_parameter_list_t *apl;
};

struct assignment_statement_t{
  struct variable_access_t *va;
  struct expression_t *e;  
  struct object_instantiation_t *oe;
};

struct statement_t;
struct if_statement_t{
  struct expression_t *e;
  struct statement_t *s1;
  struct statement_t *s2;
};

struct while_statement_t{
  struct expression_t *e;
  struct statement_t *s;
};

struct print_statement_t{
  struct variable_access_t *va;
};

struct function_block_t{
  struct variable_declaration_list_t *vdl;
  struct statement_sequence_t *ss;
};

struct statement_sequence_t;
#define STATEMENT_T_ASSIGNMENT 1
#define STATEMENT_T_SEQUENCE 2
#define STATEMENT_T_IF 3
#define STATEMENT_T_WHILE 4
#define STATEMENT_T_PRINT 5
struct statement_t {
  int type; /* 1 - assignment_statement
	     * 2 - statement_sequence
	     * 3 - if_statement
	     * 4 - while_statement
	     * 5 - print_statement
	     */

  union{
    struct assignment_statement_t *as;
    struct statement_sequence_t *ss;
    struct if_statement_t *is;
    struct while_statement_t *ws;
    struct print_statement_t *ps;
  }data;
  int line_number;
};

struct statement_sequence_t{
  struct statement_t *s;
  struct statement_sequence_t *next;
};

/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the program
 * ----------------------------------------------------------------
 */
struct program_heading_t {
  char *id;
  struct identifier_list_t *il;
};

struct class_identification_t {
  char *id;
  struct class_extend_t *extend;
  int line_number;
};

struct class_extend_t {
  char *id;
  struct class_list_t *extend_class;
};

struct class_block_t{
  struct variable_declaration_list_t *vdl; /* == variable_declaration_part */
  struct func_declaration_list_t *fdl;
  struct statement_table_t *statement_hash_table; /* Circular? */
  struct attribute_table_t *attribute_hash_table;
};

struct class_list_t;
struct class_list_t {
  struct class_identification_t *ci;
  struct class_block_t *cb;
  struct class_list_t *next;
};

struct program_t {
  struct program_heading_t *ph;
  struct class_list_t *cl;
  struct class_table_t *class_hash_table;
};



/* ----------------------------------------------------------------
 * Function declarations
 * ----------------------------------------------------------------
 */
int makekey(char* lexeme, int max_hashkeys);
char * tolower(char *s);
void print_tabs(int numOfTabs);
char *inttostring(int value);
char *longtostring(long value);

/*memory allocating functions*/
struct actual_parameter_t *set_actual_parameter(struct expression_t *e1, struct expression_t *e2, struct expression_t *e3);
struct actual_parameter_list_t *set_actual_parameter_list(struct actual_parameter_t *ap, struct actual_parameter_list_t *next);
struct array_type_t *set_array_type(struct range_t *r, struct type_denoter_t *td);
struct assignment_statement_t *set_assignment_statement(struct variable_access_t *va, struct expression_t *e, struct object_instantiation_t *oe);
struct attribute_designator_t *set_attribute_designator(struct variable_access_t *va, char *id);
void check_compatibility(char* type1, char* type2);
struct class_block_t *set_class_block(struct variable_declaration_list_t *vdl, struct func_declaration_list_t *fdl);
struct class_identification_t *set_class_identification(char *id, char *extend, int line_number);
struct class_list_t *set_class_list(struct class_identification_t* ci, struct class_block_t *cb, struct class_list_t *next);
struct expression_t *set_expression(struct simple_expression_t* se1, int relop, struct simple_expression_t* se2, struct expression_data_t *expr);
struct expression_data_t *set_expression_data(float val, char *type);
struct factor_data_t* set_factor_data(int* sign, struct factor_t* next);
struct factor_t* set_factor_t_primary(struct primary_t* p, struct expression_data_t* expr);
struct factor_t* set_factor_t_sign_factor(struct factor_data_t* f, struct expression_data_t* expr);
struct formal_parameter_section_t *set_formal_parameter_section(struct identifier_list_t *il, char *id, int is_var);
struct formal_parameter_section_list_t *set_formal_parameter_section_list(struct formal_parameter_section_t *fps, struct formal_parameter_section_list_t *next);
struct function_block_t *set_function_block(struct variable_declaration_list_t *vdl, struct statement_sequence_t *ss);
struct function_declaration_t *set_function_declaration(struct function_heading_t *fh, struct function_block_t *fb, int line_number);
struct func_declaration_list_t *set_func_declaration_list(struct function_declaration_t *fd, struct func_declaration_list_t *next);
struct function_designator_t *set_function_designator(char * id, struct actual_parameter_list_t *apl);
struct function_heading_t *set_function_heading(char *id, char *res, struct formal_parameter_section_list_t *fpsl);
struct identifier_list_t *set_identifier_list(char *id, struct identifier_list_t* next);
struct if_statement_t *set_if_statement(struct expression_t *e, struct statement_t *s1, struct statement_t *s2);
struct index_expression_list_t *set_index_expression_list(struct expression_t *e, struct index_expression_list_t *next, struct expression_data_t *expr);
struct indexed_variable_t *set_indexed_variable(struct variable_access_t *va, struct index_expression_list_t *iel, struct expression_data_t *expr);
struct method_designator_t *set_method_designator(struct variable_access_t *va, struct function_designator_t *fd);
struct object_instantiation_t *set_object_instantiation(char *id, struct actual_parameter_list_t *apl);
struct primary_data_t* set_primary_data(struct primary_t *next);
struct primary_t* set_primary_t_expression(struct expression_t *e, struct expression_data_t *expr);
struct primary_t* set_primary_t_function_designator(struct function_designator_t *fd, struct expression_data_t *expr);
struct primary_t* set_primary_t_primary(struct primary_data_t *p, struct expression_data_t *expr);
struct primary_t* set_primary_t_unsigned_constant(struct unsigned_number_t *un, struct expression_data_t *expr);
struct primary_t *set_primary_t_variable_access(struct variable_access_t* va, struct expression_data_t * expr);
struct print_statement_t *set_print_statement(struct variable_access_t *va);
struct program_t *set_program(struct program_heading_t *ph, struct class_list_t *cl);
struct program_heading_t *set_program_heading(char *id, struct identifier_list_t *il);
struct range_t *set_range(struct unsigned_number_t *min, struct unsigned_number_t *max);
int * set_sign(int sign);
float simple_expression_relop(struct simple_expression_t *se1, int relop, struct simple_expression_t *se2);
struct simple_expression_t *set_simple_expression(struct term_t *t, int addop, struct expression_data_t *expr, struct simple_expression_t *next);
struct statement_t *set_statement_assignment(struct assignment_statement_t *as, int line_number);
struct statement_t *set_statement_if(struct if_statement_t *is, int line_number);
struct statement_t *set_statement_print(struct print_statement_t *ps, int line_number);
struct statement_t *set_statement_statement_sequence(struct statement_sequence_t *ss, int line_number);
struct statement_t *set_statement_while(struct while_statement_t *ws, int line_number);
struct statement_sequence_t *set_statement_sequence(struct statement_t *s, struct statement_sequence_t *next);
struct term_t *set_term(struct factor_t *f, int mulop, struct expression_data_t* expr, struct term_t *next);
struct type_denoter_t *set_type_denoter_array(char *name, struct array_type_t *at);
struct type_denoter_t *set_type_denoter_class(char *name, struct class_list_t *cl);
struct type_denoter_t *set_type_denoter_id(char *name, char *id);
struct unsigned_number_t *set_unsigned_number(int ui, struct expression_data_t* expr);
struct variable_access_t *set_variable_access_id(char *id, char *recordname, struct expression_data_t *expr);
struct variable_access_t *set_variable_access_indexed_variable(struct indexed_variable_t *iv, char *record_name, struct expression_data_t *expr);
struct variable_access_t *set_variable_access_attribute_designator(struct attribute_designator_t *ad, char *recordname, struct expression_data_t *expr);
struct variable_access_t *set_variable_access_method_designator(struct method_designator_t *md, char *recordname, struct expression_data_t *expr);
struct variable_declaration_t *set_variable_declaration(struct identifier_list_t *il, struct type_denoter_t *tden, int line_number);
struct variable_declaration_list_t *set_variable_declaration_list(struct variable_declaration_t *vd, struct variable_declaration_list_t *next);
struct while_statement_t *set_while_statement(struct expression_t *e, struct statement_t *s);


#endif
