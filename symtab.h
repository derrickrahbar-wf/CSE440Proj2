/* symtab.h
 *
 * Holds function definitions for the symbol table. The symbol table
 * is implemented as a global hash table that contains local symbol
 * tables for each function
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#define SCOPE_FV 0
#define SCOPE_NFV 1

#define ASSIGNMENT_STATEMENT 0
#define STATEMENT_SEQUENCE 1
#define IF_STATEMENT 2
#define WHILE_STATEMENT 3
#define PRINT_STATEMENT 4

#include "shared.h"
#include "usrdef.h"
#include <stdlib.h>
#include <string.h>
#include "./uthash/src/uthash.h"


/* ----------------------------------------------------------------
 * Function declarations
 * ----------------------------------------------------------------
 */


void symtab_init();
void symtab_print(int numOfTabs);

struct attribute_key_t{
    char *id;
    int scope;
    struct function_declaration_t *function;
};

struct attribute_table_t{
    struct type_denoter_t *type;
    int line_number;
    int is_func;
    char *id; /* id, scope and function are used for the key */
    int scope;
    struct function_declaration_t *function;
    struct formal_parameter_section_list_t *params;
    UT_hash_handle hh; /* defines structure as a hashable object */
};

struct statement_table_t{
    int type;
    int line_number;
    struct function_declaration_t *function;
    
    union{
        struct assignment_statement_t *as;
        struct statement_sequence_t *ss;
        struct if_statement_t *is;
        struct while_statement_t *ws;
        struct print_statement_t *ps;
  }statement_data;

  UT_hash_handle hh; /* defines structure as a hashable object */
};

struct class_table_t {
    struct attribute_table_t *attribute_hash_table;
    struct class_table_t *extend;
    int line_number;
    char *id;
    struct statement_table_t *statement_hash_table;
    UT_hash_handle hh; /* defines structure as a hashable object */
};

#endif
