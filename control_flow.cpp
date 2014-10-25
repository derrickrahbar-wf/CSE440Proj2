#include "control_flow.h"

using namespace std;

std::vector<BasicBlock> cfg;
int current_bb = 0;
int condition_var_name = 0;


std::vector<BasicBlock> create_CFG(statement_sequence_t *ss)
{
	BasicBlock *starting_block = new BasicBlock();
	cfg.push_back(starting_block);
	add_statements_to_cfg(ss->s);
	print_CFG();

	return cfg;
}

void process_statement(statement_t *s)
{
	switch(s->type)
	{
		case STATEMENT_T_ASSIGNMENT:
			add_assignment_to_cfg(s.data->as);
			break;
		case STATEMENT_T_SEQUENCE:
			add_statements_to_cfg(s.data->ss);
			break;
		case STATEMENT_T_IF:
			add_if_statement_to_cfg(s.data->is);
			break;
		case STATEMENT_T_WHILE:
			add_while_statement_to_cfg(s.data->ws);
			break;
	}
}

void add_while_statement_to_cfg(while_statement_t *ws)
{
	/* New BB for the condition of the ws */
	std::vector<int> parent = {current_bb};
	add_next_bb(parent);
	int condition_index = current_bb;

	/* Add that condition to the newly created BB from above */
	add_condition_to_bb(ws->e);
	
	/* Update parent vector to hold current condition bb */
	parent.clear();
	parent.push_back(current_bb);

	/* Add new BB for ws body */
	add_next_bb(parent);
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
	while(ss != NULL)
	{
		process_statement(ss->s);
		ss = ss->next;
	}
}

void add_assignment_to_cfg(assignment_statement_t *as)
{
	if(!is_3_address_code(as->e))
	{
		RHS *rhs = new RHS();
		rhs->t1 = gen_term_from_expr(as->e);
		rhs->op = 
	}
	else
	{

	}

}

void add_if_statement_to_cfg(if_statement_t *ifs)
{
	int parent = current_bb;
	add_condition_to_bb(ifs->e);

	
	int if_st1_index = add_if_body_to_cfg(ifs->s1, parent);
	int if_st2_index = add_if_body_to_cfg(ifs->s2, parent);
	
	cfg[parent]->children.push_back(if_st1_index);
	cfg[parent]->children.push_back(if_st2_index);

	std::vector<int> if_statements_index = {if_st1_index, if_st2_index};

	add_next_bb(if_statements_index);
}

void add_condition_to_bb(expression_t *expr)
{
	
}


/* Adds a statement of an if statement (i.e. adds st1 or st2 of an if statement) */
int add_if_body_to_cfg(statement_t *st, int parent)
{
	current_bb++;
	my_index = current_bb;
	BasicBlock *next_block = new BasicBlock();
	next_block->parent.push_back(parent);

	cfg.push_back(next_block);

	process_statement(st);
	
	return my_index;
}

void add_next_bb(std::vector<int> parents)
{
	BasicBlock *new_block = new BasicBlock();
	current_bb++;
	for(int i : parents)
	{
		cfg[parents[i]]->children.push_back(current_bb);
		new_block->parents.push_back(parents[i]);
	}
	cfg.push_back(new_block);
}

void print_CFG()
{
	for(int i : cfg)
	{
		printf("CURRENT BB INDEX: %d\n", i);
		printf("Parents: ");
		for(int x : cfg[i]->parents)
		{
			printf("%d, ", cfg[i]->parents[x]);
		}
		printf("\nChildren: ");
		for(int j : cfg[i]->children)
		{
			printf("%d, ", cfg[i]->children[j]);
		}
		printf("\nStatements: \n");
		for(int k : cfg[i]->statements)
		{
			Statement *stmt = cfg[i]->statements[k];

			if(stmt->type == ASSIGNMENT_CF)
			{
				printf("\tASSIGNMENT: %s\n", stmt.data->va.data->id);
			}
			else
			{
				printf("\tPRINT: %s\n", stmt.data->va.data->id);
			}
			
		}

		printf("-------------------------------------------------------------\n\n");
	}
}
