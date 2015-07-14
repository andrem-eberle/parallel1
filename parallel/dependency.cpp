#include "parallel.h"
#include "series.h"
#include "dependency.h"
#include "phash.h"

extern ex_array series_all_functions;
phash * addr_deps;


int da_has_fn(series_fn * fn, dep_access_info * da) {
	int i;

	for(i=0;i<da->fns.size;i++) {
		series_fn * fn2 = (series_fn*)ea_get_element(series_fn**,da->fns,i);
		//log_debug("fns %X %X",fn,fn2);
		if (fn2 == fn) {
			return 1;
		}
	}
	return 0;
}

dep_access_info * dep_get_access(dword addr, series_fn * fn) {
	dep_access_info * dp = (dep_access_info*)ph_get_k(addr,(void*)addr,addr_deps);
	if (!dp) {
		dp = (dep_access_info *)calloc(1,sizeof(dep_access_info));
		ph_put_k(addr,(void*)addr,dp,addr_deps);
		ea_init_array(&dp->access,sizeof(dep_access),4);
		ea_init_array(&dp->mem_access,sizeof(dep_access_q),4);
		ea_init_array(&dp->fns,sizeof(series_fn*),4);
		dp->deps.arr = NULL;
		ea_add_element_p(&dp->fns,fn);
		dp->addr = addr;
		
	}
	else {
		int i;
		int ex = 0;
		for(i=0;i<dp->fns.size;i++) {
			series_fn * fn2 = (series_fn*)ea_get_element(series_fn**,dp->fns,i);
// 			log_debug("%X",*(dword*)&dp->fns.arr);
// 			log_debug("fns %X %X",fn,fn2);
			if (fn2 == fn) {
				ex = 1;
				break;
			}
		}
		if (!ex) {
			ea_add_element_p(&dp->fns,fn);
		}
	}

	return dp;
}

void dep_set_access(int type, int reg, dword addr, series_queue_entry * block, series_fn * fn) {
	//printf("add access %i %i %X\n",type,reg,addr);
	log_debug("set access %X %X %X -- %X %X",addr,block->st_addr,fn,reg,type);
	dep_access_info * dp = dep_get_access(addr,fn);
	dp->block = block;
	dep_access dpa;
	dpa.reg = reg;
	dpa.type = type;
	ea_add_element_pc(&dp->access,&dpa);

}

void match_fn_dependency(series_fn * fn, series_query * q, series_critical_point * excl) {

	P_HASH_ITERATE(it,fn->critical_points,
		series_critical_point * cp = (series_critical_point*)it->data;
		if (cp == excl) {
			P_HASH_CONTINUE(it,fn->critical_points);
		}
		if (cp->write_q && !series_cmp(q->target,cp->write_q->target)) {

		}
		);
}

void add_dep_excl(dep_access_info * da, dword addr) {
	int i;
	for(i=0;i<da->deps.size;i++) {
		dword a1 = ea_get_element(dword*,da->deps,i);
		if (da->addr == 0x36) log_debug("excl %X %X %i",a1,addr,da->deps.size);
		if (a1 == addr) return;
	}
	ea_add_element(&da->deps,addr);
}

void match_all_deps(dep_access_info * da, series_fn * fn) {

	ea_init_array(&da->deps,sizeof(dword));
	int i,j;
	for(i=0;i<da->access.size;i++) {
		dep_access * dpa = &ea_get_element(dep_access*,da->access,i);
		
		P_HASH_ITERATE(it2,addr_deps,		
			dep_access_info * da2 = (dep_access_info*)it2->data;
			if (!da_has_fn(fn,da2)) {
				P_HASH_CONTINUE(it2,addr_deps);
			}
			if (da2 == da) {
				P_HASH_CONTINUE(it2,addr_deps);
			}
			int rel = series_flux_cmp(da->addr,da2->addr,da->block,da2->block);
			if (rel == 0) {
				/* no relation, cant have a dependency */
			}
			if (rel > 0) {
				for(j=0;j<da2->access.size;j++) {
					dep_access * dpa2 = &ea_get_element(dep_access*,da2->access,j);
					//if (da->addr == 7) {printf("dpa %i %i\n",dpa->type,dpa2->type);}
					if (!((dpa->type == 2) && (dpa2->type == 2))) {
						if (dpa->reg == dpa2->reg) {
							//if (da->addr == 0x36) log_debug("got a dep 36 at %X",da2->addr);
							add_dep_excl(da,da2->addr);
						}
					}
				}
				
			}
			/* we only consider forward relations for simplicity, so ignore rel < 0 */
			
		);
	}
	

	for(i=0;i<da->mem_access.size;i++) {
		dep_access_q * q = &ea_get_element(dep_access_q*,da->mem_access,i);
printf("dep q %X\n",da->addr);
		P_HASH_ITERATE(it2,addr_deps,		
			dep_access_info * da2 = (dep_access_info*)it2->data;
			if (!da_has_fn(fn,da2)) {
				P_HASH_CONTINUE(it2,addr_deps);
			}
			if (da2 == da) {
				P_HASH_CONTINUE(it2,addr_deps);
			}
			int rel = series_flux_cmp(da->addr,da2->addr,da->block,da2->block);
			
			if (rel > 0) {
				for(j=0;j<da2->mem_access.size;j++) {
printf("da q %X\n",da2->addr);
					//log_debug("da q get %X",da2);
					dep_access_q * q2 = &ea_get_element(dep_access_q*,da2->mem_access,i);
					if (!series_cmp(q->q->target,q2->q->target)) {
						//ea_add_element(&da->deps,da2->addr);
						add_dep_excl(da,da2->addr);
					}
				}
			}

			);

	}
}
byte tmp[1024][1024];
void detransition(char ** g, int n) {
	int i,j,k;
	
	for(i=0;i<n;i++) {
		for(j=1;j<n;j++) {
			if (g[i][j] > 1) {
				tmp[i][j] = 1;
			}
			else if (g[i][j] < -1) {
				tmp[i][j] = -1;
			}
			else {
				tmp[i][j] = g[i][j];
			}
		}
	}
	int done = 0;
	while (!done) {
		done = 1;
		for(i=0;i<n;i++) {
			for(j=0;j<n;j++) {
				if (i==j) {
					continue;
				}
				if (tmp[i][j] == 1) {
					for(k=1;k<n;k++) {
						if (k == i || k == j) {
							continue;
						}
						if (tmp[j][k] == 1) {
							//done = 0;
							g[i][k] = 2;
							g[k][i] = -2;
						}
					}
				}

			}
		}
// 		if (!done) {
// 			for(i=0;i<n;i++) {
// 				for(j=0;j<n;j++) {
// 					tmp[i][j] = g[i][j];
// 				}
// 			}
// 		}
	}
}

static int kgp = 10;

void dep_dump_graphviz(char ** g,int n) {
	char dd[256];
	sprintf(dd,"C://Users//z//Desktop//gpv//gp3-%i.txt",kgp++);
	FILE * fp = fopen(dd,"wb");
	char tmp[1024];
	int i,j;

	fwrite("digraph G {\n",strlen("digraph G {\n"),1,fp);
	fwrite("size=\"800,600\";\n",strlen("size=\"800,600\";\n"),1,fp);
	fwrite("splines=true;\n",strlen("splines=true;\n"),1,fp);
	fwrite("K=3;\n",strlen("K=3;\n"),1,fp);
	fwrite("overlap=false;\n",strlen("overlap=false;\n"),1,fp);		

	int lid = 0;

	for(i=0;i<n;i++) {
		

		int h = 0;
		for(j=0;j<n;j++) {
			if (i==j) continue;
			if(g[i][j] == 1) {
				h = 1;
				break;
			}
		}

		if (h) {
			sprintf(tmp,"b_%i [label=\"%X\"];\n",i,i);
			fwrite(tmp,strlen(tmp),1,fp);	

			for(j=0;j<n;j++) {
				if (i==j) continue;
				if(g[i][j] == 1) {
					sprintf(tmp,"b_%i -> b_%i [weight=0.5 color=\"blue\" tooltip=\"2020(default)\"];\n",i,j);
					fwrite(tmp,strlen(tmp),1,fp);
				}
			}
		}

		//if (i > 8)break;
	}

	fwrite("}\n",strlen("}\n"),1,fp);
	fclose(fp);
}

void dump_dep_graph(char ** g,int n) {
	int i,j;
	for(i=0;i<n;i++) {
		for(j=0;j<n;j++) {
			if (i==j) continue;
			if(g[i][j] == 1) {
				printf("Rel between %X %X\n",i,j);
			}
		}

		//if (i > 8)break;
	}
}

void graph_dependencies(int n,series_fn * fn) {
	char ** graph_adj;
	int i;
	log_debug("Building general dependency graph");
	graph_adj = (char**)calloc(1,n*sizeof(char*));
	for(i=0;i<n;i++) {
		graph_adj[i] = (char*)calloc(1,n);
	}

	P_HASH_ITERATE(it,addr_deps,
		dep_access_info * da = (dep_access_info*)it->data;

		if (!da_has_fn(fn,da)) {
			P_HASH_CONTINUE(it,addr_deps);
		}
		for(i=0;i<da->deps.size;i++) {
			dword dpa = ea_get_element(dword*,da->deps,i);
			int mult = 1;
			if (da->addr == 0x34) log_debug("da 34 dep %X %i %X",dpa,da->deps.size,da);
			graph_adj[da->addr][dpa] = 1*mult;
			graph_adj[dpa][da->addr] = -1*mult;
		}
	);

	

	log_debug("Detransitioning dependency graph");

	detransition(graph_adj,n);

	dump_dep_graph(graph_adj,n);
	dep_dump_graphviz(graph_adj,n);

}

int fninstd = 1;

void dump_local_fn_insts(series_fn * fn) {
	char dd1[64];
	sprintf(dd1,"c:\\l-%i.txt",fninstd++);
	FILE * f = fopen(dd1,"w");
	P_HASH_ITERATE(it,addr_deps,
		dep_access_info * da = (dep_access_info*)it->data;

	if (!da_has_fn(fn,da)) {
		P_HASH_CONTINUE(it,addr_deps);
	}

	sprintf(dd1,"inst %X\n",da->addr);
	fwrite(dd1,strlen(dd1),1,f);

	);
	fclose(f);
}

void build_fn_dependency(series_fn * fn) {

	dump_local_fn_insts(fn);
	int nsize = 0;
	log_debug("Clustering all dependencies for function at %X",fn);

	P_HASH_ITERATE(it,addr_deps,
		dep_access_info * da = (dep_access_info*)it->data;
		ea_clear(&da->mem_access);
	);

	/* we now add memory dependencies */
	
	P_HASH_ITERATE(it,fn->critical_points,
		series_critical_point * cp = (series_critical_point*)it->data;
		dep_access_info * da = dep_get_access(cp->address,fn);
		
		printf("critical point at %X %s %X %X\n",cp->address,(cp->type & SERIES_QUERY_MW) ? "W" : "R", cp->write_q,cp->read_q);
		char dbg[256];
		int mode = cp->type;
		series_query * t = NULL;
		if (cp->type & SERIES_QUERY_MW) {
			series_to_readable(cp->write_q->target,dbg);
			printf("%s\n",dbg);
			t = cp->write_q;
		}

		if (cp->type & SERIES_QUERY_MR) {
			series_to_readable(cp->read_q->target,dbg);
			printf("%s\n",dbg);
			t = cp->read_q;
		}
		dep_access_q q;
		q.type = mode;
		q.q = t;
		
		ea_add_element_pc(&da->mem_access,&q);
		//log_debug("add element at %X %i",da,da->mem_access.size);

	);


	P_HASH_ITERATE(it,addr_deps,
		dep_access_info * da = (dep_access_info*)it->data;
	
		if (!da_has_fn(fn,da)) {
			P_HASH_CONTINUE(it,addr_deps);
		}
log_debug("Matching at %X",da->addr);
		if (da->addr > nsize) {
			nsize = da->addr+1;
		}
		match_all_deps(da,fn);
		);


	graph_dependencies(nsize,fn);
	
}

void dep_loop_analysis(series_fn * fn) {
	//fn->fn_info
}

void build_dependecy_tree() {

	int i;
	for(i=0;i<series_all_functions.size;i++) {
		printf("fn %i\n",i);
		series_fn * f = ea_get_element(series_fn**,series_all_functions,i);		
		build_fn_dependency(f);

		if (f->type == FUNCTION_TYPE_LOOP) {
			log_debug("Analyzing dependency for loop");

		}
		//break;
	}
}

void dep_init() {
	addr_deps = ph_create(4,32,0);
}


/*********************************************************************************
 * Dependency elimination
 */

void inter_loop_WAR_seek() {
	
}