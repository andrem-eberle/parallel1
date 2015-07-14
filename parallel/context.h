#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "parallel.h"
#include "memory.h"
#include "phash.h"

typedef struct _bases {
	mem_id addr;
	dword known_size;
	dword * c_offsets;
	dword c_off_sz;
	dword c_off_msz;
	dword * p_offsets;
	dword p_off_sz;
	dword p_off_msz;

	dword size;
} bases;

typedef struct _local_branch {
	dword start;
	dword end;
	struct _local_branch * next;
}local_branch;

#define DEP_TYPE_REG	0x01
#define DEP_TYPE_REF	0x02

#define IB_QUEUED		1
#define IB_VISITED		2

typedef struct _dep_info {
	int type;
	mem_full_ref arg;
} dep_info;

typedef struct _unbound_dependency {
	dword line;
	/* two possible types:
		either a reg, so we say the register is dependent(i.e. received something in eax we need)
		or a reg/memory value, (i.e. [eax] or [mem]). In the latter, if it's reg, we say we have 
		a dependency of both the register and its content.
	*/
	dep_info addr;
	struct _unbound_dependency * next;
} unbound_dependency;

typedef struct _iblock {
	byte status;
	dword ep;
	register_ref clobbered_registers;
	/* TODO sort to perform bsearch */
	//mem_full_ref clobbered_memory[256];
	phash * clobbered_memory;
	int c_mem_sz;
	struct _iblock * parent;

	unbound_dependency * entrance_dependencies;
	//unbound_dependency * exit_dependencies;
} iblock;

typedef struct _visit_branch {
	iblock * cntxt;
	struct _visit_branch * next;
	struct _visit_branch * prev;
} visit_branch;

typedef struct _context_queue {
	iblock * cntxt;
	struct _context_queue * next;
} context_queue;

typedef struct _component_static_data {


	unsigned int flags;

} component_static_data;

typedef struct _fn_static_analysis_data {
	unsigned int args_num;

} fn_static_data;

void context_check_branch(dword ip);
void init_context(unsigned int texsize);
iblock * add_block_entry(dword ep);
int do_jmp(dword ep, dword target, dword size, int from_inst);
int do_jmp(dword ep, dword target, dword size);
void do_branch(dword ip, dword target, dword size);
void do_branch_f(dword ip, dword target, dword size);
void do_ret(dword src, dword size);
iblock * pool_queue(dword ep);
void context_print_blocks();
void context_store_file(char * path);

#endif