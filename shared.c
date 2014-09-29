/* shared.c
 *
 * Implements all shared subroutines
 */

#include "shared.h"
#include "rulefuncs.h"
#include <string.h>

/* Global head to the attribute hash table */
struct attribute_table_t *attr_hash_table = NULL;

/*Global head to the statement hast table */
struct statement_table_t *stat_hash_table = NULL;


/* ----------------------------------------------------------------------- 
 * Returns a hashkey value for the given lexeme
 * ----------------------------------------------------------------------- 
 */
int makekey(char* lexeme, int max_hashkeys)
{
  int len;
  int i;
  long charsum = 0;

  len = strlen(lexeme);
  for (i = 0; i < len; i++){
    charsum += lexeme[i];
  }

  return charsum % max_hashkeys;
}



/* ----------------------------------------------------------------------- 
 * Converts a string to lowercase
 * ----------------------------------------------------------------------- 
 */
char * tolower(char *s)
{
  int len;
  int i;
  char *new;

  if (s != NULL)
    len = strlen(s);
  else
    len = 0;

  new = (char *) malloc(len + 1); /* +1 for '\0' */

  for (i = 0; i < len; i++) {
    /* if an uppercase character */
    if (s[i] >= 65 && s[i] <=  90)
      new[i] = s[i] + 32;
    else
      new[i] = s[i];
  }

  new[len] = '\0';

  return new;
}



/* ----------------------------------------------------------------------- 
 * Prints the specified amount of tabs
 * ----------------------------------------------------------------------- 
 */
void print_tabs(int numOfTabs)
{
  int i = 0;
  while (i < numOfTabs) {
    printf("\t");
    i++;
  }
}



/* ----------------------------------------------------------------------- 
 * Returns a string representation of the int value
 * ----------------------------------------------------------------------- 
 */
char *inttostring(int value)
{
  char *s;

  s = (char *) malloc(MAX_NEW_CHAR_SIZE);
  sprintf(s, "%d", value);

  return s;
}



/* ----------------------------------------------------------------------- 
 * Returns a string representation of the long value
 * ----------------------------------------------------------------------- 
 */
char *longtostring(long value)
{
  char *s;

  s = (char *) malloc(MAX_NEW_CHAR_SIZE);
  sprintf(s, "%ld", value);

  return s;
}

/*
Setter methods for structs making up the program 
*/

struct actual_parameter_t *set_actual_parameter(struct expression_t *e1, struct expression_t *e2, struct expression_t *e3)
{
    struct actual_parameter_t* ap = new_actual_parameter();
    ap->e1 = e1;
    ap->e2 = e2;
    ap->e3 = e3;

    return ap;
}

struct actual_parameter_list_t *set_actual_parameter_list(struct actual_parameter_t *ap, struct actual_parameter_list_t *next)
{
    struct actual_parameter_list_t* apl = new_actual_parameter_list();
    apl->ap = ap;
    apl->next = next;

    return apl;
}

struct array_type_t *set_array_type(struct range_t *r, struct type_denoter_t *td)
{
    struct array_type_t *at = new_array_type();
    at->r = r;
    at->td = td;

    return at;   
}

struct assignment_statement_t *set_assignment_statement(struct variable_access_t *va, struct expression_t *e, struct object_instantiation_t *oe)
{
    struct assignment_statement_t* as = new_assignment_statement();
    as->va = va;
    as->e = e;
    as->oe = oe;

    return as;
}

struct attribute_designator_t *set_attribute_designator(struct variable_access_t *va, char *id, int line_number)
{
    struct attribute_designator_t *ad = new_attribute_designator();
    ad->va = va;
    ad->id = (char *)malloc(strlen(id)*sizeof(char));
    strcpy(ad->id, id);

    if(va->expr != NULL)
    {
        if( is_integer(va->expr->type) || is_boolean(va->expr->type) || is_real(va->expr->type) )
        {
            error_datatype_is_not(line_number, va->expr->type, "object");
        }
    }

    return ad;
}

void check_compatibility(char* type1, char* type2)
{
    
}

struct class_block_t *set_class_block(struct variable_declaration_list_t *vdl, struct func_declaration_list_t *fdl)
{
    struct class_block_t *cb = new_class_block();
    cb->vdl = vdl;
    cb->fdl = fdl;
    create_attribute_hash_table(fdl, vdl);
    cb->attribute_hash_table = attr_hash_table;
    attr_hash_table = NULL; /*set global head back to null for next class*/
    
    create_statement_hash_table(fdl);
    cb->statement_hash_table = stat_hash_table;
    stat_hash_table = NULL; /*set global head back to null for next class */

    /* FOR DEBUGGING */
    print_hash_table(cb->attribute_hash_table);
    print_statement_hash_table(cb->statement_hash_table);


    return cb;
}

struct class_identification_t *set_class_identification(char *id, char *extend, int line_number)
{
    struct class_identification_t *ci = new_class_identification();
    ci->id = (char *)malloc(strlen(id));
    strcpy(ci->id, id);
    
    if(extend != NULL)
    {
      ci->extend = new_class_extend();
      ci->extend->id = (char *)malloc(strlen(extend));
      strcpy(ci->extend->id, extend);
      //leave pointer to class empty 
    }

    ci->line_number = line_number;

    return ci;

}

struct class_list_t *set_class_list(struct class_identification_t* ci, struct class_block_t *cb, struct class_list_t *next)
{
    struct class_list_t *cl = new_class_list();
    cl->ci = ci;
    cl->cb = cb;
    cl->next = next;

    return cl;
} 

struct expression_t *set_expression(struct simple_expression_t* se1, int relop, struct simple_expression_t* se2, int line_number)
{
    struct expression_t *e = new_expression();
    e->se1 = se1;
    e->relop = relop;
    e->se2 = se2;

    if(relop == RELOP_NONE)
    {
        /*just a simple expression given in se1*/
        e->expr = se1->expr;
    }
    else if(se1->expr != NULL && se2->expr != NULL) /*we can eval this exression*/
    {
        switch(relop)
        {
            case RELOP_EQUAL:
            case RELOP_NOTEQUAL:
                if(strcmp(se1->expr->type, se2->expr->type) == 0)
                {
                    /*they are the same type this is valid*/
                    if(relop == RELOP_NOTEQUAL) /*check if not equal*/
                    {
                        if(se1->expr->val != se2->expr->val)
                        {
                            e->expr = set_expression_data(1, "boolean"); /*this evaluated to true*/
                        }
                        else
                        {
                            e->expr = set_expression_data(0, "boolean"); /*false*/
                        }
                    }
                    else /*check if equal*/
                    {
                        if(se1->expr->val == se2->expr->val)
                        {
                            e->expr = set_expression_data(1, "boolean"); /*this evaluated to true*/
                        }
                        else
                        {
                            e->expr = set_expression_data(0, "boolean"); /*false*/
                        } 
                    }
                }
                else
                {
                    error_type_mismatch(line_number, se1->expr->type, se2->expr->type);
                }

            break;
            case RELOP_LT: /*only valid for integer and real*/
            case RELOP_GT:
            case RELOP_LE:
            case RELOP_GE:
                if((is_integer(se1->expr->type) || is_real(se1->expr->type)) && 
                   (is_integer(se2->expr->type) || is_real(se2->expr->type)))
                {
                    switch(relop)
                    {
                        case RELOP_LT:
                            if(se1->expr->val < se2->expr->val)
                            {
                                e->expr = set_expression_data(1, "boolean"); /*se1 < se2 */
                            }
                            else
                            {
                                e->expr = set_expression_data(0, "boolean"); /*false*/
                            }
                        break;
                        case RELOP_GE:
                            if(se1->expr->val >= se2->expr->val)
                            {
                                e->expr = set_expression_data(1, "boolean"); /*se1 >= se2 */
                            }
                            else
                            {
                                e->expr = set_expression_data(0, "boolean"); /*false*/
                            }
                        break;
                        case RELOP_LE:
                            if(se1->expr->val <= se2->expr->val)
                            {
                                e->expr = set_expression_data(1, "boolean"); /*se1 <= se2 */
                            }
                            else
                            {
                                e->expr = set_expression_data(0, "boolean"); /*false*/
                            }
                        break;
                        case RELOP_GT:
                            if(se1->expr->val > se2->expr->val)
                            {
                                e->expr = set_expression_data(1, "boolean"); /*se1 > se2 */
                            }
                            else
                            {
                                e->expr = set_expression_data(0, "boolean"); /*false*/
                            }
                        break;
                    }
                }
                else
                {
                    /*bad type error*/
                    if(!(is_real(se1->expr->type) || is_integer(se1->expr->type)))
                    {
                        error_datatype_is_not(line_number, se1->expr->type, "real or integer");
                    }
                    if(!(is_real(se2->expr->type) || is_integer(se2->expr->type)))
                    {
                        error_datatype_is_not(line_number, se2->expr->type, "real or integer");
                    }

                    /*set to int to continue eval*/
                    e->expr = set_expression_data(0, "boolean");   
                }
            
            break;
        }
    }

    return e;
}

struct expression_data_t *set_expression_data(float val, char *type)
{
    struct expression_data_t *ed = new_expression_data();
    ed->val = val;
    ed->type = type;

    return ed;
}

////////////////////////////////////////////////////////////////////////////////////////
//FACTOR_T METHODS
////////////////////////////////////////////////////////////////////////////////////////

struct factor_t* set_factor_t_sign_factor(int sign, struct factor_t* f, int line_number)
{
    struct factor_t *fsf = new_factor();
    fsf->type = FACTOR_T_SIGNFACTOR;
    fsf->data.f = *(set_factor_data(sign, f));

    /*This would need to be a real or an integer to have a sign in from of it */
    if(f->expr != NULL)
    {
        if(strcmp(f->expr->type, "integer") == 0 || strcmp(f->expr->type, "real") == 0)
        {
            /*This is a valid type of sign factor*/
            fsf->expr = set_expression_data((sign == SIGN_PLUS)? f->expr->val : -1*f->expr->val, 
                                            f->expr->type);
        }
        else
        {
            error_datatype_is_not(line_number, f->expr->type, "real or integer");

            /* set this to a valid type in order to continue eval*/
            fsf->expr = set_expression_data(-1, "integer");
        }
    }

    return fsf;

}

struct factor_t* set_factor_t_primary(struct primary_t* p)
{
    struct factor_t *fp = new_factor();
    fp->type = FACTOR_T_PRIMARY;
    fp->data.p = p;
    fp->expr = p->expr; /*if this is populated it will be the same for this factor*/

    return fp;
}

////////////////////////////////////////////////////////////////////////////////////////

struct factor_data_t* set_factor_data(int sign, struct factor_t* next)
{
    struct factor_data_t* fd = new_factor_data();
    fd->sign = sign;
    fd->next = next;

    return fd;
}

struct formal_parameter_section_t *set_formal_parameter_section(struct identifier_list_t *il, char *id, int is_var)
{
    struct formal_parameter_section_t *fps = new_formal_parameter_section();
    fps->il = il;
    fps->id = (char *)malloc(strlen(id)*sizeof(char));
    strcpy(fps->id, id);
    fps->is_var = is_var;

    return fps;
}

struct formal_parameter_section_list_t *set_formal_parameter_section_list(struct formal_parameter_section_t *fps, struct formal_parameter_section_list_t *next)
{
    struct formal_parameter_section_list_t *fpsl = new_formal_parameter_section_list();
    fpsl->fps = fps;
    fpsl->next = next;

    return fpsl;
}

struct function_block_t *set_function_block(struct variable_declaration_list_t *vdl, struct statement_sequence_t *ss)
{
    struct function_block_t *fb = new_function_block();
    fb->vdl = vdl;
    fb->ss = ss;

    return fb;
}

struct function_declaration_t *set_function_declaration(struct function_heading_t *fh, struct function_block_t *fb, int line_number)
{
    struct function_declaration_t *fd = new_function_declaration();
    fd->fh = fh;
    fd->fb = fb;
    fd->line_number = line_number;

    return fd;
}

struct func_declaration_list_t *set_func_declaration_list(struct function_declaration_t *fd, struct func_declaration_list_t *next)
{
    struct func_declaration_list_t *fdl = new_func_declaration_list();
    fdl->fd = fd;
    fdl->next = next;

    return fdl;
}

struct function_designator_t *set_function_designator(char * id, struct actual_parameter_list_t * apl)
{
    struct function_designator_t *fd = new_function_designator();
    fd->id = (char *)malloc(strlen(id)*sizeof(char));
    strcpy(fd->id, id);
    fd->apl = apl;

    return fd;
}

struct function_heading_t *set_function_heading(char *id, char *res, struct formal_parameter_section_list_t *fpsl)
{
    struct function_heading_t *fh = new_function_heading();
    fh->id = (char*)malloc(strlen(id));
    strcpy(fh->id, id);
    fh->res = res;
    fh->fpsl = fpsl;

    return fh;
}

struct identifier_list_t *set_identifier_list(char *id, struct identifier_list_t* next)
{
    struct identifier_list_t *il = new_identifier_list();
    il->id = (char *)malloc(strlen(id));
    strcpy(il->id, id);
    il->next = next;

    return il;
}

struct if_statement_t *set_if_statement(struct expression_t *e, struct statement_t *s1, struct statement_t *s2)
{
    struct if_statement_t *is = new_if_statement();
    is->e = e;
    is->s1 = s1;
    is->s2 = s2;

    return is;
}

struct index_expression_list_t *set_index_expression_list(struct expression_t *e, struct index_expression_list_t *next, struct expression_data_t *expr)
{
    struct index_expression_list_t *iel = new_index_expression_list();
    iel->e = e;
    iel->next = next;
    iel->expr = expr;

    return iel;
}

struct indexed_variable_t *set_indexed_variable(struct variable_access_t *va, struct index_expression_list_t *iel, int line_number)
{
    struct indexed_variable_t *iv = new_indexed_variable();
    iv->va = va;
    iv->iel = iel;
    
    /* We are assuming the index expression list only has one element */
    if(iel->expr != NULL)
    {
        if(!is_integer(iel->expr->type))
        {
            error_datatype_is_not(line_number, iel->expr->type, "integer");
        }
    }

    if(va->expr != NULL)
    {
        if(!is_array(va->expr->type))
        {
            error_datatype_is_not(line_number, va->expr->type, "array");
        }
    }
    return iv;
}

struct method_designator_t *set_method_designator(struct variable_access_t *va, struct function_designator_t *fd, int line_number)
{
    struct method_designator_t *md = new_method_designator();
    md->va = va;
    md->fd = fd;

    if(va->expr != NULL)
    {
        if( is_integer(va->expr->type) || is_boolean(va->expr->type) || is_real(va->expr->type) )
        {
            error_datatype_is_not(line_number, va->expr->type, "object");
        }
    }

    return md;
}

struct object_instantiation_t *set_object_instantiation(char *id, struct actual_parameter_list_t *apl)
{
    struct object_instantiation_t *oi = new_object_instantiation();
    oi->id = (char *)malloc(strlen(id));
    strcpy(oi->id, id);
    oi->apl = apl;

    return oi;
}

////////////////////////////////////////////////////////////////////////////////////////
//PRIMARY_T METHODS
////////////////////////////////////////////////////////////////////////////////////////

struct primary_t *set_primary_t_variable_access(struct variable_access_t *va)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_VARIABLE_ACCESS;
    p->data.va = va;
    /*This will be evaluated after parse by finding the type of the variable access*/

    return p;
}

struct primary_t* set_primary_t_unsigned_constant(struct unsigned_number_t *un)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_UNSIGNED_CONSTANT;
    p->data.un = un;
    p->expr = un->expr; /*This will set the value to the int val and the type to int*/

    return p;
}

struct primary_t* set_primary_t_function_designator(struct function_designator_t *fd)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_FUNCTION_DESIGNATOR;
    p->data.fd = fd;
    /*We do not know what the return val for this function is must eval after parse
     this will be the function return value of fd->id (find is hash table)*/

    return p;
}

struct primary_t* set_primary_t_expression(struct expression_t *e)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_EXPRESSION;
    p->data.e = e;
    p->expr = e->expr; /*the type of this primary is whatever the expression type is*/

    return p;
}

struct primary_t* set_primary_t_primary(struct primary_t *p, int line_number)
{
    /*This must be a boolean type because this was preset with a NOT*/
    struct primary_t *pr = new_primary();
    pr->type = PRIMARY_T_PRIMARY;
    pr->data.p = *(set_primary_data(p));

    if(p->expr != NULL)
    {
        if(strcmp(p->expr->type, "boolean") == 0)
        {
            /*we are good, this primary is a boolean perform NOT*/
            pr->expr = set_expression_data((p->expr->val)? 0:1, p->expr->type);
        }
        else
        {
            error_datatype_is_not(line_number, p->expr->type, "boolean");
            
            /*set the type to the correct value in order to continue eval*/
            pr->expr = set_expression_data(0, "boolean");
        }
    }
    
    /*if the expression was not set, then we need to wait until after parsing to eval*/

    return pr;
}

////////////////////////////////////////////////////////////////////////////////////////

struct primary_data_t* set_primary_data(struct primary_t *next)
{
    struct primary_data_t *pd = new_primary_data();
    pd->not = 1;
    pd->next = next;

    return pd;
}

struct print_statement_t *set_print_statement(struct variable_access_t *va)
{
    struct print_statement_t *ps = new_print_statement();
    ps->va = va;

    return ps;   
}

struct program_t *set_program(struct program_heading_t *ph, struct class_list_t *cl)
{
    printf("CREATING PROGRAM\n");
    struct program_t *p = new_program();
    p->ph = ph;
    p->cl = cl;

    return p;   
}

struct program_heading_t *set_program_heading(char *id, struct identifier_list_t *il)
{
    struct program_heading_t *ph = new_program_heading();
    ph->id = (char *)malloc(strlen(id));
    strcpy(ph->id, id);
    ph->il = il;

    return ph;   
}

struct range_t *set_range(struct unsigned_number_t *min, struct unsigned_number_t *max, int line_number)
{
    if(min->ui > max->ui)
    {
        error_array_range_invalid(line_number, min->ui, max->ui);
    }
    struct range_t *r = new_range();
    r->min = min;
    r->max = max;

    return r;
}

int * set_sign(int sign)
{
  int * s = (int *)malloc(sizeof(int));
  *s = sign;

  return s;
}

float simple_expression_relop(struct simple_expression_t *se1, int relop, struct simple_expression_t *se2)
{
    return -1;
}

struct simple_expression_t *set_simple_expression(struct term_t *t, int addop, struct simple_expression_t *next, int line_number)
{
    struct simple_expression_t *se = new_simple_expression();
    se->t = t;
    se->addop = addop;
    se->next = next;

    if(addop == ADDOP_NONE) /*this is just a term*/
    {
        se->expr = t->expr;
    }
    else if(t->expr != NULL && next->expr != NULL) /*if both expr are populated we can eval*/
    {
        char *t_type = t->expr->type;
        char *n_type = next->expr->type;

        switch(addop)
        {
            case ADDOP_PLUS:
            case ADDOP_MINUS: /*only valid for real and integer*/
                if((is_integer(t_type) || is_real(t_type)) &&
                    (is_integer(n_type) || is_real(n_type)))
                {
                    if(is_real(t_type) || is_real(n_type))
                    {
                        /*one is real so expr must be real*/
                        if(addop == ADDOP_PLUS)
                        {
                            se->expr = set_expression_data(next->expr->val + t->expr->val, "real");
                        }
                        else
                        {
                            se->expr = set_expression_data(next->expr->val - t->expr->val, "real");
                        }   
                    }
                    else
                    {
                        if(addop == ADDOP_PLUS)
                        {
                            se->expr = set_expression_data(next->expr->val + t->expr->val, "integer");
                        }
                        else
                        {
                            se->expr = set_expression_data(next->expr->val - t->expr->val, "integer");
                        }
                    }
                }
                else
                {
                    if(!(is_integer(t_type) || is_real(t_type)))
                    {
                        error_datatype_is_not(line_number, t_type, "real or integer");
                    }
                    if(!(is_integer(n_type) || is_real(n_type)))
                    {
                        error_datatype_is_not(line_number, n_type, "real or integer");
                    }

                    /*set to int so we can continue parsing*/
                    se->expr = set_expression_data(-1, "integer");
                }
            break;
            case ADDOP_OR: /*only valid for boolean*/
                if(is_boolean(t_type) && is_boolean(n_type))
                {
                    se->expr = set_expression_data((int)next->expr->val | (int)t->expr->val, "boolean");
                }
                else
                {
                    if(!is_boolean(t_type))
                    {
                        error_datatype_is_not(line_number, t_type, "boolean");
                    }
                    if(!is_boolean(n_type))
                    {
                        error_datatype_is_not(line_number, n_type, "boolean");
                    }

                    /*set to int so we can continue parsing*/
                    se->expr = set_expression_data(0, "boolean");
                }
            break;
        }
    }

    return se;
}

////////////////////////////////////////////////////////////////////////////////////////
//STATEMENT_T METHODS
////////////////////////////////////////////////////////////////////////////////////////

struct statement_t *set_statement_assignment(struct assignment_statement_t *as, int line_number)
{
    struct statement_t *s = new_statement();
    s->type = STATEMENT_T_ASSIGNMENT;
    s->data.as = as;
    s->line_number = line_number;

    return s;
}

struct statement_t *set_statement_statement_sequence(struct statement_sequence_t *ss, int line_number)
{
    struct statement_t *s = new_statement();
    s->type = STATEMENT_T_SEQUENCE;
    s->data.ss = ss;
    s->line_number = line_number;

    return s;
}

struct statement_t *set_statement_if(struct if_statement_t *is, int line_number)
{
    struct statement_t *s = new_statement();
    s->type = STATEMENT_T_IF;
    s->data.is = is;
    s->line_number = line_number;

    return s;
}

struct statement_t *set_statement_while(struct while_statement_t *ws, int line_number)
{
    struct statement_t *s = new_statement();
    s->type = STATEMENT_T_WHILE;
    s->data.ws = ws;
    s->line_number = line_number;

    return s;
}

struct statement_t *set_statement_print(struct print_statement_t *ps, int line_number)
{
    struct statement_t *s = new_statement();
    s->type = STATEMENT_T_PRINT;
    s->data.ps = ps;
    s->line_number = line_number;

    return s;
}

////////////////////////////////////////////////////////////////////////////////////////

struct statement_sequence_t *set_statement_sequence(struct statement_t *s, struct statement_sequence_t *next, int line_number)
{
    struct statement_sequence_t *ss = new_statement_sequence();
    ss->s = s;
    ss->next = next;
    ss->line_number = line_number;

    return ss;
}

struct term_t *set_term(struct factor_t *f, int mulop, struct term_t *term, int line_number)
{
    struct term_t *t = new_term();
    t->f = f;
    t->mulop = mulop;
    t->next = term;

    if(mulop == MULOP_NONE)
    {
        t->expr = f->expr; /*no next term, this is just a factor*/
    }
    /*if both expr are populated we can analyze this one*/
    else if(term->expr != NULL && f->expr != NULL)
    {
        char * f_type = f->expr->type;
        char * t_type = term->expr->type;

        /*analyze how to populate expr*/
        switch(mulop)
        {
            case MULOP_STAR: /*only valid for real and integer*/
            case MULOP_SLASH:
                   /*we know the type of both sides, therefore we can analyze*/
                   if((is_real(t_type) || is_integer(t_type)) &&  
                      (is_real(f_type) || is_integer(f_type)))
                      {
                        /*They are both either an integer or a real*/
                        
                        if(is_real(t_type) || is_real(f_type))
                        {
                            /*one is a real therefore new term will eval to real*/
                            if(mulop == MULOP_STAR)
                            {
                                /*multiply*/
                                t->expr = set_expression_data(f->expr->val * term->expr->val, "real");
                            }
                            else
                            {
                                /*divide*/
                                t->expr = set_expression_data(f->expr->val/term->expr->val, "real");   
                            }
                        }
                        /*both must be integers*/
                        else
                        {
                            if(mulop == MULOP_STAR)
                            {
                                /*multiply*/
                                t->expr = set_expression_data(f->expr->val * term->expr->val, "integer");
                            }
                            else
                            {
                                /*divide*/
                                t->expr = set_expression_data(f->expr->val/term->expr->val, "integer");   
                            }
                        }
                      }
                      else
                      {
                        /*bad type error*/
                        if(!(is_real(f_type) || is_integer(f_type)))
                        {
                            error_datatype_is_not(line_number, f_type, "real or integer");
                        }
                        if(!(is_real(t_type) || is_integer(t_type)))
                        {
                            error_datatype_is_not(line_number, t_type, "real or integer");
                        }

                        /*set to int to continue eval*/
                        t->expr = set_expression_data(-1, "integer");
                      } 
            break;
            case MULOP_MOD: /*only valid for integer*/
                if(is_integer(f_type) && is_integer(t_type))
                {
                    t->expr = set_expression_data((int)term->expr->val % (int)f->expr->val, "integer");
                }
                else
                {
                    if(!is_integer(f_type))
                    {
                        error_datatype_is_not(line_number, f_type, "integer");
                    }
                    if(!is_integer(t_type))
                    {
                        error_datatype_is_not(line_number, t_type, "integer");
                    }
                    /*set to int to continue eval*/
                    t->expr = set_expression_data(-1, "integer");
                }

            break;
            case MULOP_AND: /*only valid for boolean*/
                if(is_boolean(f_type) && is_boolean(t_type))
                {
                    t->expr = set_expression_data((int)term->expr->val & (int)f->expr->val, "boolean");
                }
                else
                {
                    if(!is_boolean(f_type))
                    {
                        error_datatype_is_not(line_number, f_type, "boolean");
                    }
                    if(!is_boolean(t_type))
                    {
                        error_datatype_is_not(line_number, t_type, "boolean");
                    }
                    /*set to int to continue eval*/
                    t->expr = set_expression_data(-1, "boolean");
                }

            break;
        }
    }

    return t;
}

////////////////////////////////////////////////////////////////////////////////////////
//TYPE_DENOTER_T METHODS
////////////////////////////////////////////////////////////////////////////////////////

struct type_denoter_t *set_type_denoter_array(char *name, struct array_type_t *at)
{
    struct type_denoter_t *td = new_type_denoter();
    td->type = TYPE_DENOTER_T_ARRAY_TYPE;
    
    td->name = (char *)malloc(strlen(name));
    strcpy(td->name, name);
    
    td->data.at = at;

    return td;
}

struct type_denoter_t *set_type_denoter_class(char *name, struct class_list_t *cl)
{
  struct type_denoter_t *td = new_type_denoter();
  td->type = TYPE_DENOTER_T_CLASS_TYPE;

  td->name = (char *)malloc(strlen(name));
  strcpy(td->name, name);

  td->data.cl = cl;

  return td;
}

struct type_denoter_t *set_type_denoter_id(char *name, char *id)
{
    struct type_denoter_t *td = new_type_denoter();
    td->type = TYPE_DENOTER_T_IDENTIFIER;
    
    td->name = (char *)malloc(strlen(name));
    strcpy(td->name, name);

    td->data.id = (char *)malloc(strlen(id));
    strcpy(td->data.id, id);

    return td; 
}

////////////////////////////////////////////////////////////////////////////////////////

struct unsigned_number_t *set_unsigned_number(int ui, struct expression_data_t* expr)
{
    struct unsigned_number_t *un = new_unsigned_number();
    un->ui = ui;
    un->expr = expr;

    return un;
}

////////////////////////////////////////////////////////////////////////////////////////
//VARIABLE_ACCESS_T METHODS
////////////////////////////////////////////////////////////////////////////////////////

struct variable_access_t *set_variable_access_id(char *id, char *recordname, struct expression_data_t *expr)
{
    struct variable_access_t *va = new_variable_access();
    va->type = VARIABLE_ACCESS_T_IDENTIFIER;
    va->data.id = (char *)malloc(strlen(id)*sizeof(char));
    strcpy(va->data.id, id);
    
    /* Info about this id will be known after parsing */
    return va;
}

struct variable_access_t *set_variable_access_indexed_variable(struct indexed_variable_t *iv, char *record_name, struct expression_data_t *expr)
{
  struct variable_access_t *va = new_variable_access();
    va->type = VARIABLE_ACCESS_T_INDEXED_VARIABLE;
    va->data.iv = iv;
    va->recordname = record_name;
    va->expr = expr;

    return va;
}

struct variable_access_t *set_variable_access_attribute_designator(struct attribute_designator_t *ad)
{
    struct variable_access_t *va = new_variable_access();
    va->type = VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR;
    va->data.ad = ad;

    return va;  
}

struct variable_access_t *set_variable_access_method_designator(struct method_designator_t *md, char *recordname, struct expression_data_t *expr)
{
    struct variable_access_t *va = new_variable_access();
    va->type = VARIABLE_ACCESS_T_METHOD_DESIGNATOR;
    va->data.md = md;
    va->recordname = recordname;
    va->expr = expr;

    return va;
}

////////////////////////////////////////////////////////////////////////////////////////

struct variable_declaration_t *set_variable_declaration(struct identifier_list_t *il, struct type_denoter_t *tden, int line_number)
{
    struct variable_declaration_t *vd = new_variable_declaration();
    vd->il = il;
    vd->tden = tden;
    vd->line_number = line_number;

    return vd;
}

struct variable_declaration_list_t *set_variable_declaration_list(struct variable_declaration_t *vd, struct variable_declaration_list_t *next)
{
    struct variable_declaration_list_t *vdl = new_variable_declaration_list();
    vdl->vd = vd;
    vdl->next = next;

    return vdl;
}

struct while_statement_t *set_while_statement(struct expression_t *e, struct statement_t *s)
{
    
    struct while_statement_t *ws = new_while_statement();
    ws->e = e;
    ws->s = s;

    return ws;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/* Helper method implemenatations (Hello again Dr. Bazzi) */
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void create_statement_hash_table(struct func_declaration_list_t *func_dec_list)
{
    while(func_dec_list != NULL)
    {
        add_statement_list_to_sht(func_dec_list->fd);

        func_dec_list = func_dec_list->next;
    }

}

void add_statement_list_to_sht(struct function_declaration_t *func_dec)
{
    struct statement_sequence_t *stat_seq = func_dec->fb->ss;
    while(stat_seq != NULL)
    {
        struct statement_table_t *statement = create_statement_node(stat_seq->s, func_dec, stat_seq->line_number);
        add_statement_to_hash_table(statement);

        stat_seq = stat_seq->next;
    }
}



void create_attribute_hash_table(struct func_declaration_list_t *func_dec_list, struct variable_declaration_list_t *var_dec_list)
{
    /* dummy function dec to use in the key of NFV nodes */
    struct function_declaration_t *NFV_dummy_func_dec = (struct function_declaration_t*)(malloc(sizeof(NFV_dummy_func_dec)));
    
    add_class_attrs_to_aht(var_dec_list, SCOPE_NFV, NFV_dummy_func_dec);
    add_class_funcs_to_aht(func_dec_list, NFV_dummy_func_dec);
}

void add_class_attrs_to_aht(struct variable_declaration_list_t *var_dec_list, int type, struct function_declaration_t *dummy_func_dec)
{
    while(var_dec_list != NULL)
    {
        parse_var_dec(var_dec_list->vd, SCOPE_NFV, dummy_func_dec);
        var_dec_list = var_dec_list->next;
    }
}

void parse_param_section(struct formal_parameter_section_t *param_section, int scope, struct function_declaration_t *func)
{
    struct identifier_list_t *id_list = param_section->il;
    while(id_list != NULL)
    {
        if(strcmp(id_list->id, func->fh->id) != 0)
        {
            struct type_denoter_t *type = generate_type_denoter(param_section->id);
            struct attribute_table_t *var = create_attribute_node(id_list->id,
                                                                  type,
                                                                  func->line_number,
                                                                  scope,
                                                                  FALSE,
                                                                  NULL,
                                                                  func);

            add_attribute_to_hash_table(var, ENTITY_VARIABLE);
        }
        else
        {
            error_variable_name_invalid(func->line_number, id_list->id);
        }

        id_list = id_list->next;
    }
}

void parse_var_dec(struct variable_declaration_t *var_dec, int scope, struct function_declaration_t *func)
{
    struct identifier_list_t *id_list = var_dec->il;
    while(id_list != NULL)
    {
        char *func_id = "";
        if(func->fh != NULL)
        {
            func_id = func->fh->id;
        }
        if(strcmp(id_list->id, func_id) != 0)
        {
            struct attribute_table_t *var = create_attribute_node(id_list->id,
                                                                  var_dec->tden,
                                                                  var_dec->line_number,
                                                                  scope,
                                                                  FALSE,
                                                                  NULL,
                                                                  func);
            add_attribute_to_hash_table(var, ENTITY_VARIABLE);
        }
        else
        {
            error_variable_name_invalid(var_dec->line_number, id_list->id);
        }
        id_list = id_list->next;
    }
}

void add_class_funcs_to_aht(struct func_declaration_list_t *func_dec_list, struct function_declaration_t *dummy_func_dec)
{
    while(func_dec_list != NULL)
    {
        struct type_denoter_t *type = generate_type_denoter(func_dec_list->fd->fh->res);
        struct attribute_table_t *func = create_attribute_node(func_dec_list->fd->fh->id,
                                                               type,
                                                               func_dec_list->fd->line_number,
                                                               SCOPE_NFV,
                                                               TRUE,
                                                               func_dec_list->fd->fh->fpsl,
                                                               dummy_func_dec);
        
        add_attribute_to_hash_table(func, ENTITY_FUNCTION);

        add_func_var_to_aht(func_dec_list->fd->fb->vdl, SCOPE_FV, func_dec_list->fd);
        
        add_func_params_to_aht(func_dec_list->fd->fh->fpsl, SCOPE_FV, func_dec_list->fd);

        func_dec_list = func_dec_list->next;
    }
}

void add_func_params_to_aht(struct formal_parameter_section_list_t *param_list, int scope, struct function_declaration_t *func)
{
    while(param_list != NULL)
    {
        parse_param_section(param_list->fps, scope, func);
        
        param_list = param_list->next;
    }
}


void add_func_var_to_aht(struct variable_declaration_list_t *var_dec_list, int scope, struct function_declaration_t *func)
{
    while(var_dec_list != NULL)
    {
        parse_var_dec(var_dec_list->vd, scope, func);
        var_dec_list = var_dec_list->next;
    }
}

struct type_denoter_t* generate_type_denoter(char* return_type)
{
    struct type_denoter_t *type = (struct type_denoter_t*)malloc(sizeof(struct type_denoter_t));
    if( !(strcmp(return_type, "integer") || strcmp(return_type, "real") || strcmp(return_type, "boolean")) )
    {
        type->type = TYPE_DENOTER_T_IDENTIFIER;
    }
    else
    {
        type->type = -1;
    }

    type->name = (char*)malloc(strlen(return_type)*sizeof(char));
    strcpy(type->name, return_type);
    return type;
}

void add_statement_to_hash_table(struct statement_table_t *statement)
{
    struct statement_table_t *item_ptr;
    HASH_FIND_STR(stat_hash_table, statement->string_key, item_ptr);
    if(item_ptr == NULL)
    {
        HASH_ADD_STR(stat_hash_table, string_key, statement);
    }   
}


void add_attribute_to_hash_table(struct attribute_table_t *attr, int entity_type)
{
    check_against_reserved_words(attr->id, attr->line_number, entity_type);
    struct attribute_table_t *item_ptr = NULL;
    
    HASH_FIND_STR(attr_hash_table, attr->string_key, item_ptr);
    if(item_ptr == NULL)
    {
        HASH_ADD_STR(attr_hash_table, string_key, attr);
    }
    else
    {
        attribute_hash_table_error(item_ptr, attr);
    }
}

void check_against_reserved_words(char* id, int line_number, int entity_type)
{
    if( !(strcmp(id, "this") || strcmp(id, "integer") || strcmp(id, "real") || strcmp(id, "boolean")) )
    {
        switch(entity_type)
        {
            case ENTITY_CLASS:
                error_class_name_invalid(line_number, id);
                break;
            case ENTITY_FUNCTION:
                error_function_name_invalid(line_number, id);
                break;
            case ENTITY_VARIABLE:
                error_variable_name_invalid(line_number, id);
                break;
        }
        
    }
}

void attribute_hash_table_error(struct attribute_table_t *item_ptr, struct attribute_table_t *failed_attr)
{
    if(item_ptr->is_func)
    {
        error_function_already_declared(failed_attr->line_number, failed_attr->id, item_ptr->line_number);
    }
    else
    {
        error_variable_already_declared(failed_attr->line_number, failed_attr->id, item_ptr->line_number);   
    }
}

struct statement_table_t* create_statement_node(struct statement_t *stat, struct function_declaration_t *function, int line_number)
{
    struct statement_table_t *statement = (struct statement_table_t*)malloc(sizeof(struct statement_table_t));
    
    statement->type = stat->type;
    statement->line_number = line_number;
    statement->function = function;
    statement->statement_data = (union statement_union*)malloc(sizeof(stat->data));
    statement->statement_data = &stat->data;

    char *key = (char*)malloc(strlen(statement->function->fh->id) + strlen("1") + sizeof(char *));
    strcpy(key, statement->function->fh->id);

    switch(statement->type)
    {
        case STATEMENT_T_ASSIGNMENT:
            strcat(key, "1");
            break;
        case STATEMENT_T_SEQUENCE:
            strcat(key, "2");
            break;
        case STATEMENT_T_IF:
            strcat(key, "3");
            break;
        case STATEMENT_T_WHILE:
            strcat(key, "4");
            break;
        case STATEMENT_T_PRINT:
            strcat(key, "5");
            break;
    }

    strcat(key, (char *)statement->statement_data);

    strcpy(statement->string_key, key);

    printf("and this is the sketchy key: %s\n", statement->string_key);

    return statement;
}

struct attribute_table_t* create_attribute_node(char* id,
                                                struct type_denoter_t *type,
                                                int line_number, 
                                                int scope, 
                                                int is_func, 
                                                struct formal_parameter_section_list_t *params, 
                                                struct function_declaration_t *function)
{
        struct attribute_table_t *func = (struct attribute_table_t*)malloc(sizeof(struct attribute_table_t));
        strncpy(func->id, id, strlen(id));
        

        func->id_length = strlen(id);
        func->type = type;
        func->line_number = line_number;
        func->scope = scope;
        func->is_func = is_func;
        func->params = params;
        func->function = function;

        int func_id_length  = 0;
        if(function->fh != NULL)
        {
            func_id_length = strlen(function->fh->id);
        }
        char *key = (char *)malloc(strlen(id) + strlen("1") + func_id_length);
        strcpy(key, id);
        
        if(scope)
        {
            strcat(key, "1");
        }
        else
        {
            strcat(key, "0");
        }
        
        if(func_id_length)
        {
            strcat(key, function->fh->id);    
        }
        
        strcpy(func->string_key, key);

        return func;
}

char* format_attr_id(char id[], int id_length)
{
    int i;
    char *real_id = (char*)malloc(id_length+1);
    for(i=0;i<id_length;i++)
    {
        real_id[i] = id[i];
    }

    return real_id;
}

void print_statement_hash_table(struct statement_table_t *stat)
{
    struct statement_table_t *t;
    printf("/////////////////STATEMENT HAST TABLE/////////////////////\n");

    for(t=stat; t!=NULL; t= t->hh.next)
    {
        printf("STATEMENT TYPE: %d LINE_NUMBER: %d FUNCTION: %s DATA PTR: %p\n", 
                t->type, t->line_number, t->function->fh->id, t->statement_data);

    }
    printf("///////////////////////////////////////////////////////////\n");
}

void print_hash_table(struct attribute_table_t* attr)
{
    struct attribute_table_t *t;
    printf("//////////////ATTRIBUTE HASH TABLE//////////////\n");
                   
    for(t=attr; t != NULL; t=t->hh.next) 
    {
        printf("TYPE: %s LINE NUMBER %d SCOPE: %d IS FUNCTION: %d ID: %s ", t->type->name, t->line_number, t->scope, t->is_func, format_attr_id(t->id, t->id_length));
        if(t->function->fh != NULL)
        {
            printf("FUNCTION NAME: %s ", t->function->fh->id);
        }
        if(t->params != NULL)
        {
            printf(" PARAMETERS: ");
            struct formal_parameter_section_list_t *p;
            for(p=t->params; p != NULL; p=p->next)
            {
                struct identifier_list_t *il;
                for(il=p->fps->il; il != NULL; il=il->next)
                {
                    printf("%s, ", il->id);
                }
                printf(" TYPE: %s ", p->fps->id);
            }
            
        }
        printf("\n");
    }
    printf("/////////////////////////////////\n\n");
}

int is_boolean(char *id)
{
    return !(strcmp(id, "boolean"));
}

int is_integer(char *id)
{
    return !(strcmp(id, "integer"));
}

int is_real(char *id)
{
    return !(strcmp(id, "real"));
}

int is_array(char *id)
{
    return !(strcmp(id, "array"));
}



