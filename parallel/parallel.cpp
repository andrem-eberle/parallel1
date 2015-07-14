#include "parallel.h"
#include "phash.h"
#include "analysis_engine.h"
#include "phase1_engine.h"
#include "series.h"
#include "inter1.h"
#include "dependency.h"

#include "instset.h"
#include "pe.h"
#include "context.h"
#include "flux.h"

#include "syscall_table.h"

#include <stdlib.h>
#include <stdio.h>

phash * dependencies;

exec_engine * l_engine;

#define STATE_FLOW	0
#define STATE_BACK	1

#define BL_ENTRIES_DEFAULT	256

processor_state * p_state;
register_ref * r_write;
program_stack * p_stack;
byte * refaddr;

/* analysis state (or phase) */
int state;

/* regular instructions table */
inst_table * in_table;
inst_table * flux_table;

void analyze(unsigned char * sect, unsigned int ep, unsigned int size, int mode);
void init_fntable();

sect_info * tex;

//#include <windows.h>
//#include <direct.h>
//#include <shlobj.h>

int main() {
//series_text_exprs();return 0;
/*	TCHAR appData[MAX_PATH];
    SHGetFolderPath(NULL,
                  CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
                  NULL,
                  SHGFP_TYPE_CURRENT,
                  appData);*/

	
	l_engine = &pe_engine;
	
	dependencies = ph_create(4,128,0);
	
	int rt = l_engine->process_file(FNAME);
	if (rt < 0) {
		return rt;
	}
	
	tex = l_engine->get_text_section();
	FILE * fp = fopen("code.tex","wb");
	if (!fp) {
		printf("File not found\n");
		return -1;
	}

	fwrite(tex->data,1,tex->size,fp);
	fclose(fp);
	
	init_fntable();
	init_context(tex->size);
	init_memory();
	init_syscall_table();
	dep_init();
	
	/* debug info */
	dbg_arrbase = tex->data;

	printf("Entry point at %X\n",tex->ep);
tex->ep = 0;
//	analyze(tex->data,tex->ep,tex->size,1); // ptest
	analyze(tex->data,0xc7,tex->size,0); // ptest
	/*FILE * fp2 = fopen("fileoffs.dat","rb");
	char * fps = (char*)malloc(4096);
	int r = fread(fps,1,4096,fp2);
	unsigned int * off = (unsigned int*)fps;
	
	while (off < (unsigned int *)(fps+r)) {
		analyze(tex->data,*off,tex->size);
	}
	fclose(fp2);*/


}

void init_fntable() {

	int i;
	in_table = (inst_table*)calloc(1,sizeof(inst_table));

	in_table->handler[INST_XOR_0] = INST_XOR_0_H;
	in_table->handler[INST_XOR_1] = INST_XOR_1_H;
	in_table->handler[INST_XOR_2] = INST_XOR_2_H;
	in_table->handler[INST_XOR_3] = INST_XOR_3_H;
	in_table->handler[INST_XOR_4] = INST_XOR_4_H;
	in_table->handler[INST_XOR_5] = INST_XOR_5_H;

	in_table->handler[INST_PUSH] = INST_PUSH_H;
	in_table->handler[INST_PUSH_IMM2] = INST_PUSH_IMM2_H;
	
	in_table->handler[INST_POP] = INST_POP_H;
	in_table->handler[INST_INC] = INST_INC_H;
	in_table->handler[INST_DEC] = INST_DEC_H;
	for(i=1;i<8;i++) {
		in_table->handler[INST_POP+i] = INST_POP_H;
		in_table->handler[INST_PUSH+i] = INST_PUSH_H;
		in_table->handler[INST_INC+i] = INST_INC_H;
		in_table->handler[INST_DEC+i] = INST_DEC_H;
	}
	in_table->handler[INST_CALL] = INST_CALL_H;
	in_table->handler[INST_MOV2] = INST_MOV2_H;
	in_table->handler[INST_JMP2]		= INST_JMP2_H;	
	in_table->handler[INST_MOV4] = INST_MOV4_H;
	in_table->handler[INST_MOV8] = INST_MOV8_H;
	in_table->handler[INST_MOV12] = INST_MOV12_H;
	in_table->handler[INST_GEN_32BITS2] = INST_GEN_32BITS2_H;
	for(i=0;i<=0xF;i++) {
		in_table->handler[INST_GEN_MOV+i]		= INST_GEN_MOV_H;
	}

	in_table->handler[INST_CMP_3] = INST_CMP_3_H;
	in_table->handler[INST_PUSH32] = INST_PUSH32_H;

	in_table->handler[INST_JMP] = INST_JMP_H;	

	in_table->handler[INST_JO] = INST_JO_H;

	/* for now link every conditional jmp on the same function, I think
		it won't be necessary to split this up */
	in_table->handler[INST_JC]		= INST_JO_H;
	in_table->handler[INST_JNC]		= INST_JO_H;
	in_table->handler[INST_JNO]		= INST_JO_H;
	in_table->handler[INST_JZ]		= INST_JO_H;
	in_table->handler[INST_JNZ]		= INST_JO_H;
	in_table->handler[INST_JBE]		= INST_JO_H;
	in_table->handler[INST_JNBE]	= INST_JO_H;
	in_table->handler[INST_JS]		= INST_JO_H;
	in_table->handler[INST_JNS]		= INST_JO_H;
	in_table->handler[INST_JP]		= INST_JO_H;
	in_table->handler[INST_JNP]		= INST_JO_H;
	in_table->handler[INST_JL]		= INST_JO_H;
	in_table->handler[INST_JNL]		= INST_JO_H;
	in_table->handler[INST_JLE]		= INST_JO_H;
	in_table->handler[INST_JNLE]	= INST_JO_H;
	in_table->handler[INST_DB_OP]   = INST_DB_OP_H;
	/*
	in_table->handler[INST_JC] = INST_JC_H;
	in_table->handler[INST_JNC] = INST_JNC_H;
	in_table->handler[INST_JNO] = INST_JNO_H;
	in_table->handler[INST_JZ] = INST_JZ_H;
	in_table->handler[INST_JNZ] = INST_JNZ_H;
	in_table->handler[INST_JBE] = INST_JBE_H;
	in_table->handler[INST_JNBE] = INST_JNBE_H;
	in_table->handler[INST_JS] = INST_JS_H;
	in_table->handler[INST_JNS] = INST_JNS_H;
	in_table->handler[INST_JP] = INST_JP_H;
	in_table->handler[INST_JNP] = INST_JNP_H;
	in_table->handler[INST_JL] = INST_JL_H;
	in_table->handler[INST_JNL]		= INST_JNL_H;
	in_table->handler[INST_JLE] = INST_JLE_H;
	in_table->handler[INST_JNLE] = INST_JNLE_H;
	*/

	in_table->handler[INST_GEN_1OP] = INST_GEN_1OP_H;
	in_table->handler[INST_RETN] = INST_RETN_H;

	in_table->handler[INST_RETN2] = INST_RETN2_H;

	flux_table = (inst_table*)calloc(1,sizeof(inst_table));

	flux_table->handler[INST_XOR_0] = F_INST_XOR_0_H;
	flux_table->handler[INST_XOR_1] = F_INST_XOR_1_H;
	flux_table->handler[INST_XOR_2] = F_INST_XOR_2_H;
	flux_table->handler[INST_XOR_3] = F_INST_XOR_3_H;
	flux_table->handler[INST_XOR_4] = F_INST_XOR_4_H;
	flux_table->handler[INST_XOR_5] = F_INST_XOR_5_H;

	flux_table->handler[INST_PUSH] = F_INST_PUSH_H;
	flux_table->handler[INST_PUSH_IMM2] = F_INST_PUSH_IMM2_H;
	
	flux_table->handler[INST_POP] = F_INST_POP_H;
	flux_table->handler[INST_INC] = F_INST_INC_H;
	flux_table->handler[INST_DEC] = F_INST_DEC_H;
	for(i=1;i<8;i++) {
		flux_table->handler[INST_POP+i] = F_INST_POP_H;
		flux_table->handler[INST_PUSH+i] = F_INST_PUSH_H;
		flux_table->handler[INST_INC+i] = F_INST_INC_H;
		flux_table->handler[INST_DEC+i] = F_INST_DEC_H;
	}
	flux_table->handler[INST_ADD_3]		= F_INST_ADD_3_H;
	flux_table->handler[INST_ADD_5]		= F_INST_ADD_5_H;
	flux_table->handler[INST_CALL]		= F_INST_CALL_H;
	flux_table->handler[INST_MOV2]		= F_INST_MOV2_H;
	flux_table->handler[INST_MOV4]		= F_INST_MOV4_H;
	flux_table->handler[INST_MOV8]		= F_INST_MOV8_H;
	flux_table->handler[INST_MOV10]		= F_INST_MOV10_H;
	flux_table->handler[INST_MOV12]		= F_INST_MOV12_H;
	flux_table->handler[INST_GEN_32BITS2] = F_INST_GEN_32BITS2_H;
	flux_table->handler[INST_LEA] = F_INST_LEA_H;
	flux_table->handler[INST_SHL32] = F_INST_SHL32_H;
	flux_table->handler[INST_OR_3] = F_INST_OR_3_H;
	flux_table->handler[INST_TEST_5] = F_INST_TEST_5_H;
	flux_table->handler[INST_INC_2] = F_INST_INC_2_H;
	flux_table->handler[INST_TEST_REP] = F_INST_REP_H;

	

	flux_table->handler[INST_GEN5_INST] = F_INST_GEN5_INST_H;
	
	flux_table->handler[INST_LEAVE]		= F_INST_LEAVE_H;
	flux_table->handler[INST_TEST] = F_INST_TEST_H;
	flux_table->handler[INST_TEST2] = F_INST_TEST2_H;
	/* all generic mov fall here */
	for(i=0;i<=0xF;i++) {
		flux_table->handler[INST_GEN_MOV+i]		= F_INST_GEN_MOV_H;
	}


	flux_table->handler[INST_CMP_3]		= F_INST_CMP_3_H;

	flux_table->handler[INST_JMP]		= F_INST_JMP_H;	
	flux_table->handler[INST_JMP2]		= F_INST_JMP2_H;	

	flux_table->handler[INST_JO]		= F_INST_JO_H;

	/* for now link every conditional jmp on the same function, I think
		it won't be necessary to split this up */
	flux_table->handler[INST_JC]		= F_INST_JO_H;
	flux_table->handler[INST_JNC]		= F_INST_JO_H;
	flux_table->handler[INST_JNO]		= F_INST_JO_H;
	flux_table->handler[INST_JZ]		= F_INST_JO_H;
	flux_table->handler[INST_JNZ]		= F_INST_JO_H;
	flux_table->handler[INST_JBE]		= F_INST_JO_H;
	flux_table->handler[INST_JNBE]		= F_INST_JO_H;
	flux_table->handler[INST_JS]		= F_INST_JO_H;
	flux_table->handler[INST_JNS]		= F_INST_JO_H;
	flux_table->handler[INST_JP]		= F_INST_JO_H;
	flux_table->handler[INST_JNP]		= F_INST_JO_H;
	flux_table->handler[INST_JL]		= F_INST_JO_H;
	flux_table->handler[INST_JNL]		= F_INST_JO_H;
	flux_table->handler[INST_JLE]		= F_INST_JO_H;
	flux_table->handler[INST_JNLE]		= F_INST_JO_H;
	
	flux_table->handler[INST_GEN_1OP]	= F_INST_GEN_1OP_H;
	flux_table->handler[INST_RETN]		= F_INST_RETN_H;
	flux_table->handler[INST_RETN2]		= F_INST_RETN_H;

	/***************************************************/

	//flux_table->handler[INST_GEN_CMP]		= F_INST_GEN_CMP2_H;
// 	flux_table->handler[INST_FDIVR]		= F_INST_FDIVR2_H;
// 	flux_table->handler[INST_FNSTCW]		= F_INST_FNSTCW2_H;
// 	flux_table->handler[INST_POP_ES]		= F_INST_POP_ES2_H;
// 	flux_table->handler[INST_FPU2_SUB7]		= F_INST_FPU2_SUB72_H;
	flux_table->handler[INST_OR_0]		= F_INST_OR_02_H;
	flux_table->handler[INST_OR_1]		= F_INST_OR_12_H;
	flux_table->handler[INST_OR_2]		= F_INST_OR_22_H;
	flux_table->handler[INST_OR_4]		= F_INST_OR_42_H;
	flux_table->handler[INST_OR_5]		= F_INST_OR_52_H;
	flux_table->handler[INST_PUSH_CS]		= F_INST_PUSH_CS2_H;
	flux_table->handler[INST_DB_OP]		= F_INST_DB_OP2_H;
	flux_table->handler[INST_ADC_0]		= F_INST_ADC_02_H;
	flux_table->handler[INST_ADC_1]		= F_INST_ADC_12_H;
	flux_table->handler[INST_ADC_2]		= F_INST_ADC_22_H;
	flux_table->handler[INST_ADC_3]		= F_INST_ADC_32_H;
	flux_table->handler[INST_ADC_4]		= F_INST_ADC_42_H;
	flux_table->handler[INST_ADC_5]		= F_INST_ADC_52_H;
	flux_table->handler[INST_PUSH_SS]		= F_INST_PUSH_SS2_H;
	flux_table->handler[INST_POP_SS]		= F_INST_POP_SS2_H;
	flux_table->handler[INST_SBB_0]		= F_INST_SBB_02_H;
	flux_table->handler[INST_SBB_1]		= F_INST_SBB_12_H;
	flux_table->handler[INST_SBB_2]		= F_INST_SBB_22_H;
	flux_table->handler[INST_SBB_3]		= F_INST_SBB_32_H;
	flux_table->handler[INST_SBB_4]		= F_INST_SBB_42_H;
	flux_table->handler[INST_SBB_5]		= F_INST_SBB_52_H;
	flux_table->handler[INST_PUSH_DS]		= F_INST_PUSH_DS2_H;
	flux_table->handler[INST_POP_DS]		= F_INST_POP_DS2_H;
	flux_table->handler[INST_AND_0]		= F_INST_AND_02_H;
	flux_table->handler[INST_AND_1]		= F_INST_AND_12_H;
	flux_table->handler[INST_AND_2]		= F_INST_AND_22_H;
	flux_table->handler[INST_AND_3]		= F_INST_AND_32_H;
	flux_table->handler[INST_AND_4]		= F_INST_AND_42_H;
	flux_table->handler[INST_AND_5]		= F_INST_AND_52_H;
	flux_table->handler[INST_ES]		= F_INST_ES2_H;
	flux_table->handler[INST_DAA]		= F_INST_DAA2_H;
	flux_table->handler[INST_SUB_0]		= F_INST_SUB_02_H;
	flux_table->handler[INST_SUB_1]		= F_INST_SUB_12_H;
	flux_table->handler[INST_SUB_2]		= F_INST_SUB_22_H;
	flux_table->handler[INST_SUB_3]		= F_INST_SUB_32_H;
	flux_table->handler[INST_SUB_4]		= F_INST_SUB_42_H;
	flux_table->handler[INST_SUB_5]		= F_INST_SUB_52_H;
	flux_table->handler[INST_CS]		= F_INST_CS2_H;
	flux_table->handler[INST_DAS]		= F_INST_DAS2_H;
	flux_table->handler[INST_SS]		= F_INST_SS2_H;
	flux_table->handler[INST_AAA]		= F_INST_AAA2_H;
	flux_table->handler[INST_CMP_0]		= F_INST_CMP_02_H;
	flux_table->handler[INST_CMP_1]		= F_INST_CMP_12_H;
	flux_table->handler[INST_CMP_4]		= F_INST_CMP_42_H;
	flux_table->handler[INST_CMP_5]		= F_INST_CMP_52_H;
	flux_table->handler[INST_DS]		= F_INST_DS2_H;
	flux_table->handler[INST_AAS]		= F_INST_AAS2_H;
	flux_table->handler[INST_PUSHAD]		= F_INST_PUSHAD2_H;
	flux_table->handler[INST_POPAD]		= F_INST_POPAD2_H;
	flux_table->handler[INST_BOUND]		= F_INST_BOUND2_H;
	flux_table->handler[INST_ARPL]		= F_INST_ARPL2_H;
	flux_table->handler[INST_FS]		= F_INST_FS2_H;
	flux_table->handler[INST_GS]		= F_INST_GS2_H;
	flux_table->handler[INST_PREOVER]		= F_INST_PREOVER2_H;
	flux_table->handler[INST_ADDROVER]		= F_INST_ADDROVER2_H;
	flux_table->handler[INST_PUSH32]		= F_INST_PUSH322_H;
	flux_table->handler[INST_IMUL]		= F_INST_IMUL2_H;
	flux_table->handler[INST_IMUL2]		= F_INST_IMUL22_H;
	flux_table->handler[INST_INSB]		= F_INST_INSB2_H;
	flux_table->handler[INST_INSD]		= F_INST_INSD2_H;
	flux_table->handler[INST_INSW]		= F_INST_INSW2_H;
	flux_table->handler[INST_OUTB]		= F_INST_OUTB2_H;
	flux_table->handler[INST_OUTW]		= F_INST_OUTW2_H;
	flux_table->handler[INST_OUTD]		= F_INST_OUTD2_H;
	flux_table->handler[INST_GEN_8BITS]		= F_INST_GEN_32BITS2_H;
	flux_table->handler[INST_GEN_32BITS]		= F_INST_GEN_32BITS2_H;
	flux_table->handler[INST_GEN_8BITS2]		= F_INST_GEN_32BITS2_H;
	flux_table->handler[INST_XCHG]		= F_INST_XCHG2_H;
	flux_table->handler[INST_XCHG2]		= F_INST_XCHG22_H;
	flux_table->handler[INST_MOV]		= F_INST_MOV2_H;
	flux_table->handler[INST_MOV3]		= F_INST_MOV32_H;
	flux_table->handler[INST_MOV5]		= F_INST_MOV52_H;
	flux_table->handler[INST_MOV6]		= F_INST_MOV62_H;
	flux_table->handler[INST_POP32]		= F_INST_POP322_H;
	flux_table->handler[INST_PAUSE]		= F_INST_PAUSE2_H;
	flux_table->handler[INST_CWDE]		= F_INST_CWDE2_H;
	flux_table->handler[INST_CDQ]		= F_INST_CDQ2_H;
	flux_table->handler[INST_CALLF]		= F_INST_CALLF2_H;
	flux_table->handler[INST_WAIT]		= F_INST_WAIT2_H;
	flux_table->handler[INST_PUSHFD]		= F_INST_PUSHFD2_H;
	flux_table->handler[INST_POPFD]		= F_INST_POPFD2_H;
	flux_table->handler[INST_SAHF]		= F_INST_SAHF2_H;
	flux_table->handler[INST_LAHF]		= F_INST_LAHF2_H;
	flux_table->handler[INST_MOV7]		= F_INST_MOV72_H;
	flux_table->handler[INST_MOV9]		= F_INST_MOV92_H;
	flux_table->handler[INST_MOVSB]		= F_INST_MOVSB2_H;
	flux_table->handler[INST_MOVSD]		= F_INST_MOVSD2_H;
	flux_table->handler[INST_CMPSB]		= F_INST_CMPSB2_H;
	flux_table->handler[INST_CMPSD]		= F_INST_CMPSD2_H;
	flux_table->handler[INST_TEST3]		= F_INST_TEST32_H;
	flux_table->handler[INST_TEST4]		= F_INST_TEST42_H;
	flux_table->handler[INST_STOSB]		= F_INST_STOSB2_H;
	flux_table->handler[INST_STOSD]		= F_INST_STOSD2_H;
	flux_table->handler[INST_LODSB]		= F_INST_LODSB2_H;
	flux_table->handler[INST_LODSD]		= F_INST_LODSD2_H;
	flux_table->handler[INST_SCASB]		= F_INST_SCASB2_H;
	flux_table->handler[INST_SCASD]		= F_INST_SCASD2_H;

	flux_table->handler[INST_LES]		= F_INST_LES2_H;
	flux_table->handler[INST_LDS]		= F_INST_LDS2_H;
	flux_table->handler[INST_MOV11]		= F_INST_MOV112_H;
	flux_table->handler[INST_ENTER]		= F_INST_ENTER2_H;
	flux_table->handler[INST_FXCH_OPT]		= F_INST_FXCH_OPT2_H;
	flux_table->handler[INST_LEAVE]		= F_INST_LEAVE2_H;
	flux_table->handler[INST_RETF]		= F_INST_RETF2_H;
	flux_table->handler[INST_RETF2]		= F_INST_RETF22_H;
	flux_table->handler[INST_INT]		= F_INST_INT2_H;
	flux_table->handler[INST_INT2]		= F_INST_INT22_H;
	flux_table->handler[INST_INT0]		= F_INST_INT02_H;
	flux_table->handler[INST_IRETD]		= F_INST_IRETD2_H;
	flux_table->handler[INST_SHL2]		= F_INST_SHL22_H;
	flux_table->handler[INST_FNOP_SEC]		= F_INST_FNOP_SEC2_H;
	flux_table->handler[INST_SHL322]		= F_INST_SHL3222_H;
	flux_table->handler[INST_SHL3]		= F_INST_SHL32_H;
	flux_table->handler[INST_SHL323]		= F_INST_SHL3232_H;
	flux_table->handler[INST_AMX]		= F_INST_AMX2_H;
	flux_table->handler[INST_ADX]		= F_INST_ADX2_H;
	flux_table->handler[INST_SETALC]		= F_INST_SETALC2_H;
	flux_table->handler[INST_XLATB]		= F_INST_XLATB2_H;
	flux_table->handler[INST_FPU1]		= F_INST_FPU12_H;
	flux_table->handler[INST_FPU2]		= F_INST_FPU22_H;

}

int is_code(dword addr, it_inner_entry ** it) {
	*it = l_engine->get_call_target(addr);

	if (tex->va <= addr && addr < tex->va + tex->vsize) {
		return 1;
	}

	return 0;
}

program_stack * init_stack() {
	program_stack * s = (program_stack*)malloc(sizeof(program_stack));
	s->m_size = STACK_STEP_SIZE;
	s->storage = (dword*)malloc(STACK_STEP_SIZE);
	s->size = 0;
	s->sp = -1;
	return s;
}

dword little_endian_convert(dword * data) {
	dword d = (*data & 0xFF) << 24;
	d |= (*data & 0xFF00) << 8;
	d |= (*data & 0xFF0000) >> 8;
	d |= (*data & 0xFF000000) >> 24;
	return d;
}

extern dword dbg_addr;
extern dword c_limit;

void dump_table();

byte * analysis_base;
inst_table * cur_handlers;

void analyze(byte * sect, dword ep, dword size, int mode) {
//ep = 0x7C8;
	state = STATE_FLOW;
	byte * ip = sect+ep;
	refaddr = sect;
	analysis_base = sect;
	
	if (mode == 0) {
		inter_load_file("inter1.b");
		ep = inter_get_ep()->address;
		printf("New entry point at: %X\n",ep);
	}

	//iblock * ib = add_block_entry((dword)ip);
	do_branch_f(0,(dword)ep,0);

	iblock * ib = pool_queue(0);
	p_state = (processor_state*)calloc(1,sizeof(processor_state));
	r_write = (register_ref*)calloc(1,sizeof(register_ref));
	inst_table * handlers = flux_table;
	inst_table * ihandlers = in_table;
	cur_handlers = handlers;
	IT_instruction * itp;
	
	p_stack = init_stack();

	int ccount = 0;

	if (mode == 0) {
		itp = inter_get_ep();
		itp = inter_get_instruction(0);
	}
	else {
		inter_write_start("inter1.b",ep);
	}

	printf("Starting phase 1: flow analysis.\n");
	log_data("Starting phase 1: flow analysis.");
	/* first phase
		here we build a flow graph and create a table containing the size of the instructions
		*/
	while(1) {		
		if (mode) {
			byte * prev;
			byte inst = *ip;
			dbg_addr = ip-sect;
			log_data("ADDR: %X, INST: %X, LIMIT: %X",ip-sect,inst,c_limit);
			prev = ip;

			ip = handlers->handler[inst](ip);

			if (!ihandlers->handler[inst]) {
				printf("Lacking handler on %X\n",inst);
				//exit(0);
			}
			else {
				ihandlers->handler[inst](prev);
			}

			if (ip == prev) {
				/* something wrong */
				printf("Error - instruction not advancing: %X at %X\n",inst,ip-analysis_base);
				return;
			}

			if (!ip) {			
				ib = pool_queue((prev-sect));

				if (!ib) {
					break;
				}

				ip = (byte*)sect+ib->ep;
			}
			else if ((unsigned)(ip-sect) >= c_limit) {
				/* entering the limit here, flag it as direct branch */
				f_branch(ip-sect,prev-sect);
				do_jmp(prev-sect,ip-sect,0);			
				ib = pool_queue((dword)(ip-sect));

				if (!ib) {
					break;
				}
				ip = sect+(ib->ep);
				printf("ip = %X %X\n",ip, ib->ep);

			}
		}
		else {
			IT_instruction * previ = itp;
			dbg_addr = itp->address;
			log_data("ADDR: %X, INST: %X, LIMIT: %X",itp->address,0,c_limit);
			itp = inter_flux_process(itp);
			
			if (!itp) {			
				ib = pool_queue((previ->address));

				if (!ib) {
					break;
				}

				itp = inter_get_instruction(ib->ep);
			}
			else if (itp->address >= c_limit) {
				/* entering the limit here, flag it as direct branch */
				f_branch(itp->address,previ->address);
				do_jmp(previ->address,ip-sect,0);			
				ib = pool_queue((dword)(itp->address));
				log_debug("pooled at %X",ib ? ib->ep : 0);
				if (!ib) {
					break;
				}
				itp = inter_get_instruction(ib->ep);


			}
		}

		//inter_write_empty(prev-sect);
		
	}

	/* debug data */
	dump_table();

	printf("End of phase 1.\n");
	log_data("End of phase 1.");
	log_data("");
	/* end of first phase */
	if (mode) {
		inter_adjust_targets("inter1.b");

		inter_file_to_ascii("inter1.b","inter1.txt");

		return;
	}

    flux_graph_dump();
	flux_look_for_cycles(NULL,0);

	/* second phase, reverse analysis and function merging */
	log_data("Start of phase 2: building expressions.");

	inter_load_from_file("inter1.b");
	
	series_init();
	state = STATE_BACK;
	handlers = in_table;

	function_table_entry * rootf = flux_get_function(ep);
	if (!rootf) {
		printf("ERROR - entry function not found?\n");
		return;
	}

	log_data("root function at %X, number of rets is %i %X",rootf->src,rootf->rets.size,rootf);
	series_current_function(rootf,NULL);

	series_queue_blocks(((dword*)rootf->rets.arr),rootf->rets.size);

	series_queue_entry * tblock = series_pool();
	//ip = (sect+tblock->addr);

	itp = inter_get_instruction(tblock->addr);

	series_start_block(tblock);

	int rt;
	dword paddr;

	while(1) {		
		rt = flux_get_prev((itp)->address,&paddr);

		if (rt) {
			if (rt == RET_BRANCH) {
				
				/* this block is finished, query the flux control for the (upward) branches 
					available */
				dword * branches;
				dword bsz;
				flux_get_branches((itp)->address,&branches,&bsz);
				/* this block is finished, tell series queue to specify the branches on them */
				series_finish_block(branches,bsz);
				/* we now queue all the blocks, the queue will sort them and discard the repeated
					blocks. If the block was already processed, we will wait untill we try to
					process it again to detect it. When this particular situation occurs, we found
					a loop. */
				series_queue_blocks(branches,bsz);

				/* now pool the series queues for the next block */
				tblock = series_pool();
				if (!tblock) {
					/* finished */
					break;
				}
				//ip = (sect+tblock->addr);
				itp = inter_get_instruction(tblock->addr);
				printf("next block at %X\n",tblock->addr);

				series_start_block(tblock);
			}
			else if (rt == RET_FUNCTION) {
				break;	
			}
		}
		else {
			itp = inter_get_instruction(paddr);
			//ip = sect+paddr;
		}
		//byte inst = *ip;

		dbg_addr = itp->address;
		log_data("ADDR: %X, INST: %X",itp->address,itp->inst);
		
		inter_process(itp);
		//handlers->handler[inst](ip);
	}

	series_dump_function("series.dump");

	log_data("End of phase 2.");

	log_data("Start of phase 3: building dependency tree.");
	build_dependecy_tree();
	log_data("End of phase 3.");
}

int is_phase_flux() {
	return state == STATE_FLOW;
}

void add_dependency(dword src, dword target, int type) {
	dependency * d = (dependency*)ph_get(src,(void*)src,dependencies);
	if (!d) {
		d = (dependency *)malloc(sizeof(dependency));
	}
	line_entry * le = d->lines;
	d->lines = (line_entry*)malloc(sizeof(line_entry));
	d->lines->next = le;
	d->lines->line = target;
	d->type = type;

}

void write_memory(dword mem, dword val) {
	
}

dword read_memory(dword mem) {
	return 0;
}