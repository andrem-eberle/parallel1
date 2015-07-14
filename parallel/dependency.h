#ifndef _DEPENDENCY_H_
#define _DEPENDENCY_H_

#include "series.h"

void build_fn_dependency(series_fn * fn);
void build_dependecy_tree();
void dep_set_access(int type, int reg, dword addr, series_queue_entry * block,series_fn * fn);
void dep_init();

typedef struct _dep_access {
	int reg;
	int type;
} dep_access;

typedef struct _dep_access_q {
	series_query * q;
	int type;
} dep_access_q;

typedef struct _dep_access_info {
	dword addr;
	series_queue_entry * block;
	ex_array access;
	ex_array mem_access;
	ex_array deps;
	//series_fn * fn;
	ex_array fns;
} dep_access_info;

#endif