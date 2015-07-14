#include "analysis_engine.h"
#include "context.h"
#include "flux.h"

extern byte * refaddr;

#define RADDR(r) (r-refaddr)
#define RADDRC(r,s) (r-refaddr+s)

byte * _f_handler_inst_push(byte * inst) {
	flux_staple(RADDR(inst),1);
	return inst+1;				
}

byte *  _f_handler_inst_add_3(byte * inst) {
	mem_full_ref src,dest;
	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte *  _f_handler_inst_add_5(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_push_imm2(byte * inst) {
	flux_staple(RADDR(inst),2);
	return inst+2;
}

byte * _f_handler_inst_pop(byte * inst) {
	flux_staple(RADDR(inst),1);
	return inst+1;
}

byte * _f_handler_inst_inc(byte * inst) {
	flux_staple(RADDR(inst),1);
	return (inst+1);
}

byte * _f_handler_inst_inc_2(byte * inst) {
	mem_full_ref src,dest;
	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;	
}

byte * _f_handler_inst_dec(byte * inst) {
	flux_staple(RADDR(inst),1);
	return (inst+1);
}

extern byte * analysis_base;

byte * _f_handler_inst_call(byte * inst) {
	dword arg1;
	arg1 = *((int *)(inst+1));
	dword laddr = (dword)(inst-analysis_base);
	log_debug("CALL issued to %X",laddr+5+arg1);
	do_branch_f(laddr,laddr+5+arg1,5);
	return inst+5;
}



byte * _f_handler_inst_gen_32bits2(byte * inst) {
	
	dword rtype, arg1, arg2;
	MODREGDEC(inst+1,arg1,arg2,rtype);
//log_debug("gen %X",arg2);
	int disp;
	if (*inst == INST_GEN_32BITS2 || *inst == INST_GEN_8BITS || *inst == INST_GEN_8BITS2) {
		disp = 1;
	}
	else if (*inst == INST_GEN_32BITS) {
		disp = 4;
	}
	
	/* TODO embbed this on the mphash */
	switch (arg2) {
		case INST_GEN_ADD:			
		case INST_GEN_OR:
		case INST_GEN_ADC:
		case INST_GEN_SBB:
		case INST_GEN_AND:
		case INST_GEN_SUB:
		case INST_GEN_XOR:
		case INST_GEN_CMP:
			log_debug("rtype %X",disp);
			if (rtype == smod_reg_I32R) {
				if (arg1 == mod_reg_SIB) {
					flux_staple(RADDR(inst),7+disp);
					return inst+7+disp;
				}
				flux_staple(RADDR(inst),6+disp);
				return inst+6+disp;
			}
			else if (rtype == smod_reg_I8R) {
				flux_staple(RADDR(inst),3+disp);
				log_debug("ret +%i",3+disp);
				return inst+3+disp;
			}			
			else if (rtype == smod_reg_RR) {
				flux_staple(RADDR(inst),2+disp);
				return inst+2+disp;
			}
			else if (rtype == smod_reg_IR) {
				if (arg1 == mod_reg_EBP) {
					flux_staple(RADDR(inst),2+disp+4);
					return inst+2+disp+4;
				}
				flux_staple(RADDR(inst),2+disp);
				return inst+2+disp;
			}

			flux_staple(RADDR(inst),2+disp);
			return inst+2+disp;
	}
}

byte * _f_handler_inst_gen_mov(byte * inst) {
	dword size = 0;
	if ((*inst) & 0x8) {
		/* 32 bits */
		size = 4;
	}
	else {
		size = 1;
	}
	byte * o = inst;
	inst = inst+1+size;
	flux_staple(RADDR(o),inst-o);

	return inst;
}

byte * _f_handler_inst_shl32(byte * inst) {
	/*mod reg gives an operand here, but it's irrelevant for this phase */
	dword rtype, arg1, arg2;
	MODREGDEC(inst+1,arg1,arg2,rtype);
	flux_staple(RADDR(inst),3);
	return inst+3;
}

byte * _f_handler_inst_or_3(byte * inst) {
	mem_full_ref src,dest;

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_mov2(byte * inst) {
	mem_full_ref src,dest;

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_mov4(byte * inst) {
	mem_full_ref src,dest;

	dword flags = 0;
	//mem_id val;

	byte * o = inst;

	inst = modreg_analysis(inst, &src, &dest, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_mov8(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_mov10(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_jmp2(byte * inst) {
	dword target = *(dword*)(inst+1);

	//flux_staple(RADDR(inst),5);
	if (do_jmp((dword)RADDR(inst), (dword)RADDR(inst+5+target),2)) {
		log_data("jmping (32-bits) rel: (%i)",target);
		return inst+5+target;
	}
	return 0;
}

byte * _f_handler_inst_jmp(byte * inst) {
	char target = *(char*)(inst+1);

	if (do_jmp((dword)RADDR(inst), (dword)RADDR(inst+2+target),2)) {
		log_data("jmping (8-bits) rel: (%i)",target);
		return inst+2+target;
	}
	return 0;
	
}

byte * _f_handler_inst_jo(byte * inst) {
	char target = *(char*)(inst+1);
	
	do_branch(RADDR(inst),(dword)RADDR(inst+2+target),2);
	return inst+2;
}

byte * _f_handler_inst_jno(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jc(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnc(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jz(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnz(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jbe(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnbe(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_js(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jns(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jp(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnp(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jl(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnl(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jle(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_jnle(byte * inst) {
	char target = *(char*)(inst+1);
	return inst+2;
}

byte * _f_handler_inst_mov12(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),inst-o+4);
	return inst+4;
}

byte * _f_handler_inst_cmp2(byte * inst) {
	flux_staple(RADDR(inst),3);
	return inst+3;
}

byte * _f_handler_inst_test(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_leave(byte * inst) {
	flux_staple(RADDR(inst),1);
	log_debug("leave");
	return inst+1;
}

byte * _f_handler_inst_gen5_inst(byte * inst) {
	dword rtype, arg1, arg2;
	MODREGDEC(inst+1,arg1,arg2,rtype);

	switch (arg2) {
		case INST_GEN_TEST1:
		case INST_GEN_TEST2:
			
			break;
		case INST_GEN_NOT:
			flux_staple(RADDR(inst),2);
			return inst+2;
	}
	flux_staple(RADDR(inst),2);
	return inst+2;
}

byte * _f_handler_inst_test2(byte * inst) {
	return inst+2;
}

#define	F_INST_TEST2_H	_f_handler_inst_test2

byte * _f_handler_inst_cmp3(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_gen_1op(byte * inst) {
	dword rtype, arg1, arg2;
	MODREGDEC(inst+1,arg1,arg2,rtype);
	dword target;
	it_inner_entry *it;

	/* TODO embbed this on the mphash */
	switch (arg2) {
		
		case INST_GEN_CALL:
			if (rtype == smod_reg_RR) {
				flux_staple(RADDR(inst),2);
				return inst+2;
			}
			target = *(dword*)(inst+2);
			log_debug("CALL to %X",target);
			
			if (!is_code(target,&it)) {
				if (it) {
					/* maybe examine here the arguments for ease of analysis? */
					log_debug("External function (%s) (%s)",it->name,it->libname);
					if (!strcmp("ExitProcess",it->name)) {
						/* its an exit process, function ends here */
						do_ret(RADDR(inst),6);
						log_debug("exit function reached");
						return 0;
					}
				}
			}
			else {
				//return (byte*)target;
				/* queue function for analysis */
				
			}
			flux_staple(RADDR(inst),6);
			return inst+6;
		case INST_GEN_JMP:
			target = *(dword*)(inst+2);
			
			it_inner_entry * it;
			if (!is_code(target,&it)) {
				log_debug("JMP to outside, assuming ret");
				/* this is used normally to call external functions in a relloc fashion,
					assume RET here */
				do_ret(RADDR(inst),6);
				return 0;
			}
			else {

			}
			flux_staple(RADDR(inst),6);
			return inst+6;
		case INST_GEN_JMPF:
			/* jmp far, let's check the target */

			break;
		case INST_GEN_INC:
		case INST_GEN_DEC:
		case INST_GEN_CALLF:
			/* this is a register call, we must flag it, but we cannot know what was called right now */
		case INST_GEN_PUSH:
			if (rtype == smod_reg_IR) {
				if (arg1 == mod_reg_EBP) {
					/* not really ebp here, but disp32 */
					flux_staple(RADDR(inst),6);
					return inst+6;
				}
				if (arg1 == mod_reg_SIB) {
					MODREGDEC(inst+2,arg1,arg2,rtype);
					flux_staple(RADDR(inst),7);
					return inst+7;
				}
				flux_staple(RADDR(inst),2);
				return inst+2;
			}
			if (rtype == smod_reg_I8R) {
				flux_staple(RADDR(inst),3);
				return inst+3;
			}
			if (rtype == smod_reg_RR) {
				flux_staple(RADDR(inst),2);
				return inst+2;
			}
			flux_staple(RADDR(inst),6);
			return inst+6;
			break;
	}

	return inst;
}

byte * _f_handler_inst_xor0(byte * inst) {
	flux_staple(RADDR(inst),1);
	return inst+1;
}

byte * _f_handler_inst_xor1(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_xor2(byte * inst) {
	flux_staple(RADDR(inst),1);
	return inst+1;
}

byte * _f_handler_inst_xor3(byte * inst) {
	
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	
	inst = modreg_analysis(inst, &dest, &src, &flags);

	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_xor4(byte * inst) {
	flux_staple(RADDR(inst),1);
	return inst+1;
}

byte * _f_handler_inst_xor5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_lea(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_retn(byte * inst) {
	
	log_data("RETN reached");
	do_ret(RADDR(inst),1);
	return 0;
}

/* this is wrong? */
byte * _f_handler_inst_gen_cmp(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fdivr(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fnstcw(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pop_es(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fpu2_sub7(byte * inst) {
	return inst;
}

byte * _f_handler_inst_or_0(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_or_1(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_or_2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_or_4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_or_5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_push_cs(byte * inst) {
	return inst;
}

byte * _f_handler_inst_db_op(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	dword rtype, arg1, arg2;
	byte * o = inst;
	dword target;
	/* these instructions have two operands */
	byte op = *(inst+1);
	switch(op) {
		case DB_INST_MOVZX:
			inst = modreg_analysis(inst+1, &dest, &src, &flags);
			flux_staple(RADDR(o),inst-o);
			return inst;
			break;
		case DB_INST_SETZ:			
			inst = modreg_analysis(inst+1, &dest, &src, &flags);
			flux_staple(RADDR(o),inst-o);
			return inst;
			break;
		case DB_INST_SETNZ:			
			inst = modreg_analysis(inst+1, &dest, &src, &flags);
			flux_staple(RADDR(o),inst-o);
			return inst;
		case DB_INST_JZ:
		case DB_INST_JGE:
			//flux_staple(RADDR(o),6);
			target = *(dword*)(inst+2);
			do_branch(RADDR(inst),(dword)RADDR(inst+6+target),6);
			return inst+6;
		case DB_INST_ADD:
			flux_staple(RADDR(o),6);
			return inst+6;
		case DB_INST_JNZ:
			flux_staple(RADDR(o),6);
			return inst+6;
		case DB_INST_IMUL:
			flux_staple(RADDR(o),4);
			return inst+4;

	}
	return inst;
}

byte * _f_handler_inst_adc_0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adc_1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adc_2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adc_3(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adc_4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adc_5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_push_ss(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pop_ss(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sbb_0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sbb_1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sbb_2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sbb_3(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_sbb_4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sbb_5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_push_ds(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pop_ds(byte * inst) {
	return inst;
}

byte * _f_handler_inst_and_0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_and_1(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_and_2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_and_3(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_and_4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_and_5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_es(byte * inst) {
	return inst;
}

byte * _f_handler_inst_daa(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sub_0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sub_1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sub_2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sub_3(byte * inst) {
	mem_full_ref src,dest;
	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
	
}

byte * _f_handler_inst_sub_4(byte * inst) {
	return inst;
}

/* sub eax, imm16/32*/
byte * _f_handler_inst_sub_5(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_cs(byte * inst) {
	return inst;
}

byte * _f_handler_inst_das(byte * inst) {
	return inst;
}

byte * _f_handler_inst_ss(byte * inst) {
	return inst;
}

byte * _f_handler_inst_aaa(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cmp_0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cmp_1(byte * inst) {
	mem_full_ref src,dest;

	dword flags = 0;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_cmp_4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cmp_5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_ds(byte * inst) {
	return inst;
}

byte * _f_handler_inst_aas(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pushad(byte * inst) {
	return inst;
}

byte * _f_handler_inst_popad(byte * inst) {
	return inst;
}

byte * _f_handler_inst_bound(byte * inst) {
	return inst;
}

byte * _f_handler_inst_arpl(byte * inst) {
	return inst;
}

extern inst_table * cur_handlers;

/* special prefix functions */

byte * _f_handler_inst_fs(byte * inst) {
	byte * o = inst;
	inst = cur_handlers->handler[*(inst+1)](inst+1);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_gs(byte * inst) {
	return inst;
}

byte * _f_handler_inst_preover(byte * inst) {
	byte * o = inst;
	inst = cur_handlers->handler[*(inst+1)](inst+1);
	flux_staple(RADDR(o),inst-o);
	return inst;
}

byte * _f_handler_inst_addrover(byte * inst) {
	return inst;
}

byte * _f_handler_inst_push32(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_imul(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),(inst-o)+4);
	return inst+4;
}

byte * _f_handler_inst_imul2(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),(inst-o)+1);
	return inst+1;
}

byte * _f_handler_inst_insb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_insd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_insw(byte * inst) {
	return inst;
}

byte * _f_handler_inst_outb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_outw(byte * inst) {
	return inst;
}

byte * _f_handler_inst_outd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_gen_8bits(byte * inst) {
	return inst;
}

byte * _f_handler_inst_gen_32bits(byte * inst) {
	return inst;
}

byte * _f_handler_inst_gen_8bits2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_xchg(byte * inst) {
	return inst;
}

byte * _f_handler_inst_xchg2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_mov(byte * inst) {
	return inst;
}

byte * _f_handler_inst_mov3(byte * inst) {
	dword flags = 0;
	mem_full_ref dest;
	mem_full_ref src;
	byte * o = inst;
	inst = modreg_analysis(inst, &dest, &src, &flags);
	flux_staple(RADDR(o),inst-o);

	return inst;
}

byte * _f_handler_inst_mov5(byte * inst) {
	return inst;
}

byte * _f_handler_inst_mov6(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pop32(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pause(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cwde(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cdq(byte * inst) {

	flux_staple(RADDR(inst),1);
	return inst+1;
}

byte * _f_handler_inst_callf(byte * inst) {
	return inst;
}

byte * _f_handler_inst_wait(byte * inst) {
	return inst;
}

byte * _f_handler_inst_pushfd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_popfd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_sahf(byte * inst) {
	return inst;
}

byte * _f_handler_inst_lahf(byte * inst) {
	return inst;
}

byte * _f_handler_inst_mov7(byte * inst) {
	return inst;
}

/* mov AL to 4-byte offset */
byte * _f_handler_inst_mov9(byte * inst) {
	flux_staple(RADDR(inst),5);
	return inst+5;
}

byte * _f_handler_inst_movsb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_movsd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cmpsb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_cmpsd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_test3(byte * inst) {
	return inst;
}

byte * _f_handler_inst_test4(byte * inst) {
	return inst;
}

byte * _f_handler_inst_test_5(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),(inst-o)+1);
	
	return inst+1;
}

byte * _f_handler_inst_stosb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_stosd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_lodsb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_lodsd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_scasb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_scasd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_gen_mov32(byte * inst) {
	return inst;
}

byte * _f_handler_inst_retn2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_les(byte * inst) {
	return inst;
}

byte * _f_handler_inst_lds(byte * inst) {
	return inst;
}

byte * _f_handler_inst_mov11(byte * inst) {
	return inst;
}

byte * _f_handler_inst_enter(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fxch_opt(byte * inst) {
	return inst;
}

byte * _f_handler_inst_retf(byte * inst) {
	return inst;
}

byte * _f_handler_inst_retf2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_int(byte * inst) {
	return inst;
}

byte * _f_handler_inst_int2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_int0(byte * inst) {
	return inst;
}

byte * _f_handler_inst_iretd(byte * inst) {
	return inst;
}

byte * _f_handler_inst_shl2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fnop_sec(byte * inst) {
	return inst;
}

byte * _f_handler_inst_shl322(byte * inst) {
	return inst;
}

byte * _f_handler_inst_shl3(byte * inst) {
	return inst;
}

byte * _f_handler_inst_shl323(byte * inst) {
	byte * o = inst;

	mem_full_ref dest;
	dword flags = MOD_FLAG_DEST_ONLY;
	inst = modreg_analysis(inst,0,&dest,&flags);

	flux_staple(RADDR(o),(inst-o));

	return inst;
}

byte * _f_handler_inst_amx(byte * inst) {
	return inst;
}

byte * _f_handler_inst_adx(byte * inst) {
	return inst;
}

byte * _f_handler_inst_setalc(byte * inst) {
	return inst;
}

byte * _f_handler_inst_xlatb(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fpu1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fpu2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fchs(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fabs(byte * inst) {
	return inst;
}

byte * _f_handler_inst_ftst(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fxam(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fld1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldl2t(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldl2e(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldpi(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldlg2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldln2(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fldz(byte * inst) {
	return inst;
}

byte * _f_handler_inst_f2xm1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fyl2x(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fptan(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fpatan(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fxtract(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fprem1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fdecstp(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fincstp(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fprem(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fyl2xp1(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fsqrt(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fsincos(byte * inst) {
	return inst;
}

byte * _f_handler_inst_frndint(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fscale(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fsin(byte * inst) {
	return inst;
}

byte * _f_handler_inst_fcos(byte * inst) {
	return inst;
}


byte * _f_handler_inst_rep(byte * inst) {
	return inst+2;
}



#undef RADDR
#undef RADDRC