#ifndef _INSTSET_H_
#define _INSTSET_H_

#define INST_ADD_0		0x00
#define INST_ADD_1		0x01
#define INST_ADD_2		0x02
#define INST_ADD_3		0x03
#define INST_ADD_4		0x04
#define INST_ADD_5		0x05

#define INST_PUSH_ES	0x06
#define INST_POP_ES		0x07

#define INST_OR_0		0x08
#define INST_OR_1		0x09
#define INST_OR_2		0x0A
#define INST_OR_3		0x0B
#define INST_OR_4		0x0C
#define INST_OR_5		0x0D

#define INST_PUSH_CS	0x0E

/* double op instructions, see file end */
#define INST_DB_OP		0x0F

#define INST_ADC_0		0x10
#define INST_ADC_1		0x11
#define INST_ADC_2		0x12
#define INST_ADC_3		0x13
#define INST_ADC_4		0x14
#define INST_ADC_5		0x15

#define INST_PUSH_SS	0x16
#define INST_POP_SS		0x17

#define INST_SBB_0		0x18
#define INST_SBB_1		0x19
#define INST_SBB_2		0x1A
#define INST_SBB_3		0x1B
#define INST_SBB_4		0x1C
#define INST_SBB_5		0x1D

#define INST_PUSH_DS	0x1E
#define INST_POP_DS		0x1F

#define INST_AND_0		0x20
#define INST_AND_1		0x21
#define INST_AND_2		0x22
#define INST_AND_3		0x23
#define INST_AND_4		0x24
#define INST_AND_5		0x25

#define INST_ES			0x26
#define INST_DAA		0x27

#define INST_SUB_0		0x28
#define INST_SUB_1		0x29
#define INST_SUB_2		0x2A
#define INST_SUB_3		0x2B
#define INST_SUB_4		0x2C
#define INST_SUB_5		0x2D

#define INST_CS			0x2E
#define INST_DAS		0x2F

#define INST_XOR_0		0x30
#define INST_XOR_1		0x31
#define INST_XOR_2		0x32
#define INST_XOR_3		0x33
#define INST_XOR_4		0x34
#define INST_XOR_5		0x35

#define INST_SS			0x36
#define INST_AAA		0x37

#define INST_CMP_0		0x38
#define INST_CMP_1		0x39
#define INST_CMP_2		0x3A
#define INST_CMP_3		0x3B
#define INST_CMP_4		0x3C
#define INST_CMP_5		0x3D

#define INST_DS			0x3E
#define INST_AAS		0x3F

#define INST_INC		0x40
#define INST_DEC		0x48
#define INST_PUSH		0x50
#define INST_POP		0x58

#define GEN_REG_EAX		0x00
#define GEN_REG_ECX		0x01
#define GEN_REG_EDX		0x02
#define GEN_REG_EBX		0x03
#define GEN_REG_ESP		0x04
#define GEN_REG_EBP		0x05
#define GEN_REG_ESI		0x06
#define GEN_REG_EDI		0x07

#define SEG_REG_CS		0x08
#define SEG_REG_SS		0x09
#define SEG_REG_DS		0x0A
//#define SEG_REG_SS		0x0B


#define EAX		GEN_REG_EAX
#define ECX		GEN_REG_ECX
#define EDX		GEN_REG_EDX
#define EBX		GEN_REG_EBX
#define ESP		GEN_REG_ESP
#define EBP		GEN_REG_EBP
#define ESI		GEN_REG_ESI
#define EDI		GEN_REG_EDI

#define INST_PUSHAD		0x60
#define INST_POPAD		0x61
#define INST_BOUND		0x62
#define INST_ARPL		0x63

#define INST_FS			0x64
#define INST_GS			0x65
#define INST_PREOVER	0x66
#define INST_ADDROVER	0x67

#define INST_PUSH32		0x68
#define INST_IMUL		0x69
#define INST_PUSH_IMM2	0x6A
#define INST_IMUL2		0x6B

#define INST_INSB		0x6C
#define INST_INSW		0x6D
#define INST_INSD		0x6D

#define INST_OUTB		0x6E
#define INST_OUTW		0x6F
#define INST_OUTD		0x6F

#define INST_JO			0x70
#define INST_JNO		0x71
#define INST_JC			0x72
#define INST_JNC		0x73
#define INST_JZ			0x74
#define INST_JNZ		0x75
#define INST_JBE		0x76
#define INST_JNBE		0x77
#define INST_JS			0x78
#define INST_JNS		0x79
#define INST_JP			0x7A
#define INST_JNP		0x7B
#define INST_JL			0x7C
#define INST_JNL		0x7D
#define INST_JLE		0x7E
#define INST_JNLE		0x7F

#define INST_GEN_8BITS		0x80
#define INST_GEN_32BITS		0x81
#define INST_GEN_8BITS2		0x82
#define INST_GEN_32BITS2	0x83

#define	INST_GEN_ADD	0x00
#define	INST_GEN_OR		0x01
#define	INST_GEN_ADC	0x02
#define	INST_GEN_SBB	0x03
#define	INST_GEN_AND	0x04
#define	INST_GEN_SUB	0x05
#define	INST_GEN_XOR	0x06
#define	INST_GEN_CMP	0x07

#define	INST_TEST		0x84
#define	INST_TEST2		0x85

#define	INST_XCHG		0x86
#define	INST_XCHG2		0x87

#define	INST_MOV		0x88
#define	INST_MOV2		0x89
#define	INST_MOV3		0x8A
#define	INST_MOV4		0x8B
#define	INST_MOV5		0x8C
#define	INST_LEA		0x8D
#define	INST_MOV6		0x8E

#define	INST_POP32		0x8F

#define	INST_PAUSE		0x90

#define	INST_CWDE		0x98
#define	INST_CDQ		0x99

#define	INST_CALLF		0x9A

#define	INST_WAIT		0x9B

#define	INST_PUSHFD		0x9C
#define	INST_POPFD		0x9D

#define	INST_SAHF		0x9E
#define	INST_LAHF		0x9F

#define	INST_MOV7		0xA0
#define	INST_MOV8		0xA1
#define	INST_MOV9		0xA2
#define	INST_MOV10		0xA3

#define	INST_MOVSB		0xA4
#define	INST_MOVSD		0xA5

#define INST_CMPSB		0xA6
#define INST_CMPSD		0xA7

#define INST_TEST3		0xA8
#define INST_TEST4		0xA9

#define INST_STOSB		0xAA
#define INST_STOSD		0xAB

#define INST_LODSB		0xAC
#define INST_LODSD		0xAD

#define INST_SCASB		0xAE
#define INST_SCASD		0xAF

#define INST_GEN_MOV	0xB0
#define INST_GEN_MOV32	0xB8

#define INST_SHL		0xC0
#define INST_SHL32		0xC1

#define INST_RETN		0xC2
#define INST_RETN2		0xC3

#define INST_LES		0xC4
#define INST_LDS		0xC5

#define INST_MOV11		0xC6
#define INST_MOV12		0xC7

#define INST_ENTER		0xC8
#define INST_LEAVE		0xC9

#define INST_RETF		0xCA
#define INST_RETF2		0xCB

#define INST_INT		0xCC
#define INST_INT2		0xCD
#define INST_INT0		0xCE

#define INST_IRETD		0xCF

#define INST_SHL2		0xD0
#define INST_SHL322		0xD1
#define INST_SHL3		0xD2
#define INST_SHL323		0xD3

#define INST_AMX		0xD4
#define INST_ADX		0xD5

#define INST_SETALC		0xD6

#define INST_XLATB		0xD7

#define INST_FPU1		0xD8

#define	INST_FADD		0x00
#define	INST_FMUL		0x01
#define	INST_FCOM		0x02
#define	INST_FCOMP		0x03
#define	INST_FSUB		0x04
#define	INST_FSUBR		0x05
#define	INST_FDIV		0x06
#define	INST_FDIVR		0x07

#define INST_FPU2		0xD9

#define	INST_FLD		0x00
#define	INST_FXCH		0x00
#define	INST_FXCH_OPT	0xC9

#define	INST_FST		0x02

#define	INST_FNOP		0x02
#define	INST_FNOP_SEC	0xD0

#define	INST_FSTP1		0x03

#define	INST_FLDENV		0x04

#define	INST_FPU2_SUB4	0x04
#define INST_FCHS		0xE0
#define INST_FABS		0xE1
#define INST_FTST		0xE4
#define INST_FXAM		0xE5

#define	INST_FLDCW		0x05

#define	INST_FPU2_SUB5	0x05
#define INST_FLD1		0xE8
#define INST_FLDL2T		0xE9
#define INST_FLDL2E		0xEA
#define INST_FLDPI		0xEB
#define INST_FLDLG2		0xEC
#define INST_FLDLN2		0xED
#define INST_FLDZ		0xEE

#define	INST_FNSTENV	0x06

#define	INST_FPU2_SUB6	0x06
#define INST_F2XM1		0xF0
#define INST_FYL2X		0xF1
#define INST_FPTAN		0xF2
#define INST_FPATAN		0xF3
#define INST_FXTRACT	0xF4
#define INST_FPREM1		0xF5
#define INST_FDECSTP	0xF6
#define INST_FINCSTP	0xF7

#define	INST_FNSTCW		0x07

#define	INST_FPU2_SUB7	0x07
#define INST_FPREM		0xF8
#define INST_FYL2XP1	0xF9
#define INST_FSQRT		0xFA
#define INST_FSINCOS	0xFB
#define INST_FRNDINT	0xFC
#define INST_FSCALE		0xFD
#define INST_FSIN		0xFE
#define INST_FCOS		0xFF

////////////////////////

#define INST_CALL		0xE8
#define INST_JMP2		0xE9
#define INST_JMP		0xEB

#define INST_TEST_REP	0xF3
#define INST_TEST_5		0xF6

#define INST_GEN5_INST	0xF7

#define	INST_GEN_TEST1	0x00
#define	INST_GEN_TEST2	0x01
#define	INST_GEN_NOT	0x02
#define	INST_GEN_NEG	0x03
#define	INST_GEN_MUL	0x04
#define	INST_GEN_IMUL	0x05
#define	INST_GEN_DIV	0x06
#define	INST_GEN_IDIV	0x06

#define INST_INC_2		0xFE
#define INST_GEN_1OP	0xFF

#define	INST_GEN_INC	0x00
#define	INST_GEN_DEC	0x01
#define	INST_GEN_CALL	0x02
#define	INST_GEN_CALLF	0x03
#define	INST_GEN_JMP	0x04
#define	INST_GEN_JMPF	0x05
#define	INST_GEN_PUSH	0x06

////////////////////////

#define mod_reg_IR			0x00
#define mod_reg_I8R			0x01
#define mod_reg_I32R		0x02
#define mod_reg_RR			0x03

#define smod_reg_IR			0x00
#define smod_reg_I8R		0x40
#define smod_reg_I32R		0x80
#define smod_reg_RR			0xC0

#define mod_reg_EAX		0x00
#define mod_reg_ECX		0x01
#define mod_reg_EDX		0x02
#define mod_reg_EBX		0x03
#define mod_reg_ESP		0x04
#define mod_reg_SIB		0x04
#define mod_reg_EBP		0x05
#define mod_reg_ESI		0x06
#define mod_reg_EDI		0x07

/* double op instructions */

#define DB_INST_ADD			0x82
#define DB_INST_JZ			0x84
#define DB_INST_JNZ			0x85
#define DB_INST_JGE			0x8D
#define DB_INST_SETZ		0x94
#define DB_INST_SETNZ		0x95
#define DB_INST_MOVZX		0xB7
#define DB_INST_IMUL		0xAF


#endif