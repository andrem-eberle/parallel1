#ifndef _A_ENGINE_H_
#define _A_ENGINE_H_

#include "parallel.h"
#include "instset.h"
#include "memory.h"

extern processor_state * p_state;
extern register_ref * r_write;
extern program_stack * p_stack;

/* Instruction handlers */
byte *  _handler_inst_push(byte * inst);
#define INST_PUSH_H			_handler_inst_push

byte * _handler_inst_push_imm2(byte * inst);
#define INST_PUSH_IMM2_H	_handler_inst_push_imm2

byte * _handler_inst_pop(byte * inst);
#define INST_POP_H			_handler_inst_pop

byte * _handler_inst_xor_0(byte * inst);
#define INST_XOR_0_H		_handler_inst_xor_0

byte * _handler_inst_xor_1(byte * inst);
#define INST_XOR_1_H		_handler_inst_xor_1

byte * _handler_inst_xor_2(byte * inst);
#define INST_XOR_2_H		_handler_inst_xor_2

byte * _handler_inst_xor_3(byte * inst);
#define INST_XOR_3_H		_handler_inst_xor_3

byte * _handler_inst_xor_4(byte * inst);
#define INST_XOR_4_H		_handler_inst_xor_4

byte * _handler_inst_xor_5(byte * inst);
#define INST_XOR_5_H		_handler_inst_xor_5
	
byte * _handler_inst_inc(byte * inst);
#define INST_INC_H			_handler_inst_inc

byte * _handler_inst_dec(byte * inst);
#define INST_DEC_H			_handler_inst_dec

byte * _handler_inst_call(byte * inst);
#define INST_CALL_H			_handler_inst_call

byte * _handler_inst_mov2(byte * inst);
#define INST_MOV2_H			_handler_inst_mov2

byte * _handler_inst_mov4(byte * inst);
#define INST_MOV4_H			_handler_inst_mov4

byte * _handler_inst_mov8(byte * inst);
#define INST_MOV8_H			_handler_inst_mov8

byte * _handler_inst_mov10(byte * inst);
#define	INST_MOV10_H	_handler_inst_mov10

byte * _handler_inst_mov12(byte * inst);
#define	INST_MOV12_H	_handler_inst_mov12

byte * _handler_inst_gen_32bits2(byte * inst);
#define	INST_GEN_32BITS2_H	_handler_inst_gen_32bits2

byte * _handler_inst_gen_mov(byte * inst);
#define	INST_GEN_MOV_H	_handler_inst_gen_mov

byte * _handler_inst_jmp(byte * inst);
#define	INST_JMP_H	_handler_inst_jmp

byte * _handler_inst_cmp_1(byte * inst);
#define	INST_CMP_1_H	_handler_inst_cmp_1

byte * _handler_inst_cmp2(byte * inst);
#define	INST_CMP_2_H	_handler_inst_cmp2

byte * _handler_inst_cmp3(byte * inst);
#define INST_CMP_3_H	_handler_inst_cmp3

byte * _handler_inst_jo(byte * inst);
#define INST_JO_H		_handler_inst_jo

byte * _handler_inst_jno(byte * inst);
#define INST_JNO_H		_handler_inst_jno

byte * _handler_inst_jc(byte * inst);
#define INST_JC_H		_handler_inst_jc

byte * _handler_inst_jnc(byte * inst);
#define INST_JNC_H		_handler_inst_jnc

byte * _handler_inst_jz(byte * inst);
#define INST_JZ_H		_handler_inst_jz

byte * _handler_inst_jnz(byte * inst);
#define INST_JNZ_H		_handler_inst_jnz

byte * _handler_inst_jbe(byte * inst);
#define INST_JBE_H		_handler_inst_jbe

byte * _handler_inst_jnbe(byte * inst);
#define INST_JNBE_H		_handler_inst_jnbe

byte * _handler_inst_js(byte * inst);
#define INST_JS_H		_handler_inst_js

byte * _handler_inst_jns(byte * inst);
#define INST_JNS_H		_handler_inst_jns

byte * _handler_inst_jp(byte * inst);
#define INST_JP_H		_handler_inst_jp

byte * _handler_inst_jnp(byte * inst);
#define INST_JNP_H		_handler_inst_jnp

byte * _handler_inst_jl(byte * inst);
#define INST_JL_H		_handler_inst_jl

byte * _handler_inst_jnl(byte * inst);
#define INST_JNL_H		_handler_inst_jnl

byte * _handler_inst_jle(byte * inst);
#define INST_JLE_H		_handler_inst_jle

byte * _handler_inst_jnle(byte * inst);
#define INST_JNLE_H		_handler_inst_jnle

byte * _handler_inst_gen_1op(byte * inst);
#define INST_GEN_1OP_H	_handler_inst_gen_1op

byte * _handler_inst_retn(byte * inst);
#define INST_RETN_H		_handler_inst_retn

byte * _handler_inst_retn2(byte * inst);
#define INST_RETN2_H		_handler_inst_retn2

byte * _handler_inst_or_0(byte * inst);
#define INST_OR_0_H 	_handler_inst_or_0

byte * _handler_inst_or_1(byte * inst);
#define INST_OR_1_H 	_handler_inst_or_1

byte * _handler_inst_or_2(byte * inst);
#define INST_OR_2_H 	_handler_inst_or_2

byte * _handler_inst_or_3(byte * inst);
#define INST_OR_3_H 	_handler_inst_or_3

byte * _handler_inst_or_4(byte * inst);
#define INST_OR_4_H 	_handler_inst_or_4

byte * _handler_inst_or_5(byte * inst);
#define INST_OR_5_H 	_handler_inst_or_5

byte * _handler_inst_push_cs(byte * inst);
#define INST_PUSH_CS_H 	_handler_inst_push_cs

byte * _handler_inst_db_op(byte * inst);
#define INST_DB_OP_H 	_handler_inst_db_op

byte * _handler_inst_adc_0(byte * inst);
#define INST_ADC_0_H 	_handler_inst_adc_0

byte * _handler_inst_adc_1(byte * inst);
#define INST_ADC_1_H 	_handler_inst_adc_1

byte * _handler_inst_adc_2(byte * inst);
#define INST_ADC_2_H 	_handler_inst_adc_2

byte * _handler_inst_adc_3(byte * inst);
#define INST_ADC_3_H 	_handler_inst_adc_3

byte * _handler_inst_adc_4(byte * inst);
#define INST_ADC_4_H 	_handler_inst_adc_4

byte * _handler_inst_adc_5(byte * inst);
#define INST_ADC_5_H 	_handler_inst_adc_5

byte * _handler_inst_push_ss(byte * inst);
#define INST_PUSH_SS_H 	_handler_inst_push_ss

byte * _handler_inst_pop_ss(byte * inst);
#define INST_POP_SS_H 	_handler_inst_pop_ss

byte * _handler_inst_sbb_0(byte * inst);
#define INST_SBB_0_H 	_handler_inst_sbb_0

byte * _handler_inst_sbb_1(byte * inst);
#define INST_SBB_1_H 	_handler_inst_sbb_1

byte * _handler_inst_sbb_2(byte * inst);
#define INST_SBB_2_H 	_handler_inst_sbb_2

byte * _handler_inst_sbb_3(byte * inst);
#define INST_SBB_3_H 	_handler_inst_sbb_3

byte * _handler_inst_sbb_4(byte * inst);
#define INST_SBB_4_H 	_handler_inst_sbb_4

byte * _handler_inst_sbb_5(byte * inst);
#define INST_SBB_5_H 	_handler_inst_sbb_5

byte * _handler_inst_push_ds(byte * inst);
#define INST_PUSH_DS_H 	_handler_inst_push_ds

byte * _handler_inst_pop_ds(byte * inst);
#define INST_POP_DS_H 	_handler_inst_pop_ds

byte * _handler_inst_and_0(byte * inst);
#define INST_AND_0_H 	_handler_inst_and_0

byte * _handler_inst_and_1(byte * inst);
#define INST_AND_1_H 	_handler_inst_and_1

byte * _handler_inst_and_2(byte * inst);
#define INST_AND_2_H 	_handler_inst_and_2

byte * _handler_inst_and_3(byte * inst);
#define INST_AND_3_H 	_handler_inst_and_3

byte * _handler_inst_and_4(byte * inst);
#define INST_AND_4_H 	_handler_inst_and_4

byte * _handler_inst_and_5(byte * inst);
#define INST_AND_5_H 	_handler_inst_and_5

byte * _handler_inst_es(byte * inst);
#define INST_ES_H 	_handler_inst_es

byte * _handler_inst_daa(byte * inst);
#define INST_DAA_H 	_handler_inst_daa

byte * _handler_inst_sub_0(byte * inst);
#define INST_SUB_0_H 	_handler_inst_sub_0

byte * _handler_inst_sub_1(byte * inst);
#define INST_SUB_1_H 	_handler_inst_sub_1

byte * _handler_inst_sub_2(byte * inst);
#define INST_SUB_2_H 	_handler_inst_sub_2

byte * _handler_inst_sub_3(byte * inst);
#define INST_SUB_3_H 	_handler_inst_sub_3

byte * _handler_inst_sub_4(byte * inst);
#define INST_SUB_4_H 	_handler_inst_sub_4

byte * _handler_inst_sub_5(byte * inst);
#define INST_SUB_5_H 	_handler_inst_sub_5

byte * _handler_inst_cs(byte * inst);
#define INST_CS_H 	_handler_inst_cs

byte * _handler_inst_das(byte * inst);
#define INST_DAS_H 	_handler_inst_das

byte * _handler_inst_ss(byte * inst);
#define INST_SS_H 	_handler_inst_ss

byte * _handler_inst_aaa(byte * inst);
#define INST_AAA_H 	_handler_inst_aaa

byte * _handler_inst_cmp_0(byte * inst);
#define INST_CMP_0_H 	_handler_inst_cmp_0

byte * _handler_inst_cmp_4(byte * inst);
#define INST_CMP_4_H 	_handler_inst_cmp_4

byte * _handler_inst_cmp_5(byte * inst);
#define INST_CMP_5_H 	_handler_inst_cmp_5

byte * _handler_inst_ds(byte * inst);
#define INST_DS_H 	_handler_inst_ds

byte * _handler_inst_aas(byte * inst);
#define INST_AAS_H 	_handler_inst_aas

byte * _handler_inst_pushad(byte * inst);
#define INST_PUSHAD_H 	_handler_inst_pushad

byte * _handler_inst_popad(byte * inst);
#define INST_POPAD_H 	_handler_inst_popad

byte * _handler_inst_bound(byte * inst);
#define INST_BOUND_H 	_handler_inst_bound

byte * _handler_inst_arpl(byte * inst);
#define INST_ARPL_H 	_handler_inst_arpl

byte * _handler_inst_fs(byte * inst);
#define INST_FS_H 	_handler_inst_fs

byte * _handler_inst_gs(byte * inst);
#define INST_GS_H 	_handler_inst_gs

byte * _handler_inst_preover(byte * inst);
#define INST_PREOVER_H 	_handler_inst_preover

byte * _handler_inst_addrover(byte * inst);
#define INST_ADDROVER_H 	_handler_inst_addrover

byte * _handler_inst_push32(byte * inst);
#define INST_PUSH32_H 	_handler_inst_push32

byte * _handler_inst_imul(byte * inst);
#define INST_IMUL_H 	_handler_inst_imul

byte * _handler_inst_imul2(byte * inst);
#define INST_IMUL2_H 	_handler_inst_imul2

byte * _handler_inst_insb(byte * inst);
#define INST_INSB_H 	_handler_inst_insb

byte * _handler_inst_insw(byte * inst);
#define INST_INSW_H 	_handler_inst_insw

byte * _handler_inst_insd(byte * inst);
#define INST_INSD_H 	_handler_inst_insd

byte * _handler_inst_outb(byte * inst);
#define INST_OUTB_H 	_handler_inst_outb

byte * _handler_inst_outw(byte * inst);
#define INST_OUTW_H 	_handler_inst_outw

byte * _handler_inst_outd(byte * inst);
#define INST_OUTD_H 	_handler_inst_outd

byte * _handler_inst_gen_8bits(byte * inst);
#define INST_GEN_8BITS_H 	_handler_inst_gen_8bits

byte * _handler_inst_gen_8bits2(byte * inst);
#define INST_GEN_8BITS2_H 	_handler_inst_gen_8bits2

byte * _handler_inst_gen_32bits2(byte * inst);
#define INST_GEN_32BITS2_H 	_handler_inst_gen_32bits2

byte * _handler_inst_test(byte * inst);
#define INST_TEST_H 	_handler_inst_test

byte * _handler_inst_test2(byte * inst);
#define INST_TEST2_H 	_handler_inst_test2

byte * _handler_inst_xchg(byte * inst);
#define INST_XCHG_H 	_handler_inst_xchg

byte * _handler_inst_xchg2(byte * inst);
#define INST_XCHG2_H 	_handler_inst_xchg2

byte * _handler_inst_mov2(byte * inst);
#define INST_MOV2_H 	_handler_inst_mov2

byte * _handler_inst_mov3(byte * inst);
#define INST_MOV3_H 	_handler_inst_mov3

byte * _handler_inst_mov5(byte * inst);
#define INST_MOV5_H 	_handler_inst_mov5

byte * _handler_inst_lea(byte * inst);
#define INST_LEA_H 	_handler_inst_lea

byte * _handler_inst_mov6(byte * inst);
#define INST_MOV6_H 	_handler_inst_mov6

byte * _handler_inst_pop32(byte * inst);
#define INST_POP32_H 	_handler_inst_pop32

byte * _handler_inst_pause(byte * inst);
#define INST_PAUSE_H 	_handler_inst_pause

byte * _handler_inst_cwde(byte * inst);
#define INST_CWDE_H 	_handler_inst_cwde

byte * _handler_inst_cdq(byte * inst);
#define INST_CDQ_H 	_handler_inst_cdq

byte * _handler_inst_callf(byte * inst);
#define INST_CALLF_H 	_handler_inst_callf

byte * _handler_inst_wait(byte * inst);
#define INST_WAIT_H 	_handler_inst_wait

byte * _handler_inst_pushfd(byte * inst);
#define INST_PUSHFD_H 	_handler_inst_pushfd

byte * _handler_inst_popfd(byte * inst);
#define INST_POPFD_H 	_handler_inst_popfd

byte * _handler_inst_sahf(byte * inst);
#define INST_SAHF_H 	_handler_inst_sahf

byte * _handler_inst_lahf(byte * inst);
#define INST_LAHF_H 	_handler_inst_lahf

byte * _handler_inst_mov7(byte * inst);
#define INST_MOV7_H 	_handler_inst_mov7

byte * _handler_inst_mov9(byte * inst);
#define INST_MOV9_H 	_handler_inst_mov9

byte * _handler_inst_movsb(byte * inst);
#define INST_MOVSB_H 	_handler_inst_movsb

byte * _handler_inst_movsd(byte * inst);
#define INST_MOVSD_H 	_handler_inst_movsd

byte * _handler_inst_cmpsb(byte * inst);
#define INST_CMPSB_H 	_handler_inst_cmpsb

byte * _handler_inst_cmpsd(byte * inst);
#define INST_CMPSD_H 	_handler_inst_cmpsd

byte * _handler_inst_test3(byte * inst);
#define INST_TEST3_H 	_handler_inst_test3

byte * _handler_inst_test4(byte * inst);
#define INST_TEST4_H 	_handler_inst_test4

byte * _handler_inst_stosb(byte * inst);
#define INST_STOSB_H 	_handler_inst_stosb

byte * _handler_inst_stosd(byte * inst);
#define INST_STOSD_H 	_handler_inst_stosd

byte * _handler_inst_lodsb(byte * inst);
#define INST_LODSB_H 	_handler_inst_lodsb

byte * _handler_inst_lodsd(byte * inst);
#define INST_LODSD_H 	_handler_inst_lodsd

byte * _handler_inst_scasb(byte * inst);
#define INST_SCASB_H 	_handler_inst_scasb

byte * _handler_inst_scasd(byte * inst);
#define INST_SCASD_H 	_handler_inst_scasd

byte * _handler_inst_gen_mov32(byte * inst);
#define INST_GEN_MOV32_H 	_handler_inst_gen_mov32

byte * _handler_inst_shl(byte * inst);
#define INST_SHL_H 	_handler_inst_shl

byte * _handler_inst_shl32(byte * inst);
#define INST_SHL32_H 	_handler_inst_shl32

byte * _handler_inst_les(byte * inst);
#define INST_LES_H 	_handler_inst_les

byte * _handler_inst_lds(byte * inst);
#define INST_LDS_H 	_handler_inst_lds

byte * _handler_inst_mov11(byte * inst);
#define INST_MOV11_H 	_handler_inst_mov11

byte * _handler_inst_enter(byte * inst);
#define INST_ENTER_H 	_handler_inst_enter

byte * _handler_inst_leave(byte * inst);
#define INST_LEAVE_H 	_handler_inst_leave

byte * _handler_inst_retf(byte * inst);
#define INST_RETF_H 	_handler_inst_retf

byte * _handler_inst_retf2(byte * inst);
#define INST_RETF2_H 	_handler_inst_retf2

byte * _handler_inst_int(byte * inst);
#define INST_INT_H 	_handler_inst_int

byte * _handler_inst_int2(byte * inst);
#define INST_INT2_H 	_handler_inst_int2

byte * _handler_inst_int0(byte * inst);
#define INST_INT0_H 	_handler_inst_int0

byte * _handler_inst_iretd(byte * inst);
#define INST_IRETD_H 	_handler_inst_iretd

byte * _handler_inst_shl2(byte * inst);
#define INST_SHL2_H 	_handler_inst_shl2

byte * _handler_inst_shl322(byte * inst);
#define INST_SHL322_H 	_handler_inst_shl322

byte * _handler_inst_shl3(byte * inst);
#define INST_SHL3_H 	_handler_inst_shl3

byte * _handler_inst_shl323(byte * inst);
#define INST_SHL323_H 	_handler_inst_shl323

byte * _handler_inst_amx(byte * inst);
#define INST_AMX_H 	_handler_inst_amx

byte * _handler_inst_adx(byte * inst);
#define INST_ADX_H 	_handler_inst_adx

byte * _handler_inst_setalc(byte * inst);
#define INST_SETALC_H 	_handler_inst_setalc

byte * _handler_inst_xlatb(byte * inst);
#define INST_XLATB_H 	_handler_inst_xlatb

byte * _handler_inst_fpu1(byte * inst);
#define INST_FPU1_H 	_handler_inst_fpu1

byte * _handler_inst_fpu2(byte * inst);
#define INST_FPU2_H 	_handler_inst_fpu2

byte * _handler_inst_jmp2(byte * inst);
#define INST_JMP2_H 	_handler_inst_jmp2

#endif

