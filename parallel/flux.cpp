#include "flux.h"
#include "phash.h"
#include "context.h"

#include <stdlib.h> 
#include <string.h> 

/* void here as we can have two type of pointed structs, union gets too messy */
/* we currently use a sparse array list for the instructions. It's a very fast 
	implementation, but wastes a lot of memory, 
	TODO change this to a compact list backed by a mapped hash */
void ** inst_flux_table;
void * current;
static dword tpos;
static dword crpos;

phash * function_table;


typedef struct _flux_graph_e {
	dword start;
	dword end;
	int status;
	int iid;
	ex_array children;
} flux_graph_e;

phash * g_data = NULL;

/* even if two functions share the same code, it's considered part of two distinct functions,
	however, we must be careful to not waste time reanalysing the same code */
function_table_entry * current_function = NULL;

void flux_init(dword size) {
	log_debug("Flux table size is %i",size);
	inst_flux_table = (void**) calloc(size,sizeof(void*));
	function_table = ph_create(4,4,0);
	tpos = 0;
	crpos = -1;
}

void flux_staple(dword src, dword size) {

	//log_data("staple at %X %X",src, *(inst_flux_table+0xA));
	/* get entry from flux table at src*/
	void ** cpos = (inst_flux_table+src);

	crpos = src;
	if (src > tpos) {
		tpos = src;
	}
	flux_element_s * st;
	flux_element_b * bt;
	
	if (!(*cpos)) {
		/* TODO this is ugly, change to a macro */
		(*(flux_element_s**)cpos) = (flux_element_s*) malloc(sizeof(flux_element_s));
		current = *cpos;
		st = (flux_element_s *)current;

		st->inst_size = FLUX_SIZE(size);
		
	}
	else {
		log_debug("branch already exist at %X",src);
		//printf("staple %X %i\n",crpos,size);
		current = *cpos;
		
		bt = (flux_element_b*)current;
		/* if we already exist, we must be a branch or function info */
		if (!FLUX_IS_BRANCH(current) && !FLUX_IS_FUNCTION(current)) {
			log_data("Error, no branch reuse?");
			return;
		}
		else if (FLUX_IS_BRANCH(current)) {
			context_check_branch(src);						
		}
		
		bt->inst_size = FLUX_SIZE_B(size,bt->inst_size);
		/*if (bt->ibranchs_sz == bt->ibranchs_fsz) {
			dword * t = bt->ibranchs;
			bt->ibranchs =  (dword*)malloc(bt->ibranchs_fsz*2
		}*/
	}

}

dword flux_get_chained(dword ip) {
	dword k = ip-1;
	while (!(inst_flux_table[k]) && k >= 0) k--;
	return k;
}

int f_valid_reentry(dword src) {
	void ** cpos = (inst_flux_table+src);
	
	void *px = *cpos;
	/*log_debug("visited? %X",px);
	if (px && !FLUX_IS_BRANCH(px) && !FLUX_IS_FUNCTION(px)) {
		return 0;
	}*/
	return px==NULL;
}

void flux_new_target(dword target, dword ip) {
	flux_element_b * bt = (flux_element_b*)inst_flux_table[target];
	f_branch_add_target(bt,ip);

}

void f_branch_add_target(flux_element_b * bt, dword target) {
	if (!bt->ibranchs) {
		bt->ibranchs_fsz = FLUX_IBRANCHES_SZ;

		bt->ibranchs = (dword *)malloc(FLUX_IBRANCHES_SZ*sizeof(dword));
		bt->ibranchs_sz = 1;
		
	}
	else {
		if (bt->ibranchs_sz == bt->ibranchs_fsz) {
			bt->ibranchs_fsz = bt->ibranchs_fsz * 2;
			dword * tmp = bt->ibranchs;
			bt->ibranchs = (dword *)malloc(bt->ibranchs_fsz*sizeof(dword));
			memcpy(bt->ibranchs,tmp,bt->ibranchs_sz*sizeof(dword));
			free(tmp);
			
		}
		bt->ibranchs_sz = bt->ibranchs_sz + 1;
	}
	bt->ibranchs[bt->ibranchs_sz-1] = target;
}

void f_branch(dword b1, dword src) {
	flux_element_s * st = (flux_element_s*)inst_flux_table[b1];
	flux_element_b * bt;
	
log_data("f_branch %X %X",st,b1);
	if (!st || !(FLUX_IS_BRANCH(st))) {
		bt = (flux_element_b*)st;
		
		bt = (flux_element_b*)calloc(1,sizeof(flux_element_b));
		bt->ibranchs = 0;
		bt->inst_size = 0;
		inst_flux_table[b1] = bt;
		
		FLUX_SET_BRANCH(bt);

		if (st) {
			bt->inst_size = st->inst_size;
			free(st);
			if (!FLUX_IS_FUNCTION(bt)) {						
				/* we were first reached directly after the previous function.
				set it as another branch */
				/* (not necessarily) */
				dword k = b1-1;
				while (!(inst_flux_table[k])) k--;
				
				f_branch_add_target(bt,k);
				FLUX_SET_BRANCH(bt);
			}
		}
		
		
	}
	else {
		bt = (flux_element_b *)st;
	}

	f_branch_add_target(bt,crpos);

}

void f_set_branch(dword where, dword src) {
	flux_element_b * bt = (flux_element_b*)inst_flux_table[where];
	FLUX_SET_BRANCH(bt);
	f_branch_add_target(bt,src);
}

void flux_loop(dword loop_entry, dword loop_branch) {
	flux_element_b * bt = (flux_element_b*)inst_flux_table[loop_entry];
	unsigned int i;
	for (i=0;i < bt->ibranchs_sz;i++) {
		if (bt->ibranchs[i] == loop_branch) {
			log_debug("Setting loop to %X, from %X",loop_entry,loop_branch);
			FLUX_SET_LOOP(bt->ibranchs[i]);
			return;
		}
	}
	log_debug("Error - loop at unexistant branch.");
	
}

void flux_branch(dword src, dword b1, dword b2, dword size) {
	flux_staple(src,size);
	f_branch(b1,src);
	f_branch(b2,src);
}

void flux_jmp(dword src, dword b1, dword size) {
	flux_staple(src,size);
	f_branch(b1,src);
}


void flux_fn_fill_rpath(function_table_entry * fn) {
	if (!g_data) {
		log_debug("Error: no graph data available to fill function");
		return;
	}
	
}

void flux_function_entry(dword src, dword target) {
	
	log_debug("flux_function_entry at %X, %X",src,target);
	flux_element_s * st = (flux_element_s*)inst_flux_table[target];

	if (!st) {
		st = (flux_element_s*)malloc(sizeof(flux_element_s));
		inst_flux_table[target] = st;
		st->inst_size = 0;
	}

	FLUX_SET_FUNCTION(st);

	function_table_entry * fe = (function_table_entry*)ph_get(target,(void*)target,function_table);

	if (!fe) {
		fe = flux_function_generic(target);
	}

}

void flux_set_function(dword src) {
	
	function_table_entry * fe = (function_table_entry*)ph_get(src,(void*)src,function_table);
	
	if (!fe) {
		fe = flux_function_generic(src);
	}
	else {
		current_function = fe;
	}
	log_debug("Setting function at %X",current_function);
}

function_table_entry * flux_function_generic(dword src)  {
	function_table_entry * fe = (function_table_entry*)malloc(sizeof(function_table_entry));
	ea_init_array(&fe->rets,sizeof(unsigned int));
	//current_function = fe;
	fe->src = src;
	ph_put(src,(void*)src,fe,function_table);

	return fe;
}

function_table_entry * flux_get_function(dword addr) {
	function_table_entry * fe = (function_table_entry*)ph_get(addr,(void*)addr,function_table);

	return fe;
}

void flux_call(dword src, dword btarget, dword size) {
	flux_staple(src,size);
	flux_function_entry(src,btarget);	
}

void flux_ret(dword src, dword size) {
	flux_staple(src,size);
	log_debug("Add ret to function with source at %X, %X, %X",current_function->src,current_function,src);
	ea_add_element(&current_function->rets,src);		
}

void dump_table() {
	FILE * fp = fopen("flux.dump","w");
	char buf[512];
	flux_element_s * st;
	flux_element_b * bt;
	dword i,j;
	for(i=0;i<tpos;i++) {
		if (inst_flux_table[i]) {
			char branches[256];
			branches[0] = 0;
			int is_branch = 0;
			st = (flux_element_s*)inst_flux_table[i];
			if (FLUX_IS_BRANCH(st)) {
				bt = (flux_element_b*)inst_flux_table[i];
				is_branch = 1;
				char tmp[64];
				//printf("%i\n",bt->ibranchs_sz);
				for (j = 0;j< bt->ibranchs_sz ; j++) {
					sprintf(tmp,"%X ",bt->ibranchs[j]);					
					strcat(branches,tmp);
				}
				
			}
			sprintf(buf,"%X: %i %s (%X, %X)\n",i,FLUX_SIZE(st->inst_size),branches,FLUX_IS_FUNCTION(st),FLUX_IS_BRANCH(st));
			//printf("%s\n",buf);
			fwrite(buf,1,strlen(buf),fp);
		}
	}
	fclose(fp);
}

dword inline flux_get_size(dword addr) {
	return ((flux_element_s*)inst_flux_table[addr])->inst_size;
}

int flux_get_prev(dword addr, dword * prev) {
	
	if (!inst_flux_table[addr]) {
		printf("Error - flux on invalid address: %X.\n",addr);
		return RET_INVALID;
	}
	flux_element_s * st;
	flux_element_b * bt;
	
	st = (flux_element_s*)inst_flux_table[addr];
	
	if (FLUX_IS_FUNCTION(st)) {
		/* end of function */
		return RET_FUNCTION;
	}
	if (FLUX_IS_BRANCH(st)) {
		/* end of function */
		return RET_BRANCH;
	}
	/* TODO change this, make a hash or something */
	void ** t = inst_flux_table+addr-1;
	
	while (!(*t)) t--;
	
	*prev = (t-inst_flux_table);
	return RET_ADDR;
	
}

int flux_get_branches(dword addr, dword ** branches, dword * size) {
	flux_element_b * bt = (flux_element_b *)inst_flux_table[addr];
	if (!FLUX_IS_BRANCH(bt)) {
		return RET_INVALID;
	}
	(*branches) = bt->ibranchs;
	*size = bt->ibranchs_sz;
	return RET_ADDR;
}

void flux_graph_dump() {
	int i;
	FILE * fp = fopen("flux.graph.dump","w");
	char data[1024];
	for(i=0;i<=tpos;i++) {
		if (inst_flux_table[i]) {
			flux_element_b * fb = (flux_element_b*)inst_flux_table[i];
			if (FLUX_IS_BRANCH(fb)) {
				sprintf(data,"ADDR: %X INST SIZE: %X\n",i,FLUX_SIZE(fb->inst_size));
				fwrite(data,1,strlen(data),fp);
				fflush(fp);			
				int j;
				for(j=0;j<fb->ibranchs_sz;j++) {
					sprintf(data,"INBOUND FROM %X (is loop? %s) \n",fb->ibranchs[j]&0x3FFFFFFF,FLUX_IS_LOOP(fb->ibranchs[j]) ? "yes" : "no");
					fwrite(data,1,strlen(data),fp);
				}
			}
		}		
	}
	fclose(fp);
}

//ea_init_array(&mem_changes,sizeof(series_query*));


int flux_dfs(flux_graph_e * node, dword ref) {
	//printf("DFS %X\n",node->start);
	int i;
	if (node->status == 1) {
		printf("loop at %X\n",node->start);
		flux_loop(node->start,ref);
		return 1;
	}
	else if (node->status == 2) {
		return 0;
	}
	node->status = 1;

	for(i=0;i<node->children.size;i++) {
		dword idx = ea_get_element(dword*,node->children,i);
		//printf("child %X\n",idx);
		flux_graph_e * fge = (flux_graph_e*)ph_get(idx,(void*)idx,g_data);

		//printf("get at %X %X\n",idx,fge);
		//printf("%X\n",node->start);
		int st = flux_dfs(fge,node->end);
		if (st) {
			ea_remove_element(&node->children,i);
			i--;
		}

	}
	node->status = 2;

	return 0;
}

void flux_dump_graphviz() {
	FILE * fp = fopen("C://Users//z//Desktop//gpv//gp2.txt","wb");
	char tmp[1024];
	int i;

	fwrite("digraph G {\n",strlen("digraph G {\n"),1,fp);
	fwrite("size=\"800,600\";\n",strlen("size=\"800,600\";\n"),1,fp);
	fwrite("splines=true;\n",strlen("splines=true;\n"),1,fp);
	fwrite("K=3;\n",strlen("K=3;\n"),1,fp);
	fwrite("overlap=false;\n",strlen("overlap=false;\n"),1,fp);		
		
	int lid = 0;
	
	P_HASH_ITERATE(it,g_data,
		flux_graph_e * fe = (flux_graph_e*)it->data;

		if (lid < fe->iid) {
			sprintf(tmp,"b_%i [label=\"%X\"];\n",fe->iid,fe->start);
			fwrite(tmp,strlen(tmp),1,fp);
			lid = fe->iid;
			for(i=0;i<fe->children.size;i++) {
				dword idx = ea_get_element(dword*,fe->children,i);
				flux_graph_e * fge = (flux_graph_e*)ph_get(idx,(void*)idx,g_data);
				sprintf(tmp,"b_%i -> b_%i [weight=0.5 color=\"blue\" tooltip=\"2020(default)\"];\n",fe->iid,fge->iid);
				fwrite(tmp,strlen(tmp),1,fp);
			}
		}
		
	);

	fwrite("}\n",strlen("}\n"),1,fp);
	fclose(fp);
}

void flux_look_for_cycles_file(char * file) {
	FILE * fp = fopen(file,"rb");
	flux_graph_e * root;
	fseek(fp,0,SEEK_END);
	int fsz = ftell(fp);
	byte * data = (byte*)malloc(fsz);
	fseek(fp,0,SEEK_SET);
	fread(data,1,fsz,fp);
	fclose(fp);


}
 
void flux_write_table(char * path) {
	FILE * fp = fopen(path,"wb");
	int i;
	for(i=0;i<=tpos;i++) {
		if (inst_flux_table[i]) {
			flux_element_b * fb = (flux_element_b*)inst_flux_table[i];
			if (FLUX_IS_BRANCH(fb)) {
				
			}
		}
	}
	fclose(fp);

}

flux_graph_e * p_root;

/* we build a graph here for analysis, 
	TODO build this during the phase 1 run */
void flux_look_for_cycles(IT_instruction * it, dword ep) {
	log_debug("Looking for cycles");
	dword i;
	ex_array fns;
	ea_init_array(&fns,sizeof(dword));
	ea_add_element(&fns,ep);
	int id = 1;
	dword * inout_table = (dword*)calloc(1,(tpos+1)*sizeof(dword));

	for(i=0;i<=tpos;i++) {
		if (inst_flux_table[i]) {			
			flux_element_b * fb = (flux_element_b*)inst_flux_table[i];
			IT_instruction * p = inter_get_instruction(i);

			if (FLUX_IS_BRANCH(fb)) {
				inout_table[i] |= 1;
				dword j;
				for(j=0;j<fb->ibranchs_sz;j++) {
					dword ibound = fb->ibranchs[j]&0x3FFFFFFF;
					inout_table[ibound] |= 2;
					//printf("branch %X %X\n",i,ibound);
				}
			}
			else if (FLUX_IS_FUNCTION(fb)) {
				ea_add_element(&fns,i);
				inout_table[i] |= 1;
			}
			else if (p->inst == IOP_RET) {				
				/* TODO change this */
				inout_table[i] |= 2;
			}
		}
	}

	g_data = ph_create(4,1024,0);

	dword ref = 0;
	for(i=0;i<=tpos;i++) {
		//printf("%i %X\n",inout_table[i] ,i);
		if (inout_table[i] & 1) {
			/* in */
			ref = i;
		}
		if (inout_table[i] & 3) {
			/* out */
			flux_graph_e * fge = new flux_graph_e;
			fge->start = ref;
			fge->end = i;
			fge->status = 0;
			fge->iid = id++;
			
			ea_init_array(&fge->children,sizeof(dword));
			ph_put(ref,(void*)ref,fge,g_data);
			ph_put(i,(void*)i,fge,g_data);
			//printf("Block from %X to %X\n",ref,i);
		}
	}	

	for(i=0;i<=tpos;i++) {
		if (inst_flux_table[i]) {
			flux_element_b * fb = (flux_element_b*)inst_flux_table[i];
			if (FLUX_IS_BRANCH(fb)) {
				dword j;
				flux_graph_e * fge = (flux_graph_e*)ph_get(i,(void*)i,g_data);

				for(j=0;j<fb->ibranchs_sz;j++) {
					
					//printf("%X %X\n",i,fb->ibranchs[j]&0x3FFFFFFF);
					dword ibound = fb->ibranchs[j]&0x3FFFFFFF;
					//log_debug("%X child from %X",i,ibound);
					flux_graph_e * fge2 = (flux_graph_e*)ph_get(ibound,(void*)ibound,g_data);
					ea_add_element(&fge2->children,i);
				}
			}
		}		
	}

	for(i=0;i<fns.size;i++) {
		dword iip = ea_get_element(dword*,fns,i);
		flux_graph_e * root = (flux_graph_e*)ph_get(iip,(void*)iip,g_data);

		//printf("%X %X\n",iip,root);
		if (root) flux_dfs(root,0);
		p_root = root;
	}

	//printf("flux loop fin\n");

	//flux_dump_graphviz();
}



typedef struct _bg_list {
	struct _bg_list * next;
	flux_graph_e * g;
} bg_list;

int flux_block_reachable(dword b1, dword b2) {
	bg_list * queue = 0;
	bg_list * queue_t = 0;
	queue_t = queue = (bg_list*)malloc(sizeof(bg_list));
	queue->next = 0;
	queue->g = (flux_graph_e*)ph_get(b1,(void*)b1,g_data);;;
	bg_list * r;
	while(queue) {
		r = queue;
		
		queue = queue->next;
		if (!queue) {
			queue_t = 0;
		}
		if (r->g->start == b2) {
			return 1;
		}
		int i;
		
		for(i=0;i<r->g->children.size;i++) {
			dword idx = ea_get_element(dword*,r->g->children,i);
			flux_graph_e * fge = (flux_graph_e*)ph_get(idx,(void*)idx,g_data);
			bg_list * t = (bg_list*)malloc((sizeof(bg_list)));
			t->g = fge;
			t->next = 0;
			if (queue_t) {
				queue_t->next = t;				
			}
			else {
				queue = t;
			}
			queue_t = t;
		}
	}
	return 0;
}

/* compare two blocks in flux order, 0 means b1 comes first, 1 b2 comes first, 2 means no relation */
int flux_cmp_blocks(dword b1, dword b2) {
	//printf("reach %X %X\n",b1,b2);
	if (flux_block_reachable(b1,b2)) {
		return 0;
	}
	if (flux_block_reachable(b2,b1)) {
		return 1;
	}

	return 2;
}