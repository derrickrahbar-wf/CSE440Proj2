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

struct expression_data_t* get_expr_expr_data(struct expression_t *expr, int line_number);
struct expression_data_t* get_obj_inst_expr_data(struct object_instantiation_t *obj_inst);
struct expression_data_t* get_va_expr_data(struct variable_access_t* va);
struct expression_data_t* get_simple_expr_expr_data(struct simple_expression_t *simple_expression, int line_number);
int evaluate_this(char *id);;


#endif /* SEMANTIC_H */
