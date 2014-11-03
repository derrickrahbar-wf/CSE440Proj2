/* symtab.h
 *
 * Holds function definitions for the symbol table. The symbol table
 * is implemented as a global hash table that contains local symbol
 * tables for each function
 */

#ifndef FREE_MEM_H
#define FREE_MEM_H


/* ----------------------------------------------------------------
 * Function declarations
 * ----------------------------------------------------------------
 */


void remove_program_tree(struct program_t *program);
void remove_program_heading(struct program_heading_t *ph);
void remove_class_list(struct class_list_t *cl);
void remove_class_hash_table(struct class_table_t* cht);
void remove_attr_hash_table(struct attribute_table_t *aht);
void remove_type_denoter(struct type_denoter_t *t);
void remove_array_type(struct array_type_t *at);
void remove_range(struct range_t *r);
void remove_unsigned_number(struct unsigned_number_t *un);
void remove_formal_param_sec_list(struct formal_parameter_section_list_t *fpsl);
void remove_func_decl(struct function_declaration_t *fd);
void remove_function_heading(struct function_heading_t *fh);
void remove_function_block(struct function_block_t *fb);
void remove_variable_declaration_list(struct variable_declaration_list_t *vdl);
void remove_variable_declaration(struct variable_declaration_t *vd);
void remove_statement_sequence(struct statement_sequence_t *ss);
void remove_statement(struct statement_t *ss);
void remove_assignment_statement(struct assignment_statement_t *as);
void remove_expression(struct expression_t *e);
void remove_simple_expression(struct simple_expression_t *se);
void remove_term(struct term_t *t);
void remove_factor(struct factor_t *f);
void remove_factor_data(struct factor_data_t *fd);
void remove_primary(struct primary_t *p);
void remove_function_designator(struct function_designator_t *fd);
void remove_actual_parameter_list(struct actual_parameter_list_t *apl);
void remove_actual_parameter(struct actual_parameter_t *ap);
void remove_variable_access(struct variable_access_t *va);
void remove_indexed_variable(struct indexed_variable_t *iv);
void remove_attribute_designator(struct attribute_designator_t *ad);
void remove_index_expression_list(struct index_expression_list_t *iel);
void remove_method_designator(struct method_designator_t *md);
void remove_object_instantiation(struct object_instantiation_t *oe);
void remove_if_statement(struct if_statement_t *is);
void remove_while_statement(struct while_statement_t *ws);
void remove_print_statement(struct print_statement_t *ps);
void remove_expression_data(struct expression_data_t *ed);
void remove_statement_hash_table(struct statement_table_t *sht);
void remove_statement_union(union statement_union *su);
void remove_identifier_list(struct identifier_list_t *idl);
void remove_class_identification(struct class_identification_t *ci);
void remove_class_block(struct class_block_t *cb);
void remove_function_declaration_list(struct func_declaration_list_t *fdl);



/* ONES NOT PICKED UP */
void remove_formal_param_sec(struct formal_parameter_section_t *fps);

#endif