#include "memory.h"
#include "analysis_engine.h"
#include "phash.h"

#include <string.h>

int label = 0;

phash * memory;

void init_memory() {
	memory = ph_create(4,1024,0);
}

int next_label() {
	return ++label;
}

dword memory_new_label() {
	return next_label();
}

dword mem_hash_id(mem_id addr) {
	return addr.arg;
}

void mm_get_data(mem_full_ref src, mem_id * data) {
	switch (src.base.type) {
		case ID_TYPE_REG:
			memcpy(data, &p_state->regs[src.base.arg].data,sizeof(mem_id));
			break;
		case ID_TYPE_IREG:
			memory_read(p_state->regs[src.base.arg].data,src.disp);
			break;
		case ID_TYPE_ABSOLUTE:
			data->type = ID_TYPE_ABSOLUTE;
			data->arg = memory_read(src.base,src.disp);			
			break;
		case ID_TYPE_IMMEDIATE:
			memcpy(data,&src.base,sizeof(mem_id));
			break;
	}

}

dword memory_read(mem_id base, dword disp) {
	return 0;
}

void memory_write() {

}


void finalize_write(mem_full_ref target, mem_full_ref src) {
	/*mem_id data;
	
	switch(target.base.type) {
		case ID_TYPE_REG:			
			p_state->regs[target.base.arg].src = src;
			mm_get_data(src, &data);
			break;
		case ID_TYPE_IREG:
			log_data("Writing data. Type: IREG. Target: (%s) %s. SRC - %s",stringify_reg(target.base.arg),stringify_hex(target.disp),stringify_mem_ref(&src));
			break;
		case ID_TYPE_ABSOLUTE:
			log_data("Writing data. Type: ABS. Target: (%X) %s. SRC - %s",target.base.arg,stringify_hex(target.disp),stringify_mem_ref(&src));
			break;
	}*/
}

byte * modreg_sib_analysis(byte * inst, mem_full_ref *srco, mem_full_ref*desto, mem_full_ref*vt) {
	dword rtype, arg1, arg2;
	int isize = 1;
	MODREGDEC(inst,(arg1),(arg2),rtype);
	if (rtype == 0x40) {
		vt->mult = 2;
	}
	else if (rtype == 0x80) {
		vt->mult = 4;
	}
	else if (rtype == 0xC0) {
		vt->mult = 8;
	}
log_debug("sib args %X %X %X",arg1,arg2,*(inst));
	vt->rdisp = 0;
	vt->bdisp = 0;
	if (arg1 == mod_reg_EBP) {
log_debug("special sib");
		if (rtype == 0x00) {
			vt->bdisp = *(dword*)(inst+1);
			isize+4;
		}
		else if (rtype == 0x40) {
			vt->bpbdisp = 1;
			vt->bdisp = *(inst+1);
			isize+1;
		}
		else if (rtype == 0x80) {
			vt->bpbdisp = 1;
			vt->bdisp = *(dword*)(inst+1);
			isize+4;
			vt->mult = 8;
		}
		else {
			/* 0xC0 */
			isize += 4;
		}
	}
	else {
		vt->rdisp = arg1+1;
	}
	dword reg = arg2;
	vt->base.arg = reg;

	return inst+isize;
}

byte * modreg_analysis(byte * inst, mem_full_ref * src, mem_full_ref * dest, dword * flags) {
	if (src) memset(src,0,sizeof(mem_full_ref));
	if (dest) memset(dest,0,sizeof(mem_full_ref));
	dword rtype, arg1, arg2;
	char disp;
	dword disp32;
	mem_full_ref *srco=0, *desto=0, *vt;

	int modSize = 0;

	if ((*flags) & MOD_FLAG_DEST_ONLY) {
		
	}
	else {
		srco = src;
	}

	desto = dest;

	MODREGDEC(inst+1,(arg1),(arg2),rtype);

	if (srco) {
		srco->base.type = ID_TYPE_REG;
		srco->base.arg = arg1;
		srco->disp = 0;
		srco->bpbdisp = 0;
	}

	if ((*flags) & MOD_FLAG_DEST_ONLY) {
		desto->base.arg = arg1;	
		desto->bpbdisp = 0;
		vt = desto;
	}
	else {
		desto->base.arg = arg2;
		vt = srco;
	}
	vt->bpbdisp = 0;
log_debug("modreg type %X",rtype);
	switch (rtype) {					
		case smod_reg_RR:
			vt->base.type = ID_TYPE_REG;
					
			vt->disp = 0;
			if (flags) (*flags) |= (MOD_FLAG_REG);

			return inst+2;
		/* following cases have memory pointer access */
		case smod_reg_I8R:
			modSize = 1;
			vt->base.type = ID_TYPE_IREG;
			vt->disp = *(byte*)(inst+2);
			if (arg1 == mod_reg_SIB) {
				log_debug("sib");
				/* sib with displacement (x86 is amazing) */
				inst = modreg_sib_analysis(inst+2,srco,desto,vt);
				MODREGDEC(inst+2,(arg1),(arg2),rtype);
				
				return inst+modSize;
			}
			return inst+3;
		case smod_reg_I32R:
			modSize = 4;
			vt->base.type = ID_TYPE_IREG;
			vt->disp = *(int*)(inst+2);

			if (arg1 == mod_reg_SIB) {
				inst = modreg_sib_analysis(inst+2,srco,desto,vt);
				return inst+modSize;
			}
			
			return inst+6;
		case smod_reg_IR:
			modSize = 0;
			vt->base.type = ID_TYPE_IREG;
			vt->disp = 0;
			if (arg1 == mod_reg_EBP) {

				/* silly x86 standard, in this particular case there's no
					register in source, but an immediate value */
				if (flags) (*flags) |= (MOD_FLAG_IMM);
				return inst+6;
			}
			else if (arg1 == mod_reg_SIB) {
				if (arg1 == mod_reg_SIB) {
					inst = modreg_sib_analysis(inst+2,srco,desto,vt);
				}
				return inst+modSize;
				
			}
			else {
				if (flags) (*flags) |= MOD_FLAG_IREG;
				
			}
			return inst+2;
	}
	/* shouldn't reach here */
	printf("ERROR, invalid area for modregdec?\n");
	return 0;
}
