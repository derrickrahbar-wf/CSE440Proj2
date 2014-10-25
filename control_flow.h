#include <vector>
#include <iostream>

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



class Term {
	public:
		int type;
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

class Statement {
	public:
		char *lhs;
		RHS *rhs;
};

class BasicBlock {
	public:
		std::vector<int> parents;
		std::vector<int> children;
		std::vector<Statement> statements;	
};



std::vector<BasicBlock> create_CFG(statement_sequence_t *ss);

void process_statement(statement_t *s);

void add_while_statement_to_cfg(while_statement_t *ws);

void add_statements_to_cfg(statement_sequence_t *ss);

void add_assignment_to_cfg(assignment_statement_t *as);

void add_if_statement_to_cfg(if_statement_t *ifs);

void add_condition_to_bb(expression_t *expr);

assignment_statement_t* create_as_from_expr(expression_t *expr);

int add_if_body_to_cfg(statement_t *st, int parent);

void add_next_bb(std::vector<int> parents);

void print_CFG();
