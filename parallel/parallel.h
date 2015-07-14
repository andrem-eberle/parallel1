#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include <stdio.h>
#include "log.h"

//#define FNAME "g:\\Mount&Blade Warband Dedicated\\mb_warband_dedicated.exe"
//#define FNAME "C:\\Program Files (x86)\\DOSBox-0.74\\DOSBox.exe"

#define FNAME "c:\\users\\z\\desktop\\ptest\\Release\\ptest.exe"

//#define FNAME "a:\\desktop\\ptest\\Release\\ptest.exe"
//#define FNAME "C:\\Users\\z\\Documents\\Visual Studio 2008\\Projects\\dllf\\Debug\\dllf.dll"

#define word unsigned short
#define byte unsigned char
#define dword unsigned int

typedef struct _sect_info {
	byte * data;
	unsigned int size;
	unsigned int ep;
	unsigned int va;
	unsigned int vsize;
} sect_info;

typedef struct _import_table_internal_entry {
	char * name;
	dword idx;
	dword vaddr;
	char * libname;
} it_inner_entry;


typedef struct _exec_engine {
	int (*process_file)(char*);
	sect_info *(*get_text_section)(void);
	it_inner_entry *(*get_call_target)(dword);

} exec_engine;

typedef struct _line_entry {
	dword line;
	struct _line_entry *next;
} line_entry;


typedef struct _dependency {
	line_entry * lines;
	byte type;
	dword arg;
} dependency;

typedef struct _dep_list {
	dependency * data;
	struct _dep_list * next;
} dep_list;


#define STACK_STEP_SIZE	8192

typedef struct _program_stack {
	dword * storage;
	dword size;
	dword m_size;
	dword sp;
} program_stack;

/* a 256 position array for now, must be replaced with a perfect hash
	to encompass all functions */
typedef struct _inst_table {
	(byte *) (*handler[256]) (byte * inst);
} inst_table;

#include "instset.h"

#define POP_STACK(s,p,t)
#define PUSH_STACK(s,p,t)

#define MODREGDEC(i,a1,a2,r)	r = (*(i) & 0xC0); \
								if (r == smod_reg_RR) { a1 = *(i) & 0x07; a2 = (*(i) & 0x38) >> 3; } \
								else if (r == smod_reg_I8R) { a1 = *(i) & 0x07; a2 = (*(i) & 0x38) >> 3; } \
								else if (r == smod_reg_IR) { a1 = *(i) & 0x07; a2 = (*(i) & 0x38) >> 3; } \
								else if (r == smod_reg_I32R) {a1 = *(i) & 0x07; a2 = (*(i) & 0x38) >> 3; }

#define SINGLE_INST_REG(i,ia, r, rw) r = (i) & 0x07; rw->regs[r] = (dword)(ia);

void add_dependency(dword src, dword target, int type);
int is_code(dword addr, it_inner_entry ** it);

int is_phase_flux();



#endif