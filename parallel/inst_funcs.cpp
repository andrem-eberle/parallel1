#include "analysis_engine.h"
#include "context.h"
#include "series.h"
#include "syscall_table.h"

#include "inter1.h"

extern byte * refaddr;

extern byte * analysis_base;

extern unsigned int dbg_addr;
#define RADDR(r) (r-refaddr)
#define RADDRC(r,s) (r-refaddr+s)

byte * get_ip_pointer() {
	return refaddr+dbg_addr;	
}

uint32 get_current_address() {
	return RADDR(get_ip_pointer());
}

byte * general_conditional_jump8(byte * inst, uint32 flags) {
	return inst+2;
}

byte * general_conditional_jump32(byte * inst, uint32 flags) {
	return inst+5;
}

byte * general_reg_math_abs(byte * inst,int op, int reg, int val) {
	return 0;
}

/* modreg funcs */
byte * general_math_function_R(byte * inst, int op, int carry) {
	mem_full_ref src,dest;

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &src, &dest, &flags);

	dest.base.type = ID_TYPE_REG;
	inter_write_math_op(op,&dest,&src,0);
	log_debug("XOR %i %i %i %i",dest.base.type,dest.base.arg,src.base.type,src.base.arg);
	//series_math_operation(op,&src,&dest,carry);
	//series_or(&src,&dest);
	return inst;
}


byte * general_cmp_function_R(byte * inst) {
	mem_full_ref src = {0,},dest = {0,};

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &src, &dest, &flags);

	inter_access_memory(&src);
	dest.base.type = ID_TYPE_REG;

	inter_access_memory(&dest);
	
	
	//series_math_operation(op,&src,&dest,carry);
	//series_or(&src,&dest);
	return inst;
}

/* val funcs */

byte * general_cmp_function_imm8(byte * inst) {
	return 0;
}

byte * general_cmp_function_imm32(byte * inst) {
	return 0;
}

byte * general_math_function_imm8(byte * inst, int op, int carry, int reg) {
	mem_full_ref src,dest;

	//series_ref_reg(&dest,reg);
	//series_ref_imm(&src,*(inst+1));

	inter_write_math_op(op,&src,&dest,0);
	//series_math_operation(op,&src,&dest,carry);
	return inst;
}

byte * general_math_function_imm8_AX(byte * inst, int op, int carry) {
	return general_math_function_imm8(inst,op,carry,EAX);
}

byte * general_math_function_imm32(byte * inst, int op, int carry, int reg) {
	mem_full_ref src,dest;

	//series_ref_reg(&dest,reg);
	//series_ref_imm(&src,*(unsigned int*)(inst+1));

	//series_math_operation(op,&src,&dest,carry);
	return inst;
}

byte * general_OR_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_OR,0);
}

byte * general_ADC_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_ADD,1);
}

byte * general_SBB_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_SUB,1);
}

byte * general_ADD_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_ADD,0);
}

byte * general_AND_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_AND,0);
}

byte * general_SUB_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_SUB,0);
}

byte * general_XOR_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_XOR,0);
}

byte * general_MUL_function_R(byte * inst) {
	return general_math_function_R(inst,SERIES_MUL,0);
}

byte * general_push(byte * inst) {
	dword reg = (*inst)&0x07;
	mem_full_ref mr = {0,};
	mr.base.type = ID_TYPE_REG;
	mr.base.arg = reg;

	inter_write_push_op(&mr);
	return inst+1;
}

byte * general_oper_imm_R(byte * inst, int size, int msize, int use_signal) {
	mem_full_ref src = {0,},dest= {0,};

	//dword flags = MOD_FLAG_DEST_ONLY;
	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &src, &dest, &flags);
	
	int operation = -1;
	int carry = 0;
	
	switch (dest.base.arg) {
		case INST_GEN_ADD:
			operation = SERIES_ADD;
			break;
		case INST_GEN_OR:
			operation = SERIES_OR;
			break;
		case INST_GEN_ADC:
			carry = 1;
			operation = SERIES_ADD;
			break;
		case INST_GEN_SBB:
			carry = 1;
			operation = SERIES_SUB;
			break;
		case INST_GEN_AND:
			operation = SERIES_AND;
			break;
		case INST_GEN_SUB:
			operation = SERIES_SUB;
			break;
		case INST_GEN_XOR:
			operation = SERIES_XOR;
			break;
		case INST_GEN_CMP:
			//return inst+(size == 8 ? 1 : 4);
			operation = OPER_CMP;
			break;
	}
	
	dword val = 0;
	if (size == 8) {
		val = *(inst);
		//series_ref_imm(&src,*(inst+1));
		//series_math_operation(operation,&src,&dest,carry);
		inst = inst+1;
	}
	else if (size == 32) {
		val = *(unsigned int *)(inst);
		//series_ref_imm(&src,*(unsigned int *)(inst+1));
		//series_math_operation(operation,&src,&dest,carry);
		inst = inst+4;
	}
	else {
		log_debug("ERROR invalid size on operation");
	}

	mem_full_ref mr1 = {0,};
	mr1.base.type = ID_TYPE_IMMEDIATE;
	mr1.base.arg = val;
	mr1.base.signal = use_signal;
	
	log_debug("Immediate operation %X, op %i, val %X",dest.base.arg,operation,val);

	if (operation == OPER_CMP) {
		inter_access_memory(&src);
	}
	else {
		inter_write_math_op(operation,&src,&mr1,0);
	}

	return inst;
}

void general_call(dword target) {
	inter_write_call(target);
}

void general_jmp(dword target) {
	inter_write_jmp(target);
}

void general_jc(int type, dword target) {
	inter_write_jc(type,target);
}

void general_ret() {
	inter_write_ret();
}

/******************************************************************
 * Arch specific
 */

/* 0x8 */
byte * _handler_inst_or_0(byte * inst) {
	return general_OR_function_R(inst);
}


/* 0x9 */
byte * _handler_inst_or_1(byte * inst) {
	return general_OR_function_R(inst);
}


/* 0xA */
byte * _handler_inst_or_2(byte * inst) {
	return general_OR_function_R(inst);
}


/* 0xB */
byte * _handler_inst_or_3(byte * inst) {
	return general_OR_function_R(inst);
}


/* 0xC */
byte * _handler_inst_or_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_OR,0,EAX);
}


/* 0xD */
byte * _handler_inst_or_5(byte * inst) {
	return general_math_function_imm32(inst,SERIES_OR,0,EAX);
}


/* 0xE */
byte * _handler_inst_push_cs(byte * inst) {
	//series_push_reg(SEG_REG_CS);
	return inst+1;
}


/* 0xF */
byte * _handler_inst_db_op(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	dword rtype, arg1, arg2;
	byte * o = inst;
	dword target;

	/* these instructions have two operands */
	byte op = *(inst+1);
	switch (op) {
		case DB_INST_JGE:
			target = *(dword*)(inst+2);
			printf("target jge %X\n",inst+6+target-analysis_base);
			general_jc(SERIES_JZ,inst+6+target-analysis_base);
			return (inst+6);
		case DB_INST_IMUL:
			return general_math_function_R(inst+1,SERIES_MUL,0);
			
	}

	return inst;
}


/* 0x10 */
byte * _handler_inst_adc_0(byte * inst) {
	return general_ADC_function_R(inst);
}


/* 0x11 */
byte * _handler_inst_adc_1(byte * inst) {
	return general_ADC_function_R(inst);
}


/* 0x12 */
byte * _handler_inst_adc_2(byte * inst) {
	return general_ADC_function_R(inst);
}


/* 0x13 */
byte * _handler_inst_adc_3(byte * inst) {
	return general_ADC_function_R(inst);
}


/* 0x14 */
byte * _handler_inst_adc_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_ADD,1,EAX);
}


/* 0x15 */
byte * _handler_inst_adc_5(byte * inst) {
	return general_math_function_imm8(inst,SERIES_ADD,1,EAX);
}


/* 0x16 */
byte * _handler_inst_push_ss(byte * inst) {
	series_push_reg(SEG_REG_SS);
	return inst+1;
}


/* 0x17 */
byte * _handler_inst_pop_ss(byte * inst) {
	series_pop_reg(SEG_REG_SS);
	return inst;
}


/* 0x18 */
byte * _handler_inst_sbb_0(byte * inst) {
	return general_SBB_function_R(inst);
}


/* 0x19 */
byte * _handler_inst_sbb_1(byte * inst) {
	return general_SBB_function_R(inst);
}


/* 0x1A */
byte * _handler_inst_sbb_2(byte * inst) {
	return general_SBB_function_R(inst);
}


/* 0x1B */
byte * _handler_inst_sbb_3(byte * inst) {
	return general_SBB_function_R(inst);
}


/* 0x1C */
byte * _handler_inst_sbb_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_SUB,1,EAX);
}


/* 0x1D */
byte * _handler_inst_sbb_5(byte * inst) {
	return general_math_function_imm8(inst,SERIES_SUB,0,EAX);
}


/* 0x1E */
byte * _handler_inst_push_ds(byte * inst) {
	//series_push_reg(SEG_REG_DS);
	return inst+1;
}


/* 0x1F */
byte * _handler_inst_pop_ds(byte * inst) {
	//series_pop_reg(SEG_REG_DS);
	return inst+1;
}


/* 0x20 */
byte * _handler_inst_and_0(byte * inst) {
	return general_AND_function_R(inst);
}


/* 0x21 */
byte * _handler_inst_and_1(byte * inst) {
	return general_AND_function_R(inst);
}


/* 0x22 */
byte * _handler_inst_and_2(byte * inst) {
	return general_AND_function_R(inst);
}


/* 0x23 */
byte * _handler_inst_and_3(byte * inst) {
	return general_AND_function_R(inst);
}


/* 0x24 */
byte * _handler_inst_and_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_AND,0,EAX);
}


/* 0x25 */
byte * _handler_inst_and_5(byte * inst) {
	return general_math_function_imm32(inst,SERIES_AND,0,EAX);
}


/* 0x26 */
byte * _handler_inst_es(byte * inst) {
	/* TODO override influences our logic? */
	return inst+1;
}


/* 0x27 */
byte * _handler_inst_daa(byte * inst) {
	printf("Not implemented 27\n");
	exit(0);
	return inst;
}


/* 0x28 */
byte * _handler_inst_sub_0(byte * inst) {
	return general_SUB_function_R(inst);
}


/* 0x29 */
byte * _handler_inst_sub_1(byte * inst) {
	return general_SUB_function_R(inst);
}


/* 0x2A */
byte * _handler_inst_sub_2(byte * inst) {
	return general_SUB_function_R(inst);
}


/* 0x2B */
byte * _handler_inst_sub_3(byte * inst) {
	return general_SUB_function_R(inst);
}


/* 0x2C */
byte * _handler_inst_sub_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_SUB,0,EAX);
}


/* 0x2D */
byte * _handler_inst_sub_5(byte * inst) {
	return general_math_function_imm32(inst,SERIES_SUB,0,EAX);
}


/* 0x2E */
byte * _handler_inst_cs(byte * inst) {
	return general_SUB_function_R(inst);
}


/* 0x2F */
byte * _handler_inst_das(byte * inst) {
	printf("Not implemented 2F\n");
	exit(0);
	return inst;
}

/* 0x30 */
byte * _handler_inst_xor_0(byte * inst) {
	return general_XOR_function_R(inst);
}

/* 0x31 */
byte * _handler_inst_xor_1(byte * inst) {
	return general_XOR_function_R(inst);
}

/* 0x32 */
byte * _handler_inst_xor_2(byte * inst) {
	return general_XOR_function_R(inst);
}

/* 0x33 */
byte * _handler_inst_xor_3(byte * inst) {
	return general_XOR_function_R(inst);
}

/* 0x34 */
byte * _handler_inst_xor_4(byte * inst) {
	return general_math_function_imm8(inst,SERIES_XOR,0,EAX);
}

/* 0x35 */
byte * _handler_inst_xor_5(byte * inst) {
	return general_math_function_imm32(inst,SERIES_XOR,0,EAX);
}

/* 0x36 */
byte * _handler_inst_ss(byte * inst) {
	printf("Not implemented 36\n");
	exit(0);
	return inst;
}


/* 0x37 */
byte * _handler_inst_aaa(byte * inst) {
	printf("Not implemented 37\n");
	exit(0);
	return inst;
}


/* 0x38 */
byte * _handler_inst_cmp_0(byte * inst) {
	return general_cmp_function_R(inst);
}

/* 0x39 */
byte * _handler_inst_cmp_1(byte * inst) {
	return general_cmp_function_R(inst);
}

/* 0x3A */
byte * _handler_inst_cmp2(byte * inst) {
	return general_cmp_function_R(inst);
}

/* 0x3B */
byte * _handler_inst_cmp3(byte * inst) {
	return general_cmp_function_R(inst);
}


/* 0x3C */
byte * _handler_inst_cmp_4(byte * inst) {
	return general_cmp_function_imm8(inst);
}


/* 0x3D */
byte * _handler_inst_cmp_5(byte * inst) {
	return general_cmp_function_imm32(inst);
}


/* 0x3E */
byte * _handler_inst_ds(byte * inst) {
	printf("Not implemented 3E\n");
	exit(0);
	return inst;
}


/* 0x3F */
byte * _handler_inst_aas(byte * inst) {
	printf("Not implemented 3F\n");
	exit(0);
	return inst;
}


/* 0x40 */
byte * _handler_inst_inc(byte * inst) {
	dword reg = (*inst)&0x07;
	general_reg_math_abs(inst,SERIES_ADD,reg,1);
	return (inst+1);
}


/* 0x48 */
byte * _handler_inst_dec(byte * inst) {
	dword reg = (*inst)&0x07;
	general_reg_math_abs(inst,SERIES_ADD,reg,-1);
	return (inst+1);
}


/* 0x50 */
byte * _handler_inst_push(byte * inst) {
	

	return general_push(inst);
	//series_push_reg(reg);

	//return inst+1;				
}


/* 0x58 */
byte * _handler_inst_pop(byte * inst) {
	dword reg = (*inst)&0x07;
	mem_full_ref mr1 = {0,};
	mr1.base.type = ID_TYPE_REG;
	mr1.base.arg = reg;

	inter_write_pop_op(&mr1);
		
	//series_pop_reg(reg);

	return inst+1;
}


/* 0x60 */
byte * _handler_inst_pushad(byte * inst) {
	int i;
	for(i=EAX;i<= EDI;i++) {
		//series_push_reg(i);
	}
	
	return inst+1;
}


/* 0x61 */
byte * _handler_inst_popad(byte * inst) {
	int i;
	for(i=EAX;i<= EDI;i++) {
		//series_pop_reg(i);
	}

	return inst+1;
}


/* 0x62 */
byte * _handler_inst_bound(byte * inst) {
	printf("Not implemented 62\n");
	exit(0);
	return inst;
}


/* 0x63 */
byte * _handler_inst_arpl(byte * inst) {
	/* i dont think this is even available in user mode, ignore this for now */
	printf("Not implemented 63\n");
	exit(0);
	return inst;
}


/* 0x64 */
byte * _handler_inst_fs(byte * inst) {
	printf("Not implemented 64\n");
	exit(0);
	return inst;
}


/* 0x65 */
byte * _handler_inst_gs(byte * inst) {
	printf("Not implemented 65\n");
	exit(0);
	return inst;
}


/* 0x66 */
byte * _handler_inst_preover(byte * inst) {
	printf("Not implemented 66\n");
	exit(0);
	return inst;
}


/* 0x67 */
byte * _handler_inst_addrover(byte * inst) {
	printf("Not implemented 67\n");
	exit(0);
	return inst;
}


/* 0x68 */
byte * _handler_inst_push32(byte * inst) {
	/* arg is imm */
	//series_push_imm(*(unsigned int*)(inst+1));
	return inst+5;
}


/* 0x69 */
byte * _handler_inst_imul(byte * inst) {
	mem_full_ref src,dest;

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &src, &dest, &flags);
	uint32 oper = *(unsigned int *)inst;

	mem_full_ref src1;
	//series_math_operation(SERIES_MUL,&src,&dest,0);
	//series_math_operation(SERIES_MUL,&src,&dest,0);
	
	return inst+4;
}


/* 0x6A */
byte * _handler_inst_push_imm2(byte * inst) {
	mem_full_ref r1 = {0,};
	r1.base.type = ID_TYPE_IMMEDIATE;
	r1.base.signal = 1;
	r1.base.arg = *(inst+1);
	inter_write_push_op(&r1);
	//series_push_imm(*(inst+1));
	return inst+2;
}


/* 0x6B */
byte * _handler_inst_imul2(byte * inst) {
	printf("Not implemented 6B\n");
	exit(0);
	return inst;
}


/* 0x6C */
byte * _handler_inst_insb(byte * inst) {
	printf("Not implemented 6C\n");
	exit(0);
	return inst;
}


/* 0x6D */
byte * _handler_inst_insw(byte * inst) {
	printf("Not implemented 6D\n");
	exit(0);
	return inst;
}


/* 0x6D */
byte * _handler_inst_insd(byte * inst) {
	printf("Not implemented 6D\n");
	exit(0);
	return inst;
}


/* 0x6E */
byte * _handler_inst_outb(byte * inst) {
	printf("Not implemented 6E\n");
	exit(0);
	return inst;
}


/* 0x6F */
byte * _handler_inst_outw(byte * inst) {
	printf("Not implemented 6F\n");
	exit(0);
	return inst;
}


/* 0x6F */
byte * _handler_inst_outd(byte * inst) {
	printf("Not implemented 6F\n");
	exit(0);
	return inst;
}


/* 0x70 */
byte * _handler_inst_jo(byte * inst) {
	char target = *(char*)(inst+1);

	general_jc(SERIES_JZ,inst+2+target-analysis_base);
	do_branch((dword)(inst+2+target),0,0);
	return inst+2;
}


/* 0x71 */
byte * _handler_inst_jno(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x72 */
byte * _handler_inst_jc(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x73 */
byte * _handler_inst_jnc(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x74 */
byte * _handler_inst_jz(byte * inst) {
	char target = *(char*)(inst+1);
	general_jc(SERIES_JZ,inst+2+target-analysis_base);
	return inst+2;
}


/* 0x75 */
byte * _handler_inst_jnz(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x76 */
byte * _handler_inst_jbe(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x77 */
byte * _handler_inst_jnbe(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x78 */
byte * _handler_inst_js(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x79 */
byte * _handler_inst_jns(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7A */
byte * _handler_inst_jp(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7B */
byte * _handler_inst_jnp(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7C */
byte * _handler_inst_jl(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7D */
byte * _handler_inst_jnl(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7E */
byte * _handler_inst_jle(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x7F */
byte * _handler_inst_jnle(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}


/* 0x80 */
byte * _handler_inst_gen_8bits(byte * inst) {
	return general_oper_imm_R(inst,8,8,0);
}

/* 0x81 */
byte * _handler_inst_gen_32bits(byte * inst) {
	return general_oper_imm_R(inst,32,32,0);
}


/* 0x82 */
byte * _handler_inst_gen_8bits2(byte * inst) {
	return general_oper_imm_R(inst,8,8,0);
}


/* 0x83 */
byte * _handler_inst_gen_32bits2(byte * inst) {
	return general_oper_imm_R(inst,8,32,0);
}


/* 0x84 */
byte * _handler_inst_test(byte * inst) {
	printf("Not implemented 84\n");
	exit(0);
	return inst;
}


/* 0x85 */
byte * _handler_inst_test2(byte * inst) {
	printf("Not implemented 85\n");
	exit(0);
	return inst;
}


/* 0x86 */
byte * _handler_inst_xchg(byte * inst) {
	printf("Not implemented 86\n");
	exit(0);
	return inst;
}


/* 0x87 */
byte * _handler_inst_xchg2(byte * inst) {
	printf("Not implemented 87\n");
	exit(0);
	return inst;
}


/* 0x89 */
byte * _handler_inst_mov2(byte * inst) {
	mem_full_ref src = {0,},dest = {0,};

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	src.disp = 0;
	src.base.type = ID_TYPE_REG;

	inter_write_mem_write(&dest,&src);

	//series_write(&dest,&src,RADDR(o));
	//finalize_write(dest,src);

	return inst;
}


/* 0x8A */
byte * _handler_inst_mov3(byte * inst) {
	printf("Not implemented 8A\n");
	exit(0);
	return inst;
}


/* 0x8B */
byte * _handler_inst_mov4(byte * inst) {
	mem_full_ref src = {0,},dest = {0,};

	dword flags = 0;
	byte * o = inst;

	inst = modreg_analysis(inst, &src, &dest, &flags);

	dest.disp = 0;
	dest.base.type = ID_TYPE_REG;

	//finalize_write(dest,src);
	//series_write(&dest,&src,RADDR(o));
	inter_write_mem_write(&dest,&src);
	return inst;
}


/* 0x8C */
byte * _handler_inst_mov5(byte * inst) {
	printf("Not implemented 8C\n");
	exit(0);
	return inst;
}


/* 0x8D */
byte * _handler_inst_lea(byte * inst) {
	printf("Not implemented 8D\n");
	exit(0);
	return inst;
}


/* 0x8E */
byte * _handler_inst_mov6(byte * inst) {
	printf("Not implemented 8E\n");
	exit(0);
	return inst;
}


/* 0x8F */
byte * _handler_inst_pop32(byte * inst) {
	printf("Not implemented 8F\n");
	exit(0);
	return inst;
}


/* 0x90 */
byte * _handler_inst_pause(byte * inst) {
	printf("Not implemented 90\n");
	exit(0);
	return inst;
}


/* 0x98 */
byte * _handler_inst_cwde(byte * inst) {
	printf("Not implemented 98\n");
	exit(0);
	return inst;
}


/* 0x99 */
byte * _handler_inst_cdq(byte * inst) {
	printf("Not implemented 99\n");
	exit(0);
	return inst;
}


/* 0x9A */
byte * _handler_inst_callf(byte * inst) {
	printf("Not implemented 9A\n");
	exit(0);
	return inst;
}


/* 0x9B */
byte * _handler_inst_wait(byte * inst) {
	printf("Not implemented 9B\n");
	exit(0);
	return inst;
}


/* 0x9C */
byte * _handler_inst_pushfd(byte * inst) {
	printf("Not implemented 9C\n");
	exit(0);
	return inst;
}


/* 0x9D */
byte * _handler_inst_popfd(byte * inst) {
	printf("Not implemented 9D\n");
	exit(0);
	return inst;
}


/* 0x9E */
byte * _handler_inst_sahf(byte * inst) {
	printf("Not implemented 9E\n");
	exit(0);
	return inst;
}


/* 0x9F */
byte * _handler_inst_lahf(byte * inst) {
	printf("Not implemented 9F\n");
	exit(0);
	return inst;
}


/* 0xA0 */
byte * _handler_inst_mov7(byte * inst) {
	printf("Not implemented A0\n");
	exit(0);
	return inst;
}


/* 0xA1 */
byte * _handler_inst_mov8(byte * inst) {

	return inst+5;
}


/* 0xA2 */
byte * _handler_inst_mov9(byte * inst) {
	printf("Not implemented A2\n");
	exit(0);
	return inst;
}


/* 0xA3 */
byte * _handler_inst_mov10( byte * inst ) {
	dword target = *(dword *)(inst+1);
	series_addr_ax_write_abs(target,RADDR(inst));
	return inst+5;
}


/* 0xA4 */
byte * _handler_inst_movsb(byte * inst) {
	printf("Not implemented A4\n");
	exit(0);
	return inst;
}


/* 0xA5 */
byte * _handler_inst_movsd(byte * inst) {
	printf("Not implemented A5\n");
	exit(0);
	return inst;
}


/* 0xA6 */
byte * _handler_inst_cmpsb(byte * inst) {
	printf("Not implemented A6\n");
	exit(0);
	return inst;
}


/* 0xA7 */
byte * _handler_inst_cmpsd(byte * inst) {
	printf("Not implemented A7\n");
	exit(0);
	return inst;
}


/* 0xA8 */
byte * _handler_inst_test3(byte * inst) {
	printf("Not implemented A8\n");
	exit(0);
	return inst;
}


/* 0xA9 */
byte * _handler_inst_test4(byte * inst) {
	printf("Not implemented A9\n");
	exit(0);
	return inst;
}


/* 0xAA */
byte * _handler_inst_stosb(byte * inst) {
	printf("Not implemented AA\n");
	exit(0);
	return inst;
}


/* 0xAB */
byte * _handler_inst_stosd(byte * inst) {
	printf("Not implemented AB\n");
	exit(0);
	return inst;
}


/* 0xAC */
byte * _handler_inst_lodsb(byte * inst) {
	printf("Not implemented AC\n");
	exit(0);
	return inst;
}


/* 0xAD */
byte * _handler_inst_lodsd(byte * inst) {
	printf("Not implemented AD\n");
	exit(0);
	return inst;
}


/* 0xAE */
byte * _handler_inst_scasb(byte * inst) {
	printf("Not implemented AE\n");
	exit(0);
	return inst;
}


/* 0xAF */
byte * _handler_inst_scasd(byte * inst) {
	printf("Not implemented AF\n");
	exit(0);
	return inst;
}


/* 0xB0 */
byte * _handler_inst_gen_mov(byte * inst) {
	return inst;
}


/* 0xB8 */
byte * _handler_inst_gen_mov32(byte * inst) {
	dword imm = *(dword*)(inst+1);
	dword reg = (*inst) & 0x7;
	series_addr_reg_write_imm(imm,reg,RADDR(inst));
	return inst;
}


/* 0xC0 */
byte * _handler_inst_shl(byte * inst) {
	printf("Not implemented C0\n");
	exit(0);
	return inst;
}


/* 0xC1 */
byte * _handler_inst_shl32(byte * inst) {
	printf("Not implemented C1\n");
	exit(0);
	return inst;
}


/* 0xC2 */
byte * _handler_inst_retn(byte * inst) {
	general_ret();
	log_data("RETN reached, leaving super context");
	return 0;
}


/* 0xC3 */
byte * _handler_inst_retn2(byte * inst) {
	general_ret();
	log_data("RETN2 reached, leaving super context");
	return 0;
}


/* 0xC4 */
byte * _handler_inst_les(byte * inst) {
	printf("Not implemented C4\n");
	exit(0);
	return inst;
}


/* 0xC5 */
byte * _handler_inst_lds(byte * inst) {
	printf("Not implemented C5\n");
	exit(0);
	return inst;
}


/* 0xC6 */
byte * _handler_inst_mov11(byte * inst) {
	printf("Not implemented C6\n");
	exit(0);
	return inst;
}


/* 0xC7 */
byte * _handler_inst_mov12(byte * inst) {
	mem_full_ref dest = {0,};
	mem_full_ref src = {0,};
	dword flags = MOD_FLAG_DEST_ONLY;

	byte * o = inst;
	inst = modreg_analysis(inst,0,&dest,&flags);

	src.base.type = ID_TYPE_IMMEDIATE;
	src.base.arg = *(dword*)inst;
	src.disp = 0;

	inter_write_mem_write(&dest,&src);

	//finalize_write(dest,src);
	//series_write(&dest,&src,RADDR(o));

	return inst+4;
}


/* 0xC8 */
byte * _handler_inst_enter(byte * inst) {
	printf("Not implemented C8\n");
	exit(0);
	return inst;
}


/* 0xC9 */
byte * _handler_inst_leave(byte * inst) {
	printf("Not implemented C9\n");
	exit(0);
	return inst;
}


/* 0xCA */
byte * _handler_inst_retf(byte * inst) {
	printf("Not implemented CA\n");
	exit(0);
	return inst;
}


/* 0xCB */
byte * _handler_inst_retf2(byte * inst) {
	printf("Not implemented CB\n");
	exit(0);
	return inst;
}


/* 0xCC */
byte * _handler_inst_int(byte * inst) {
	printf("Not implemented CC\n");
	exit(0);
	return inst;
}


/* 0xCD */
byte * _handler_inst_int2(byte * inst) {
	printf("Not implemented CD\n");
	exit(0);
	return inst;
}


/* 0xCE */
byte * _handler_inst_int0(byte * inst) {
	printf("Not implemented CE\n");
	exit(0);
	return inst;
}


/* 0xCF */
byte * _handler_inst_iretd(byte * inst) {
	printf("Not implemented CF\n");
	exit(0);
	return inst;
}


/* 0xD0 */
byte * _handler_inst_shl2(byte * inst) {
	printf("Not implemented D0\n");
	exit(0);
	return inst;
}


/* 0xD1 */
byte * _handler_inst_shl322(byte * inst) {
	printf("Not implemented D1\n");
	exit(0);
	return inst;
}


/* 0xD2 */
byte * _handler_inst_shl3(byte * inst) {
	printf("Not implemented D2\n");
	exit(0);
	return inst;
}


/* 0xD3 */
byte * _handler_inst_shl323(byte * inst) {
	printf("Not implemented D3\n");
	exit(0);
	return inst;
}


/* 0xD4 */
byte * _handler_inst_amx(byte * inst) {
	printf("Not implemented D4\n");
	exit(0);
	return inst;
}


/* 0xD5 */
byte * _handler_inst_adx(byte * inst) {
	printf("Not implemented D5\n");
	exit(0);
	return inst;
}


/* 0xD6 */
byte * _handler_inst_setalc(byte * inst) {
	printf("Not implemented D6\n");
	exit(0);
	return inst;
}


/* 0xD7 */
byte * _handler_inst_xlatb(byte * inst) {
	printf("Not implemented D7\n");
	exit(0);
	return inst;
}


/* 0xD8 */
byte * _handler_inst_fpu1(byte * inst) {
	printf("Not implemented D8\n");
	exit(0);
	return inst;
}



/* 0xE8 */
byte * _handler_inst_call(byte * inst) {
	dword arg1;
	arg1 = *((int *)(inst+1));

	dword laddr = (dword)(inst-analysis_base);
	general_call(laddr+5+arg1);
	/* context change */
	return inst + 5 + arg1;

}

/* 0xE9 */
byte * _handler_inst_jmp2(byte * inst) {
	byte * target = (*(dword*)(inst+1))+inst+5;
	general_jmp(target-analysis_base);

	return inst;
}

/* 0xEB */
byte * _handler_inst_jmp(byte * inst) {
	
	char target = *(char*)(inst+1);
	general_jmp((dword)(inst+2+target-analysis_base));
	
// 	if (do_jmp((dword)inst, (dword)(inst+2+target),2,1)) {
// 		log_data("jmping rel: (%i)",target);
// 		return inst+2+target;
// 	}
	return 0;

}

/* 0xFF */
byte * _handler_inst_gen_1op(byte * inst) {
	dword rtype, arg1, arg2;
	MODREGDEC(inst+1,arg1,arg2,rtype);
	dword target;
	series_fn_info * fni;
	it_inner_entry * it;

	/* TODO embbed this on the mphash */
	switch (arg2) {
		case INST_GEN_INC:
			break;
		case INST_GEN_DEC:
			break;
		case INST_GEN_CALL:
			target = *(dword*)(inst+2);
			inter_call(target);
			if (!is_code(target,&it)) {
				if ((fni = syscall_get_info(target))) {
					//series_process_call(fni,RADDR(inst));
				}
				/* clobber everything, look in stack for arguments */
				return inst+6;
			}
			else {
				return (byte*)target;
			}
		case INST_GEN_CALLF:
			break;
		case INST_GEN_JMP:
			break;
		case INST_GEN_JMPF:
			break;
		case INST_GEN_PUSH:
			break;
	}

	return inst;
}


byte * _handler_inst_fpu2(byte * inst) {
	printf("Not implemented D9\n");
	exit(0);
	return inst;
}



