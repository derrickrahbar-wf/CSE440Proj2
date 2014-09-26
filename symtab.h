/* symtab.h
 *
 * Holds function definitions for the symbol table. The symbol table
 * is implemented as a global hash table that contains local symbol
 * tables for each function
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "shared.h"
#include "usrdef.h"
#include <stdlib.h>
#include <string.h>



/* ----------------------------------------------------------------
 * Function declarations
 * ----------------------------------------------------------------
 */


void symtab_init();
void symtab_print(int numOfTabs);
struct class_sym_tab* new_class_sym_tab();

struct sym_tab {
    struct class_sym_tab *class_tab;
    struct id_sym_tab *id_tab;
};

struct class_sym {
    char *id;
    struct class_sym *extend;
    struct class_block_t *cb;
    int line_number; //to check if acutally declared or just referenced
    int equiv_id; //unique id shared by all equiv classes -1 of not yet set

};

struct class_sym_tab {
    struct class_sym *class_sym;
    struct class_sym_tab *next;
};

struct id_sym {
    char *id;
    struct type_denoter_t *type;
};

struct id_sym_tab {
    struct id_sym *id;
    struct id_sym_tab *next;
};

#endif
