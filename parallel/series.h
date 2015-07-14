#ifndef _SERIES_H_
#define _SERIES_H_

#include "exarray.h"
#include "flux.h"
#include "memory.h"

#include "parallel.h"
#include "phash.h"

/**********************************************************************
 * Conditional jumps
 */

#define SERIES_JZ	1

/**********************************************************************
 * Queue
 */

#define SERIES_BLK_FLAG_LOOP		0x01
#define SERIES_BLK_FLAG_VISITED		0x02
#define SERIES_BLK_FLAG_LOOP_EXIT	0x04

struct _series_fn_wrapper;

typedef struct _series_queue_entry {
	dword addr;
	dword flags;
	dword visit_count;

	dword st_addr;
	
	ex_array alternate_routes;
	struct _series_queue_entry * next;
} series_queue_entry;

typedef struct _series_queue_fn_entry {
	struct _series_fn_wrapper * fn;
	struct _series_queue_fn_entry * next;
} series_queue_fn_entry;

void series_queue_blocks(dword * rep, dword sz);
series_queue_entry * series_pool();

/**********************************************************************
 * Encoding
 */

/* TODO move this into an arch specific series encoding header */
#define SERIES_EAX	0x01
#define SERIES_EBX	0x02
#define SERIES_ECX	0x03
#define SERIES_EDX	0x04
#define SERIES_ESP	0x05
#define SERIES_EBP	0x06
#define SERIES_ESI	0x07
#define SERIES_EDI	0x08

#define SERIES_NUM	0x09

#define SERIES_MS	0x0A
#define SERIES_ME	0x0B

#define SERIES_GS	0x0C
#define SERIES_GE	0x0D

/* -not- is an operator, not an operand, but since it's unary, I will
	place it here, to simplify the parsing automaton */
#define SERIES_NOT	0x0E

/* stochastic elements, we will not predict them, at least not
	precisely, in static analysis */
#define SERIES_UNK	0x0F

/* Non-terminal elements, refers some rule in the query grammar */
#define SERIES_NT	0x10

/* a symbol, followed by (one) byte, indicating an index in a symbol table.
 these expressions are being evaluated in other queries, and will not be 
 present here */
#define SERIES_SYM	0x10

#define SERIES_LBL	0x11

#define SERIES_MEMSYMB	0x12

#define SERIES_ADD	0x01
#define SERIES_SUB	0x02
#define SERIES_MUL	0x03
#define SERIES_DIV	0x04
#define SERIES_MOD	0x05

#define SERIES_AND	0x06
#define SERIES_OR	0x07
#define SERIES_XOR	0x08

#define OPER_CMP	0x60


typedef struct _series_expression {
	ex_array expr;
	unsigned int hash;
} s_expr;

inline s_expr * create_expression() {
	s_expr * ex = (s_expr*)malloc(sizeof(s_expr));
	ea_init_array(&ex->expr,sizeof(unsigned char), 32);
	ex->hash = 0;
	return ex;
}

inline void destroy_expression(s_expr * ex) {
	ea_destroy_array(&ex->expr);
	free(ex);
}

inline s_expr * subset_expression(char * sp, char * dp) {
	s_expr * sex = create_expression();
	/* we exclude both the MS and ME ([ and ]) symbols here */
	ea_guarantee_size(&sex->expr,(dp-sp));
	memcpy(sex->expr.arr,sp,(dp-sp));
	*(((char*)sex->expr.arr)+(dp-sp)) = 0;
	sex->expr.size = dp-sp;
	return sex;
}

inline void expr_add_number(s_expr * ex, int number) {
	ea_add_element(&ex->expr,SERIES_NUM);
	ea_guarantee_size(&ex->expr,sizeof(int));
	int * bref = (int*)ea_get_ptr(&ex->expr);
	*bref = number;
	ex->expr.size += sizeof(int);
	ex->hash = 0;
	
}

inline void expr_add_label(s_expr * ex, int label) {
	ea_add_element(&ex->expr,SERIES_LBL);
	ea_guarantee_size(&ex->expr,sizeof(int));
	int * bref = (int*)ea_get_ptr(&ex->expr);
	*bref = label;
	ex->expr.size += sizeof(int);
	ex->hash = 0;
	
}

inline void expr_add_msymb(s_expr * ex, int val) {
	ea_add_element(&ex->expr,SERIES_MEMSYMB);
	ea_guarantee_size(&ex->expr,sizeof(int));
	int * bref = (int*)ea_get_ptr(&ex->expr);
	*bref = val;
	ex->expr.size += sizeof(int);
	ex->hash = 0;
}

inline void expr_start_memory(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_MS);
	ex->hash = 0;
}

inline void expr_end_memory(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_ME);
	ex->hash = 0;
}

inline void expr_start_group(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_GS);
	ex->hash = 0;
}

inline void expr_end_group(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_GE);
	ex->hash = 0;
}

inline void expr_eax(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_EAX);
	ex->hash = 0;
}

inline void expr_ebx(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_EBX);
	ex->hash = 0;
}

inline void expr_ecx(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_ECX);
	ex->hash = 0;
}

inline void expr_edx(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_EDX);
	ex->hash = 0;
}

inline void expr_esp(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_ESP);
	ex->hash = 0;
}

inline void expr_ebp(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_EBP);
	ex->hash = 0;
}

inline void expr_esi(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_ESI);
	ex->hash = 0;
}

inline void expr_edi(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_EDI);
	ex->hash = 0;
}

inline void expr_add(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_ADD);
	ex->hash = 0;
}

inline void expr_sub(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_SUB);
	ex->hash = 0;
}

inline void expr_div(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_DIV);
	ex->hash = 0;
}

inline void expr_mul(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_MUL);
	ex->hash = 0;
}

inline void expr_mod(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_MOD);
}

inline void expr_and(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_AND);
	ex->hash = 0;
}

inline void expr_or(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_OR);
	ex->hash = 0;
}

inline void expr_xor(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_XOR);
	ex->hash = 0;
}

inline void expr_not(s_expr * ex) {
	ea_add_element(&ex->expr,SERIES_NOT);
	ex->hash = 0;
}

inline void expr_op(s_expr * ex, int operation) {
	ea_add_element(&ex->expr,operation);
	ex->hash = 0;
}

inline unsigned int expr_hash(s_expr * ex) {
	if (ex->hash) {
		return ex->hash;
	}
	ex->hash = 1;
	
	unsigned char * c = (unsigned char *)ex->expr.arr;
	unsigned char * cend = c+ex->expr.size;

	while (c < cend) {
		ex->hash = *c ? ex->hash* (*c) : ex->hash+1;
		c++;
	}

	return ex->hash;
}


/**********************************************************************
 * Engine
 */

#define SERIES_QUERY_NN		0x00
#define SERIES_QUERY_MW		0x01
#define SERIES_QUERY_MR		0x02

#define SERIES_QUERY_END	0x04
#define SERIES_QUERY_LI		0x08

#define SERIES_QUERY_REG	0x10
#define SERIES_QUERY_MEM	0x20

#define SERIES_QUERY_MMDEF  0x40

#define SERIES_IS_MEM_ACCESS(e) ((((unsigned char *)e->expr.arr)[0]) == SERIES_MS)

typedef struct _series_query {
	//s_expr * expr;
	ex_array exprs;
	s_expr * target;
	dword type;
	//ex_array hooks;
	dword src;
	ex_array cloned_queries;
	phash * generation_rules;
	struct _series_query *root_query;
	dword loopref_addr;

	dword msymbol;
	struct _series_query *t_link;
} series_query;

typedef struct _series_hook {
	struct _series_query * query;
	//dword hook;
} series_hook;

typedef struct _series_hook_container {
	dword hook;
	ex_array hooks;
} series_hooks;

#define RET_FLAG_NEW			0x01

typedef struct _series_ret_data {
	dword flags;
	s_expr * out;
} series_ret_data;

typedef struct _series_fn_info {
	/* type here is series_arg_data */
	ex_array args_data;
	/* series_ret_data */
	ex_array globals;
	series_ret_data ret;

	/* stack arguments size, (i.e. size of passed arguments, should be zero 
		for non standard calls, it should only contain the delta of the stack pointer */
	uint32 stack_size;

	/* extra relevant information, like known size/format of memory data, etc */
	void * extra;
} series_fn_info;

typedef struct _series_query_container {
	phash * queries;
	phash * mem_data;
} series_query_container;

/* loop 'sub' function */
#define FUNCTION_TYPE_LOOP		0x1

typedef struct _series_fn_wrapper {
	function_table_entry * function;
	series_fn_info * fn_info;

	//ex_array queries;
	//ex_array cur_queries;
	series_query_container queries;
	series_query_container cur_queries;

	phash * hooks;

	phash * critical_points;

	series_queue_entry * current_block;
	series_queue_entry * head_block;
	int type;

	series_queue_entry * squeue_h;
	series_queue_entry * squeue_t;
	int series_use_partial_queue;
	series_queue_entry * squeue_ref;
} series_fn;

typedef struct _series_critical_point {
	series_query * read_q;
	series_query * write_q;
	dword address;
	series_queue_entry * block;
	dword type;
} series_critical_point;

void series_finish_block(unsigned int * branches, unsigned int size);
void series_start_block(series_queue_entry * block_s);
void series_current_function(function_table_entry * fn, phash * hooks);
int series_ex_match(s_expr * ex, s_expr * ex2);

/**********************************************************************
 * Control
 */

void series_clear_fn();

void series_write(mem_full_ref * dest, mem_full_ref * src, dword srcaddr);

void series_operation(int soperation, mem_full_ref * sop1, mem_full_ref * sop2,
					  int doperation, mem_full_ref * dop1, mem_full_ref * dop2,
					  dword srcaddr);
void series_operation_dest(int operation, mem_full_ref * op1, mem_full_ref * op2, mem_full_ref * src, dword srcaddr);
void series_operation_src(int operation, mem_full_ref * op1, mem_full_ref * op2, mem_full_ref * dest, dword srcaddr);

void series_dump_function(char* fname);

void series_process_call(series_fn_info * fn, dword srcaddr);

void series_init();
int series_cmp(s_expr * val, s_expr * val2);

/**********************************************************************
 * Debug
 */

void series_to_readable(s_expr * ex, char * out);
char * series_print_expr(s_expr * ex);

/**********************************************************************
 * Wrapper convenience methods
 */

/* addr is a memory absolute address */
void series_addr_ax_write_abs(dword addr, dword inst_addr);
void series_addr_reg_write_imm(dword imm, dword reg, dword inst_addr);

void series_push(dword inst_addr);

void series_math_operation(int operation, mem_full_ref * src, mem_full_ref * dest, int carry);
void series_assign(mem_full_ref * src, mem_full_ref * dest);

#define series_add(s,d) series_math_operation(SERIES_ADD,s,d,0)
#define series_sub(s,d) series_math_operation(SERIES_SUB,s,d,0)
#define series_mult(s,d) series_math_operation(SERIES_MUL,s,d,0)
#define series_div(s,d) series_math_operation(SERIES_DIV,s,d,0)
#define series_mod(s,d) series_math_operation(SERIES_MOD,s,d,0)

#define series_xor(s,d) series_math_operation(SERIES_XOR,s,d,0)
#define series_or(s,d) series_math_operation(SERIES_OR,s,d,0)
#define series_and(s,d) series_math_operation(SERIES_AND,s,d,0)

void series_ref_reg(mem_full_ref * rf, int reg);
void series_ref_AX(mem_full_ref * rf);
void series_ref_imm(mem_full_ref * rf, uint32 val);
void series_push_reg(uint32 reg);
void series_pop_reg(uint32 reg);
void series_push_imm(uint32 val);
void series_text_exprs();

int series_flux_cmp(dword addr1, dword addr2, series_queue_entry * block1, series_queue_entry * block2);
void series_parse_expression(mem_full_ref * ref, s_expr ** ex);

#endif