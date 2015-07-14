#include "series.h"

extern unsigned int dbg_addr;

void series_addr_reg_write_imm(dword imm, dword reg, dword inst_addr) {
	mem_full_ref dest;
	mem_full_ref src;

	dest.base.type = ID_TYPE_REG;
	dest.base.arg = reg;
	dest.disp = 0;
	dest.mult = 0;

	src.base.type = ID_TYPE_IMMEDIATE;
	src.base.arg = imm;
	src.disp = 0;
	src.mult = 0;
	series_write(&dest,&src,inst_addr);
}

void series_addr_reg_write_abs(dword addr, dword reg, dword inst_addr) {
	mem_full_ref dest;
	mem_full_ref src;

	dest.base.type = ID_TYPE_REG;
	dest.base.arg = reg;
	dest.disp = 0;
	dest.mult = 0;

	src.base.type = ID_TYPE_ABSOLUTE;
	src.base.arg = addr;
	src.disp = 0;
	src.mult = 0;
	series_write(&dest,&src,inst_addr);
}

void series_addr_ax_write_abs(dword addr, dword inst_addr) {
	series_addr_reg_write_abs(addr,EAX,inst_addr);
	
}



void series_push(dword inst_addr) {
	mem_full_ref dest;
	mem_full_ref op;
	dest.disp = 0;
	op.disp = 0;

	dest.base.type = ID_TYPE_REG;
	dest.base.arg = ESP;
	op.base.type = ID_TYPE_IMMEDIATE;
	op.base.arg = sizeof(int);
	series_operation_src(SERIES_SUB,&dest,&op,&dest,inst_addr);
}

void series_push_imm(uint32 val) {

}

void series_push_reg(uint32 reg) {

}

void series_pop_reg(uint32 reg) {

}

void series_assign(mem_full_ref * src, mem_full_ref * dest) {
	series_write(dest,src,dbg_addr);
}

void series_math_operation(int operation, mem_full_ref * src, mem_full_ref * dest, int carry) {
	
	series_operation_src(operation,src,dest,dest,dbg_addr);
	//series_operation_src(operation,dest,dest,src,dbg_addr);
}

void series_ref_reg(mem_full_ref * rf, int reg) {
	rf->disp = rf->mult = 0;
	rf->base.type = ID_TYPE_REG;
	rf->base.arg = reg;
}

void series_ref_AX(mem_full_ref * rf) {
	series_ref_reg(rf,EAX);
}

void series_ref_imm(mem_full_ref * rf, uint32 val) {
	rf->disp = rf->mult = 0;
	rf->base.type = ID_TYPE_IMMEDIATE;
	rf->base.arg = val;
}