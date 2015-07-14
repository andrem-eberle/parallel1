#ifndef _INTER1_H_
#define _INTER1_H_

#include "parallel.h"

#include "memory.h"

//arithmetic
#define IOP_AND 1
#define IOP_OR 2
#define IOP_NOT 3
#define IOP_XOR 4
#define IOP_ADD 5
#define IOP_SUB 6
#define IOP_MUL 7
#define IOP_DIV 8
#define IOP_IDIV 9
#define IOP_ADC 17

#define IOP_SBB 18
#define IOP_ROR 19

//memory
#define IOP_MOV 20

//branch
#define IOP_JMP 21
#define IOP_JC 22
#define IOP_CALL 23
#define IOP_RET 24

// misc
#define IOP_ACCESS 25

//parameter
#define I_IMMEDIATE 1
#define I_STATIC 2
#define I_REGISTER 3
#define I_REGREF 4

// meta
#define IOP_EP	100

typedef struct _IT_param {
	int type;
	unsigned int base_value; // V or R
	int base_signal;
	int r1; // these and below only for I_REGREF
	int disp;
	int mod;
	int disp2;
	int rdisp;
} IT_param;


typedef struct _IT_instruction {
	dword laddr;
	dword address;
	int inst;
	IT_param p1;
	IT_param p2;
	IT_param p3;
} IT_instruction;

typedef struct _IT_inst_block {
	dword laddr;
	IT_instruction it_block[16];
	int size;
} IT_inst_block;

void inter_write_start(char * file, dword ep);
void inter_write_op(int operation, IT_param * param1, IT_param * param2, IT_param * param3, dword address);
void inter_write_ret();
void inter_write_call(dword target);
void inter_write_jmp(dword target);
void inter_write_jc(int type, dword target);
void inter_write_math_op(int sop, mem_full_ref * p1, mem_full_ref * p2, mem_full_ref * p3);
void inter_file_to_ascii(char * in, char * out);
int inter_series_op_trans(int op);
IT_instruction * inter_get_ep();

void inter_write_push_op(mem_full_ref * p1);
void inter_write_pop_op(mem_full_ref * p1);
void inter_write_empty(dword address);
void inter_write_mem_write(mem_full_ref * where, mem_full_ref * what);
void inter_access_memory(mem_full_ref * mem);
void inter_call(dword target);
void inter_math_process(dword op,IT_param * p1, IT_param * p2, IT_param * p3);
void inter_mov_process(dword op,IT_param * p1, IT_param * p2, IT_param * p3);

void inter_load_file(char * file);
void inter_load_from_file(char * file);
void inter_adjust_targets(char * infile);
IT_instruction * inter_flux_process(IT_instruction * it);

IT_instruction * inter_get_instruction_e(dword addr);
IT_instruction * inter_get_instruction(dword addr);
void inter_process(IT_instruction * it);

#endif