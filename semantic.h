/*
 * semantic.h
 *
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H


void semantic_analysis(struct program_t *p);
void validate_assignment_statement(struct statement_table_t* statement);
void validate_statement_sequence(struct statement_table_t* statement);
void validate_if_statement(struct statement_table_t* statement);
void validate_while_statement(struct statement_table_t* statement);
void validate_print_statement(struct statement_table_t* statement);

void eval_statement(struct statement_t *statement);

struct statement_table_t *create_statement_table(struct statement_t *statement);

char * get_expr_type(struct expression_t *expr);
char *get_obj_inst_type(struct object_instantiation_t *obj_inst);
char * get_va_type(struct variable_access_t* va);



#endif /* SEMANTIC_H */
