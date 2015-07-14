#include "parallel.h"
#include "context.h"
#include "flux.h"

#include <stdlib.h>
#include <string.h>

phash * base_elements;
phash * bl_entries_hash;

iblock * root_context = 0;
iblock * current_context = 0;

/* local queue, for internal branches (conditional jumps) */
context_queue * l_queue = 0;
/* general queue, for functions (calls) */
context_queue * v_queue = 0;

local_branch * a_branch = 0;
local_branch * a_branch_c = 0;
dword c_limit = 0;

visit_branch * visit_stack_h = 0;
visit_branch * visit_stack_t = 0;



void init_context(unsigned int texsize) {
	flux_init(texsize);
	base_elements = ph_create(4,256,0);
	bl_entries_hash = ph_create(4,256,0);
}

void add_base(mem_id addr, dword disp) {

	bases * b;
	b = (bases*)ph_get(mem_hash_id(addr),(void*)(addr.arg),base_elements);
	if (!b) {
		b = (bases*)malloc(sizeof(bases));
		b->addr = addr;
		b->p_off_sz = 0;
		b->p_off_msz = 0;
		b->known_size = 0;
		b->c_off_sz = 0;
		b->c_off_msz = 0;
		ph_put(mem_hash_id(addr),(void*)(addr.arg),b,base_elements);
	}
	if (b->known_size < (disp+sizeof(dword))) {
		b->known_size = (disp+sizeof(dword));
	}
	if (disp) {
		if (b->p_off_sz == b->p_off_msz) {
			if (!b->p_off_msz) {
				b->p_off_msz = 16;
			}
			else {
				b->p_off_msz *= 2;
			}
			dword * tmp = (dword*)malloc(b->p_off_msz+sizeof(dword));
			memcpy(tmp,b->p_offsets,b->p_off_sz*sizeof(dword));
			if (b->p_offsets) free(b->p_offsets);
			b->p_offsets = tmp;
		}
		b->p_offsets[b->p_off_sz++] = disp;
	}
}

dword addr_hash(dword addr) {
	return addr*addr;
}



iblock * add_block_entry(dword ep) {
	if (current_context) {
		/* TODO clean up */
	}
	iblock * ib;
	if ((ib = (iblock*)ph_get(addr_hash(ep),(void*)ep,bl_entries_hash))) {
		return ib;
	}

	ib = (iblock*)calloc(1,sizeof(iblock));
	ib->clobbered_memory = ph_create(2,32,0);
	ib->status = 0;
	ib->ep = ep;
	ib->parent = (current_context);
	ph_put(addr_hash(ep),(void*)ep,ib,bl_entries_hash);
	//current_context = ib;
	
	if (!root_context) {
		root_context = ib;
	}

	return ib;

}

void visit_backtrack(dword src, iblock * new_ib) {
	visit_branch * t = visit_stack_t;
	visit_branch * p = 0;
	//printf("backtrack %X %X\n",new_ib->ep,new_ib->parent ? new_ib->parent->ep : 0);
	while (t && t->cntxt->ep != new_ib->ep) {
		//printf("at %X %X\n",t->cntxt->ep,new_ib->ep);
		t = t->prev;
	}
	if (t) {
		/* loop detected */
		printf("loop at %X\n",t->cntxt->ep);
		flux_loop(t->cntxt->ep,src);
	}

	t = visit_stack_t;
	while (t && t->cntxt != new_ib->parent) {
		t = t->prev;
	}

	if (t) {
		p = t->next;
		t->next = (visit_branch*)malloc(sizeof(visit_branch));
		t->next->cntxt = new_ib;
		t->next->next = 0;
		t->next->prev = t;
		visit_stack_t = t->next;
	}
	else {
		p = visit_stack_h;
		visit_stack_h = (visit_branch*)malloc(sizeof(visit_branch));
		visit_stack_t = visit_stack_h;
		visit_stack_t->cntxt = new_ib;
		visit_stack_t->next = visit_stack_t->prev = 0;
	}

	while (p) {
		t = p->next;
		free(p);
		p = t;
	}
}

void enter_block(iblock * ib) {
	local_branch * t = a_branch, * tt;

	while (t) {
		tt = t->next;
		free(t);
		t = tt;
	}
	a_branch = (local_branch*)malloc(sizeof(local_branch));
	a_branch->start = ib->ep;
	a_branch->next = 0;
	a_branch_c = a_branch;
	c_limit = UINT_MAX;
	current_context = ib;
	//visit_backtrack(0,ib);

}


int do_jmp(dword ip, dword target, dword size, int from_inst) {
	
	if (is_phase_flux()) {
		a_branch_c->end = ip;
		dword mbound = UINT_MAX;
		local_branch * t = a_branch;

		iblock * ib = add_block_entry(target);
		current_context = ib;
		//visit_backtrack(ip,ib);

		while (t) {	
			if (t->start == target) {
				log_debug("already visited %X",target);
				return 0;
			}
			if (t->start < target && t->end >= target) {				
				/* we are jumping to the middle of a branch, mark it as
				branch and leave */

				if (from_inst) {
					f_branch(target,ip);
				}
				return 0;
			}

			//printf("%X %X %X\n",target , t->start , mbound);
			if (t->start >= target && t->start < mbound) {
				mbound = t->start;
			}
			t = t->next;
		}		

		a_branch_c->next = (local_branch*)malloc(sizeof(local_branch));
		a_branch_c = a_branch_c->next;
		a_branch_c->start = target;
		a_branch_c->next = 0;
		//printf("cl %X\n",c_limit);
		c_limit = mbound;
		//printf("cl %X\n",c_limit);

		if (size) {
			flux_jmp(ip, target, size);
		}
	}

	return 1;
}

int do_jmp( dword ep, dword target, dword size ) {
	return do_jmp(ep,target,size,1);
}

void do_branch(dword ip, dword target, dword size) {
	if (is_phase_flux()) {
		log_data("branch to: (%X), from (%X)",target,ip);
		
		context_queue * cq = l_queue;
		int toqueue = 1;
		while (cq) {
			if (cq->cntxt->ep == target) {
				log_debug("Already queueed");
				toqueue = 0;
				break;
			}
			cq = cq->next;
		}
		iblock * ib;

		if (toqueue) {
			ib = add_block_entry(target);
			
			cq = l_queue;
			l_queue = (context_queue*)malloc(sizeof(context_queue));
			l_queue->next = cq;
			l_queue->cntxt = ib;	
			
		}

		if (size) {
			flux_branch(ip,ip+size,target, size);
		}
		ib = add_block_entry(ip+size);
		current_context = ib;
		//visit_backtrack(ip,ib);
	}	
}

void do_branch_f(dword ip, dword target, dword size) {
	
	if (is_phase_flux()) {
		iblock * ib = add_block_entry(target);
		context_queue * cq = v_queue;
		v_queue = (context_queue*)malloc(sizeof(context_queue));
		v_queue->next = cq;
		v_queue->cntxt = ib;

		//if (size) {
			flux_call(ip,target,size);
		//}
		//else {
			//flux_function_entry(ip,target);
		//}
	}
}

void do_ret(dword src, dword size) {
	flux_ret(src,size);
}

void context_check_branch(dword ip) {
	/* are we the block start? */
	if (ip != current_context->ep) {
		//log_debug("block branch visit repeated ");
		context_queue * cq = l_queue;
		context_queue * cp = NULL;
		while (cq) {
			if (cq->cntxt->ep == ip) {
				log_debug("Branch being visited in natural flow, removing. %X",ip);
				/* we don't need to queue this, we are already visiting */
				if (cp) {
					cp->next = cq->next;
				}
				else {
					l_queue = cq->next;
				}
				free(cq);
				dword chain = flux_get_chained(ip);
				log_debug("chained %X",chain);
				flux_new_target(ip,chain);
				break;
			}
			cp = cq;
			cq = cq->next;
		}
	}
}

iblock * pool_queue(dword ep) {
	
	iblock * ib;
	context_queue * cq = 0;
	
	cq = l_queue;

	while (l_queue) {
		cq = l_queue;
		log_debug("view %X",cq->cntxt->ep);
		l_queue = l_queue->next;
		ib = cq->cntxt;		
		if (do_jmp(ep,ib->ep,0,0)) {
			log_debug("Entering new branch at %X, %X",cq->cntxt->ep,ep);
			current_context = ib;	
			free(cq);
			return ib;
		}
		log_debug("%X",l_queue);
		free(cq);
		
	}
	
	
	if (v_queue) {		
		cq = v_queue;
		v_queue = v_queue->next;
		ib = cq->cntxt;
		log_debug("Entering new function at: %X",ib->ep);
		enter_block(ib);
		flux_set_function(ib->ep);
		free(cq);
		return ib;
	}
	return 0;

}

void context_write(mem_id addr, dword line) {

	switch (addr.type) {
		case ID_TYPE_REG:
			current_context->clobbered_registers.regs[addr.arg] = 1;
			break;
		case ID_TYPE_LABEL:
		case ID_TYPE_ABSOLUTE:
			if (ph_get(mem_hash_id(addr),(void *)(&addr),current_context->clobbered_memory)) {
				return;
			}
			ph_put(mem_hash_id(addr),(void *)&addr,(void *)&addr,current_context->clobbered_memory);
			//current_context->clobbered_memory[current_context->c_mem_sz++] = addr;
			break;
	}		
}

void context_read(mem_id addr, dword context_val, dword line ) {
	/*int dep = 1;
	switch (addr.type) {
		case ID_TYPE_REG:
			if (!current_context->clobbered_registers.regs[addr.arg]) {
				dep = 0;
			}
			break;
		case ID_TYPE_LABEL:
		case ID_TYPE_ABSOLUTE:
			if (current_context->clobbered_memory[context_val].type) {
				dep = 0;
			}
			break;
	}
	if (!dep) {
		return;
	}

	unbound_dependency * udep = (unbound_dependency*) malloc(sizeof(unbound_dependency));
	unbound_dependency * t = current_context->entrance_dependencies;
	udep->next = t;
	current_context->entrance_dependencies = udep;

	udep->addr = addr;
	udep->line = line;*/
}

void context_print_blocks() {
	local_branch * t = a_branch;

	while (t) {
		printf("Block from %X to %X\n",t->start,t->end);
		t = t->next;
	}
}

void context_store_file(char * path) {
	FILE * fp = fopen(path,"wb");
	
	P_HASH_ITERATE(it,bl_entries_hash,
		iblock * ib = (iblock*)it->data;
		printf("ep %X\n",ib->ep);
	
	)

}