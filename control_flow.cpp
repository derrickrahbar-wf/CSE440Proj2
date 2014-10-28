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
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <utility>


using namespace std;

std::vector<BasicBlock*> cfg;
int current_bb = 0;
int condition_var_name = 0;
int IN3ADD_op = -1;
int extended_bb_label = 1;
int g_val_num = 0;

std::vector<int> statements_to_remove;

std::vector<std::unordered_map<string, int>*> expr_tables;
std::vector<std::unordered_map<string, Val_obj*>*> id_tables;
std::vector<std::unordered_map<int, int>*> const_tables; /* constant will be first, valnum second */


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
	print_CFG();
	remove_dummy_nodes();
	//define_extended_bbs();
	print_CFG();

	// value_numbering();
	// cout << "\n\n====================================================\n\n";
	// cout << "AFTER VALUE NUMBERING\n";
	// print_CFG();

	return cfg;
}

/*This method removes all dummy nodes in the cfg
  with the exception of the ending node, which may
  be empty, this can be defined if it has no children */
void remove_dummy_nodes()
{
    BasicBlock *bb;
    std::vector<int> dummy_nodes;
    for(int i=0; i<cfg.size(); i++)
    {
        bb = cfg[i];
        /* This is a bb with no statements in it that is not
            the ending block, thus we can remove it */
        if(bb->statements.size() == 0 && bb->children.size() > 0)
        {
            dummy_nodes.push_back(i);
            cout << "\nDUMMY NODE " << i << endl;
        }
    }


    for(int i=0;i<dummy_nodes.size();i++)
    {
	    bb = cfg[dummy_nodes[i]];
	    cout << "Found dummy node " << dummy_nodes[i] << endl;
	    /*copy all the children of this node as children to the parents 
	      and vise versa */
	    for(int p = 0; p< bb->parents.size(); p++)
	    {
	        int parent_index = bb->parents[p];
	        cout << "PARENT " << parent_index << endl;
	        /*erase dummy bb as a child to this parent */
	        vector<int>::iterator position = find(cfg[parent_index]->children.begin(), cfg[parent_index]->children.end(), dummy_nodes[i]);
	        if (position != cfg[parent_index]->children.end()) 
	        {
	       	  cfg[parent_index]->children.erase(position);
	        }

	        for(int c=0 ; c<bb->children.size(); c++)
	        {
	            int child_index = bb->children[c];
	            cout << "CHILD " << child_index << endl;
	            /*erase dummy bb as a parent to this child */
	            vector<int>::iterator position = find(cfg[child_index]->parents.begin(), cfg[child_index]->parents.end(), dummy_nodes[i]);
	            if (position != cfg[child_index]->parents.end())
	            {
	   				cfg[child_index]->parents.erase(position);	
	            } // == vector.end() means the element was not found
	                
	             /*push back new child of parent*/
		    	cfg[parent_index]->children.push_back(child_index);

		    	/*push back new parent of child */
		    	cfg[child_index]->parents.push_back(parent_index);
	        }
	    }
    }

    /* we can finally delete the nodes */
    for(int i=0;i<dummy_nodes.size();i++)
    {
    	cfg.erase(cfg.begin() + dummy_nodes[i]);
    }

    /*Now we must go through the bb and update any indexes
      that may have changed */
    for(int i=0; i<cfg.size(); i++)
    {
    	BasicBlock *bb = cfg[i];

    	/*update parent indexes*/
    	for(int p=0; p<bb->parents.size() ; p++)
    	{
    		bb->parents[p] = get_updated_index(bb->parents[p], dummy_nodes);
    	}
    	/*update child indexes*/
    	for(int c=0; c<bb->children.size() ; c++)
    	{
    		bb->children[c] = get_updated_index(bb->children[c], dummy_nodes);	
    	}
    }
}

/* Takes in a list of indexes that are going to be deleted
   and processes what the incoming index will be after those deletions */
int get_updated_index(int index, vector<int> dummy_nodes)
{
	for(int i=0; i<dummy_nodes.size();i++)
	{
		/* if the index is greater than the node that will
		   be deleted, reduce its index by 1 because it will
		   become one less when this node is eventually deleted */
		   if(index > dummy_nodes[i])
		   {
		   		index--;
		   }
	}

	return index;
}

void value_numbering()
{
	for(int i = 0; i < cfg.size(); i++)
	{
		/* push back a new undordered_map */
		id_tables.push_back(new unordered_map<string, Val_obj*>);
		expr_tables.push_back(new unordered_map<string, int>);
		const_tables.push_back(new unordered_map<int, int>);

		create_tables_for_bb(i);
	}
}

void create_tables_for_bb(int cfg_index)
{
	BasicBlock *bb = cfg[cfg_index];

	for(int i=0; i < bb->parents.size(); i++)
	{
		int parent_index = bb->parents[i];
		cout << "Analysis for parent " << parent_index << " of bb " << cfg_index << endl;
		/* We only would want a parent of the child with the same ebb and it defined before them*/
		if(cfg_index > parent_index && bb->extended_bb == cfg[parent_index]->extended_bb)
		{
			cout << "Copying bb tables of parent " << parent_index << " into bb " << cfg_index << endl;  
			copy_parent_tables_to_child(parent_index, cfg_index);
		}
	}

	process_bb_stats_for_tables(cfg_index);
}

void copy_parent_tables_to_child(int parent_index, int child_index)
{
	for ( auto it = expr_tables[parent_index]->begin(); it != expr_tables[parent_index]->end(); ++it )
		expr_tables[child_index]->insert(make_pair(it->first,it->second));	

	for ( auto it = id_tables[parent_index]->begin(); it != id_tables[parent_index]->end(); ++it )
	{
		Val_obj *tmp = new Val_obj();
		tmp->val_num = it->second->val_num;
		tmp->is_const = it->second->is_const;

		id_tables[child_index]->insert(make_pair(it->first ,tmp));
	}

	for ( auto it = const_tables[parent_index]->begin(); it != const_tables[parent_index]->end(); ++it )
		const_tables[child_index]->insert(make_pair(it->first, it->second));
}

void process_bb_stats_for_tables(int index)
{
	BasicBlock *bb = cfg[index];

	cout << "Processing bb " << index << endl;

	for(int i = 0; i < bb->statements.size(); i++)
	{
		cout << "Processing statement " << i << " in bb "<< index << endl;
		process_statement_for_tables(bb->statements[i], index);
	}

	/*We have processed all statements for this current 
	  basic block, if there are any statements in the
	  statements_to_remove vector we should now remove them*/
	  for(int i=0;i<statements_to_remove.size(); i++)
	  {
	  	bb->statements.erase(bb->statements.begin() + statements_to_remove[i]);
	  }

	  /*Clear the statements to remove vector for the next bb*/
	  statements_to_remove.clear();

	  cout << "finishing processing bb " << index << endl;
}

void process_statement_for_tables(Statement *stat, int table_index)
{
	if(stat->rhs->t2 == NULL)
	{
		cout << "proccessing a single statement\n";
		process_singular_stat_for_tables(stat, table_index);
	}
	else
	{
		cout << "proccessing a multi statement\n";
		process_multi_stat_for_tables(stat, table_index);
	}
}

void process_multi_stat_for_tables(Statement *stat, int table_index)
{
	Term *t1 = stat->rhs->t1;
	Term *t2 = stat->rhs->t2;
	RHS *rhs = stat->rhs;
	Val_obj *lhs_vo = new Val_obj();

	/*Both of the terms are constants, we can do 
	constant expression evaluation */
	if(t2->type == TERM_TYPE_CONST && t1->type == TERM_TYPE_CONST)
	{
		cout << "Both terms are constants\n";
		int const_val_num = calc_const_and_add_to_table(t1->data.constant, rhs->op, t2->data.constant, table_index);
		/* update this statement to equal the new evaluated constant*/
		optimize_stat_with_const(stat, const_val_num, table_index);
		
		lhs_vo->is_const = true;
		lhs_vo->val_num = const_val_num;
	}
	/* One of the terms is a constant */
	else if(t2->type == TERM_TYPE_CONST || t1->type == TERM_TYPE_CONST)
	{
		cout << "One of the terms is a constant\n";
		int const_val_num;
		Val_obj *vo;
		
		/* the first term is the constant*/
		if(t1->type == TERM_TYPE_CONST)
		{
			cout << "t1 is the constant\n";
			/* get t1 constant value number and t2's variable value number */
			const_val_num = eval_const(t1->data.constant, table_index);
			vo = eval_id(t2->data.var, table_index);
		
			/* t2's variable is currently a constant, we can do constant evaluation 
			   because t1 is a constant */
			if(vo->is_const)
			{
				std::unordered_map<int, int>::const_iterator check = const_table_find(vo->val_num, table_index);
				
				/* has not yet been added */
				if(check == const_tables[table_index]->end())
				{
					cout << "process_multi_stat_for_tables if stmt\n";
					error_unknown(-1); /*this should not happen */
					return;
				}

				int c_v_num = calc_const_and_add_to_table(t1->data.constant, rhs->op, check->first, table_index);
				lhs_vo->is_const = true;
				lhs_vo->val_num = c_v_num;
			}
			/* t2 is just a variable, we must add the operation to the expression table */
			else
			{
				lhs_vo = eval_expr(stat, table_index);
			}
		}
		/* the second term is the constant */
		else
		{
			cout << "t2 is the constant\n";
			const_val_num = eval_const(t2->data.constant, table_index);
			vo = eval_id(t1->data.var, table_index);

			if(vo->is_const)
			{
				std::unordered_map<int, int>::const_iterator check = const_table_find(vo->val_num, table_index);
				
				/* has not yet been added */
				if(check == const_tables[table_index]->end())
				{
					cout << "process_multi_stat_for_tables else if stmt\n";
					error_unknown(-1); /*this should not happen */
					return;
				}

				int c_v_num = calc_const_and_add_to_table(check->first, rhs->op, t2->data.constant, table_index);
				lhs_vo->is_const = true;
				lhs_vo->val_num = c_v_num;
			}
			else
			{
				lhs_vo = eval_expr(stat, table_index);
			}
		}	
	}
	/* Neither of the terms are constants */
	else
	{
		cout << "Neither of the terms are constants\n";
		Val_obj *vo1, *vo2;
	
		/* grab the value number for each of the term id's */
		cout << "Grab both ids\n";
		vo1 = eval_id(t1->data.var, table_index);
		vo2 = eval_id(t2->data.var, table_index);

		/*Both of the variables currently evaluate to constants we can
		  perform constant eval */
		if(vo1->is_const && vo2->is_const)
		{
			cout << "Both ids are currently constants\n";
			std::unordered_map<int, int>::const_iterator check1 = const_table_find(vo1->val_num, table_index);
			std::unordered_map<int, int>::const_iterator check2 = const_table_find(vo2->val_num, table_index);
			
			/* has not yet been added */
			if(check1 == const_tables[table_index]->end() || check2 == const_tables[table_index]->end())
			{
				cout << "process_multi_stat_for_tables else stmt\n";
				error_unknown(-1); /*this should not happen */
				return;
			}

			int c_v_num = calc_const_and_add_to_table(check1->first, rhs->op, check2->first, table_index);
			lhs_vo->is_const = true;
			lhs_vo->val_num = c_v_num;
		}
		/* At least one of the variables is not currently a constant, we must 
		   add the expression to the table */
		else
		{
			cout << "At least one id is not a constant adding the expr\n";	
			lhs_vo = eval_expr(stat, table_index);
		}
	}

	cout << "Check if other variables have the same valobj\n";
	/* Before we add the new value to our lhs var, let us check to see 
	   if any other variables currenlty evaluate to this same expression */
	std::vector<string> duplicate_ids = find_ids_with_same_val_obj(lhs_vo, table_index);

	cout << "Optimized statement with the duplicate_ids\n";
	/* perform optimization of the statement according to the ids with 
	duplicate value numbers */
	process_ids_with_current_val_num(duplicate_ids, stat, table_index);

	cout << "Add id to table with val obj\n";
	/*Finally we add the new value number to our
	  current lhs id*/
	string key(stat->lhs);
	id_tables[table_index]->insert(make_pair(key, lhs_vo));

	cout << "Finished processing " << key << endl;

}

/* Takes in an expression optimizes it based on the operation
    and then adds it to the expression/constant table 
    returns the Val_obj associated with the expression
    to be added to the lhs id */
Val_obj* eval_expr(Statement *stat, int table_index)
{
	Val_obj *lhs_vo = new Val_obj();

	switch(stat->rhs->op)
	{
		case STAT_PLUS:
			stat->rhs =  optimize_plus_expr(stat->rhs, table_index);
			break;
		case STAT_MINUS:
			stat->rhs =  optimize_minus_expr(stat->rhs, table_index);
			break;
		case STAT_STAR:
			stat->rhs =  optimize_star_expr(stat->rhs, table_index);
			break;
		case STAT_SLASH:
			stat->rhs =  optimize_slash_expr(stat->rhs, table_index);
			break;
		case STAT_MOD:
			stat->rhs =  optimize_mod_expr(stat->rhs, table_index);
			break;
		case STAT_EQUAL:
		case STAT_LE:
		case STAT_GE:
			stat->rhs =  optimize_e_le_ge_expr(stat->rhs, table_index);
			break;	
		case STAT_NOTEQUAL:
		case STAT_LT:
		case STAT_GT:
			stat->rhs = optimize_ne_gt_lt_exprs(stat->rhs, table_index);
			break;
	
	}

	/* the rhs was able to be optimized into a constant*/
	if(stat->rhs->t2 == NULL && stat->rhs->t1->type == TERM_TYPE_CONST)
	{
		lhs_vo->val_num = eval_const(stat->rhs->t1->data.constant, table_index);
		lhs_vo->is_const = true;
	}
	/* rhs is still an expression add to expression table */
	else 
	{
		/* We know this cannot be a constant */
		lhs_vo->is_const = false;

		int t1_num = get_appr_value_num_for_term(stat->rhs->t1, table_index);
		int t2_num = get_appr_value_num_for_term(stat->rhs->t2, table_index);

		stringstream ss;
		ss << t1_num << stat->rhs->op << t2_num;
		string expr_string = ss.str();

		/* these are communative ops, we want to sort their expression
		   so a+b and b+a map to the same entry */
		if(stat->rhs->op == STAT_STAR || stat->rhs->op == STAT_PLUS 
			|| stat->rhs->op == STAT_EQUAL || stat->rhs->op == STAT_NOTEQUAL)
		{
			std::sort(expr_string.begin(), expr_string.end());
		}

		/* check to see if the expression already exists */
		std::unordered_map<string, int>::const_iterator check = expr_tables[table_index]->find (expr_string);

		/*Expression has not yet been added */
		if(check == expr_tables[table_index]->end())
		{
			/*introduce a new valnum for the expression and add*/
			expr_tables[table_index]->insert(make_pair(expr_string,g_val_num));
			lhs_vo->val_num = g_val_num;
			g_val_num++;
		}
		/* The expression already exists, return the corresponding valnum for it */
		else
		{
			lhs_vo->val_num = check->second; 
		}

	}

	return lhs_vo;

}

/* takes in a constant, checks to see if exists
   already in const_table if not gives it new val_num
   and adds to table, returns the val_num for const */
int eval_const(int constant, int table_index)
{
    std::unordered_map<int, int>::const_iterator check = const_tables[table_index]->find (constant);

    /*constant has not yet been added 
      therefore we need to introduce a 
      new val_num for this */
    if(check == const_tables[table_index]->end())
    {
    	int const_val_num = g_val_num;
    	g_val_num++;

    	const_tables[table_index]->insert(make_pair(constant, const_val_num)); /*insert into table */
    	
    	return const_val_num; /* return the val_num associated with the constant */
    }

    return check->second; /*constant already exists, return its val_num */
}

/* takes in a var, checks to see if exists already in id_table 
   if not gives it new val_num with is_const = false and adds to table, 
   returns the Val_obj for id */
Val_obj* eval_id(char *var, int table_index)
{
	cout << "Evaluating id " << var << endl;
	string key(var);
	cout << "Got string " << key << endl;
    std::unordered_map<string, Val_obj*>::const_iterator check = id_tables[table_index]->find (key);

    cout << "Performed table find on index " << table_index << endl;
    /*variable has not yet been added 
      therefore we need to introduce a 
      new val_num for this it would not
      be a constant either because it 
      is not in the table yet */
    if(check == id_tables[table_index]->end())
    {
    	cout << var << " has not yet been added\n";
    	Val_obj *vo = new Val_obj();
    	vo->val_num = g_val_num;
    	vo->is_const = false;
    	cout << "Add " << key << " with valnum " << g_val_num << endl;
    	id_tables[table_index]->insert(make_pair(key, vo)); /* add new val to the table */

    	g_val_num++; /*increase the g_val_num for the next val_num to be used*/

    	cout << "Return val_obj\n";
    	return vo;
    }

    cout << "Id already exitsts return " << check->second->val_num;
    /* the id already exisits, return the val_obj associated with it */
    return check->second;
}

/* Takes in a term, gets val_num from const table if a const,
    otherwise gets val_num from id_table if a var */
int get_appr_value_num_for_term(Term *t, int table_index)
{
	if(t->type == TERM_TYPE_CONST)
	{
		return eval_const(t->data.constant, table_index);
	}
	else
	{
		return eval_id(t->data.var, table_index)->val_num;
	}
}

/* processes statements that are only composed of a t1
	meaning a = b */
void process_singular_stat_for_tables(Statement *stat, int table_index)
{
	Val_obj *valObj = new Val_obj(), *temp_obj;

	/* a = 1 , it is a constant 
		add constant val to cons_table 
		and create a Val_obj for the lhs id */
	if(stat->rhs->t1->type == TERM_TYPE_CONST)
	{
		valObj = new Val_obj();
		valObj->val_num = eval_const(stat->rhs->t1->data.constant, table_index);
		valObj->is_const = true;
	}

	/* a = b, it is a var add var */ 
	else
	{
		temp_obj = eval_id(stat->rhs->t1->data.var, table_index);
		valObj->val_num = temp_obj->val_num;
		valObj->is_const = temp_obj->is_const;

		/*if it is currently
			evaluating to a constant, optimize the statement
			to be a = #, the actual constant b is */
		if(valObj->is_const)
		{
			optimize_singl_stat_with_const(stat, valObj->val_num, table_index);
		}
	}
	
	/*Let us check is the lhs is already defined as this rhs
	  if so, we can delete this statement entirely because it doesnt
	  change the value of the lhs id */ 
	std::vector<string> duplicate_ids = find_ids_with_same_val_obj(valObj, table_index);	
	for(int i=0 ;i<duplicate_ids.size(); i++)
	{
		string lhs(stat->lhs);
		if(lhs.compare(duplicate_ids[i]) == 0)
		{
			find_stat_and_add_to_remove_list(stat, table_index);
		}
	}
	 
	/*update the lhs id with the rhs valObj*/  	
	string key(stat->lhs);
	id_tables[table_index]->insert(make_pair(key, valObj));
}

/* Takes in a statement to be optimized and a val_num associated
	with that constant, gets the constants value and updates
	the statement */
void optimize_singl_stat_with_const(Statement *stat, int val_num, int table_index)
{
	std::unordered_map<int, int>::const_iterator const_it = const_table_find(val_num, table_index);

	/*Expression has not yet been added */
	if(const_it == const_tables[table_index]->end())
	{
		cout << "optimize_singl_stat_with_const\n";
		error_unknown(-1); /*this should not happen */
		return;
	}

	/*update state to equal just the constant */
	stat->rhs->t1->type = TERM_TYPE_CONST;
	stat->rhs->t1->data.constant = 	const_it->first;
	stat->rhs->t2 = NULL;
	stat->rhs->op = STAT_NONE;
}

void find_stat_and_add_to_remove_list(Statement* stat, int table_index)
{	
	/*Add this to the list of statements to remove from the bb*/
	std::vector<Statement*>::iterator dup_stat = find (cfg[table_index]->statements.begin(), cfg[table_index]->statements.end(), stat);
	if (dup_stat == cfg[table_index]->statements.end())
	{
		cout << "find_stat_and_add_to_remove_list\n";
		error_unknown(-1); /*We should find the statement, this should never execute*/
	}
	else
	{
		/*get index of the redundant statement to remove at the
		  end of this bb analysis */
		int pos =  dup_stat - cfg[table_index]->statements.begin();

		statements_to_remove.push_back(pos); /*add to list of redundant stats*/
	}	
}

/* Takes in a statement that was found to just eval to a constant
	will update the statement to just equal the constant 
	associated with the incoming val_num */
void optimize_stat_with_const(Statement *stat, int val_num, int table_index)
{
	std::unordered_map<int, int>::const_iterator check = const_table_find(val_num, table_index);

	/* has not yet been added */
	if(check == const_tables[table_index]->end())
	{
		cout << "optimize_stat_with_const\n";
		error_unknown(-1); /*this should not happen */
		return;
	}	

	stat->rhs->t2 = NULL;
	stat->rhs->op = STAT_NONE;
	stat->rhs->t1->type = TERM_TYPE_CONST;
	stat->rhs->t1->data.constant = check->first;
}

/* takes in two constant inegers and evaluates them
   then adds to the constant value table and returns its
   corresponding val_num */
int calc_const_and_add_to_table(int const1, int op, int const2, int table_index)
{
	int evalconst;
	switch(op)
	{
		case STAT_PLUS :
			evalconst = const1 + const2;
			break;
		case STAT_MINUS:
			evalconst = const1 - const2;
			break;
		case STAT_STAR:
			evalconst = const1 * const2;
			break;
		case STAT_SLASH: 
			evalconst = const1 / const2;
			break;
		case STAT_MOD:
			evalconst = const1 % const2;
			break;
		case STAT_EQUAL:
			evalconst = (const1 == const2)? 1 : 0;
			break;
		case STAT_NOTEQUAL:
			evalconst = (const1 != const2)? 1 : 0;
			break;
		case STAT_LT: 
			evalconst = (const1 < const2)? 1 : 0;
			break;
		case STAT_GT: 
			evalconst = (const1 > const2)? 1 : 0;
			break;
		case STAT_LE:
			evalconst = (const1 <= const2)? 1 : 0;
			break;
		case STAT_GE :
			evalconst = (const1 >= const2)? 1 : 0;
			break;
		case STAT_NONE:
			cout << "calc_const_and_add_to_table\n";
			error_unknown(-1); /*we shouldnt get here */
			evalconst = -1;
			break;
	}

	/*add or find in const table and return val_num */
	return eval_const(evalconst, table_index);
}

/* Takes in a val_obj and searches the id_table for
   any other entrys with the same val_obj returns the 
   vector of ids with the same val_obj */
std::vector<string> find_ids_with_same_val_obj(Val_obj *vo, int table_index)
{
	std::vector<string> ids;

	for (auto it = id_tables[table_index]->begin(); it != id_tables[table_index]->end(); ++it )
	{
		if(it->second->val_num == vo->val_num)
		{
			ids.push_back(it->first); /* this id has the same valnum as desired */
		}
	}

	return ids;
}

/* perform optimization of the statement according to the ids with 
	duplicate value numbers */
void process_ids_with_current_val_num(std::vector<string> ids, Statement *stat, int table_index)
{
	/*if statement already equals a single constant, dont update 
		is fully optimized */
	if(stat->rhs->t2 == NULL && stat->rhs->t1->type == TERM_TYPE_CONST)
	{
		return;
	}

	string lhs(stat->lhs);
	for(int i=0 ; i<ids.size();i++)
	{
		/*if the id is the same as our current lhs statement
		  this means the statement is already evaluated to 
		  this and thus this statement is redundant, we can
		  delete it */
		if(lhs.compare(ids[i]) == 0)
		{
			find_stat_and_add_to_remove_list(stat, table_index);
		}
		/* otherwise we can change the statement to be equal to this 
			variable, because it is already evaluated to this value */
		else
		{
			char *t1 = new char [ids[i].length()+1];
			strcpy(t1, ids[i].c_str());

			stat->rhs->t1->type = TERM_TYPE_VAR;
			stat->rhs->t1->data.var = t1;
			stat->rhs->t2 = NULL;
			stat->rhs->op = STAT_NONE;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////
//		The following methods take in the rhs of the current statement being	//
//		evaluated in the table_index bb, they wil check for possible 			//
//		optimizations and return the optimized rhs to the caller 				//
//////////////////////////////////////////////////////////////////////////////////

RHS * optimize_plus_expr(RHS *rhs, int table_index)
{
	/* a = 0 + b, a = b + 0 --> a = b
	   either t1 is a constant that is 0
	   or t2 is a constant that is 0 */
	if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 0) 
		|| (rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 0))
	{
		/*t2 is the variable, move it to t1*/
		if(rhs->t2->type == TERM_TYPE_VAR)
		{
			rhs->t1 = rhs->t2;
		}

		/*remove t2 so it is just the value of the var*/
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;		
	}
	/* a = b + c, where b evals to 0 
	   can replace with a = c */
	else if(is_var_and_currently_0(rhs->t1, table_index))
	{
		/*replace t1 with t2 */
		rhs->t1 = rhs->t2;
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}

	/* a = b + c, where c evals to 0 
   	   can replace with a = b */
	else if(is_var_and_currently_0(rhs->t2, table_index))
	{
		/* remove t2 because it is 0 */
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
	}

	/* a = b+b --> a = 2 * b
	   t1 and t2 must both be vars and have 
	   the same value num */
	else if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		/*preserve value of t2 and replace t1 with constant of 2
		  update the operation to be multiplication */
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 2;
		rhs->op = STAT_STAR;
	}

	return rhs;
}

RHS * optimize_minus_expr(RHS *rhs, int table_index)
{
	/* a = b - 0 --> a = b
	   t2 must be a constant that is 0 
	   or t2 must be a var that is 
	   currently evaluating to a constant 0 */
	if((rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 0)
		|| is_var_and_currently_0(rhs->t2, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /* remove t2 as it does not change the rhs */
	}

	/* a = 0 - b --> a = -b 
	   t1 must be a constant that is 0 
	   or t1 must be a var that is 
	   currently evaluating to a constant 0*/
	else if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 0)
		|| is_var_and_currently_0(rhs->t1, table_index))
	{
		/* replace t1 with t2 and update the sign of the term*/
		rhs->t1 = rhs->t2;
		rhs->t1->sign = (rhs->t1->sign == STAT_SIGN_POSITIVE)? STAT_SIGN_NEGATIVE : STAT_SIGN_POSITIVE;
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
	}

	/* a = b - b --> a = 0
	   t1 and t2 must be vars and have same val nums */
	else if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0; /* remove t2 and replace t1 as constant 0 */
	}

	return rhs;

}

RHS * optimize_star_expr(RHS *rhs, int table_index)
{
	/* a = b * 0, a = 0 * b --> a = 0 
	   t2,t1 must either be a constant that is 0
	   or a var that is currently eval to a 
	   constant that is 0 */
	if((rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 0) 
		|| (rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 0) 
		|| is_var_and_currently_0(rhs->t1, table_index) || is_var_and_currently_0(rhs->t2, table_index))
	{
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0;
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /* remove t2 not changing rhs */
	}

	/* a = b * 1 --> a = b | a = b * -1 --> a = -b
		t2 must be a constant that is 1
		or a var that is currently eval 
		to a constant that is 1 or -1 */
	else if((rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 1)
			|| is_var_and_currently_1(rhs->t2, table_index) || is_var_and_currently_num(-1, rhs->t2, table_index))
	{
		if(is_var_and_currently_num(-1, rhs->t2, table_index))
		{
			/* flip the term sign to account for * -1 */
			rhs->t1->sign = (rhs->t1->sign == STAT_SIGN_POSITIVE)? STAT_SIGN_NEGATIVE : STAT_SIGN_POSITIVE;
		}

		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /*remove t2 not doing anything */
	}

	/* a = 1 * b --> a = b | a = -1 * b --> a = -b
		t1 must be a constant that is 1
		or a var that is currently eval 
		to a constant that is 1 or -1*/
	else if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 1)
			|| is_var_and_currently_1(rhs->t1, table_index) || is_var_and_currently_num(-1, rhs->t1, table_index))
	{	
		if(is_var_and_currently_num(-1, rhs->t1, table_index))
		{
			/* flip the term sign to account for * -1 */
			rhs->t2->sign = (rhs->t2->sign == STAT_SIGN_POSITIVE)? STAT_SIGN_NEGATIVE : STAT_SIGN_POSITIVE;
		}
		rhs->t1 = rhs->t2;
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /*move t1 to t2 and remove t2 */
	}


	return rhs;
}

RHS * optimize_slash_expr(RHS *rhs, int table_index)
{
	/* a = 0/b --> a = 0 
	   t1 must be a constant that is 0
	   or a var that is currently eval to a 
	   constat that is 0 */
	if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 0)
		|| is_var_and_currently_0(rhs->t1, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /*remove t2 and leave rhs with just t1=0*/
	}
	/* a = b/1 --> a = b 
	   t2 must be a constant that is 1 
	   or a val that is currently eval 
	   to constant that is 1 */
	else if((rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 1)
		|| is_var_and_currently_1(rhs->t2, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /*remove t2 and leave rhs with just t1 value */
	}
	/* a = b/b --> a = 1
	   t1 and t2 must both have same val numbers */
	else if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 1; /* remove t2 and replace t1 with constant value 1*/
	}

	return rhs;
}

RHS * optimize_mod_expr(RHS *rhs, int table_index)
{
	/* a = b % 1 --> a = 0
	   t2 must be a constant that is 1 
	   or a val that is currently eval to 
	   a constant that is 1 */
	if((rhs->t2->type == TERM_TYPE_CONST && rhs->t2->data.constant == 1)
		|| is_var_and_currently_1(rhs->t2, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0; /* remove t2 and replace t1 with constant value 0 */
	}

	/* a = 1 % b --> a = 1
	   t1 must be a constant that is 1 
	   or a var that is currently eval to 
	   a constatnt that is 1*/
	else if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 1)
		|| is_var_and_currently_1(rhs->t1, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE; /* remove t2 and leave t1=1*/
	}

	/* a = 0 % b --> a = 0
   t1 must be a constant that is 0 
   or a var that is currently eval to 
   a constatnt that is 0 */
	else if((rhs->t1->type == TERM_TYPE_CONST && rhs->t1->data.constant == 0)
		|| is_var_and_currently_0(rhs->t1, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0; /* remove t2 and replace t1 with constant value 0 */ 
	}

	/* a = b % b --> a = 0 
	   t1 and t2 must have same val nums */
	else if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0;
	}

	return rhs; 
}

/* optimizes >=, <=, and == given they can only
   be optimized to true if the variables 
   are the same */
RHS * optimize_e_le_ge_expr(RHS *rhs, int table_index)
{
	/* b == b, b >= b, b <= b, replace with 1 for true 
	   t1 and t2 must both be vars with same val nums */
	if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 1;
	}

	/* if t1 is a constant and t2 is currently eval 
	 	to a constant */
	else if(rhs->t1->type == TERM_TYPE_CONST && is_current_constant(rhs->t2->data.var, table_index))
	{
		int new_t1 = const_val_num_matches(eval_id(rhs->t2->data.var, table_index)->val_num, 
										   rhs->t1->data.constant, rhs->op, true, table_index);
		/* set t1 to whether or not they are true */
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = new_t1;
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}

	/* if t2 is a constant and t1 is currently eval 
 	to a constant */
	else if(rhs->t2->type == TERM_TYPE_CONST && is_current_constant(rhs->t1->data.var, table_index))
	{
		int new_t1 = const_val_num_matches(eval_id(rhs->t1->data.var, table_index)->val_num, 
										   rhs->t2->data.constant, rhs->op, false, table_index);
		/* set t1 to whether or not they are true */
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = new_t1;
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}

	return rhs;
}

/* optimizes >, <, and != given they can only
   be optimized to false if the variables 
   are the same */
RHS * optimize_ne_gt_lt_exprs(RHS *rhs, int table_index)
{
	/* b != b, b > b, b < b, replace with 0 for false 
	   t1 and t2 must both be vars with same val nums */
	if(rhs->t1->type == TERM_TYPE_VAR && rhs->t2->type == TERM_TYPE_VAR
		&& have_same_val_nums(rhs->t1->data.var, rhs->t2->data.var, table_index))
	{
		rhs->t2 = NULL;
		rhs->op = STAT_NONE;
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = 0;
	}

	/* if t1 is a constant and t2 is currently eval 
 	to a constant */
	else if(rhs->t1->type == TERM_TYPE_CONST && is_current_constant(rhs->t2->data.var, table_index))
	{
		int new_t1 = const_val_num_matches(eval_id(rhs->t2->data.var, table_index)->val_num, 
										   rhs->t1->data.constant, rhs->op, true, table_index);
		/* set t1 to whether or not they are true */
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = new_t1;
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}

	/* if t2 is a constant and t1 is currently eval 
 	to a constant */
	else if(rhs->t2->type == TERM_TYPE_CONST && is_current_constant(rhs->t1->data.var, table_index))
	{
		int new_t1 = const_val_num_matches(eval_id(rhs->t1->data.var, table_index)->val_num, 
										   rhs->t2->data.constant, rhs->op, false, table_index);
		/* set t1 to whether or not they are true */
		rhs->t1->type = TERM_TYPE_CONST;
		rhs->t1->data.constant = new_t1;
		rhs->op = STAT_NONE;
		rhs->t2 = NULL;
	}

	return rhs;
}

///////////////////////////////////////////////////////////////////////////////////


bool is_var_and_currently_0(Term *t, int table_index)
{
	return is_var_and_currently_num(0, t, table_index);
}

bool is_var_and_currently_1(Term *t, int table_index)
{
	return is_var_and_currently_num(1, t, table_index);
}

/*takes in a term and returns true if the term is 
a var and that var is currently evaluated to a 
constant that is the num param in the current table */
bool is_var_and_currently_num(int num_to_compare, Term *t, int table_index)
{
	if(t->type == TERM_TYPE_CONST)
	{
		return false; /* not a var */
	}

	Val_obj *id_obj = eval_id(t->data.var, table_index);

	if(id_obj->is_const)
	{
		std::unordered_map<int, int>::const_iterator constant = const_table_find(id_obj->val_num, 
																				 table_index);

		/* has not yet been added */
		if(constant == const_tables[table_index]->end())
		{
			cout << "is_var_and_currently_num\n";
			error_unknown(-1); /*this should not happen */
			return false;
		}

		return (constant->first == num_to_compare)? true : false;
	}

	return false; /* val was not a constant */
}

/* takes in a char * and check to see if it is 
currently evaluating to a constant */
bool is_current_constant(char *id, int table_index)
{
	return eval_id(id, table_index)->is_const;
}

/*takes in a constant val number to find the corresponding constant
and compares the numbers depending on the relational op 
if t1 is the constant t1_is_const will be true */
int const_val_num_matches(int val_num, int constant, int relop, bool t1_is_const, int table_index)
{
	std::unordered_map<int, int>::const_iterator const_it = const_table_find(val_num, table_index);

	/*Expression has not yet been added */
	if(const_it == const_tables[table_index]->end())
	{
		cout << "const_val_num_matches\n";
		error_unknown(-1); /*this should not happen */
		return 0;
	}

	int found_const = const_it->first;

	switch(relop)
	{
		case STAT_EQUAL:
			return (constant == found_const)? 1 : 0;
			break;
		case STAT_NOTEQUAL:
			return (constant != found_const)? 1 : 0;
			break;
		case STAT_LT:
			return ((constant < found_const) && t1_is_const)? 1 : 0;
			break;
		case STAT_GT:
			return ((constant > found_const) && t1_is_const)? 1 : 0;
			break;
		case STAT_LE:
			return (((constant < found_const) && t1_is_const) || (constant == found_const))? 1 : 0;
			break;
		case STAT_GE:
			return (((constant > found_const) && t1_is_const) || (constant == found_const))? 1 : 0;
			break;
	}

	cout << "const_val_num_matches\n";
	error_unknown(-1); /*should not reach here */
	return 0;
}

/* looks up in id table return true if have same val nums */
bool have_same_val_nums(char *var1, char *var2, int table_index)
{
	return eval_id(var1, table_index)->val_num == eval_id(var2, table_index)->val_num;
}



/* Finds the entry in the constant table that corresponds to the 
	incoming val_num or the end if not found */
std::unordered_map<int, int>::const_iterator const_table_find(int val_num, int table_index)
{
	for (auto it = const_tables[table_index]->begin(); it != const_tables[table_index]->end(); ++it )
	{
		if(it->second == val_num)
			return it;
	}

	return const_tables[table_index]->end();
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

	
	/* Receive the starting block of the then statement */
	int if_st1_index = add_if_body_to_cfg(ifs->s1, parent);
	int end_st1_index = current_bb;

	/* same logic as above for the else stmt */
	int if_st2_index = add_if_body_to_cfg(ifs->s2, parent);
	int end_st2_index = current_bb;
	
	cfg[parent]->children.push_back(if_st1_index);
	cfg[parent]->children.push_back(if_st2_index);

	std::vector<int> if_statements_index;
	if_statements_index.push_back(end_st1_index);
	if_statements_index.push_back(end_st2_index);

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

	cout << "factor_term_count\n";
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
	
	cout << "primary_term_count\n";
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

	cout << "gen_term_from_factor\n";
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

	cout << "gen_term_from_primary\n";
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

bool has_one_parent_above(int index)
{
	int parent_above_count = 0;
	for(int i=0 ;i<cfg[index]->parents.size(); i++)
	{
		if(index > cfg[index]->parents[i])
		{
			parent_above_count++;
		}
	}

	return (parent_above_count == 1)? true : false;
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
	/*if the node has only one parent directly above him, 
	  he can remain in the same bb as him */	
	else if(!has_one_parent_above(bb_index))
	{
		if(!is_separated_from_parents(bb_index))
		{
			cfg[bb_index]->extended_bb = extended_bb_label;
			extended_bb_label++;
			changed = true;
		}
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
		printf("\n \nCURRENT BB INDEX: %d\n", i);
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

			printf("\tASSIGNMENT: ");
			
			printf("%s = ", stmt->lhs);
			if(stmt->rhs->t1->sign == STAT_SIGN_NEGATIVE)
			{
				printf("-");
			}
			printf("%s", t1);

			if(stmt->rhs->t2 != NULL)
			{
				printf(" %s ", op.c_str());
				if(stmt->rhs->t2->sign == STAT_SIGN_NEGATIVE)
				{
					printf("-");
				}
				printf("%s", t2);
			}
			cout << endl;
			
		}

		printf("-------------------------------------------------------------");
	}
}
