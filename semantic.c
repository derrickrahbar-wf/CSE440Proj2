/*
 * semantic.c
 *
 * Implements all functions that participate in semantic analysis.
 */


#include "shared.h"
#include "semantic.h"
#include "rulefuncs.h"
#include "usrdef.h"
#include "symtab.h"



/* ----------------------------------------------------------------------- 
 * Carries out semantic analysis on a program
 * ----------------------------------------------------------------------- 
 */
void semantic_analysis(struct program_t *p)
{
    struct class_table_t *ct;
    struct statement_table_t *st;
    /*analyze all classes*/               
    for(ct= p->class_hash_table; ct != NULL; ct=ct->hh.next) 
    {
        /*validate all statements written in the class */
        for(st=ct->statement_hash_table; st!=NULL; st=st->hh.next)
        {
            switch(st->type)
            {
                case STATEMENT_T_ASSIGNMENT:
                    validate_assignment_statement(st);
                    break;
                case STATEMENT_T_SEQUENCE:
                    validate_statement_sequence(st);
                    break;
                case STATEMENT_T_IF:
                    validate_if_statement(st);
                    break;
                case STATEMENT_T_WHILE:
                    validate_while_statement(st);
                    break;
                case STATEMENT_T_PRINT:
                    validate_print_statement(st);
                    break;
            }
        }
    }
}

void validate_assignment_statement(struct statement_table_t* statement)
{
    struct assignment_statement_t *a_stat = statement->statement_data->as;

    char *va_type = get_va_type(a_stat->va);

    if(a_stat->e != NULL)
    {    
        char *expr_type = get_expr_type(a_stat->e);

        if(strcmp(va_type, expr_type)) /*types are different*/
        {
            error_type_mismatch(statement->line_number, va_type, expr_type);
        }
    }
    else /*has an object_instantiation*/
    {
        char *obj_inst_type = get_obj_inst_type(a_stat->oe);

        if(strcmp(va_type, obj_inst_type)) /*types are different*/
        {
            error_type_mismatch(statement->line_number, va_type, obj_inst_type);
        }

    }
}

void validate_statement_sequence(struct statement_table_t* statement)
{
    struct statement_sequence_t *stat_seq = statement->statement_data->ss;

    while(stat_seq != NULL)
    {
        eval_statement(stat_seq->s); 
        stat_seq = stat_seq->next;
    }
}

void validate_if_statement(struct statement_table_t* statement)
{
    struct if_statement_t *if_stat = statement->statement_data->is;

    char * expr_type = get_expr_type(if_stat->e);

    if(strcmp("boolean", expr_type)) /*expression is not boolean*/
    {
        error_datatype_is_not(statement->line_number, expr_type, "boolean");
    }
    
    /*validate both statements */
    eval_statement(if_stat->s1);
    eval_statement(if_stat->s2);
}

void validate_while_statement(struct statement_table_t* statement)
{
    struct while_statement_t *while_stat = statement->statement_data->ws;

    char *expr_type = get_expr_type(while_stat->e);

    if(strcmp("boolean", expr_type)) /*expression is not boolean*/
    {
        error_datatype_is_not(statement->line_number, expr_type, "boolean");
    }

    /*validate statement*/
    eval_statement(while_stat->s);
}

void validate_print_statement(struct statement_table_t* statement)
{
    struct print_statement_t *pr_stat = statement->statement_data->ps;

    get_va_type(pr_stat->va); /*validating will happen within here*/

}

struct statement_table_t *create_statement_table(struct statement_t *statement)
{
    struct statement_table_t* statement_table = (struct statement_table_t*)malloc(sizeof(struct statement_table_t));

    statement_table->line_number = statement->line_number;
    statement_table->statement_data = (union statement_union *)malloc(sizeof(*statement));

    switch(statement->type)
    {
        case STATEMENT_T_ASSIGNMENT:
            statement_table->statement_data->as = statement->data.as;
            break;
        case STATEMENT_T_SEQUENCE:
            statement_table->statement_data->ss = statement->data.ss;
            break;
        case STATEMENT_T_IF:
            statement_table->statement_data->is = statement->data.is;
            break;
        case STATEMENT_T_WHILE:
            statement_table->statement_data->ws = statement->data.ws;
            break;
        case STATEMENT_T_PRINT:
            statement_table->statement_data->ps = statement->data.ps;
            break;
    }
    
    return statement_table;
}

void eval_statement(struct statement_t *statement)
{
    switch(statement->type) /*must wrap these in statement_table_t structs*/
    {
        case STATEMENT_T_ASSIGNMENT:
            validate_assignment_statement(create_statement_table(statement));
            break;
        case STATEMENT_T_SEQUENCE:
            validate_statement_sequence(create_statement_table(statement));
            break;
        case STATEMENT_T_IF:
            validate_if_statement(create_statement_table(statement));
            break;
        case STATEMENT_T_WHILE:
            validate_while_statement(create_statement_table(statement));
            break;
        case STATEMENT_T_PRINT:
            validate_print_statement(create_statement_table(statement));
            break;
    }
}


/*GRAMMAR GET TYPES*/
char * get_expr_type(struct expression_t *expr)
{
    if(expr->expr != NULL && expr->expr->type != NULL)
    {
        return expr->expr->type; /*we already have type*/
    }
    else
    {

    }
}

char *get_obj_inst_type(struct object_instantiation_t *obj_inst)
{

}

char * get_va_type(struct variable_access_t* va)
{

}

