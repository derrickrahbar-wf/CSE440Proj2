/* shared.c
 *
 * Implements all shared subroutines
 */

#include "shared.h"
#include "rulefuncs.h"


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

struct attribute_designator_t *set_attribute_designator(struct variable_access_t *va, char *id)
{
    struct attribute_designator_t *ad = new_attribute_designator();
    ad->va = va;
    ad->id = (char *)malloc(strlen(id));
    strcpy(ad->id, id);

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

struct expression_t *set_expression(struct simple_expression_t* se1, int relop, struct simple_expression_t* se2, struct expression_data_t *expr)
{
    struct expression_t *e = new_expression();
    e->se1 = se1;
    e->relop = relop;
    e->se2 = se2;
    e->expr = expr;

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

struct factor_t* set_factor_t_sign_factor(struct factor_data_t* f, struct expression_data_t* expr)
{
    struct factor_t *fsf = new_factor();
    fsf->type = FACTOR_T_SIGNFACTOR;
    fsf->data.f = *f;
    fsf->expr = expr;

    return fsf;

}

struct factor_t* set_factor_t_primary(struct primary_t* p, struct expression_data_t* expr)
{
    struct factor_t *fp = new_factor();
    fp->type = FACTOR_T_PRIMARY;
    fp->data.p = p;
    fp->expr = expr;

    return fp;
}

////////////////////////////////////////////////////////////////////////////////////////

struct factor_data_t* set_factor_data(int* sign, struct factor_t* next)
{
    struct factor_data_t* fd = new_factor_data();
    fd->sign = (int *)malloc(sizeof(int));
    fd->sign = sign;
    fd->next = next;

    return fd;
}

struct formal_parameter_section_t *set_formal_parameter_section(struct identifier_list_t *il, char *id, int is_var)
{
    struct formal_parameter_section_t *fps = new_formal_parameter_section();
    fps->il = il;
    fps->id = (char *)malloc(strlen(id));
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
    fd->id = (char *)malloc(strlen(id));
    strcpy(fd->id, id);
    fd->apl = apl;

    return fd;
}

struct function_heading_t *set_function_heading(char *id, char *res, struct formal_parameter_section_list_t *fpsl)
{
    struct function_heading_t *fh = new_function_heading();
    fh->id = (char *)malloc(strlen(id));
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

struct indexed_variable_t *set_indexed_variable(struct variable_access_t *va, struct index_expression_list_t *iel, struct expression_data_t *expr)
{
    struct indexed_variable_t *iv = new_indexed_variable();
    iv->va = va;
    iv->iel = iel;
    iv->expr = expr;

    return iv;
}

struct method_designator_t *set_method_designator(struct variable_access_t *va, struct function_designator_t *fd)
{
    struct method_designator_t *md = new_method_designator();
    md->va = va;
    md->fd = fd;

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

struct primary_t *set_primary_t_variable_access(struct variable_access_t *va, struct expression_data_t *expr)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_VARIABLE_ACCESS;
    p->data.va = va;
    p->expr = expr;

    return p;
}

struct primary_t* set_primary_t_unsigned_constant(struct unsigned_number_t *un, struct expression_data_t *expr)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_UNSIGNED_CONSTANT;
    p->data.un = un;
    p->expr = expr;

    return p;
}

struct primary_t* set_primary_t_function_designator(struct function_designator_t *fd, struct expression_data_t *expr)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_FUNCTION_DESIGNATOR;
    p->data.fd = fd;
    p->expr = expr;

    return p;
}

struct primary_t* set_primary_t_expression(struct expression_t *e, struct expression_data_t *expr)
{
    struct primary_t *p = new_primary();
    p->type = PRIMARY_T_EXPRESSION;
    p->data.e = e;
    p->expr = expr;

    return p;
}

struct primary_t* set_primary_t_primary(struct primary_data_t *p, struct expression_data_t *expr)
{
    struct primary_t *pr = new_primary();
    pr->type = PRIMARY_T_PRIMARY;
    pr->data.p = *p;
    pr->expr = expr;

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

struct range_t *set_range(struct unsigned_number_t *min, struct unsigned_number_t *max)
{
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

struct simple_expression_t *set_simple_expression(struct term_t *t, int addop, struct expression_data_t *expr, struct simple_expression_t *next)
{
    struct simple_expression_t *se = new_simple_expression();
    se->t = t;
    se->addop = addop;
    se->expr = expr;
    se->next = next;

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

struct statement_sequence_t *set_statement_sequence(struct statement_t *s, struct statement_sequence_t *next)
{
    struct statement_sequence_t *ss = new_statement_sequence();
    ss->s = s;
    ss->next = next;

    return ss;
}

struct term_t *set_term(struct factor_t *f, int mulop, struct expression_data_t* expr, struct term_t *next)
{
    struct term_t *t = new_term();
    t->f = f;
    t->mulop = mulop;
    t->expr = expr;
    t->next = next;

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
    va->data.id = (char *)malloc(strlen(id));
    strcpy(va->data.id, id);
    va->recordname = recordname;
    va->expr = expr;

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

struct variable_access_t *set_variable_access_attribute_designator(struct attribute_designator_t *ad, char *recordname, struct expression_data_t *expr)
{
    struct variable_access_t *va = new_variable_access();
    va->type = VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR;
    va->data.ad = ad;
    va->recordname = recordname;
    va->expr = expr;

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

