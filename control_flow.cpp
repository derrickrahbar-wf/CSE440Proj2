extern "C"
{
	#include "shared.h"
	#include "symtab.h"
	#include "rulefuncs.h"
	#include "semantic.h"
	#include "error.h"
}
#include "control_flow.h"
#include <sstream>
#include <string>

using namespace std;

std::vector<BasicBlock*> cfg;
int current_bb = 0;
int condition_var_name = 0;
int IN3ADD_op = -1;
int extended_bb_label = 1;

std::vector<std::unordered_map<string, int>> expr_tables;
std::vector<std::unordered_map<string, Val_obj*>> id_tables;
std::vector<std::unordered_map<int, int>> const_tables;


statement_sequence_t * reverse_ss(statement_sequence_t* ss)
{
	statement_sequence_t *ss_temp1= ss, *ss_previous= NULL;

	while(ss != NULL && ss->next != NULL)
	{
		ss_temp1 = ss->next;
		ss->next = ss_previous;
		ss_previous = ss;
		ss = ss_temp1;
	}

	ss->next = ss_previous;
	return ss;
}

void print_statement_list(statement_sequence_t* ss)
{
	while(ss != NULL)
	{
		cout << ss->line_number<< endl;
		ss = ss->next;
	}
}

std::vector<BasicBlock*> create_CFG(statement_sequence_t *ss)
{
	BasicBlock *starting_block = new BasicBlock();
	cfg.push_back(starting_block);
	add_statements_to_cfg(ss);
	define_extended_bbs();
	print_CFG();

	value_numbering();

	return cfg;
}

void value_numbering()
{
	for(int i = 0; i < cfg.size(); i++)
	{
		create_tables_for_bb(i);
	}
}

void create_tables_for_bb(int cfg_index)
{
	BasicBlock *bb = cfg[cfg_index];

	for(int i=0; i < bb->parents.size(); i++)
	{
		int parent_index = bb->parents[i];
		if(cfg_index > parent_index && bb->extended_bb == cfg[parent_index]->extended_bb)
		{
			copy_parent_tables_to_child(parent_index, cfg_index);
		}
	}

	process_bb_stats_for_tables(cfg_index);
}

void copy_parent_tables_to_child(int parent_index, int child_index)
{
	for ( auto it = expr_tables[parent_index].begin(); it != expr_tables[parent_index].end(); ++it )
		expr_tables[child_index].insert(it->first, it->second);

	for ( auto it = id_tables[parent_index].begin(); it != id_tables[parent_index].end(); ++it )
	{
		Val_obj *tmp = new Val_obj();
		tmp->val_num = it->second->val_num;
		tmp->is_const = it->second->is_const;

		id_tables[child_index].insert(it->fist, tmp);
	}

	for ( auto it = const_tables[parent_index].begin(); it != const_tables[parent_index].end(); ++it )
		const_tables[child_index].insert(it->fist, it->second);
}

void process_bb_stats_for_tables(int index)
{
	BasicBlock *bb = cfg[index];

	for(int i = 0; i < bb->statements.size(); i++)
	{
		process_statement_for_tables(bb->statement[i], index);
	}
}

void process_statement_for_tables(Statement *stat, int table_index)
{
	if(stat->rhs->t2 == NULL)
	{
		process_singular_stat_for_tables(stat, table_index);
	}
	else
	{
		process_multi_stat_for_tables(stat, table_index);
	}
}

void process_multi_stat_for_tables(Statement *stat, int table_index)
{
	Term *t1 = stat->rhs->t1;
	Term *t2 = stat->rhs->t2;
	RHS *rhs = stat->rhs;

	if(t2->type == TERM_TYPE_CONST && t1->type == TERM_TYPE_CONST)
	{
		int const_val_num = calc_const_and_add_to_table(t1->data.constant, rhs->op, t2->data.constant, table_index);
		optimize_stat_with_const(stat, const_val_num, table_index);
		
		Val_obj *vo = new Val_obj();
		vo->is_const = true;
		vo->val_num = const_val_num;
		
		string key(stat->lhs);
		id_tables[table_index][key] = vo; 
	}
	else if(t2->type == TERM_TYPE_CONST || t1->type == TERM_TYPE_CONST)
	{
		int const_val_num;
		Val_obj *vo;
		if(t1->type == TERM_TYPE_CONST)
		{
			const_val_num = eval_const(t2->data.constant, table_index);
			vo = eval_id(t1->data.var, table_index);

			if(vo->is_const)
			{
				std::unordered_map<int, int>::const_iterator check = const_table_find(vo->val_num, table_index);
				int c_v_num = calc_const_and_add_to_table(t1->data.constant, rhs->op, check->first, table_index);
			}
		}
		else
		{
			const_val_num = eval_const(t1->data.constant, table_index);
			vo = eval_id(t2->data.var, table_index);	
		}
		
	}

}

void process_singular_stat_for_tables(Statement *stat, int table_index)
{
	Val_obj *valObj;
	if(stat->t1->type == TERM_TYPE_CONST)
	{
		valObj = new Val_obj();
		valObj->val_num = eval_const(stat->t1->data.constant, table_index);
		valObj->is_const = true;
	}
	else
	{
		valObj = eval_id(stat->t1->data.var, table_index);
		if(valObj->is_const)
		{
			optimize_singl_stat_with_const(stat, valObj->val_num, table_index);
		}
	}
	
	string key(stat->lhs);
	id_tables[table_index][key] = valObj;
}

void optimize_stat_with_const(Statement *stat, int val_num, int table_index)
{
	std::unordered_map<int, int>::const_iterator check = const_table_find(val_num, table_index);
	Term *t = new Term();

	t->type = TERM_TYPE_CONST;
	t->data.constant = check->first;

	stat->rhs->t2 = NULL;
	stat->rhs->op = STAT_NONE;
	stat->rhs->t1 = t;
}

std::unordered_map<int, int>::const_iterator const_table_find(int val_num, int table_index)
{
	for ( auto it = const_tables[table_index].begin(); it != const_tables[table_index].end(); ++it )
	{
		if(it->second == val_num)
			return it;
	}

	return const_tables[table_index].end();
}

void process_statement(statement_t *s)
{
	switch(s->type)
	{
		case STATEMENT_T_ASSIGNMENT:
			add_assignment_to_cfg(s->data.as);
			break;
		case STATEMENT_T_SEQUENCE:
			add_statements_to_cfg(s->data.ss);
			break;
		case STATEMENT_T_IF:
			add_if_statement_to_cfg(s->data.is);
			break;
		case STATEMENT_T_WHILE:
			add_while_statement_to_cfg(s->data.ws);
			break;
	}
}

void add_while_statement_to_cfg(while_statement_t *ws)
{
	/* New BB for the condition of the ws */
	std::vector<int> parent;
	
	if(cfg[current_bb]->statements.size() != 0)
	{
		parent.push_back(current_bb);
		add_next_bb(parent);
	}
	int condition_index = current_bb;

	/* Add that condition to the newly created BB from above */
	add_condition_to_bb(ws->e);
	
	/* Update parent vector to hold current condition bb */
	parent.clear();
	parent.push_back(current_bb);

	/* Add new BB for ws body */
	add_next_bb(parent);
	cout << "While statement type : " << ws->s->type << endl;
	process_statement(ws->s);
	
	/* Ending block of while body is set as a parent of the condition BB */
	cfg[condition_index]->parents.push_back(current_bb);
	cfg[current_bb]->children.push_back(condition_index);

	parent.clear();

	/* Create BB that joins final BB of ws body and ws condition */
	parent.push_back(condition_index);
	parent.push_back(current_bb);
	add_next_bb(parent);
}

void add_statements_to_cfg(statement_sequence_t *ss)
{
	ss = reverse_ss(ss);

	while(ss != NULL)
	{
		process_statement(ss->s);
		ss = ss->next;
	}
}

void add_assignment_to_cfg(assignment_statement_t *as)
{
	Statement *stat = new Statement();
	stat->lhs = create_id(as->va->data.id);
	stat->rhs = get_rhs_from_expr(as->e);

	cfg[current_bb]->statements.push_back(stat);
}

void add_if_statement_to_cfg(if_statement_t *ifs)
{
	int parent = current_bb;
	add_condition_to_bb(ifs->e);

	
	int if_st1_index = add_if_body_to_cfg(ifs->s1, parent);
	int if_st2_index = add_if_body_to_cfg(ifs->s2, parent);
	
	cfg[parent]->children.push_back(if_st1_index);
	cfg[parent]->children.push_back(if_st2_index);

	std::vector<int> if_statements_index;
	if_statements_index.push_back(if_st1_index);
	if_statements_index.push_back(if_st2_index);

	add_next_bb(if_statements_index);
}

void add_condition_to_bb(expression_t *expr)
{
	Statement *stat = new Statement();
	RHS *rhs = get_rhs_from_expr(expr);


	stat->lhs = create_temp_id();
	stat->rhs = rhs;

	cfg[current_bb]->statements.push_back(stat);
}

RHS* get_rhs_from_expr(expression_t *expr)
{
	RHS *rhs = new RHS();
	if(!is_3_address_code(expr))
	{
		rhs->t1 = gen_term_from_expr(expr);
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}
	else
	{
		rhs = IN3ADD_gen_rhs_from_3_add_expr(expr);
	}

	return rhs;
}

bool is_3_address_code(expression_t *expr)
{
	if(expr_term_count(expr) > 2)
	{
		return false;
	}
	return true;
}

int expr_term_count(expression_t *expr)
{
	if(expr == NULL)
	{
		return 0;
	}
	return se_term_count(expr->se1) + se_term_count(expr->se2);
}

int se_term_count(simple_expression_t *se)
{
	if(se == NULL)
	{
		return 0;
	}

	return se_term_count(se->next) + term_term_count(se->t);
}

int term_term_count(term_t *t)
{
	if(t == NULL)
	{
		return 0;
	}
	return factor_term_count(t->f) + term_term_count(t->next);
}

int factor_term_count(factor_t *f)
{
	if(f == NULL)
	{
		return 0;
	}

	switch(f->type)
	{
		case FACTOR_T_SIGNFACTOR:
			return factor_term_count(f->data.f->next);
			break;
		case FACTOR_T_PRIMARY:
			return primary_term_count(f->data.p);
			break;
	}

	error_unknown(-1);
	return -1;
}

int primary_term_count(primary_t *p)
{
	if(p == NULL)
	{
		return 0;
	}

	switch(p->type)
	{
		case PRIMARY_T_VARIABLE_ACCESS:
		case PRIMARY_T_UNSIGNED_CONSTANT:
			return 1;
			break;
		case PRIMARY_T_EXPRESSION:
			return expr_term_count(p->data.e);
			break;
	}
	
	error_unknown(-1);
	return -1;
}

/* Expression is longer than 3 address base orignally. Must be split up */
Term* gen_term_from_expr(expression_t *expr)
{
	Term *se1_term = gen_term_from_se(expr->se1);
	
	if(expr->se2 == NULL)
	{
		return se1_term;
	}

	RHS *rhs = new RHS();

	rhs->t1 = se1_term;
	rhs->op = relop_to_statop(expr->relop);
	rhs->t2 = gen_term_from_se(expr->se2);

	char *lhs = create_and_insert_stat(rhs);
	
	return create_temp_term(lhs);
}

Term* gen_term_from_se(simple_expression_t *se)
{
	Term *t_term = gen_term_from_term(se->t);

	if(se->next == NULL)
	{
		return t_term;
	}

	RHS *rhs = new RHS();

	rhs->t1 = gen_term_from_se(se->next);
	rhs->op = addop_to_statop(se->addop);
	rhs->t2 = t_term;

	char *lhs = create_and_insert_stat(rhs);

	return create_temp_term(lhs);
}

Term* gen_term_from_term(term_t *t)
{
	Term *f_term = gen_term_from_factor(t->f);

	if(t->next == NULL)
	{
		return f_term;
	}

	RHS *rhs = new RHS();

	rhs->t1 = gen_term_from_term(t->next);
	rhs->op = mulop_to_statop(t->mulop);
	rhs->t2 = f_term;

	char *lhs = create_and_insert_stat(rhs);

	return create_temp_term(lhs);
}

Term* gen_term_from_factor(factor_t *f)
{
	factor_data_t *f_data;
	switch(f->type)
	{
		case FACTOR_T_SIGNFACTOR:
			f_data = f->data.f;
			cout << "WE ARE 321 with sign: " << f_data->sign << endl;
			if(f_data->sign == SIGN_PLUS)
			{
				return gen_term_from_factor(f_data->next);
			}
			else
			{
				return create_negative_factor_term(f_data->next);
			}
			break;

		case FACTOR_T_PRIMARY:
			return gen_term_from_primary(f->data.p);
			break;
	}

	error_unknown(-1);
	return NULL;
}

Term* gen_term_from_primary(primary_t *p)
{	
	Term *t;
	switch(p->type)
	{
		case PRIMARY_T_VARIABLE_ACCESS:
			t = new Term();
			t->type = TERM_TYPE_VAR;
			t->data.var = create_id(p->data.va->data.id);
			return t;
			break;
		
		case PRIMARY_T_UNSIGNED_CONSTANT:
			t = new Term();
			t->type = TERM_TYPE_CONST;
			t->data.constant = p->data.un->ui;
			return t;	
			break;
		
		case PRIMARY_T_EXPRESSION:
			return gen_term_from_expr(p->data.e);
			break;
	}

	error_unknown(-1);
	return NULL;
}

Term* create_negative_factor_term(factor_t *f)
{
	Term *t1 = new Term();
	t1->sign = STAT_SIGN_NEGATIVE;
	t1->data.constant = 1;
	t1->type = TERM_TYPE_CONST;

	RHS *rhs = new RHS();
	rhs->t1 = t1;
	rhs->op = STAT_STAR;
	rhs->t2 = gen_term_from_factor(f);

	char *lhs = create_and_insert_stat(rhs);

	return create_temp_term(lhs);
}

/* This expression is in 3 address code, parse through to create rhs */
RHS* IN3ADD_gen_rhs_from_3_add_expr(expression_t *expr)
{
	RHS *rhs = new RHS();
	
	std::vector<Term*> terms = IN3ADD_get_terms_from_expr(expr);

	rhs->t1 = terms[0];
	rhs->t2 = terms[1];
	rhs->op = IN3ADD_op;
	IN3ADD_op = -1;

	return rhs;
}

std::vector<Term*> IN3ADD_get_terms_from_expr(expression_t *expr)
{
	std::vector<Term*> terms;
	terms = IN3ADD_get_terms_from_se(expr->se1);
	
	if(expr->se2 != NULL)
	{
		IN3ADD_op = relop_to_statop(expr->relop);
		terms.push_back(IN3ADD_get_terms_from_se(expr->se2)[0]);
	}

	return terms;
}

std::vector<Term*> IN3ADD_get_terms_from_se(simple_expression_t *se)
{
	std::vector<Term*> terms;
	std::vector<Term*> se_term = IN3ADD_get_terms_from_term(se->t);
	

	if(se->next != NULL)
	{
		IN3ADD_op = addop_to_statop(se->addop);
		terms = IN3ADD_get_terms_from_se(se->next);
	}

	terms.insert(terms.end(), se_term.begin(), se_term.end());

	return terms;
}

std::vector<Term*> IN3ADD_get_terms_from_term(term_t *t)
{
	std::vector<Term*> terms;
	std::vector<Term*> term_factor = IN3ADD_get_terms_from_factor(t->f);
	
	if(t->next != NULL)	
	{
		IN3ADD_op = mulop_to_statop(t->mulop);
		terms = IN3ADD_get_terms_from_term(t->next);
	}

	terms.insert(terms.end(), term_factor.begin(), term_factor.end());

	return terms;
}

std::vector<Term*> IN3ADD_get_terms_from_factor(factor_t *f)
{
	std::vector<Term*> terms;

	switch(f->type)
	{
		case FACTOR_T_SIGNFACTOR:
			terms = IN3ADD_get_terms_from_factor(f->data.f->next);
			terms[0]->sign = ((f->data.f->sign == SIGN_PLUS) ? STAT_SIGN_POSITIVE : STAT_SIGN_NEGATIVE);
			break;

		case FACTOR_T_PRIMARY:
			terms = IN3ADD_get_terms_from_primary(f->data.p);
			break;
	}

	return terms;
}

std::vector<Term*> IN3ADD_get_terms_from_primary(primary_t *p)
{
	std::vector<Term*> terms;
	Term *t;
	switch(p->type)
	{
		case PRIMARY_T_VARIABLE_ACCESS:
			t = new Term();
			t->type = TERM_TYPE_VAR;
			t->data.var = create_id(p->data.va->data.id);
			terms.push_back(t);
			break;

		case PRIMARY_T_UNSIGNED_CONSTANT:
			t = new Term();
			t->type = TERM_TYPE_CONST;
			t->data.constant = p->data.un->ui;
			terms.push_back(t);
			break;

		case PRIMARY_T_EXPRESSION:
			terms = IN3ADD_get_terms_from_expr(p->data.e);
	}

	return terms;
}

int relop_to_statop(int relop)
{
	switch(relop)
	{
		case RELOP_EQUAL:
			return STAT_EQUAL;
			break;
		case RELOP_NOTEQUAL:
			return STAT_NOTEQUAL;
			break;
		case RELOP_LT:
			return STAT_LT;
			break;
		case RELOP_GT:
			return STAT_GT;
			break;
		case RELOP_LE:
			return STAT_LE;
			break;
		case RELOP_GE:
			return STAT_GE;
			break;	
	}

	return RELOP_NONE;
}

int mulop_to_statop(int mulop)
{
	switch(mulop)
	{
		case MULOP_STAR:
			return STAT_STAR;
			break;
		case MULOP_SLASH:
			return STAT_SLASH;
			break;
		case MULOP_MOD:
			return STAT_MOD;
			break;
	}

	return MULOP_NONE;
}

int addop_to_statop(int addop)
{
	switch(addop)
	{
		case ADDOP_PLUS:
			return STAT_PLUS;
			break;
		case ADDOP_MINUS:
			return STAT_MINUS;
			break;
	}

	return ADDOP_NONE;
}

/* Adds a statement of an if statement (i.e. adds st1 or st2 of an if statement) */
int add_if_body_to_cfg(statement_t *st, int parent)
{
	current_bb++;
	int my_index = current_bb;
	BasicBlock *next_block = new BasicBlock();
	next_block->parents.push_back(parent);

	cfg.push_back(next_block);

	process_statement(st);
	
	return my_index;
}

void add_next_bb(std::vector<int> parents)
{
	BasicBlock *new_block = new BasicBlock();
	current_bb++;
	for(int i=0; i<parents.size(); i++)
	{
		cfg[parents[i]]->children.push_back(current_bb);
		new_block->parents.push_back(parents[i]);
	}
	cfg.push_back(new_block);
}

char* create_id(char* id)
{

	char *new_id = (char*)malloc(sizeof(char)*strlen(id) + 1);
	strncpy(new_id, id, strlen(id) +1);
	return new_id;
}

char* create_temp_id()
{
	char s[32];
	int num_len = sprintf(s, "%d", condition_var_name);

	char actual_id[num_len + 1];
	sprintf(actual_id, "$%d", condition_var_name);

	condition_var_name++;

	return create_id(actual_id);
}

Term* create_temp_term(char* id)
{
	Term *t = new Term();
	t->type = TERM_TYPE_VAR;
	t->data.var = create_id(id);

	return t;
}

char* create_and_insert_stat(RHS *rhs)
{
	Statement *stat = new Statement();
	stat->lhs = create_temp_id();
	stat->rhs = rhs;

	cfg[current_bb]->statements.push_back(stat); 
	return stat->lhs;
}

void define_extended_bbs()
{
	bool changed = true;
	int bb_index = 0;
	extended_bb_label = 1; //ensure we are starting at 1 
	while(changed)
	{	
		changed = false;
		changed = extended_bb_alg(bb_index, changed);
	}
}

bool is_separated_from_parents(int bb_index)
{
	int ebb = cfg[bb_index]->extended_bb;
	int parent_index;
	for(int i=0 ; i<cfg[bb_index]->parents.size(); i++)
	{
		parent_index = cfg[bb_index]->parents[i];
		if(ebb == cfg[parent_index]->extended_bb)
		{
			/* if the parent is larger, will be a condition
			let it stay the same if needed */
			if(parent_index < bb_index)
			{
				return false;
			}
			
		}
	}

	return true;
}


bool extended_bb_alg(int bb_index, bool changed)
{
	if(has_one_parent(bb_index))
	{
		/*handle root node cases*/
		if(bb_index == 0 && cfg[bb_index]->extended_bb == 0)
		{
			cfg[bb_index]->extended_bb = extended_bb_label;
			extended_bb_label++;
			changed= true;
		}

	}	
	else if(!is_separated_from_parents(bb_index))
	{
		cfg[bb_index]->extended_bb = extended_bb_label;
		extended_bb_label++;
		changed = true;
	}		

	changed |= populate_children_bbs(bb_index, cfg[bb_index]->extended_bb, changed);

	int child_index;
	for(int i=0;i<cfg[bb_index]->children.size() ; i++)
	{
		/* To avoid loop we wont ever process children with 
		smaller indexes than their parent b/c this would mean
		they are the condition to a while loop and would already
		be processed before this */
		child_index = cfg[bb_index]->children[i];
		if(child_index > bb_index)
		{
			changed |= extended_bb_alg(child_index, changed);
		}
		
	}

	return changed;
}

bool has_one_parent(int bb_index)
{
	return (cfg[bb_index]->parents.size() > 1)? false : true; 
}

bool populate_children_bbs(int parent_index, int parent_ebb, bool changed)
{
	int child_index;
	for(int i=0 ; i < cfg[parent_index]->children.size();i++)
	{
		child_index = cfg[parent_index]->children[i];

		/* set child ebb to parents ebb*/
		if(cfg[child_index]->extended_bb == 0)
		{
			cfg[child_index]->extended_bb = parent_ebb;
			changed = true;
		}		
	}

	return changed;
}

void print_CFG()
{
	for(int i = 0; i < cfg.size(); i++)
	{
		printf("CURRENT BB INDEX: %d\n", i);
		cout << "Extended bb: " << cfg[i]->extended_bb << endl;
		printf("Parents: ");
		for(int x=0 ; x <cfg[i]->parents.size() ; x++)
		{
			printf("%d, ", cfg[i]->parents[x]);
		}
		printf("\nChildren: ");
		for(int j=0 ;j < cfg[i]->children.size(); j++)
		{
			printf("%d, ", cfg[i]->children[j]);
		}
		printf("\nStatements: \n");
		for(int k=0 ; k< cfg[i]->statements.size(); k++)
		{
			Statement *stmt = cfg[i]->statements[k];
			string op;
			switch(stmt->rhs->op)
			{
				case STAT_PLUS :
					op = "+";
					break;
				case STAT_MINUS:
					op = "-";
					break;
				case STAT_STAR:
					op = "*";
					break;
				case STAT_SLASH: 
					op = "/";
					break;
				case STAT_MOD:
					op = "\%";
					break;
				case STAT_EQUAL:
					op = "==";
					break;
				case STAT_NOTEQUAL:
					op = "!=";
					break;
				case STAT_LT: 
					op = "<";
					break;
				case STAT_GT: 
					op = ">";
					break;
				case STAT_LE:
					op = "<=";
					break;
				case STAT_GE :
					op = ">=";
					break;
				case STAT_NONE:
					op = "----";
					break;
			}
			char *t1, *t2;
			string str;
			if(stmt->rhs->t1->type == TERM_TYPE_CONST)
			{
				stringstream ss;
				ss << stmt->rhs->t1->data.constant;
				str = ss.str();
				t1 = new char [str.length()+1];
				strcpy(t1, str.c_str());
			}
			else
			{
				t1 = stmt->rhs->t1->data.var;
			}

			if(stmt->rhs->t2 != NULL)
			{
				if(stmt->rhs->t2->type == TERM_TYPE_CONST)
				{
					stringstream ss;
					ss << stmt->rhs->t2->data.constant;
					str = ss.str();
					t2 = new char [str.length()+1];
					strcpy(t2, str.c_str());
				}
				else
				{
					t2 = stmt->rhs->t2->data.var;
				}
			}

			printf("\tASSIGNMENT: %s = %s ", stmt->lhs, t1);
			if(stmt->rhs->t2 != NULL)
			{
				printf(" %s %s", op.c_str(), t2);
			}
			cout << endl;
			
		}

		printf("-------------------------------------------------------------\n\n");
	}
}
