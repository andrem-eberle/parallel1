#ifndef _PHASE1_ENGINE_
#define _PHASE1_ENGINE_

/* Instruction handlers for flux building */

byte *  _f_handler_inst_add_3(byte * inst);
#define F_INST_ADD_3_H			_f_handler_inst_add_3

byte *  _f_handler_inst_add_5(byte * inst);
#define F_INST_ADD_5_H			_f_handler_inst_add_5

byte *  _f_handler_inst_or_3(byte * inst);
#define F_INST_OR_3_H			_f_handler_inst_or_3

byte *  _f_handler_inst_push(byte * inst);
#define F_INST_PUSH_H			_f_handler_inst_push

byte * _f_handler_inst_push_imm2(byte * inst);
#define F_INST_PUSH_IMM2_H	_f_handler_inst_push_imm2

byte * _f_handler_inst_pop(byte * inst);
#define F_INST_POP_H			_f_handler_inst_pop

byte * _f_handler_inst_xor0(byte * inst);
#define F_INST_XOR_0_H		_f_handler_inst_xor0

byte * _f_handler_inst_xor1(byte * inst);
#define F_INST_XOR_1_H		_f_handler_inst_xor1

byte * _f_handler_inst_xor2(byte * inst);
#define F_INST_XOR_2_H		_f_handler_inst_xor2

byte * _f_handler_inst_xor3(byte * inst);
#define F_INST_XOR_3_H		_f_handler_inst_xor3

byte * _f_handler_inst_xor4(byte * inst);
#define F_INST_XOR_4_H		_f_handler_inst_xor4

byte * _f_handler_inst_xor5(byte * inst);
#define F_INST_XOR_5_H		_f_handler_inst_xor5
	
byte * _f_handler_inst_shl32(byte * inst);
#define F_INST_SHL32_H		_f_handler_inst_shl32

byte * _f_handler_inst_inc(byte * inst);
#define F_INST_INC_H			_f_handler_inst_inc

byte * _f_handler_inst_inc_2(byte * inst);
#define F_INST_INC_2_H			_f_handler_inst_inc_2

byte * _f_handler_inst_dec(byte * inst);
#define F_INST_DEC_H			_f_handler_inst_dec

byte * _f_handler_inst_call(byte * inst);
#define F_INST_CALL_H			_f_handler_inst_call

byte * _f_handler_inst_mov2(byte * inst);
#define F_INST_MOV2_H			_f_handler_inst_mov2

byte * _f_handler_inst_mov4(byte * inst);
#define F_INST_MOV4_H			_f_handler_inst_mov4

byte * _f_handler_inst_mov8(byte * inst);
#define F_INST_MOV8_H			_f_handler_inst_mov8

byte * _f_handler_inst_mov10(byte * inst);
#define F_INST_MOV10_H			_f_handler_inst_mov10

byte * _f_handler_inst_mov12(byte * inst);
#define	F_INST_MOV12_H	_f_handler_inst_mov12

/* 8 and 16 bits fall here */
byte * _f_handler_inst_gen_mov(byte * inst);
#define	F_INST_GEN_MOV_H	_f_handler_inst_gen_mov

byte * _f_handler_inst_gen_32bits2(byte * inst);
#define	F_INST_GEN_32BITS2_H	_f_handler_inst_gen_32bits2

byte * _f_handler_inst_gen_32bits(byte * inst);
#define	F_INST_GEN_32BITS_H	_f_handler_inst_gen_32bits

byte * _f_handler_inst_leave(byte * inst);
#define F_INST_LEAVE_H			_f_handler_inst_leave

byte * _f_handler_inst_jmp(byte * inst);
#define	F_INST_JMP_H	_f_handler_inst_jmp

byte * _f_handler_inst_jmp2(byte * inst);
#define	F_INST_JMP2_H	_f_handler_inst_jmp2

byte * _f_handler_inst_test(byte * inst);
#define	F_INST_TEST_H	_f_handler_inst_test

byte * _f_handler_inst_test2(byte * inst);
#define	F_INST_TEST2_H	_f_handler_inst_test2

byte * _f_handler_inst_gen5_inst(byte * inst);
#define	F_INST_GEN5_INST_H	_f_handler_inst_gen5_inst

byte * _f_handler_inst_cmp2(byte * inst);
#define	F_INST_CMP_2_H	_f_handler_inst_cmp2

byte * _f_handler_inst_cmp3(byte * inst);
#define F_INST_CMP_3_H	_f_handler_inst_cmp3

byte * _f_handler_inst_jo(byte * inst);
#define F_INST_JO_H		_f_handler_inst_jo

byte * _f_handler_inst_jno(byte * inst);
#define F_INST_JNO_H		_f_handler_inst_jno

byte * _f_handler_inst_jc(byte * inst);
#define F_INST_JC_H		_f_handler_inst_jc

byte * _f_handler_inst_jnc(byte * inst);
#define F_INST_JNC_H		_f_handler_inst_jnc

byte * _f_handler_inst_jz(byte * inst);
#define F_INST_JZ_H		_f_handler_inst_jz

byte * _f_handler_inst_jnz(byte * inst);
#define F_INST_JNZ_H		_f_handler_inst_jnz

byte * _f_handler_inst_jbe(byte * inst);
#define F_INST_JBE_H		_f_handler_inst_jbe

byte * _f_handler_inst_jnbe(byte * inst);
#define F_INST_JNBE_H		_f_handler_inst_jnbe

byte * _f_handler_inst_js(byte * inst);
#define F_INST_JS_H		_f_handler_inst_js

byte * _f_handler_inst_jns(byte * inst);
#define F_INST_JNS_H		_f_handler_inst_jns

byte * _f_handler_inst_jp(byte * inst);
#define F_INST_JP_H		_f_handler_inst_jp

byte * _f_handler_inst_jnp(byte * inst);
#define F_INST_JNP_H		_f_handler_inst_jnp

byte * _f_handler_inst_jl(byte * inst);
#define F_INST_JL_H		_f_handler_inst_jl

byte * _f_handler_inst_jnl(byte * inst);
#define F_INST_JNL_H		_f_handler_inst_jnl

byte * _f_handler_inst_jle(byte * inst);
#define F_INST_JLE_H		_f_handler_inst_jle

byte * _f_handler_inst_jnle(byte * inst);
#define F_INST_JNLE_H		_f_handler_inst_jnle

byte * _f_handler_inst_gen_1op(byte * inst);
#define F_INST_GEN_1OP_H	_f_handler_inst_gen_1op

byte * _f_handler_inst_retn(byte * inst);
#define F_INST_RETN_H		_f_handler_inst_retn

byte * _f_handler_inst_lea(byte * inst);
#define F_INST_LEA_H		_f_handler_inst_lea

/***********************************************/

byte * _f_handler_inst_gen_cmp(byte * inst);
#define F_INST_GEN_CMP2_H	_f_handler_inst_gen_cmp

byte * _f_handler_inst_fdivr(byte * inst);
#define F_INST_FDIVR2_H	_f_handler_inst_fdivr

byte * _f_handler_inst_fnstcw(byte * inst);
#define F_INST_FNSTCW2_H	_f_handler_inst_fnstcw

byte * _f_handler_inst_pop_es(byte * inst);
#define F_INST_POP_ES2_H	_f_handler_inst_pop_es

byte * _f_handler_inst_fpu2_sub7(byte * inst);
#define F_INST_FPU2_SUB72_H	_f_handler_inst_fpu2_sub7

byte * _f_handler_inst_or_0(byte * inst);
#define F_INST_OR_02_H	_f_handler_inst_or_0

byte * _f_handler_inst_or_1(byte * inst);
#define F_INST_OR_12_H	_f_handler_inst_or_1

byte * _f_handler_inst_rep(byte * inst);
#define F_INST_REP_H _f_handler_inst_rep

byte * _f_handler_inst_or_2(byte * inst);
#define F_INST_OR_22_H	_f_handler_inst_or_2

byte * _f_handler_inst_or_4(byte * inst);
#define F_INST_OR_42_H	_f_handler_inst_or_4

byte * _f_handler_inst_or_5(byte * inst);
#define F_INST_OR_52_H	_f_handler_inst_or_5

byte * _f_handler_inst_push_cs(byte * inst);
#define F_INST_PUSH_CS2_H	_f_handler_inst_push_cs

byte * _f_handler_inst_db_op(byte * inst);
#define F_INST_DB_OP2_H	_f_handler_inst_db_op

byte * _f_handler_inst_adc_0(byte * inst);
#define F_INST_ADC_02_H	_f_handler_inst_adc_0

byte * _f_handler_inst_adc_1(byte * inst);
#define F_INST_ADC_12_H	_f_handler_inst_adc_1

byte * _f_handler_inst_adc_2(byte * inst);
#define F_INST_ADC_22_H	_f_handler_inst_adc_2

byte * _f_handler_inst_adc_3(byte * inst);
#define F_INST_ADC_32_H	_f_handler_inst_adc_3

byte * _f_handler_inst_adc_4(byte * inst);
#define F_INST_ADC_42_H	_f_handler_inst_adc_4

byte * _f_handler_inst_adc_5(byte * inst);
#define F_INST_ADC_52_H	_f_handler_inst_adc_5

byte * _f_handler_inst_push_ss(byte * inst);
#define F_INST_PUSH_SS2_H	_f_handler_inst_push_ss

byte * _f_handler_inst_pop_ss(byte * inst);
#define F_INST_POP_SS2_H	_f_handler_inst_pop_ss

byte * _f_handler_inst_sbb_0(byte * inst);
#define F_INST_SBB_02_H	_f_handler_inst_sbb_0

byte * _f_handler_inst_sbb_1(byte * inst);
#define F_INST_SBB_12_H	_f_handler_inst_sbb_1

byte * _f_handler_inst_sbb_2(byte * inst);
#define F_INST_SBB_22_H	_f_handler_inst_sbb_2

byte * _f_handler_inst_sbb_3(byte * inst);
#define F_INST_SBB_32_H	_f_handler_inst_sbb_3

byte * _f_handler_inst_sbb_4(byte * inst);
#define F_INST_SBB_42_H	_f_handler_inst_sbb_4

byte * _f_handler_inst_sbb_5(byte * inst);
#define F_INST_SBB_52_H	_f_handler_inst_sbb_5

byte * _f_handler_inst_push_ds(byte * inst);
#define F_INST_PUSH_DS2_H	_f_handler_inst_push_ds

byte * _f_handler_inst_pop_ds(byte * inst);
#define F_INST_POP_DS2_H	_f_handler_inst_pop_ds

byte * _f_handler_inst_and_0(byte * inst);
#define F_INST_AND_02_H	_f_handler_inst_and_0

byte * _f_handler_inst_and_1(byte * inst);
#define F_INST_AND_12_H	_f_handler_inst_and_1

byte * _f_handler_inst_and_2(byte * inst);
#define F_INST_AND_22_H	_f_handler_inst_and_2

byte * _f_handler_inst_and_3(byte * inst);
#define F_INST_AND_32_H	_f_handler_inst_and_3

byte * _f_handler_inst_and_4(byte * inst);
#define F_INST_AND_42_H	_f_handler_inst_and_4

byte * _f_handler_inst_and_5(byte * inst);
#define F_INST_AND_52_H	_f_handler_inst_and_5

byte * _f_handler_inst_es(byte * inst);
#define F_INST_ES2_H	_f_handler_inst_es

byte * _f_handler_inst_daa(byte * inst);
#define F_INST_DAA2_H	_f_handler_inst_daa

byte * _f_handler_inst_sub_0(byte * inst);
#define F_INST_SUB_02_H	_f_handler_inst_sub_0

byte * _f_handler_inst_sub_1(byte * inst);
#define F_INST_SUB_12_H	_f_handler_inst_sub_1

byte * _f_handler_inst_sub_2(byte * inst);
#define F_INST_SUB_22_H	_f_handler_inst_sub_2

byte * _f_handler_inst_sub_3(byte * inst);
#define F_INST_SUB_32_H	_f_handler_inst_sub_3

byte * _f_handler_inst_sub_4(byte * inst);
#define F_INST_SUB_42_H	_f_handler_inst_sub_4

byte * _f_handler_inst_sub_5(byte * inst);
#define F_INST_SUB_52_H	_f_handler_inst_sub_5

byte * _f_handler_inst_cs(byte * inst);
#define F_INST_CS2_H	_f_handler_inst_cs

byte * _f_handler_inst_das(byte * inst);
#define F_INST_DAS2_H	_f_handler_inst_das

byte * _f_handler_inst_ss(byte * inst);
#define F_INST_SS2_H	_f_handler_inst_ss

byte * _f_handler_inst_aaa(byte * inst);
#define F_INST_AAA2_H	_f_handler_inst_aaa

byte * _f_handler_inst_cmp_0(byte * inst);
#define F_INST_CMP_02_H	_f_handler_inst_cmp_0

byte * _f_handler_inst_cmp_1(byte * inst);
#define F_INST_CMP_12_H	_f_handler_inst_cmp_1

byte * _f_handler_inst_cmp_4(byte * inst);
#define F_INST_CMP_42_H	_f_handler_inst_cmp_4

byte * _f_handler_inst_cmp_5(byte * inst);
#define F_INST_CMP_52_H	_f_handler_inst_cmp_5

byte * _f_handler_inst_ds(byte * inst);
#define F_INST_DS2_H	_f_handler_inst_ds

byte * _f_handler_inst_aas(byte * inst);
#define F_INST_AAS2_H	_f_handler_inst_aas

byte * _f_handler_inst_pushad(byte * inst);
#define F_INST_PUSHAD2_H	_f_handler_inst_pushad

byte * _f_handler_inst_popad(byte * inst);
#define F_INST_POPAD2_H	_f_handler_inst_popad

byte * _f_handler_inst_bound(byte * inst);
#define F_INST_BOUND2_H	_f_handler_inst_bound

byte * _f_handler_inst_arpl(byte * inst);
#define F_INST_ARPL2_H	_f_handler_inst_arpl

byte * _f_handler_inst_fs(byte * inst);
#define F_INST_FS2_H	_f_handler_inst_fs

byte * _f_handler_inst_gs(byte * inst);
#define F_INST_GS2_H	_f_handler_inst_gs

byte * _f_handler_inst_preover(byte * inst);
#define F_INST_PREOVER2_H	_f_handler_inst_preover

byte * _f_handler_inst_addrover(byte * inst);
#define F_INST_ADDROVER2_H	_f_handler_inst_addrover

byte * _f_handler_inst_push32(byte * inst);
#define F_INST_PUSH322_H	_f_handler_inst_push32

byte * _f_handler_inst_imul(byte * inst);
#define F_INST_IMUL2_H	_f_handler_inst_imul

byte * _f_handler_inst_imul2(byte * inst);
#define F_INST_IMUL22_H	_f_handler_inst_imul2

byte * _f_handler_inst_insb(byte * inst);
#define F_INST_INSB2_H	_f_handler_inst_insb

byte * _f_handler_inst_insd(byte * inst);
#define F_INST_INSD2_H	_f_handler_inst_insd

byte * _f_handler_inst_insw(byte * inst);
#define F_INST_INSW2_H	_f_handler_inst_insw

byte * _f_handler_inst_outb(byte * inst);
#define F_INST_OUTB2_H	_f_handler_inst_outb

byte * _f_handler_inst_outw(byte * inst);
#define F_INST_OUTW2_H	_f_handler_inst_outw

byte * _f_handler_inst_outd(byte * inst);
#define F_INST_OUTD2_H	_f_handler_inst_outd

byte * _f_handler_inst_gen_8bits(byte * inst);
#define F_INST_GEN_8BITS2_H	_f_handler_inst_gen_8bits

byte * _f_handler_inst_gen_8bits2(byte * inst);
#define F_INST_GEN_8BITS22_H	_f_handler_inst_gen_8bits2

byte * _f_handler_inst_xchg(byte * inst);
#define F_INST_XCHG2_H	_f_handler_inst_xchg

byte * _f_handler_inst_xchg2(byte * inst);
#define F_INST_XCHG22_H	_f_handler_inst_xchg2

byte * _f_handler_inst_mov3(byte * inst);
#define F_INST_MOV32_H	_f_handler_inst_mov3

byte * _f_handler_inst_mov5(byte * inst);
#define F_INST_MOV52_H	_f_handler_inst_mov5

byte * _f_handler_inst_mov6(byte * inst);
#define F_INST_MOV62_H	_f_handler_inst_mov6

byte * _f_handler_inst_pop32(byte * inst);
#define F_INST_POP322_H	_f_handler_inst_pop32

byte * _f_handler_inst_pause(byte * inst);
#define F_INST_PAUSE2_H	_f_handler_inst_pause

byte * _f_handler_inst_cwde(byte * inst);
#define F_INST_CWDE2_H	_f_handler_inst_cwde

byte * _f_handler_inst_cdq(byte * inst);
#define F_INST_CDQ2_H	_f_handler_inst_cdq

byte * _f_handler_inst_callf(byte * inst);
#define F_INST_CALLF2_H	_f_handler_inst_callf

byte * _f_handler_inst_wait(byte * inst);
#define F_INST_WAIT2_H	_f_handler_inst_wait

byte * _f_handler_inst_pushfd(byte * inst);
#define F_INST_PUSHFD2_H	_f_handler_inst_pushfd

byte * _f_handler_inst_popfd(byte * inst);
#define F_INST_POPFD2_H	_f_handler_inst_popfd

byte * _f_handler_inst_sahf(byte * inst);
#define F_INST_SAHF2_H	_f_handler_inst_sahf

byte * _f_handler_inst_lahf(byte * inst);
#define F_INST_LAHF2_H	_f_handler_inst_lahf

byte * _f_handler_inst_mov7(byte * inst);
#define F_INST_MOV72_H	_f_handler_inst_mov7

byte * _f_handler_inst_mov9(byte * inst);
#define F_INST_MOV92_H	_f_handler_inst_mov9

byte * _f_handler_inst_movsb(byte * inst);
#define F_INST_MOVSB2_H	_f_handler_inst_movsb

byte * _f_handler_inst_movsd(byte * inst);
#define F_INST_MOVSD2_H	_f_handler_inst_movsd

byte * _f_handler_inst_cmpsb(byte * inst);
#define F_INST_CMPSB2_H	_f_handler_inst_cmpsb

byte * _f_handler_inst_cmpsd(byte * inst);
#define F_INST_CMPSD2_H	_f_handler_inst_cmpsd

byte * _f_handler_inst_test3(byte * inst);
#define F_INST_TEST32_H	_f_handler_inst_test3

byte * _f_handler_inst_test4(byte * inst);
#define F_INST_TEST42_H	_f_handler_inst_test4

byte * _f_handler_inst_test_5(byte * inst);
#define F_INST_TEST_5_H	_f_handler_inst_test_5


byte * _f_handler_inst_stosb(byte * inst);
#define F_INST_STOSB2_H	_f_handler_inst_stosb

byte * _f_handler_inst_stosd(byte * inst);
#define F_INST_STOSD2_H	_f_handler_inst_stosd

byte * _f_handler_inst_lodsb(byte * inst);
#define F_INST_LODSB2_H	_f_handler_inst_lodsb

byte * _f_handler_inst_lodsd(byte * inst);
#define F_INST_LODSD2_H	_f_handler_inst_lodsd

byte * _f_handler_inst_scasb(byte * inst);
#define F_INST_SCASB2_H	_f_handler_inst_scasb

byte * _f_handler_inst_scasd(byte * inst);
#define F_INST_SCASD2_H	_f_handler_inst_scasd

byte * _f_handler_inst_gen_mov32(byte * inst);
#define F_INST_GEN_MOV322_H	_f_handler_inst_gen_mov32

byte * _f_handler_inst_shl32(byte * inst);
#define F_INST_SHL322_H	_f_handler_inst_shl32

byte * _f_handler_inst_retn2(byte * inst);
#define F_INST_RETN22_H	_f_handler_inst_retn2

byte * _f_handler_inst_les(byte * inst);
#define F_INST_LES2_H	_f_handler_inst_les

byte * _f_handler_inst_lds(byte * inst);
#define F_INST_LDS2_H	_f_handler_inst_lds

byte * _f_handler_inst_mov11(byte * inst);
#define F_INST_MOV112_H	_f_handler_inst_mov11

byte * _f_handler_inst_enter(byte * inst);
#define F_INST_ENTER2_H	_f_handler_inst_enter

byte * _f_handler_inst_fxch_opt(byte * inst);
#define F_INST_FXCH_OPT2_H	_f_handler_inst_fxch_opt

byte * _f_handler_inst_leave(byte * inst);
#define F_INST_LEAVE2_H	_f_handler_inst_leave

byte * _f_handler_inst_retf(byte * inst);
#define F_INST_RETF2_H	_f_handler_inst_retf

byte * _f_handler_inst_retf2(byte * inst);
#define F_INST_RETF22_H	_f_handler_inst_retf2

byte * _f_handler_inst_int(byte * inst);
#define F_INST_INT2_H	_f_handler_inst_int

byte * _f_handler_inst_int2(byte * inst);
#define F_INST_INT22_H	_f_handler_inst_int2

byte * _f_handler_inst_int0(byte * inst);
#define F_INST_INT02_H	_f_handler_inst_int0

byte * _f_handler_inst_iretd(byte * inst);
#define F_INST_IRETD2_H	_f_handler_inst_iretd

byte * _f_handler_inst_shl2(byte * inst);
#define F_INST_SHL22_H	_f_handler_inst_shl2

byte * _f_handler_inst_fnop_sec(byte * inst);
#define F_INST_FNOP_SEC2_H	_f_handler_inst_fnop_sec

byte * _f_handler_inst_shl322(byte * inst);
#define F_INST_SHL3222_H	_f_handler_inst_shl322

byte * _f_handler_inst_shl323(byte * inst);
#define F_INST_SHL3232_H	_f_handler_inst_shl323

byte * _f_handler_inst_amx(byte * inst);
#define F_INST_AMX2_H	_f_handler_inst_amx

byte * _f_handler_inst_adx(byte * inst);
#define F_INST_ADX2_H	_f_handler_inst_adx

byte * _f_handler_inst_setalc(byte * inst);
#define F_INST_SETALC2_H	_f_handler_inst_setalc

byte * _f_handler_inst_xlatb(byte * inst);
#define F_INST_XLATB2_H	_f_handler_inst_xlatb

byte * _f_handler_inst_fpu1(byte * inst);
#define F_INST_FPU12_H	_f_handler_inst_fpu1

byte * _f_handler_inst_fpu2(byte * inst);
#define F_INST_FPU22_H	_f_handler_inst_fpu2

byte * _f_handler_inst_fchs(byte * inst);
#define F_INST_FCHS2_H	_f_handler_inst_fchs

byte * _f_handler_inst_fabs(byte * inst);
#define F_INST_FABS2_H	_f_handler_inst_fabs

byte * _f_handler_inst_ftst(byte * inst);
#define F_INST_FTST2_H	_f_handler_inst_ftst

byte * _f_handler_inst_fxam(byte * inst);
#define F_INST_FXAM2_H	_f_handler_inst_fxam

byte * _f_handler_inst_fld1(byte * inst);
#define F_INST_FLD12_H	_f_handler_inst_fld1

byte * _f_handler_inst_fldl2t(byte * inst);
#define F_INST_FLDL2T2_H	_f_handler_inst_fldl2t

byte * _f_handler_inst_fldl2e(byte * inst);
#define F_INST_FLDL2E2_H	_f_handler_inst_fldl2e

byte * _f_handler_inst_fldpi(byte * inst);
#define F_INST_FLDPI2_H	_f_handler_inst_fldpi

byte * _f_handler_inst_fldlg2(byte * inst);
#define F_INST_FLDLG22_H	_f_handler_inst_fldlg2

byte * _f_handler_inst_fldln2(byte * inst);
#define F_INST_FLDLN22_H	_f_handler_inst_fldln2

byte * _f_handler_inst_fldz(byte * inst);
#define F_INST_FLDZ2_H	_f_handler_inst_fldz

byte * _f_handler_inst_f2xm1(byte * inst);
#define F_INST_F2XM12_H	_f_handler_inst_f2xm1

byte * _f_handler_inst_fyl2x(byte * inst);
#define F_INST_FYL2X2_H	_f_handler_inst_fyl2x

byte * _f_handler_inst_fptan(byte * inst);
#define F_INST_FPTAN2_H	_f_handler_inst_fptan

byte * _f_handler_inst_fpatan(byte * inst);
#define F_INST_FPATAN2_H	_f_handler_inst_fpatan

byte * _f_handler_inst_fxtract(byte * inst);
#define F_INST_FXTRACT2_H	_f_handler_inst_fxtract

byte * _f_handler_inst_fprem1(byte * inst);
#define F_INST_FPREM12_H	_f_handler_inst_fprem1

byte * _f_handler_inst_fdecstp(byte * inst);
#define F_INST_FDECSTP2_H	_f_handler_inst_fdecstp

byte * _f_handler_inst_fincstp(byte * inst);
#define F_INST_FINCSTP2_H	_f_handler_inst_fincstp

byte * _f_handler_inst_fprem(byte * inst);
#define F_INST_FPREM2_H	_f_handler_inst_fprem

byte * _f_handler_inst_fyl2xp1(byte * inst);
#define F_INST_FYL2XP12_H	_f_handler_inst_fyl2xp1

byte * _f_handler_inst_fsqrt(byte * inst);
#define F_INST_FSQRT2_H	_f_handler_inst_fsqrt

byte * _f_handler_inst_fsincos(byte * inst);
#define F_INST_FSINCOS2_H	_f_handler_inst_fsincos

byte * _f_handler_inst_frndint(byte * inst);
#define F_INST_FRNDINT2_H	_f_handler_inst_frndint

byte * _f_handler_inst_fscale(byte * inst);
#define F_INST_FSCALE2_H	_f_handler_inst_fscale

byte * _f_handler_inst_fsin(byte * inst);
#define F_INST_FSIN2_H	_f_handler_inst_fsin

byte * _f_handler_inst_fcos(byte * inst);
#define F_INST_FCOS2_H	_f_handler_inst_fcos




#endif