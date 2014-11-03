extern "C"
{
    #include "shared.h"
#include "rulefuncs.h"
#include <string.h>
}

#include "free_mem.h"
#include <iostream>

using namespace std;
void remove_program_tree(struct program_t *program)
{
    if(program != NULL)
    {
        remove_program_heading(program->ph);
        remove_class_list(program->cl);
        remove_class_hash_table(program->class_hash_table);

        free(program);
    }
}

void remove_program_heading(struct program_heading_t *ph)
{
    if(ph != NULL)
    {
        
        free(ph->id);
        remove_identifier_list(ph->il);

        free(ph);
    }
}

void remove_class_list(struct class_list_t *cl)
{
    if(cl != NULL)
    {
        remove_class_identification(cl->ci);
        remove_class_block(cl->cb);
        remove_class_list(cl->next);

        free(cl);    
    }
    
}

void remove_class_hash_table(struct class_table_t* cht)
{
    if(cht != NULL)
    {
        remove_attr_hash_table(cht->attribute_hash_table);
        remove_class_hash_table(cht->extend);
        remove_statement_hash_table(cht->statement_hash_table);
        remove_class_list(cht->class_list);
        free(cht->id);

        free(cht);
    }
}

void remove_attr_hash_table(struct attribute_table_t *aht)
{
    if(aht != NULL)
    {
        remove_type_denoter(aht->type);
        remove_formal_param_sec_list(aht->params);
        remove_func_decl(aht->function);
        remove_expression_data(aht->expr);
        free(aht);
    }
}

void remove_type_denoter(struct type_denoter_t *t)
{
    if(t != NULL)
    {
        free(t->name);
        switch(t->type)
        {
            case TYPE_DENOTER_T_ARRAY_TYPE:
                remove_array_type(t->data.at);
                break;
            case TYPE_DENOTER_T_CLASS_TYPE:
                remove_class_list(t->data.cl);
                break;
            case TYPE_DENOTER_T_IDENTIFIER:
                free(t->data.id);
                break;
        }

        free(t);
    }
}

void remove_array_type(struct array_type_t *at)
{
    if(at != NULL)
    {
        remove_range(at->r);
        remove_type_denoter(at->td);

        free(at);
    }
}

void remove_range(struct range_t *r)
{
    if(r != NULL)
    {
        remove_unsigned_number(r->min);
        remove_unsigned_number(r->max);

        free(r);
    }
}

void remove_unsigned_number(struct unsigned_number_t *un)
{
    if(un != NULL)
    {
        remove_expression_data(un->expr);

        free(un);
    }
}


void remove_formal_param_sec_list(struct formal_parameter_section_list_t *fpsl)
{
    if(fpsl != NULL)
    {
        remove_formal_param_sec(fpsl->fps);
        remove_formal_param_sec_list(fpsl->next);

        free(fpsl);
    }
}

void remove_formal_param_sec(struct formal_parameter_section_t *fps)
{   
    if(fps != NULL)
    {
        remove_identifier_list(fps->il);
        free(fps->id);

        free(fps);
    }
}

void remove_func_decl(struct function_declaration_t *fd)
{
    if(fd != NULL)
    {
        remove_function_heading(fd->fh);
        remove_function_block(fd->fb);

        free(fd);
    }
}

void remove_function_heading(struct function_heading_t *fh)
{
    if(fh != NULL)
    {
        free(fh->id);
        free(fh->res);
        remove_formal_param_sec_list(fh->fpsl);

        free(fh);
    }
}

void remove_function_block(struct function_block_t *fb)
{
    if(fb != NULL)
    {
        remove_variable_declaration_list(fb->vdl);
        remove_statement_sequence(fb->ss);

        free(fb);
    }
}

void remove_variable_declaration_list(struct variable_declaration_list_t *vdl)
{
    if(vdl != NULL)
    {
        remove_variable_declaration(vdl->vd);
        remove_variable_declaration_list(vdl->next);

        free(vdl);
    }
}

void remove_variable_declaration(struct variable_declaration_t *vd)
{
    if(vd != NULL)
    {
        remove_identifier_list(vd->il);
        remove_type_denoter(vd->tden);

        free(vd);
    }
}

void remove_statement_sequence(struct statement_sequence_t *ss)
{
    if(ss != NULL)
    {
        remove_statement(ss->s);
        remove_statement_sequence(ss->next);

        free(ss);
    }
}

void remove_statement(struct statement_t *ss)
{
    if(ss != NULL)
    {
        switch(ss->type)
        {
            case STATEMENT_T_ASSIGNMENT:
                remove_assignment_statement(ss->data.as);
                break;
            case STATEMENT_T_SEQUENCE:
                remove_statement_sequence(ss->data.ss);
                break;
            case STATEMENT_T_IF:
                remove_if_statement(ss->data.is);
                break;
            case STATEMENT_T_WHILE:
                remove_while_statement(ss->data.ws);
                break;
            case STATEMENT_T_PRINT:
                remove_print_statement(ss->data.ps);
                break;
        }

        free(ss);
    }
}

void remove_assignment_statement(struct assignment_statement_t *as)
{
    if(as != NULL)
    {
        remove_variable_access(as->va);
        remove_expression(as->e);
        remove_object_instantiation(as->oe);

        free(as);
    }
}

void remove_expression(struct expression_t *e)
{
    if(e != NULL)
    {
        remove_simple_expression(e->se1);
        remove_simple_expression(e->se2);
        remove_expression_data(e->expr);

        free(e);
    }
}

void remove_simple_expression(struct simple_expression_t *se)
{
    if(se != NULL)
    {
        remove_term(se->t);
        remove_expression_data(se->expr);
        remove_simple_expression(se->next);

        free(se);
    }
}

void remove_term(struct term_t *t)
{
    if(t != NULL)
    {
        remove_factor(t->f);
        remove_expression_data(t->expr);
        remove_term(t->next);

        free(t);
    }
}

void remove_factor(struct factor_t *f)
{
    if(f != NULL)
    {
        switch(f->type)
        {
            case FACTOR_T_SIGNFACTOR:
                remove_factor_data(f->data.f);
                break;
            case FACTOR_T_PRIMARY:
                remove_primary(f->data.p);
                break;
        }
        remove_expression_data(f->expr);

        free(f);
    }
}

void remove_factor_data(struct factor_data_t *fd)
{
    if(fd != NULL)
    {
        remove_factor(fd->next);

        free(fd);
    }
}

void remove_primary(struct primary_t *p)
{
    if(p != NULL)
    {
        switch(p->type)
        {
            case PRIMARY_T_VARIABLE_ACCESS:
                remove_variable_access(p->data.va);
                break;
            case PRIMARY_T_UNSIGNED_CONSTANT:
                remove_unsigned_number(p->data.un);
                break;
            case PRIMARY_T_FUNCTION_DESIGNATOR:
                remove_function_designator(p->data.fd);
                break;
            case PRIMARY_T_EXPRESSION:
                remove_expression(p->data.e);
                break;
            case PRIMARY_T_PRIMARY:
                remove_primary(p->data.next);
                break;
        }
        remove_expression_data(p->expr);

        free(p);
    }
}

void remove_function_designator(struct function_designator_t *fd)
{
    if(fd != NULL)
    {
        free(fd->id);
        remove_actual_parameter_list(fd->apl);

        free(fd);
    }
}

void remove_actual_parameter_list(struct actual_parameter_list_t *apl)
{
    if(apl != NULL)
    {   
        remove_actual_parameter(apl->ap);
        remove_actual_parameter_list(apl->next);

        free(apl);
    }
}

void remove_actual_parameter(struct actual_parameter_t *ap)
{
    if(ap != NULL)
    {
        remove_expression(ap->e1);
        remove_expression(ap->e2);
        remove_expression(ap->e3);

        free(ap);
    }
}

void remove_variable_access(struct variable_access_t *va)
{
    if(va != NULL)
    {
        switch(va->type)
        {
            case VARIABLE_ACCESS_T_IDENTIFIER:
                free(va->data.id);
                break;
            case VARIABLE_ACCESS_T_INDEXED_VARIABLE:
                remove_indexed_variable(va->data.iv);
                break;
            case VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR:
                remove_attribute_designator(va->data.ad);
                break;
            case VARIABLE_ACCESS_T_METHOD_DESIGNATOR:
                remove_method_designator(va->data.md);
                break;
        }
        free(va->recordname);
        remove_expression_data(va->expr);

        free(va);
    }
}

void remove_indexed_variable(struct indexed_variable_t *ad)
{
    if(ad != NULL)
    {
        remove_variable_access(ad->va);
        remove_index_expression_list(ad->iel);
        remove_expression_data(ad->expr);

        free(ad);
    }
}

void remove_attribute_designator(struct attribute_designator_t *ad)
{
    if(ad != NULL)
    {
        remove_variable_access(ad->va);
        free(ad->id);

        free(ad);
    }
}

void remove_index_expression_list(struct index_expression_list_t *iel)
{
    if(iel != NULL)
    {
        remove_expression(iel->e);
        remove_index_expression_list(iel->next);
        remove_expression_data(iel->expr);

        free(iel);
    }
}

void remove_method_designator(struct method_designator_t *md)
{
    if(md != NULL)
    {
        remove_variable_access(md->va);
        remove_function_designator(md->fd);

        free(md);
    }
}

void remove_object_instantiation(struct object_instantiation_t *oe)
{
    if(oe != NULL)
    {
        free(oe->id);
        remove_actual_parameter_list(oe->apl);

        free(oe);
    }
}

void remove_if_statement(struct if_statement_t *is)
{
    if(is != NULL)
    {
        remove_expression(is->e);
        remove_statement(is->s1);
        remove_statement(is->s2);

        free(is);
    }
}

void remove_while_statement(struct while_statement_t *ws)
{
    if(ws != NULL)
    {
        remove_expression(ws->e);
        remove_statement(ws->s);

        free(ws);
    }
}

void remove_print_statement(struct print_statement_t *ps)
{
    if(ps != NULL)
    {
        remove_variable_access(ps->va);

        free(ps);
    }
}

void remove_expression_data(struct expression_data_t *ed)
{

}

void remove_statement_hash_table(struct statement_table_t *sht)
{
    if(sht != NULL)
    {
        remove_func_decl(sht->function);
        remove_statement_union(sht->statement_data);

        free(sht);    
    }
    
}

void remove_statement_union(union statement_union *su)
{
    if(su != NULL)
    {
        remove_assignment_statement(su->as);
        remove_statement_sequence(su->ss);
        remove_if_statement(su->is);
        remove_while_statement(su->ws);
        remove_print_statement(su->ps);

        free(su);
    }
}

void remove_identifier_list(struct identifier_list_t *idl)
{
    if(idl != NULL)
    {
        free(idl->id);
        remove_identifier_list(idl->next);

        free(idl);    
    }
    
}

void remove_class_identification(struct class_identification_t *ci)
{
    if(ci != NULL)
    {
        free(ci->id);

        free(ci);    
    }
    
}


void remove_class_block(struct class_block_t *cb)
{
    if(cb != NULL)
    {
        remove_variable_declaration_list(cb->vdl);
        remove_function_declaration_list(cb->fdl);
        remove_statement_hash_table(cb->statement_hash_table);
        remove_attr_hash_table(cb->attribute_hash_table);

        free(cb);   
    }
}

void remove_function_declaration_list(struct func_declaration_list_t *fdl)
{
    if(fdl != NULL)
    {
        remove_func_decl(fdl->fd);
        remove_function_declaration_list(fdl->next);

        free(fdl);
    }
}
