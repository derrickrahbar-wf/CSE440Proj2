#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

#include <vector>
#include <iostream>
#include <unordered_map>

#define STAT_PLUS 0
#define STAT_MINUS 1
#define STAT_STAR 2
#define STAT_SLASH 3
#define STAT_MOD 4
#define STAT_EQUAL 5
#define STAT_NOTEQUAL 6
#define STAT_LT 7
#define STAT_GT 8
#define STAT_LE 9
#define STAT_GE 10
#define STAT_NONE -1

#define STAT_SIGN_POSITIVE 0
#define STAT_SIGN_NEGATIVE -1

#define TERM_TYPE_CONST 0
#define TERM_TYPE_VAR 1

using namespace std;

class Term {
	public:
		int type;
		int sign;
		union
		{
			int constant;
			char *var;
		}data;
};


class RHS {
	public:
		Term *t1;
		int op;
		Term *t2;
};

class Val_obj {
	public:
		int val_num;
		bool is_const;
};

class Statement {
	public:
		char *lhs;
		RHS *rhs;
		bool is_goto;
		int goto_index;
};

class BasicBlock {
	public:
		std::vector<int> parents;
		std::vector<int> children;
		std::vector<Statement*> statements;
		int extended_bb; //ranged from 1-n 
};


int eval_term_const(Term *t);
statement_sequence_t * reverse_ss(statement_sequence_t* ss);
void print_statement_list(statement_sequence_t* ss);
std::vector<BasicBlock*> create_CFG(statement_sequence_t *ss);
void value_numbering();
void create_tables_for_bb(int cfg_index);
void copy_parent_tables_to_child(int parent_index, int child_index);
void process_bb_stats_for_tables(int index);
void process_statement_for_tables(Statement *stat, int table_index);
void process_multi_stat_for_tables(Statement *stat, int table_index);
Val_obj* eval_expr(Statement *stat, int table_index);
int eval_const(int constant, int table_index);
Val_obj* eval_id(char *var, int table_index);
int get_appr_value_num_for_term(Term *t, int table_index);
void process_singular_stat_for_tables(Statement *stat, int table_index);
Val_obj *optimize_singl_stat_with_const(Statement *stat, int val_num, int table_index);
void optimize_stat_with_const(Statement *stat, int val_num, int table_index);
int calc_const_and_add_to_table(int const1, int op, int const2, int table_index);
std::vector<string> find_ids_with_same_val_obj(Val_obj *vo, int table_index);
void process_ids_with_current_val_num(std::vector<string> ids, Statement *stat, int table_index);
RHS * optimize_plus_expr(RHS *rhs, int table_index);
RHS * optimize_minus_expr(RHS *rhs, int table_index);
RHS * optimize_star_expr(RHS *rhs, int table_index);
RHS * optimize_slash_expr(RHS *rhs, int table_index);
RHS * optimize_mod_expr(RHS *rhs, int table_index);
RHS * optimize_e_le_ge_expr(RHS *rhs, int table_index);
RHS * optimize_ne_gt_lt_exprs(RHS *rhs, int table_index);
bool is_var_and_currently_0(Term *t, int table_index);
bool is_var_and_currently_1(Term *t, int table_index);
bool is_var_and_currently_num(int num_to_compare, Term *t, int table_index);
bool is_current_constant(char *id, int table_index);
int const_val_num_matches(int val_num, int constant, int relop, bool t1_is_const, int table_index);
bool have_same_val_nums(char *var1, char *var2, int table_index);
std::unordered_map<int, int>::const_iterator const_table_find(int val_num, int table_index);
void process_statement(statement_t *s);
void add_while_statement_to_cfg(while_statement_t *ws);
void add_statements_to_cfg(statement_sequence_t *ss);
void add_assignment_to_cfg(assignment_statement_t *as);
void add_if_statement_to_cfg(if_statement_t *ifs);
void add_condition_to_bb(expression_t *expr);
RHS* get_rhs_from_expr(expression_t *expr);
bool is_3_address_code(expression_t *expr);
int expr_term_count(expression_t *expr);
int se_term_count(simple_expression_t *se);
int term_term_count(term_t *t);
int factor_term_count(factor_t *f);
int primary_term_count(primary_t *p);
Term* gen_term_from_expr(expression_t *expr);
Term* gen_term_from_se(simple_expression_t *se);
Term* gen_term_from_term(term_t *t);
Term* gen_term_from_factor(factor_t *f);
Term* create_negative_factor_term(factor_t *f);
RHS* IN3ADD_gen_rhs_from_3_add_expr(expression_t *expr);
std::vector<Term*> IN3ADD_get_terms_from_expr(expression_t *expr);
std::vector<Term*> IN3ADD_get_terms_from_se(simple_expression_t *se);
std::vector<Term*> IN3ADD_get_terms_from_term(term_t *t);
std::vector<Term*> IN3ADD_get_terms_from_factor(factor_t *f);
std::vector<Term*> IN3ADD_get_terms_from_primary(primary_t *p);
int relop_to_statop(int relop);
int mulop_to_statop(int mulop);
int addop_to_statop(int addop);
int add_if_body_to_cfg(statement_t *st, int parent);
void add_next_bb(std::vector<int> parents);
char* create_id(char* id);
char* create_temp_id();
Term* create_temp_term(char* id);
char* create_and_insert_stat(RHS *rhs);
void define_extended_bbs();
bool is_separated_from_parents(int bb_index);
bool extended_bb_alg(int bb_index, bool changed);
bool has_one_parent(int bb_index);
bool populate_children_bbs(int parent_index, int parent_ebb, bool changed);
void print_CFG();
void remove_dummy_nodes();
int get_updated_index(int index, vector<int> dummy_nodes);



/*ONE IT DOESNT CATCH*/
Term* gen_term_from_primary(primary_t *p);
void find_stat_and_add_to_remove_list(Statement* stat, int table_index);


#endif /* CONTROL_FLOW_H */
