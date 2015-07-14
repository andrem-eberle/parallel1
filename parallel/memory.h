#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "parallel.h"

#define ID_TYPE_LABEL		0x01
#define ID_TYPE_ABSOLUTE	0x02
#define ID_TYPE_REG			0x03
#define ID_TYPE_IMMEDIATE	0x04
#define ID_TYPE_IREG		0x05

#define MOD_FLAG_MM			0x01
#define MOD_FLAG_REG		0x02
#define MOD_FLAG_IMM		0x04
#define MOD_FLAG_IREG		0x08
#define MOD_FLAG_DEST_ONLY	0x10

typedef struct _mem_id {
	byte type;
	dword arg;
	int signal;
} mem_id;


typedef struct _register_ref {
	dword regs[8];
} register_ref;

typedef struct _mem_full_ref {
	mem_id base;
	int disp;
	int mult;
	int bdisp;
	int bpbdisp;
	int rdisp;
} mem_full_ref;

typedef struct _val_info {
	mem_id data;
	mem_full_ref src;
} val_info;


typedef struct _processor_state {
	/*dword eax;
	dword ebx;
	dword ecx;
	dword edx;
	dword esi;
	dword edi;
	dword ebp;
	dword esp;*/
	val_info regs[8];
} processor_state;


dword mem_hash_id(mem_id addr);
void finalize_write(mem_full_ref target, mem_full_ref src);
byte * modreg_analysis(byte * inst, mem_full_ref * src, mem_full_ref * dest, dword * flags);

dword memory_read(mem_id base, dword disp);
void init_memory();
dword memory_new_label();

#endif