#include "parallel.h"
#include "flux.h"
#include "series.h"
#include "phash.h"
#include "memory.h"
#include "dependency.h"

/***********************************************************************************
 * Series query "object" manipulation
 */

inline void series_internal_build_query_key(series_query_container * c, series_query * q,
											phash ** ntable,s_expr ** key) {
	if (q->type & SERIES_QUERY_MMDEF) {
		*ntable = c->mem_data;
		*key = q->target;
				log_debug("built mem access key for %s",series_print_expr(q->target));
	}
	else {
		*ntable = c->queries;
		*key = q->target;
	}
	
}

void series_internal_remove_query(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;

	series_internal_build_query_key(c,q,&ntable,&key);

	ph_del(expr_hash(key),key,ntable);
}

void series_internal_add_query(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;
	series_internal_build_query_key(c,q,&ntable,&key);
	ph_put(expr_hash(key),key,q,ntable);

}

series_query * series_internal_get_query(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;

	series_internal_build_query_key(c,q,&ntable,&key);

	return (series_query*)ph_get(expr_hash(key),key,ntable);
}

/*void series_internal_remove_memquery(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;

	//series_internal_build_query_key(c,q,&ntable,&key);
	ntable = c->mem_data;
	key = q->target;

	ph_del(expr_hash(key),key,ntable);
}

void series_internal_add_memquery(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;

	ntable = c->mem_data;
	key = q->target;

	ph_put(expr_hash(key),key,q,ntable);

}

series_query * series_internal_get_memquery(series_query_container * c, series_query * q) {
	phash * ntable;
	s_expr * key;

	ntable = c->mem_data;
	key = q->target;

	return (series_query*)ph_get(expr_hash(key),key,ntable);
}*/

series_query * series_internal_get_query_key(series_query_container * c, s_expr *key) {
	return (series_query*)ph_get(expr_hash(key),key,c->queries);
}



/* merges the queries, we always insert q2 in q1, we do not free q2 here, which is
	responsibility of the caller */
void series_merge_queries(series_query * q1, series_query * q2) {
	unsigned int i,j;
	if (q1 == q2) {
		log_debug("ERROR - trying to merge a query with itself");
		return;
	}
	log_debug("Merging queries with targets %s and %s (%X %X)",series_print_expr(q1->target),series_print_expr(q2->target),q1,q2);
	
	/*
	if ((*q2)->root_query == (*q1)) {
		series_query * t = *q2;
		*q2 = *q1;
		*q1 = t;
	}
	else if ((*q1)->root_query == (*q2)) {

	}*/

	if (q1->target) {
		for(j=0;j<q2->exprs.size;j++) {
			s_expr * ex2 = ea_get_element(s_expr**,q2->exprs,j);
			int mtch = 0;
			for(i=0;i<q1->exprs.size;i++) {
				s_expr * ex = ea_get_element(s_expr**,q1->exprs,i);
				if (series_ex_match(ex,ex2)) {
					log_debug("We had a match of expressions, between %s and %s",series_print_expr(ex),series_print_expr(ex2));
					mtch = 1;
					break;
				}
			}
			if (!mtch) {
				log_debug("%i %X",q1->exprs.size,ex2);
				ea_add_element_p(&q1->exprs,ex2);
				log_debug("%i %X %X",q1->exprs.size,ea_get_element(s_expr**,q1->exprs,0),ea_get_element(s_expr**,q1->exprs,1));
				log_debug("Adding expression %s",series_print_expr(ex2));

				/* here we remove the expression from q2, to prevent it from being
					freed later */
				ea_remove_element(&q2->exprs,j);
				j--;
				log_debug("%i",q2->exprs.size);
			}
		}		
	}
	else {

	}

}


/***********************************************************************************
 * Queue & analysis sequence
 */

series_queue_fn_entry * fn_queue_h = 0;
series_queue_fn_entry * fn_queue_t = 0;

series_fn * current_function;

/* stores the keys here, since we have to look in the queue to see if a particular
	branch has already been queued, we use a hash to increase the efficiency of the
	process */
phash * squeue_hash;

void series_queue_init() {
	
	squeue_hash = ph_create(2,8,0);
}

/**
 * Searchs all critical points, and replace q2 refs with q1
 */
void series_flush_cp(series_query * q1, series_query * q2) {
	P_HASH_ITERATE(it,current_function->critical_points,
		series_critical_point * cp = (series_critical_point*)it->data;
		if (cp->write_q == q2) {
			cp->write_q = q1;
		}
		if (cp->read_q == q2) {
			cp->read_q = q1;
		}
		);
}

/* TODO there are at least 4 points creating series_queue_entry, join them */
static void add_queue_block_i(series_queue_entry * pos, dword rep, dword flags) {
	series_queue_entry * pnext;
	if (!pos) {
		pnext = current_function->squeue_h;
		current_function->squeue_h = (series_queue_entry*)malloc(sizeof(series_queue_entry));
		current_function->squeue_h->addr = rep;
		current_function->squeue_h->st_addr = rep;
		current_function->squeue_h->next = pnext;
		current_function->squeue_h->visit_count = 0;
		current_function->squeue_h->flags = flags;
		ea_init_array(&current_function->squeue_h->alternate_routes,sizeof(dword));

		if (!pnext) {
			current_function->squeue_t = current_function->squeue_h;
		}
	}
	else {
		pnext = pos->next;
		pos->next = (series_queue_entry*)malloc(sizeof(series_queue_entry));
		pos->next->addr = rep;
		pos->next->st_addr = rep;
		pos->next->next = pnext;
		pos->next->visit_count = 0;
		pos->next->flags = flags;
		ea_init_array(&pos->next->alternate_routes,sizeof(dword));
		if (!pnext) {
			current_function->squeue_t = pos->next;
		}
	}

	ph_put(rep,(void*)rep,current_function->squeue_t,squeue_hash);

}

static void add_queue_block_ex(series_queue_entry * bl, series_fn * func) {
	log_debug("add_queue_block_ex %X %X",bl->addr,func);
	if (!func->squeue_t) {
		func->squeue_h = bl;
		func->squeue_t = func->squeue_h;
	}
	else {
		func->squeue_t->next = bl;
		func->squeue_t = func->squeue_t->next;
	}
	func->squeue_t->next = 0;

}

static void add_queue_block(series_fn * func, dword rep, dword flags) {
	if (func->squeue_h && func->series_use_partial_queue) {
		series_queue_entry * t;
		log_data("Adding block on partial queue, %X",func->squeue_ref);
		if (!func->squeue_ref) {
			t = func->squeue_h;
			func->squeue_h = (series_queue_entry*)malloc(sizeof(series_queue_entry));
			func->squeue_ref = func->squeue_h;
		}
		else {
			t = func->squeue_ref->next;
			func->squeue_ref->next = (series_queue_entry*)malloc(sizeof(series_queue_entry));
			func->squeue_ref = func->squeue_ref->next;
		}
		
		func->squeue_ref->next = t;
		func->squeue_ref->addr = rep;
		func->squeue_ref->st_addr = rep;
		func->squeue_ref->flags = flags;
		func->squeue_ref->visit_count = 0;
		ea_init_array(&func->squeue_ref->alternate_routes,sizeof(dword));
		log_data("%X %X",func->squeue_ref->addr,func->squeue_ref->next);
	}
	else {
		series_queue_entry * t = (series_queue_entry*)malloc(sizeof(series_queue_entry));
		t->addr = rep;
		t->st_addr = rep;
		t->flags = flags;
		t->visit_count = 0;
		
		ea_init_array(&t->alternate_routes,sizeof(dword));
		
		add_queue_block_ex(t,func);

		if (func->series_use_partial_queue) {
			func->squeue_ref = func->squeue_h;
		}

	}

	ph_put(rep,(void*)rep,func->squeue_t,squeue_hash);
}

void series_push_fn() {
	log_debug("pushing current function. Addr: %X",current_function->function->src);
	series_queue_fn_entry * t = fn_queue_h;
	fn_queue_h = (series_queue_fn_entry*)malloc(sizeof(series_queue_fn_entry));
	fn_queue_h->fn = current_function;
	fn_queue_h->next = t;
	if (!t) {
		fn_queue_t = t;
	}
	
}

void series_wrap_loop_function() {
	series_dump_function("local.dump.txt");
	//exit(0);
}

series_fn * series_pop_fn() {
	series_wrap_loop_function();
	log_debug("popping current function. Addr: %X",current_function->function->src);
	current_function = fn_queue_h->fn;
	series_queue_fn_entry * t = fn_queue_h;
	series_fn *f = fn_queue_h->fn;
	fn_queue_h = fn_queue_h->next;
	free(t);
	return f;
}

void series_sort_blocks(dword * rep, dword sz) {
	dword i,j;

	for(i=0;i<sz;i++) {
		if (FLUX_IS_LOOP(rep[i])) {
			for(j=i+1;j<sz;j++) {
				if (!FLUX_IS_LOOP(rep[j])) {
					dword tmp = rep[i];
					rep[i] = rep[j];
					rep[j] = tmp;
				}
			}
		}
	}
}

void series_queue_blocks(dword * rep, dword sz) {
	series_queue_entry * qstart = current_function->squeue_t;
	series_queue_entry * tmp;

	series_queue_entry * last_block = 0;
	dword i,j;
	int has_loop = 0;
	series_fn * cur_func = current_function;
	series_sort_blocks(rep,sz);

	for(i=0; i<sz; i++) {
		dword addr = FLUX_BRANCH_ADDR(rep[i]);
		log_data("requesting block add %X.",addr);
		int to_queue = 1;
		if (tmp = (series_queue_entry *)ph_get(addr,(void*)addr,squeue_hash)) {
			log_debug("Block already exists. Current %X, %X. Bl %X",cur_func->current_block,cur_func->current_block->addr,tmp);
			/* block already exists, let's examine it */
			if (tmp->flags & SERIES_BLK_FLAG_VISITED) {
				/* already visited and back in queue? probably a loop */
				log_data("Re-queue requested on already visited block. Addr: %X. Flags %X",tmp->addr,tmp->flags);
				if (tmp->flags & SERIES_BLK_FLAG_LOOP_EXIT) {
					log_data("Block is loop exit, and has been visited. Ignoring.");
					continue;
				}
				if (tmp->flags & SERIES_BLK_FLAG_LOOP) {
					/* finished the loop visit, re-queue the block for proper 
						analysis (notice that the loop end must be queued AFTER
						other branches here, if any)*/
					//current_function->squeue_t = current_function->squeue_ref;
					//to_queue = 2;
					//last_block = tmp;
					//add_queue_block(cur_func,addr,0);					
					add_queue_block_ex(tmp,cur_func);
				}
			}
			continue;
		}

		if (cur_func->current_block && cur_func->current_block->flags & SERIES_BLK_FLAG_LOOP) {
			/* if we are a loop function, then we may have already queued
				the target as external function, check it */
			for(j=0; j<cur_func->current_block->alternate_routes.size; j++) {
				if (ea_get_element(dword*,cur_func->current_block->alternate_routes,j) == addr) {
					log_debug("Not queuing block, since it was treated as function: %X",addr);
					to_queue = 0;
					break;
				}
			}
		}
		if (to_queue == 0) {
			continue;
		}
		log_data("Block add %X, %i",addr, to_queue);
		if (to_queue == 1) {			
			if (FLUX_IS_LOOP(rep[i])) {
				current_function->current_block->flags |= SERIES_BLK_FLAG_LOOP;
				has_loop = 1;
				series_push_fn();
				ea_add_element(&current_function->current_block->alternate_routes,addr);
				function_table_entry * fe = flux_function_generic(addr);
				series_current_function(fe,NULL);
				current_function->type = FUNCTION_TYPE_LOOP;
				add_queue_block(current_function,addr,SERIES_BLK_FLAG_LOOP_EXIT);
			}
			else {				
				add_queue_block(cur_func,addr,0);
			}
		}
		else if (to_queue == 2) {
			/* we must leave this block to the end */
		}
		log_data("Block added %X",addr);
	}	
		
	if (last_block) {
		log_data("last block %X",last_block);
		/* get back into the queue, at the last position (from our point of view, i.e. squeue_ref matters */
		if (current_function->squeue_ref) {
			tmp = current_function->squeue_ref->next;
			current_function->squeue_ref->next = last_block;
			last_block->next = tmp;
		}
		else {
			tmp = current_function->squeue_h;
			current_function->squeue_h = last_block;
			current_function->squeue_ref = current_function->squeue_h;
			last_block->next = tmp;
		}
		
	}
	if (has_loop) {
		
	}

}

series_queue_entry * series_pool() {
	series_queue_entry * t = current_function->squeue_h;
series_queue_entry * tt = current_function->squeue_h;log_data("pooling %X %X",tt,current_function); while (tt) {log_data("queue addr %X",tt->addr);tt = tt->next;}

	if (!t) {
		/* TODO improve this, 0 may be an address */
		return 0;
	}
	current_function->squeue_h = current_function->squeue_h->next;
	if (!current_function->squeue_h) {
		current_function->squeue_t = current_function->squeue_h;
	}
	dword addr = t->addr;

	if (t == current_function->squeue_ref) {
		current_function->squeue_ref = 0;
	}
	
	//ph_del(addr,(void*)addr,squeue_hash);
	//free(t);

	//return addr;
	return t;
}

/***********************************************************************************
 * Series building
 */

#define STATE_OPERAND	0
#define STATE_OPERATOR	1

unsigned char * series_do_replace(s_expr * val, s_expr * trans, s_expr * target,
					   unsigned char * c) {

	unsigned char * newc;

	if (val->expr.size != trans->expr.size) {
		int dif = val->expr.size-trans->expr.size;
		if (val->expr.size > trans->expr.size) {
			unsigned char * tmp = c+trans->expr.size;
			unsigned char * limit = c+(target->expr.size-(val->expr.size - trans->expr.size));
			log_debug("tmp %X %X %X %i %i",tmp,c,limit,limit-tmp,dif);
			while (tmp < limit) {
				*tmp = *(tmp+dif);
				tmp++;
			}
		}
		else if (val->expr.size < trans->expr.size) {
			unsigned int coff = c-(unsigned char*)target->expr.arr;
			ea_guarantee_size(&target->expr,-dif);
			c = (unsigned char*)target->expr.arr+coff;
			unsigned char * tmp = (unsigned char*)target->expr.arr+target->expr.size-dif;
			unsigned char * limit = c+trans->expr.size;

			log_debug("tmp %i %X %X %X %i %i %i %i %X",c-tmp,tmp,c,limit,limit-tmp,dif,trans->expr.size,target->expr.size,target->expr.arr);
			while (tmp >= limit) {
				*tmp = *(tmp+dif);
				tmp--;
			}			
		}
		target->expr.size += (trans->expr.size - val->expr.size);
		
	}

	newc = c+trans->expr.size;
	memcpy(c,trans->expr.arr,trans->expr.size);

	return newc;

}

int series_cmpa(s_expr * val, unsigned char * c) {
	return memcmp(val->expr.arr,c,val->expr.size);
}

int series_cmp(s_expr * val, s_expr * val2) {
	if (val->expr.size != val2->expr.size) {
		return val->expr.size - val2->expr.size;
	}
	return memcmp(val->expr.arr,val2->expr.arr,val->expr.size);
}

inline void series_cat(s_expr * dest, s_expr * val) {
	ea_guarantee_size(&dest->expr,val->expr.size);
	unsigned char * p = (unsigned char *)ea_get_ptr(&dest->expr);
	unsigned char * pv = (unsigned char *)ea_get_ptr(&val->expr);
	memcpy(p,val->expr.arr,val->expr.size);
	dest->expr.size += val->expr.size;
}

/* deterministic finite automaton for replacing an expression by another,
	maybe generalize it for parsing? */
int series_dfa_replace(s_expr * val, s_expr * trans, s_expr * target) {
	unsigned char * c = (unsigned char*) target->expr.arr;
	unsigned char * base = c;
	
	unsigned char * valc = (unsigned char *) val->expr.arr;
	unsigned char * tc = (unsigned char *) trans->expr.arr;
		char dbg1[256];char dbg2[256];char dbg3[256];series_to_readable(val,dbg1);series_to_readable(trans,dbg2);series_to_readable(target,dbg3);
		char dbout[2048];sprintf(dbout,"automaton replace %s with %s on %s",dbg1,dbg2,dbg3);log_data(dbout);

	int found = 0;
	int state = STATE_OPERAND;
	do {
		if (state == STATE_OPERAND) {
			if (*c == SERIES_NUM) {
				/* skip number */
				c++;
				c += (sizeof(int));
				state = STATE_OPERATOR;
			}
			else {
				if (!series_cmpa(val,c)) {
					found = 1;
					c = series_do_replace(val, trans, target,c);
					base = (unsigned char*)target->expr.arr;
					//c+= trans->expr.size;
					state = STATE_OPERATOR;
				}
				else {
					if (*c != SERIES_MS && *c != SERIES_ME && *c != SERIES_GS && *c != SERIES_GE) {
						state = STATE_OPERATOR;
					}
					/* do nothing */
					c++;
				}
				
			}
		}
		else if (state == STATE_OPERATOR) {
			if (*c != SERIES_MS && *c != SERIES_ME && *c != SERIES_GS && *c != SERIES_GE) {
				state = STATE_OPERAND;
			}
			c++;

		}
		else {
			log_debug("ERROR - invalid state on series machine?");
		}

	} while(c < (base+target->expr.size));

	/* fix the array here */
	int cc = 0;
	unsigned char * t = (unsigned char*) target->expr.arr;
	while (t<c) { cc++; t++; }
	target->expr.size = cc;
	
log_data(series_print_expr(target));

//int i;c = (unsigned char *) target->expr.arr;for(i=0;i<target->expr.size;i++)
//log_data("%X",*(c+i));
	return found;

}

void series_dfa_mem_lookup(unsigned char ** target, unsigned int * sz, s_expr ** out) {
	unsigned char * c = *target;
	unsigned char * fin = (c+*sz);
	unsigned char * base = c;

	unsigned char * cstart;
	(*out) = 0;

	int state = 0;

	while (c < fin) {
		if (*c == SERIES_MS) {
			state = 1;
			cstart = c;
		}
		else if (state && *c == SERIES_ME) {			
			/* found a memory accesss, note: we do not consider nested accesses */
			(*out) = create_expression();			
			ea_guarantee_size(&(*out)->expr,(c-cstart)+1);
			memcpy((*out)->expr.arr,cstart,(c-cstart)+1);
			(*out)->expr.size = (c-cstart)+1;
			break;
		}
		c++;
	}
	(*sz) -= (c-base);
	(*target) = c;
}

unsigned char * series_dfa_lookup(s_expr * lookup, s_expr * target) {
	unsigned char * c = (unsigned char *)target->expr.arr;
	unsigned char * fin = (c+target->expr.size);

	while (c < fin) {
		if (!memcmp(c,target->expr.arr,target->expr.size)) {
			return c;
		}
		c++;
	}
	return NULL;
}

void series_try_solve(s_expr * ex) {
	
}

int series_supress(series_query * root, series_query * q1, series_query * q2) {
	int rt = 0;
	
	if (q1->root_query == q2) {		
		series_query * t = q2;
		q2 = q1;
		q1 = t;
		rt = 1;
	}
	else if (q2 == q2->root_query) {
		/* a root query is being supressed, find and remove it
			from the main queries */
		series_internal_remove_query(&current_function->queries,q2);
	}
	/*for(i=0;i<root->hooks.size;i++) {
		series_hook * h = ((series_hook*)root->hooks.arr)+i;
		if (h->query == q2) {
			h->query = q1;
		}

	}*/
	
//	destroy_expression(q2->expr);
	//if (q2->target) destroy_expression(q2->target);
	//ea_destroy_array(&q2->hooks);
	
	//free(q2);
	return rt;
}

void series_solve(s_expr * ex) {
	char ** ops = (char**)malloc(ex->expr.size*sizeof(char*));
	int k = 0;
	char * t = (char*)ex->expr.arr;
	char * end = t+ex->expr.size;
	while (t < end) {
		t++;		
	}
}

int series_ex_match(s_expr * ex, s_expr * ex2) {
	log_debug("%i %i, %i",ex->expr.size , ex2->expr.size , !memcmp(ex->expr.arr,ex2->expr.arr,ex->expr.size));
	if (ex->expr.size == ex2->expr.size && !memcmp(ex->expr.arr,ex2->expr.arr,ex->expr.size)) {
		return 1;
	}

	/* TODO solve */
	return 0;
}

/*void series_merge() {
	
	/* first remove all repeated queries */
/*	unsigned int i,j;

	for(i=0;i < current_function->cur_queries.size;i++) {
		/* TODO use a hash to improve this */
/*		series_query *q = ((series_query**)current_function->cur_queries.arr)[i];
		log_debug("On %i: %X",i,q);
		for(j=i+1;j < current_function->cur_queries.size;j++) {

			series_query *q2 = ((series_query**)current_function->cur_queries.arr)[j];
			//log_data(" %X %X %i %i; %i %X",q,q2,i,j,current_function->cur_queries.size,((series_query**)current_function->cur_queries.arr)[18]);
			if ( q == q2) {
				log_debug("Removing repeated query from the array %i",current_function->cur_queries.size);
				ea_remove_element(&current_function->cur_queries,j);
				j--;
			}
		}
	}

	for(i=0;i < current_function->cur_queries.size;i++) {

		/* TODO use a hash to improve this */
/*		series_query *q = ((series_query**)current_function->cur_queries.arr)[i];
		
		for(j=i+1;j < current_function->cur_queries.size;j++) {

			series_query *q2 = ((series_query**)current_function->cur_queries.arr)[j];
			//log_data(" %X %X %i %i; %i %X",q,q2,i,j,current_function->cur_queries.size,((series_query**)current_function->cur_queries.arr)[18]);
	
			if (q->target && q2->target) {
				char dbg[256],dbg2[256];
				//series_to_readable(q->target,dbg);series_to_readable(q2->target,dbg2);
				//log_data("%s %s",dbg,dbg2);
				int c1 = series_cmp(q->target,q2->target);
				int c2 = 0;
				if (!c1) {
					c2 = series_cmp(q->expr,q2->expr);
				}
				//log_data("cmp %s %s %i %i;; %i %i" ,dbg,dbg2,c1,c2,i,j);
				int supress = 0;
				if (!c1 && !c2) {
					
					series_to_readable(q->target,dbg);series_to_readable(q->expr,dbg2);
					log_data("merging %s=%s %i %i, %X %X, %X",dbg,dbg2,i,j,q,q2,q->root_query);
					supress = 1;
					//ea_add_element(&indices,j);
				}	
				else if (!c1) {
					//supress = 1;
					series_merge_query(q,q2);
				}

				if (supress) {
					int rt = series_supress(q->root_query,q,q2);
					ea_remove_element(&current_function->cur_queries,j);
					if (rt) {
						/* it was a root query, swap places with q */
/*						((series_query**)current_function->cur_queries.arr)[i] = q2;
						q = q2;
					}					

					j--;
				}
			}
			else if (!q->target && !q2->target) {
				if (!series_cmp(q->expr,q2->expr)) {
					char dbg[256],dbg2[256];series_to_readable(q2->expr,dbg);series_to_readable(q->expr,dbg2);
					log_data("merging 0=%s %s %X %X %i",dbg,dbg2,q,q2,current_function->cur_queries.size);
					int rt = series_supress(q->root_query,q,q2);
					if (rt) {
						/* it was a root query, swap places with q */
/*						((series_query**)current_function->cur_queries.arr)[i] = q2;
						q = q2;
					}
										
					ea_remove_element(&current_function->cur_queries,j);
						int k;for(k = 0; k < current_function->cur_queries.size ; k++) {
							series_query * q = ((series_query**)current_function->cur_queries.arr)[k];
							log_data("On %i %X",k,q);
						}
					j--;
				}
			}			
		}
	}

}*/

void series_query_destroy(series_query * q) {
	unsigned int i;
	for(i=0;i<q->exprs.size;i++) {
		s_expr * ex = ea_get_element(s_expr**,q->exprs,i);
		destroy_expression(ex);
	}
	ea_destroy_array(&q->exprs);
	destroy_expression(q->target);
	free(q);
}

series_query * series_add_memory_access(s_expr * form, int type, dword src) {
	series_query * q = (series_query*) calloc(1,sizeof(series_query));
	q->target = form;
	q->type = type | SERIES_QUERY_MMDEF;
	series_query * q2;

	if (q2 = series_internal_get_query(&current_function->cur_queries,q)) {
		/* we already have an equivalent query, use it */
		q2->type |= q->type;
		free(q);
		q = q2;
	}
	else {
		series_internal_add_query(&current_function->cur_queries,q);
	}

	series_critical_point * cp;
	cp = (series_critical_point*)ph_get(src,(void*)src,current_function->critical_points);
	if (!cp) {
		cp = (series_critical_point*)calloc(1,sizeof(series_critical_point));
		cp->address = src;
		cp->block = current_function->current_block;
		ph_put(src,(void*)src,cp,current_function->critical_points);
	}

	if (type & SERIES_QUERY_MW) {
		cp->type = SERIES_QUERY_MW;
		cp->write_q = q;
	}
	if (type & SERIES_QUERY_MR) {
		cp->type = SERIES_QUERY_MR;
		cp->read_q = q;
	}

	return q;

}

series_query * series_add_query_expression(s_expr * ex,s_expr * target, dword src, int type) {	
	log_debug("Adding query with target %s, expr is %s",series_print_expr(target),series_print_expr(ex));
	series_query *q = (series_query*) calloc(1,sizeof(series_query));
	q->target = target;
	q->type = type;
	q->root_query = q;
	//if (SERIES_IS_MEM_ACCESS(q->target)) {
	//	q->type |= SERIES_QUERY_MEM;
	//}
	ea_init_array(&q->exprs,sizeof(s_expr*));
	q->generation_rules = ph_create(2,2,0);
	series_query * q2;

	if (q2 = series_internal_get_query(&current_function->queries,q)) {
		//log_data("ERROR - re-adding already existing query");
		log_data("Adding to existing query.");
		/* we already have a query for this target. It could be an error, but can also be a valid situation, when we
			generate it in another branch, so add to it for now, and set it as a 'cloned query'
			NOTE here we aren't actually cloning the query, the previous context is probably irrelevant
			for us, we will however merge them eventually if necessary */
		//ea_add_element_p(&q2->cloned_queries,q);
		//ea_destroy_array(&q->exprs);
		//free(q);
		//q = q2;		
	}

	ea_add_element_p(&q->exprs,ex);

	series_internal_add_query(&current_function->queries,q);
	series_internal_add_query(&current_function->cur_queries,q);

	if (target) {
		if (((unsigned char *)target->expr.arr)[0] == SERIES_MS) {
			q->type |= SERIES_QUERY_MEM;
		}
		else {
			q->type |= SERIES_QUERY_REG;
		}
	}

	q->src = src;
	
	return q;
}

int series_evaluate(s_expr * target) {
	
	if (series_internal_get_query_key(&current_function->cur_queries,target)) {
		return 1;
	}
	
	return 0;

}

int series_is_num(s_expr * ex) { 
	return ((unsigned char*)ex->expr.arr)[0] == SERIES_NUM;
}

int series_mem_symbol = 0;

s_expr * series_merged_exprs(series_query * q) {
	s_expr * mex = create_expression();
	unsigned int i;
	
	s_expr * ex = ea_get_element(s_expr**,q->exprs,0);
	log_debug("merge expr %s",series_print_expr(ex));
	ea_guarantee_size(&mex->expr,ex->expr.size);
	char * c = (char*)(mex->expr.arr);
	memcpy(c,ex->expr.arr,ex->expr.size);
	c+= ex->expr.size;
	mex->expr.size += ex->expr.size;
	log_debug("%s %i %i",series_print_expr(ex),ex->expr.size,mex->expr.fsize);
	for(i=1;i<q->exprs.size;i++) {
		expr_or(mex);
		c++;
		ex = ea_get_element(s_expr**,q->exprs,i);
		log_debug("merge expr2 %s",series_print_expr(ex));
		ea_guarantee_size(&mex->expr,ex->expr.size);
		memcpy(c,ex->expr.arr,ex->expr.size);
		c+= ex->expr.size;
		mex->expr.size += ex->expr.size;
	}
	
	return mex;
}

void series_query_value(s_expr * val, s_expr * trans, dword src, int newval) {
	unsigned int i,j;
	int found = 0;

	ex_array mem_changes; 
	ea_init_array(&mem_changes,sizeof(series_query*));

	ex_array to_add_queries;
	ea_init_array(&to_add_queries,sizeof(series_query*));

	log_debug("Query val for %s = %s, number of queries is %i",series_print_expr(val),series_print_expr(trans),current_function->cur_queries.queries->num_entries);
	P_HASH_ITERATE(it,current_function->cur_queries.queries,
	{
		series_query *q = (series_query*)it->data;
		log_debug("Examining target %s",series_print_expr(q->target));
		if (q->target == val) {
			P_HASH_CONTINUE(it,current_function->cur_queries.queries);
		}

		for(i=0;i<q->exprs.size;i++) {
			s_expr * ex = ea_get_element(s_expr**,q->exprs,i);
			log_debug("Examining expr %s",series_print_expr(ex));
			int rt = series_dfa_replace(val,trans,ex);
			found |= rt;
			if (rt) {
				log_debug("found and replaced on %s",series_print_expr(ex));
			}
		}

		if (q->type & SERIES_QUERY_MEM) {
			/* we must replace the left side expression also? */
			log_debug("Found a memory access expression. Target is %s",series_print_expr(q->target));

			int rt = series_dfa_replace(val,trans,q->target);
			q->target->hash = 0;

			if (rt) {
				log_debug("found and replaced on %s",series_print_expr(q->target));
				
				series_query * q2 = series_internal_get_query(&current_function->cur_queries, q);

				if (!q2) {
					//series_internal_add_query(&current_function->cur_queries, q);
					/* can't add to hash here, lest we detroy the iterator */
					ea_add_element_p(&to_add_queries,q);
				}
				else {
					log_debug("Merging with another clashed query");
					/* here we have a 'clash', meaning that two distinct queries converged to
						the same one, merge them */
					/* NOTE we merge q into q2, so we don't need to update the hash */
					series_merge_queries(q2,q);
					series_flush_cp(q2,q);
					destroy_expression(q->target);
					ea_destroy_array(&q->exprs);
					free(q);
					q = q2;
				}

				/* now we add the query, since there was an alteration to a internal memory access
				*/
				
				ea_add_element_p(&mem_changes,q);

				P_HASH_REMOVE_IT(it,current_function->cur_queries.queries);

			}
		}
		
	}
	);

	P_HASH_ITERATE(it,current_function->cur_queries.mem_data,
		/* here we examine the memory accesses, and also update them, we
			want to know what they translate to */
		series_query *q = (series_query*)it->data;
		int rt = series_dfa_replace(val,trans,q->target);
		found |= rt;
		if (rt) {
			log_debug("found and replaced on %s",series_print_expr(q->target));
		}
	);

	for(j=0;j<to_add_queries.size;j++) {
		series_query *q = *(((series_query**)to_add_queries.arr)+j);
		series_internal_add_query(&current_function->cur_queries, q);
	}

	ea_destroy_array(&to_add_queries);
	for(j=0;j<mem_changes.size;j++) {
		/* for each memory change, we must check if it matches internal expressions.
			this is equivalent to bringing new target->expr pairs to the situation */
		series_query *q = *(((series_query**)mem_changes.arr)+j);
		log_debug("Querying recursively.");
		log_debug(series_print_expr(q->target));
		series_query_value(q->target,series_merged_exprs(q),src,0);
	}

	ea_destroy_array(&mem_changes);
	
	if (newval && !series_is_num(val)) {// && !series_is_num(trans) ) {
		found = series_evaluate(val);	
		if (!found) {
			series_add_query_expression(trans,val,src,0);
		}
	}

	if (newval) {
		/* finally we must sweep the expressions to find if there are memory accesses, and 
			create special entries for those */
		
		if (SERIES_IS_MEM_ACCESS(val)) {
			/* if target is a mem access, we have a write */
			char * ms = ((char*)val->expr.arr)+1;
			char * c = ((char*)val->expr.arr)+val->expr.size-1;
			s_expr * inner_memory = subset_expression(ms,c);

			series_add_memory_access(inner_memory,SERIES_QUERY_MW,src);

		}

		uint32 i;
		char * c = (char*)trans->expr.arr;
		char * ms = NULL;
		for(i=0;i<trans->expr.size;i++) {
			if (c[i] == SERIES_MS) {
				/* we found a memory access, since in i386 there are no multiple memory access
					instructions, we will assume it is the only one */
				ms = c;
			}
			else if (c[i] == SERIES_ME) {
				if (!ms) {
					log_debug("ERROR - invalid memory access state?");
				}
				c = c+i;
				break;
			}
			c++;
		}
		if (ms) {
			/* we have a memory read */
			/* we exclude both the MS and ME ([ and ]) symbols here */
			s_expr * inner_memory = subset_expression(ms+1,c);
			log_debug("created mem access for %s, %i %i %X %i",series_print_expr(inner_memory),*(ms+1),*c,trans->expr.arr, inner_memory->expr.size);
			series_add_memory_access(inner_memory,SERIES_QUERY_MR,src);
			
		}
	}
	
}

ex_array series_all_functions;

void series_current_function(function_table_entry * fn, phash * inherited_hooks) {
	series_fn * sfn = (series_fn*)calloc(1,sizeof(series_fn));
	sfn->fn_info = (series_fn_info*)malloc(sizeof(series_fn_info));
	sfn->function = fn;
	//ea_init_array(&sfn->queries,sizeof(series_query*));
	//ea_init_array(&sfn->cur_queries,sizeof(series_query*));
	sfn->queries.queries = ph_create(2,4,0);
	sfn->queries.mem_data = ph_create(2,4,0);
	sfn->cur_queries.queries = ph_create(2,4,0);
	sfn->cur_queries.mem_data = ph_create(2,4,0);

	if (inherited_hooks) {
		sfn->hooks = inherited_hooks;
	}
	else {
		sfn->hooks = ph_create(2,4,0);
	}

	sfn->critical_points = ph_create(2,16,0);

	ea_add_element_p(&series_all_functions,sfn);
	current_function = sfn;
	series_clear_fn();
}

series_query * series_clone_query(series_query * query) {
	series_query * q = (series_query *)malloc(sizeof(series_query));
	//log_data("cloning %s",series_print_expr(query->target));
	
	q->t_link = query->t_link ? series_clone_query(query->t_link) : 0;
	q->type = query->type;
	q->msymbol = query->msymbol;
	ea_init_array(&q->exprs,sizeof(s_expr*));
	uint32 i;
	for(i=0;i<query->exprs.size;i++) {
		s_expr * ex = create_expression();
		s_expr * qex = ea_get_element(s_expr**,query->exprs,i);
		ea_clone_array(&ex->expr,&qex->expr);
		ea_add_element_p(&q->exprs,ex);
	}

	q->src = query->src;
	q->root_query = query->root_query;
	if (query->target) {
		q->target = create_expression();
		ea_clone_array(&q->target->expr,&query->target->expr);
	}
	else {
		q->target = 0;
	}

	//ea_add_element_p(query->cloned_queries,q);
	//q->root_query = query;

	return q;

}

void series_normalize_queries() {
	ph_clear(current_function->queries.queries);
	P_HASH_ITERATE(it,current_function->cur_queries.queries,
		series_internal_add_query(&current_function->queries,(series_query*)it->data);
	);

	P_HASH_ITERATE(it,current_function->cur_queries.mem_data,
		series_internal_add_query(&current_function->queries,(series_query*)it->data);
		);

}

series_fn * series_finish_loop() {
	
	log_debug("Finishing a loop visit");
	series_normalize_queries();
	series_dump_function("series.loop.dump");
	exit(0);
	/* now we build generation rules */
	/*P_HASH_ITERATE(it,current_function->cur_queries.queries,
		series_query * q = (series_query*)it->data;
		/*s_expr * ex;
		ex = (s_expr*)q->exprs.arr;
		/*if (q->exprs.size == 1 && !series_cmp(q->target,ex)) {
			/* the in expression is the same as the out, do nothing */
		/*	P_HASH_CONTINUE(it,current_function->cur_queries.queries);
		}
		int i;
		for(i=0;i<q->exprs.size;i++) {
			ex = ea_get_element((s_expr**),&q->exprs,i);
			series_dfa_lookup(
		}*/
	/*	P_HASH_ITERATE(it2,current_function->cur_queries.queries,
			series_query * q2 = (series_query*)it2->data;
			for(i=0;i<q->exprs.size;i++) {
				ex = ea_get_element((s_expr**),&q->exprs,i);
				unsigned char * c = series_dfa_lookup(q->target,ex);
				if (c) {

				}
			}
			
			);
	);
	
*/
	series_fn * cur = current_function;
	//cur->
	series_fn * l_fn = series_pop_fn();
	return l_fn;

}

void series_hook_query(series_query * q, unsigned int * branches, unsigned int size, unsigned int loopcount) {
	uint32 j;
	int reuse = 1;
	
	for (j=0; j < size ; j++) {
		if (FLUX_IS_LOOP(branches[j]) && current_function->current_block->visit_count > 1) {
			continue;
		}
		series_hook * hook = (series_hook*)malloc(sizeof(series_hook));
		
		dword hook_addr = FLUX_BRANCH_ADDR(branches[j]);

		series_hooks * hk = (series_hooks*)ph_get(hook_addr,(void*)hook_addr,current_function->hooks);
		if (!hk) {
			hk = (series_hooks*)malloc(sizeof(series_hooks));
			hk->hook = hook_addr;
			ea_init_array(&hk->hooks,sizeof(series_hook*));

			ph_put(hook_addr,(void*)hook_addr,hk,current_function->hooks);
		}

		if ((size-loopcount) > 1) {
			/* if we have more than one 'hook' (i.e. we branch to two or more branches)
				each query will become a clone, to be merged later when we merge 
				back into one branch/trunk */
			/* however there's no reason not to reuse the current query in one of the branches,
				so just clone others */
			if (reuse) {
				reuse = 0;
				hook->query = q;
			}
			else {
				hook->query = series_clone_query(q);
			}
		}
		else {
			/* if not, we can reuse the current query without losses */
			hook->query = q;
		}
		log_data("Adding hook to %X, target was %s",hk->hook,series_print_expr(q->target));

		ea_add_element_p(&hk->hooks,hook);			
	}		
	
}

extern unsigned int dbg_addr;

void series_finish_block(unsigned int * branches, unsigned int size) {
	unsigned int j;
	unsigned int loopcount = 0;

	if ((current_function->current_block->flags & (SERIES_BLK_FLAG_LOOP|SERIES_BLK_FLAG_VISITED)) ==
		(SERIES_BLK_FLAG_LOOP|SERIES_BLK_FLAG_VISITED)) {

		series_fn * loop_fn = series_finish_loop();
	}
	/* count the loops retroactive branches, if we already visited this node */
	if (current_function->current_block->visit_count > 1) {		
		for (j=0; j <size ; j++) {
			if (FLUX_IS_LOOP(branches[j]) ) {
				loopcount++;
			}
		}
	}

	log_data("visit count %i",current_function->current_block->visit_count);
	
	P_HASH_ITERATE(it,current_function->cur_queries.queries,
		series_query * q = (series_query*)it->data;
		series_hook_query(q,branches,size,loopcount);
	);

	P_HASH_ITERATE(it,current_function->cur_queries.mem_data,
		series_query * q = (series_query*)it->data;
		series_hook_query(q,branches,size,loopcount);
	);

	ph_clear(current_function->cur_queries.queries);
	ph_clear(current_function->cur_queries.mem_data);

	current_function->current_block->st_addr = dbg_addr;

}

void series_start_block(series_queue_entry * block_s) {
	int block_start = block_s->addr;
	log_data("Starting block analysis at %X. Number of expressions is %i",block_start,current_function->queries.queries->num_entries);
	
	block_s->flags |= SERIES_BLK_FLAG_VISITED;
	block_s->visit_count++;
	current_function->current_block =  block_s;
	unsigned int j;

	series_hooks * hk = (series_hooks*)ph_get(block_start,(void*)block_start,current_function->hooks);

	if (hk) {		
		for (j=0; j <hk->hooks.size ; j++) {
			series_hook * hook = *(((series_hook **)hk->hooks.arr)+j);
			//log_debug("Examining %s",series_print_expr(hook->query->target));
			
			/* TODO remove hook */
			//ea_add_element_p(&current_function->cur_queries,hook->query);
			/* first we look up the table to see if we already have an expr with the same key */
			
			series_query * qex;
		
			qex = series_internal_get_query(&current_function->cur_queries, hook->query);
					
			if (qex) {
				/* if so, we will merge them */
				series_merge_queries(qex,hook->query);
				series_flush_cp(qex,hook->query);
				/* now we must look at the queries hash, and guarantee we point to the query
					we will retain */
				series_internal_add_query(&current_function->queries,qex);
				series_query_destroy(hook->query);
			}
			else {
				/* if not, we simply add it */
				series_internal_add_query(&current_function->cur_queries, hook->query);
			}

			//if (!qex) qex = hook->query;
			log_debug("Hooked query %s on %X", series_print_expr(qex? qex->target : hook->query->target),block_start);			
			//log_debug("%i",qex->exprs.size);int i; for(i=0;i<qex->exprs.size;i++) {log_data("%X",ea_get_element(s_expr**,qex->exprs,i));log_data("%s",series_print_expr(ea_get_element(s_expr**,qex->exprs,i)));}	

		}
	}

	log_data("Block inherited %i queries.",current_function->cur_queries.queries->num_entries);

	/* check all repeated targets and merge when possible */
	//series_merge();
	//if (current_function->queries.size >=1 ) printf("q %X\n",((series_query**)current_function->queries.arr)[0]->target);
	
}

/***********************************************************************************
 * Series control
 */

phash * queried_data = 0;
unsigned int regbitmap;

unsigned int query_hash(s_expr * ex) {
	unsigned int i;
	unsigned int hash = 0;
	if (ex->expr.size < 4) {
		for (i=0; i<ex->expr.size;i++) {
			hash += ((unsigned char *)ex->expr.arr)[i];
		}
	}
	else {
		for (i=0; i<ex->expr.size/4;i++) {
			hash += ((unsigned int *)ex->expr.arr)[i];
		}
	}

	return (hash*hash) ^ ex->expr.size;
}

void series_add_register(int reg, s_expr * ex) {
	switch(reg) {
		case EAX:
			expr_eax(ex);
			break;
		case EBX:
			expr_ebx(ex);
			break;
		case ECX:
			expr_ecx(ex);
			break;
		case EDX:
			expr_edx(ex);
			break;
		case ESI:
			expr_esi(ex);
			break;
		case EDI:
			expr_edi(ex);
			break;
		case ESP:
			expr_esp(ex);
			break;
		case EBP:
			expr_ebp(ex);
			break;
	}
}


void series_parse_expression(mem_full_ref * ref, s_expr ** ex) {
	(*ex) = create_expression();
	switch(ref->base.type) {
		case ID_TYPE_LABEL:
			expr_add_label((*ex),ref->base.arg);
			break;
		case ID_TYPE_ABSOLUTE:
			expr_start_memory((*ex));
			expr_add_number((*ex),ref->base.arg);
			if (ref->disp) {
				expr_add((*ex));
				expr_add_number((*ex),ref->disp);
			}
			expr_end_memory((*ex));
			break;
		case ID_TYPE_REG:			
			series_add_register(ref->base.arg,(*ex));
			break;
		case ID_TYPE_IMMEDIATE:
			expr_add_number((*ex),ref->base.arg);
			break;
		case ID_TYPE_IREG:
			expr_start_memory((*ex));
			series_add_register(ref->base.arg,(*ex));
			if (ref->disp) {
				expr_add((*ex));
				expr_add_number((*ex),ref->disp);
			}
			if (ref->mult) {
				expr_mul((*ex));
				expr_add_number((*ex),ref->mult);
			}
			if (ref->rdisp) {
				expr_add((*ex));
				series_add_register(ref->rdisp-1,(*ex));
			}
			expr_end_memory((*ex));
			break;
	}
}

void series_parse_operation(int operation, mem_full_ref * op1, mem_full_ref * op2, s_expr ** ex) {
	/* encase in () for now */
	s_expr * tmp, * tmp2;
	series_parse_expression(op1,&tmp);
	series_parse_expression(op2,&tmp2);
	(*ex) = create_expression();
	expr_start_group((*ex));
	series_cat((*ex),tmp);
	expr_op((*ex),operation);
	series_cat((*ex),tmp2);
	expr_end_group((*ex));
	destroy_expression(tmp);
	destroy_expression(tmp2);

	printf("parse operation %i %X\n",(*ex)->expr.size,((unsigned char*)(*ex)->expr.arr)[1]);
}


void series_write_internal(s_expr * destx, s_expr * srcx, dword srcaddr, int dtype) {
	series_query_value(destx,srcx,srcaddr,1);
	/*
	switch(dtype) {
		case ID_TYPE_LABEL:
			break;
		case ID_TYPE_ABSOLUTE:
			/*if (!series_evaluate(destx))  {
				series_add_query_expression(srcx,destx,srcaddr);
			}
			else {
				series_query_value(destx,srcx);
			}*//*
			series_query_value(destx,srcx,srcaddr,1);
			break;
		case ID_TYPE_REG:
			/*if (!series_evaluate(destx))  {
				series_add_query_expression(srcx,destx,srcaddr);
			}
			else {
				series_query_value(destx,srcx);
			}*//*
			series_query_value(destx,srcx,srcaddr,1);
			break;
		case ID_TYPE_IMMEDIATE:
			/* shouldn´t happen *//*
			printf("Error - Write on an immediate type?\n");
			break;
		case ID_TYPE_IREG:
			/*if (!series_evaluate(destx))  {
				series_add_query_expression(srcx,destx,srcaddr);
			}
			else {
				series_query_value(destx,srcx);
			}*//*
			series_query_value(destx,srcx,srcaddr,1);
			break;
	}
	*/
	
}

void series_operation(int soperation, mem_full_ref * sop1, mem_full_ref * sop2,
					  int doperation, mem_full_ref * dop1, mem_full_ref * dop2,
					  dword srcaddr) {
	s_expr * srcx, * destx;
	series_parse_operation(soperation,sop1,sop2,&srcx);
	series_parse_operation(doperation,dop1,dop2,&destx);
	series_write_internal(destx,srcx,srcaddr,ID_TYPE_REG);
	
}

void series_operation_dest(int operation, mem_full_ref * op1, mem_full_ref * op2, mem_full_ref * src, dword srcaddr) {
	s_expr * destx, * srcx;
	if (src->base.type == ID_TYPE_REG || src->base.type == ID_TYPE_IREG) {
		dep_set_access(2,src->base.arg,srcaddr,current_function->current_block,current_function);
	}
	
	series_parse_expression(src,&srcx);
	series_parse_operation(operation,op1,op2,&destx);
	series_write_internal(destx,srcx,srcaddr,ID_TYPE_REG);
	
}

void series_operation_src(int operation, mem_full_ref * op1, mem_full_ref * op2, mem_full_ref * dest, dword srcaddr) {
	//log_data("Series operation on SRC. op1: %i, %i, %i. op2: %i, %i, %i. dest: %i, %i, %i. OP %i",
	//	op1->base.type,op1->base.arg,op1->disp,op2->base.type,op2->base.arg,op2->disp,
	//	dest->base.type,dest->base.arg,dest->disp, operation);
	s_expr * destx, * srcx;

	if (op1->base.type == ID_TYPE_REG || op1->base.type == ID_TYPE_IREG) {
		dep_set_access(2,op1->base.arg,srcaddr,current_function->current_block,current_function);
	}

	if (op2->base.type == ID_TYPE_REG || op2->base.type == ID_TYPE_IREG) {
		dep_set_access(2,op2->base.arg,srcaddr,current_function->current_block,current_function);
	}

	if (dest->base.type == ID_TYPE_REG) {
		/* if its a write to a register, just set it here */
		dep_set_access(1,dest->base.arg,srcaddr,current_function->current_block,current_function);
	}
	series_parse_expression(dest,&destx);
	series_parse_operation(operation,op1,op2,&srcx);
	series_write_internal(destx,srcx,srcaddr,dest->base.type);
	
}

void series_write(mem_full_ref * dest, mem_full_ref * src, dword srcaddr) {
	log_data("Series write. src: %i, %i, %X. dest: %i, %i, %i",src->base.type,src->base.arg,src->disp,dest->base.type,dest->base.arg,dest->disp);
	
	s_expr * destx, * srcx;
	series_parse_expression(src,&srcx);
	series_parse_expression(dest,&destx);
	series_write_internal(destx,srcx,srcaddr,dest->base.type);

	if (src->base.type == ID_TYPE_REG || src->base.type == ID_TYPE_IREG) {
		dep_set_access(2,src->base.arg,srcaddr,current_function->current_block,current_function);
	}

	if (dest->base.type == ID_TYPE_IREG) {
		/* if its a write to a register, just set it here */
		dep_set_access(2,dest->base.arg,srcaddr,current_function->current_block,current_function);
	}

	if (dest->base.type == ID_TYPE_REG) {
		/* if its a write to a register, just set it here */
		dep_set_access(1,dest->base.arg,srcaddr,current_function->current_block,current_function);
	}
}

void series_process_call(series_fn_info * fn, dword srcaddr) {
	/* TODO a bit of architectue hardwiring here, move it to an arch/ other place */
	/* first we write on -eax-, x86 hardwiring for return value */
	mem_full_ref dest, src;
	dest.base.type = ID_TYPE_REG;
	dest.base.arg = EAX;
	dest.disp = 0;
	if (fn->ret.flags & RET_FLAG_NEW) {
		src.base.type = ID_TYPE_LABEL;
		src.disp = 0;
		src.base.arg = memory_new_label();
	}
	series_write(&dest,&src,srcaddr);
}

void series_clear_fn() {
	if (queried_data) {
		ph_destroy(queried_data);
	}
	queried_data = ph_create(2,32,0);
	regbitmap = 0;
}

void series_init() {	
	ea_init_array(&series_all_functions,sizeof(series_fn*));

	series_queue_init();
}

/***********************************************************************************
 * Debug & console print
 */

void series_to_readable(s_expr * ex, char * out) {
	if (!ex) {
		out[0] = 0;
		return;
	}
	int state = STATE_OPERAND;
	unsigned char * c = (unsigned char *)ex->expr.arr;
	unsigned char * fin = c+(ex->expr.size);
	int k = 0;
	char tmp[32];
	int sz;
	while (c < fin) {
		
		if (state == STATE_OPERAND) {
			switch (*c) {
				case SERIES_EAX:
					memcpy(out+k,"eax",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_EBX:
					memcpy(out+k,"ebx",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_ECX:
					memcpy(out+k,"ecx",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_EDX:
					memcpy(out+k,"edx",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_ESP:
					memcpy(out+k,"esp",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_EBP:
					memcpy(out+k,"ebp",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_ESI:
					memcpy(out+k,"esi",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_EDI:
					memcpy(out+k,"edi",3);
					k += 3;
					c++;
					state = STATE_OPERATOR;
					break;
				case SERIES_LBL:
					sprintf(tmp,"@(%X)",*(int*)(c+1));
					sz = strlen(tmp);
					memcpy(out+k,tmp,sz);
					k += sz;
					c += (sizeof(int)+1);
					state = STATE_OPERATOR;
					break;
				case SERIES_NUM:
					sprintf(tmp,"%X",*(int*)(c+1));
					sz = strlen(tmp);
					memcpy(out+k,tmp,sz);
					k += sz;
					c += (sizeof(int)+1);
					state = STATE_OPERATOR;
					break;
				case SERIES_MEMSYMB:
					sprintf(tmp,"&(%X)",*(int*)(c+1));
					sz = strlen(tmp);
					memcpy(out+k,tmp,sz);
					k += sz;
					c += (sizeof(int)+1);
					state = STATE_OPERATOR;
					break;
				case SERIES_MS:
					out[k++] = '[';
					c++;
					break;
				case SERIES_ME:
					out[k++] = ']';
					c++;
					break;
				case SERIES_GS:
					out[k++] = '(';
					c++;
					break;
				case SERIES_GE:
					out[k++] = ')';
					c++;
					break;
				case SERIES_NOT:
					break;
				case SERIES_UNK:
					out[k++] = '?';
					c++;
					break;
				default:
					sprintf(out,"invalid expression. Expected OPERAND, got %X.\n",*c);
					printf("invalid expression\n");
					return;
			}
		}
		else if (state == STATE_OPERATOR) {
			switch (*c) {
				case SERIES_ADD:
					out[k++] = '+';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_SUB:
					out[k++] = '-';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_MUL:
					out[k++] = '*';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_DIV:
					out[k++] = '/';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_MOD:
					out[k++] = '%';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_AND:
					out[k++] = '&';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_OR:
					out[k++] = '|';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_XOR:
					out[k++] = '^';
					c++;
					state = STATE_OPERAND;
					break;
				case SERIES_MS:
					out[k++] = '[';
					c++;
					break;
				case SERIES_ME:
					out[k++] = ']';
					c++;
					break;
				case SERIES_GS:
					out[k++] = '(';
					c++;
					break;
				case SERIES_GE:
					out[k++] = ')';
					c++;
					break;
				default:
					sprintf(out,"invalid expression. Expected OPERATOR, got %X.\n",*c);
					printf("invalid expression\n");
					return;
			}
		}
	}
	out[k++] = 0;

}

char series_ex_dbg[12][1024];
unsigned int series_ex_dbg_idx = 0;

char * series_print_expr(s_expr * ex) {
	int i = series_ex_dbg_idx++;
	series_to_readable(ex,series_ex_dbg[i]);
	if (series_ex_dbg_idx >= 12) {
		series_ex_dbg_idx = 0;
	}
	return series_ex_dbg[i];
}

char * series_print_query(series_query * q) {
	uint32 j;
	s_expr * merge = series_merged_exprs(q);
	for(j=0;j<q->exprs.size;j++) {
		
//		series_to_readable(ex,series_ex_dbg[i]);
		//if (series_ex_dbg_idx >= 12) {
		//	series_ex_dbg_idx = 0;
		//}
	}
	destroy_expression(merge);
//	return series_ex_dbg[i];
	return NULL;
}

void series_dump_function(char * fname) {
	FILE * fp = fopen(fname,"wb");
	char dbg[1024];

	P_HASH_ITERATE(it,current_function->queries.queries,
		series_query * q = (series_query*)it->data;
		
		//fwrite(q->expr->expr.arr,1,q->expr->expr.size,fp);
		//sprintf(dbg,"\n");
		if (q->target) {
			series_to_readable(q->target,dbg);
			fwrite(dbg,1,strlen(dbg),fp);
		}
		else {
			sprintf(dbg,"mem symbol %i ",q->type);
			fwrite(dbg,1,strlen(dbg),fp);
		}
		fwrite("=",1,1,fp);
		unsigned int i;
		for(i=0;i<q->exprs.size;i++) {
			if (i) {
				fwrite("|",1,1,fp);
			}
			s_expr * ex = ea_get_element(s_expr**,q->exprs,i);
			series_to_readable(ex,dbg);
			fwrite(dbg,1,strlen(dbg),fp);			
		}
		//sprintf(dbg," %X ",q);
		//fwrite(dbg,1,strlen(dbg),fp);

		fwrite("\n",1,1,fp);
	);

	P_HASH_ITERATE(it,current_function->queries.mem_data,
		series_query * q = (series_query*)it->data;
		series_to_readable(q->target,dbg);
		fwrite(dbg,1,strlen(dbg),fp);
		if (q->type & SERIES_QUERY_MW) {
			fwrite(" W",1,2,fp);
		}

		if (q->type & SERIES_QUERY_MR) {
			fwrite(" R",1,2,fp);
		}
		fwrite("\n",1,1,fp);
		);
	fclose(fp);
	
}

int series_flux_cmp(dword addr1, dword addr2, series_queue_entry * block1, series_queue_entry * block2) {
	if (block1 == block2) {
		return addr1-addr2;
	}

	int rt = flux_cmp_blocks(block1->st_addr,block2->st_addr);
	//printf("rel %X %X %i\n",addr1,addr2,rt);
	if (rt == 0) {
		return -1;
	}

	if (rt == 1) {
		return 1;
	}

	return 0;

	
}

/***********************************************************************************
 * Unit tests
 */

void series_text_exprs() {
	mem_full_ref mr1;
	char dbg[256];
	memset(&mr1,0,sizeof(mr1));
	mr1.base.type = ID_TYPE_IREG;
	mr1.base.arg = mod_reg_EDI;
	mr1.rdisp = mod_reg_EDX;
	mr1.mult = 4;

	s_expr * ex1;
	series_parse_expression(&mr1,&ex1);

	series_to_readable(ex1,dbg);

	printf("%s\n",dbg);
}