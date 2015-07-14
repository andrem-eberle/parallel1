#ifndef _FLUX_H_
#define _FLUX_H_

#include "parallel.h"
#include "exarray.h"
#include "inter1.h"

#define FLUX_IBRANCHES_SZ	2

#define FLUX_BRANCH				0x80
#define FLUX_FUNCTION			0x40

#define FLUX_LOOP				0x80000000

#define FLUX_IS_LOOP(b)			((b) & FLUX_LOOP)
#define FLUX_SET_LOOP(b)		((b) |= FLUX_LOOP)

#define FLUX_BRANCH_ADDR(b)		((b) & (~FLUX_LOOP))

#define FLUX_SIZE(s)			(s & 0x3F)
#define FLUX_SIZE_B(s,os)		((os&(FLUX_BRANCH+FLUX_FUNCTION))+FLUX_SIZE(s))

#define FLUX_IS_BRANCH(c)		((((flux_element_s*)c)->inst_size)&FLUX_BRANCH)
#define FLUX_SET_BRANCH(b)		((flux_element_b*)b)->inst_size |= FLUX_BRANCH

#define FLUX_IS_FUNCTION(c)		((((flux_element_s*)c)->inst_size)&FLUX_FUNCTION)
#define FLUX_SET_FUNCTION(b)	((flux_element_b*)b)->inst_size |= FLUX_FUNCTION

/* generic return values */
#define RET_FUNCTION	2
#define RET_BRANCH		1
#define RET_ADDR		0
#define RET_INVALID		-1

/* TODO replace by earray */
typedef struct _flux_element_b {
	unsigned char inst_size;
	dword ibranchs_sz;
	dword ibranchs_fsz;
	dword * ibranchs;
} flux_element_b;

typedef struct _flux_element_s {
	unsigned char inst_size;
} flux_element_s;

typedef struct _flux_function_rpath {
	ex_array search_order;
} flux_function_rpath;

typedef struct _function_table_entry {
	dword src;
	ex_array rets;
	flux_function_rpath * reverse_path;
} function_table_entry;

void flux_init(dword size);

void flux_set_function(dword src);
void flux_staple(dword src, dword size);
void flux_branch(dword src, dword b1, dword b2, dword size);
void flux_jmp(dword src, dword b1, dword size);
void flux_call(dword src, dword btarget, dword size);
void flux_ret(dword src, dword size);
int f_valid_reentry(dword src);
function_table_entry * flux_get_function(dword addr);

void f_branch(dword b1, dword src);
void flux_function_entry(dword src);
void dump_table();
int flux_get_prev(dword addr, dword * prev);
int flux_get_branches(dword addr, dword ** branches, dword * size);
void flux_loop(dword loop_entry, dword loop_branch);
function_table_entry * flux_function_generic(dword src);
void flux_look_for_cycles(IT_instruction * it, dword ep);
void f_branch_add_target(flux_element_b * bt, dword target);
void flux_graph_dump();

dword flux_get_chained(dword ip);
void flux_new_target(dword target, dword ip);
int flux_cmp_blocks(dword b1, dword b2);

#endif