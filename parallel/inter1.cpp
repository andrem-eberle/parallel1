#include "inter1.h"
#include <stdio.h>
#include "phash.h"
#include "flux.h"
#include "context.h"
#include "series.h"

/***************************************************************
 * Write functions
 */

static FILE * fp = 0;
extern unsigned int dbg_addr;

IT_instruction * IT_base_data = NULL;
IT_instruction * IT_end_data = NULL;
dword IT_base_ep = 0;

phash * IT_instructions_hash = NULL;
phash * IT_instructions_hash_e = NULL;

dword inst_id = 0;

void inter_mem_param_translate(IT_param * p, mem_full_ref * m) {
	p->base_value = m->base.arg;
	p->rdisp = m->rdisp;
	switch (m->base.type) {
		case ID_TYPE_IREG:
			p->type = I_REGREF;	
			p->disp2 = m->bdisp;
			p->disp = m->disp;
			p->mod = m->mult;
			p->r1 = m->bpbdisp;			 
			break;
		case ID_TYPE_ABSOLUTE:
			p->type = I_STATIC;
			break;
		case ID_TYPE_REG:
			p->type = I_REGISTER;
			break;
		case ID_TYPE_IMMEDIATE:
			p->type = I_IMMEDIATE;
			break;
	}
}

void inter_param_translate(IT_param * p, mem_full_ref * m) {
	m->rdisp = p->rdisp;
	switch(p->type) {
		case I_IMMEDIATE:
			m->base.type = ID_TYPE_IMMEDIATE;
			m->base.arg = p->base_value;
			break;
		case I_STATIC:
			m->base.type = ID_TYPE_ABSOLUTE;
			m->base.arg = p->base_value;
			break;
		case I_REGISTER:
			m->base.type = ID_TYPE_REG;
			m->base.arg = p->base_value;
			break;
		case I_REGREF:
			m->base.type = ID_TYPE_IREG;
			m->base.arg = p->base_value;
			m->bdisp = p->disp2;
			m->disp = p->disp;
			m->mult = p->mod;
			m->bpbdisp = p->r1;
			break;
	}
}

void inter_write_finish() {
	fclose(fp);
}

void inter_process(IT_instruction * it) {
	switch(it->inst) {
		case IOP_ADD:
		case IOP_OR :
		case IOP_NOT :
		case IOP_XOR :
		case IOP_SUB :
		case IOP_MUL :
		case IOP_DIV :
		case IOP_IDIV:
		case IOP_ADC:
			inter_math_process(it->inst,&it->p1,&it->p2,&it->p3);
			break;
		case IOP_MOV:
			inter_mov_process(it->inst,&it->p1,&it->p2,&it->p3);
			break;
		case IOP_RET:
			break;
		default:
			break;
	}

}

void inter_load_from_file(char * file) {
	dword last_addr = 0;
	int st = 1;
	FILE * fpx = fopen(file,"rb");
	fseek(fpx,0,SEEK_END);
	int sz = ftell(fpx);
	fseek(fpx,0,SEEK_SET);

	IT_base_data = (IT_instruction *)(malloc(sz));
	fread(IT_base_data,1,sz,fpx);
	fclose(fpx);
	IT_end_data = IT_base_data+(sz/sizeof(IT_instruction));

	if (IT_instructions_hash) {
		ph_destroy(IT_instructions_hash);
	}

	IT_instructions_hash = ph_create(8,256,0);


	if (IT_instructions_hash_e) {
		ph_destroy(IT_instructions_hash);
	}

	IT_instructions_hash_e = ph_create(8,256,0);

	IT_instruction * t = IT_base_data;

	while (t<IT_end_data) {
//		printf("%X %X\n",t->address,t->inst);
		if (st) {
			ph_put_k(t->address,(void*)t->address,t,IT_instructions_hash);
			st = 0;
		}
		else {
			if (last_addr != t->address) {
				ph_put_k((t-1)->address,(void*)(t-1)->address,t-1,IT_instructions_hash_e);
				ph_put_k(t->address,(void*)t->address,t,IT_instructions_hash);
			}
		}
		last_addr = t->address;
		t++;
	}

	ph_put_k((t-1)->address,(void*)(t-1)->address,t-1,IT_instructions_hash_e);
	
}

IT_instruction * inter_get_instruction_e(dword addr) {
	return (IT_instruction*)ph_get(addr,(void*)addr,IT_instructions_hash_e);
}

IT_instruction * inter_get_instruction(dword addr) {
	//return (IT_instruction*)ph_get(addr,(void*)addr,IT_instructions_hash);
	return IT_base_data+addr;
}

void inter_write_start(char * file, dword ep) {
	fp = fopen(file,"wb");
	log_debug("Intermediate write started on %s. Instruction size is %i",file,
		sizeof(dword)+sizeof(int)+3*sizeof(IT_param));
	IT_param p1;
	p1.type = I_IMMEDIATE;
	p1.base_value = ep;
	inter_write_op(IOP_EP,&p1,0,0,0);

	inter_write_empty(0);
}

void inter_write_op(int operation, IT_param * param1, IT_param * param2, IT_param * param3, dword address) {
	fwrite(&address,1,sizeof(dword),fp);
	fwrite(&inst_id,1,sizeof(dword),fp);
	inst_id++;
	fwrite(&operation,1,sizeof(int),fp);

	IT_param zero = {0,};
	if (param1){
		fwrite(param1,1,sizeof(IT_param),fp);
	}
	else {
		fwrite(&zero,1,sizeof(IT_param),fp);
	}
	if (param2){
		fwrite(param2,1,sizeof(IT_param),fp);
	}
	else {
		fwrite(&zero,1,sizeof(IT_param),fp);
	}
	if (param3){
		fwrite(param3,1,sizeof(IT_param),fp);
	}
	else {
		fwrite(&zero,1,sizeof(IT_param),fp);
	}
	
	fflush(fp);
}

void inter_write_call(dword target) {
	IT_param p1;
	p1.type = I_IMMEDIATE;
	p1.base_value = target;
	inter_write_op(IOP_CALL,&p1,0,0,dbg_addr);	
}

void inter_write_jc(int type, dword target) {
	IT_param p1;
	p1.type = I_IMMEDIATE;
	p1.base_value = target;
	inter_write_op(IOP_JC,&p1,0,0,dbg_addr);
}

void inter_write_ret() {
	inter_write_op(IOP_RET,0,0,0,dbg_addr);
}

void inter_write_jmp(dword target) {
	IT_param p1;
	p1.type = I_IMMEDIATE;
	p1.base_value = target;
	inter_write_op(IOP_JMP,&p1,0,0,dbg_addr);

}

int inter_series_to_op_trans( int sop ) {
	int iop = 0;
	switch (sop) {
		case SERIES_ADD:
			iop = IOP_ADD;
			break;
		case SERIES_SUB:
			iop = IOP_SUB;
			break;
		case SERIES_XOR:
			iop = IOP_XOR;
			break;
		case SERIES_MUL:
			iop = IOP_MUL;
			break;
	}

	return iop;
}

int inter_op_to_series_trans( int op ) {
	switch (op) {
		case IOP_ADD:
		case IOP_ADC:
			return SERIES_ADD;
		case IOP_SUB:
		case IOP_SBB:
			return SERIES_SUB;
		case IOP_MUL:
			return SERIES_MUL;
		case IOP_DIV:
			return SERIES_DIV;
		case IOP_IDIV:
			return SERIES_MOD;
		case IOP_XOR:
			return SERIES_XOR;
		case IOP_OR:
			return SERIES_OR;
			
	}

	return 0;
}

void inter_write_empty(dword address) {
	inter_write_op(0,0,0,0,address);
}

void inter_access_memory(mem_full_ref * mem) {
	IT_param p1;
	inter_mem_param_translate(&p1,mem);

	inter_write_op(IOP_ACCESS,&p1,0,0,dbg_addr);
}

void inter_write_mem_write(mem_full_ref * where, mem_full_ref * what) {
	IT_param p1,p2;
	inter_mem_param_translate(&p1,where);
	if (what) {
		inter_mem_param_translate(&p2,what);
	}
	//printf("p1 %X %X\n",p1.type,p1.base_value);
	inter_write_op(IOP_MOV,&p1,what ? &p2 : 0,0,dbg_addr);

}

void inter_call(dword target) {
	mem_full_ref r1;
	r1.base.type = ID_TYPE_IMMEDIATE;
	r1.base.arg = 0;
	inter_write_push_op(&r1);
}

void inter_write_pop_op(mem_full_ref * p1) {
	mem_full_ref mr1 = {0,},mr2 = {0,};

	mr1.base.type = ID_TYPE_IREG;
	mr1.base.arg = ESP;

	inter_write_mem_write(p1,&mr1);

	mr1.base.type = ID_TYPE_REG;
	mr2.base.type = ID_TYPE_IMMEDIATE;
	mr2.base.arg = 4;

	inter_write_math_op(SERIES_ADD,&mr1,&mr2,0);
}

void inter_write_push_op(mem_full_ref * p1) {
	mem_full_ref mr1 = {0,},mr2 = {0,};
	
	mr1.base.type = ID_TYPE_IREG;
	mr1.base.arg = ESP;

	inter_write_mem_write(&mr1,p1);

	mr1.base.type = ID_TYPE_REG;
	mr2.base.type = ID_TYPE_IMMEDIATE;
	mr2.base.arg = 4;

	inter_write_math_op(SERIES_SUB,&mr1,&mr2,0);

}

// r1 is always the destination
void inter_write_math_op(int sop, mem_full_ref * r1, mem_full_ref * r2, mem_full_ref * r3) {
	int iop = inter_series_to_op_trans(sop);
	IT_param p1,p2,p3;
	inter_mem_param_translate(&p1,r1);
	if (r2) {
		inter_mem_param_translate(&p2,r2);
	}
	if (r3) {
		inter_mem_param_translate(&p3,r3);
	}

	inter_write_op(iop,&p1,r2?&p2:0,r3?&p3:0,dbg_addr);
}

/***************************************************************
 * Series process functions
 */

int inter_read_fn(dword *address, dword * laddr, int * op, IT_param * p1, IT_param * p2, IT_param * p3, FILE * fp) {
	int rt = 0;
	rt |= fread(laddr,1,sizeof(dword),fp);
	rt |= fread(address,1,sizeof(dword),fp);
	rt |= fread(op,1,sizeof(int),fp);
	rt |= fread(p1,1,sizeof(IT_param),fp);
	rt |= fread(p2,1,sizeof(IT_param),fp);
	rt |= fread(p3,1,sizeof(IT_param),fp);

	return rt;
}

void inter_read_fn_from_mem(byte * mem, dword *address, int * op, IT_param * p1, IT_param * p2, IT_param * p3) {
	int rt;
	mem += sizeof(dword);
	*address = *(dword*)mem;
	mem += sizeof(dword);
	*op = *(int*)mem;
	mem += sizeof(int);
	
	*p1 = *(IT_param*)mem;
	mem += sizeof(IT_param);
	*p2 = *(IT_param*)mem;
	mem += sizeof(IT_param);
	*p3 = *(IT_param*)mem;
	mem += sizeof(IT_param);

}

void inter_mov_process(dword op,IT_param * p1, IT_param * p2, IT_param * p3) {
	mem_full_ref mr1,mr2,mr3;
	inter_param_translate(p1,&mr1);
	inter_param_translate(p2,&mr2);

	series_assign(&mr2,&mr1);
}

void inter_math_process(dword op,IT_param * p1, IT_param * p2, IT_param * p3) {
	mem_full_ref mr1,mr2,mr3;
	inter_param_translate(p1,&mr1);
	inter_param_translate(p2,&mr2);
	
	series_math_operation(inter_op_to_series_trans(op),&mr2,&mr1,op == IOP_ADC || op == IOP_SBB);
}

void inter_op_handle(IT_instruction * it) {

}

IT_instruction * inter_flux_process(IT_instruction * it) {
	//log_debug("Inter flux processing with op %X at %X",it->inst,it->address);

	if (it->inst == IOP_JC) {
		do_branch(it->address,it->p1.base_value,1);
	}
	else if (it->inst == IOP_JMP) {
		do_jmp(it->address,it->p1.base_value,1);
		return IT_base_data+it->p1.base_value;
	}
	else if (it->inst == IOP_CALL) {
		do_branch_f(it->address,it->p1.base_value,1);
	}
	else if (it->inst == IOP_RET) {
		do_ret(it->address,0);
		return 0;
	}
	else {
		flux_staple((it-IT_base_data),1);
	}
	return it+1;
}

IT_instruction * inter_get_ep() {
	return IT_base_data+IT_base_ep;
}

void inter_load_file(char * file) {
	int op;
	IT_param p1,p2,p3;
	dword addr,vaddr;
	FILE * fp = fopen(file,"rb");
	int sz = 0;
	while (!feof(fp)) {
		inter_read_fn(&addr,&vaddr,&op,&p1,&p2,&p3,fp);
		sz++;
	}
	IT_base_data = (IT_instruction *)calloc(sz,sizeof(IT_instruction));
	IT_instruction * it = IT_base_data;
	fseek(fp,0,SEEK_SET);

	fread(IT_base_data,sizeof(IT_instruction),sz,fp);
	fclose(fp);

	IT_base_ep = IT_base_data->p1.base_value;

}

void inter_process_file(char * file) {

	int op;
	IT_param p1,p2,p3;
	mem_full_ref sp1,sp2,sp3;
	dword addr,vaddr;

	FILE * fp = fopen(file,"rb");
	if (!fp) {
		printf("Invalid intermediate file\n");
		return;
	}

	do {
		inter_read_fn(&addr,&vaddr,&op,&p1,&p2,&p3,fp);
		switch(op) {
			case IOP_ADD:
			case IOP_OR :
			case IOP_NOT :
			case IOP_XOR :
			case IOP_SUB :
			case IOP_MUL :
			case IOP_DIV :
			case IOP_IDIV:
			case IOP_ADC:
				inter_math_process(op,&p1,&p2,&p3);
				break;
			
		}
	} while (!feof(fp));

}

int IT_cmp(const void* p1, const void* p2) {
	IT_inst_block * it1 = (IT_inst_block*)p1;
	IT_inst_block * it2 = (IT_inst_block*)p2;

	return it1->laddr-it2->laddr;
}

void inter_adjust_targets(char * infile) {
	FILE * fp = fopen(infile,"rb");
	dword addr,laddr;
	int op;
	int i,j;
	IT_param p1,p2,p3;
	IT_instruction * it;
	int sz = 0;
	while (!feof(fp)) {
		inter_read_fn(&addr,&laddr,&op,&p1,&p2,&p3,fp);
		sz++;
	}

	IT_inst_block * bl = new IT_inst_block[sz];
	int k = 0;

	int last_addr = -1;

	fseek(fp,0,SEEK_SET);
	while (!feof(fp)) {
		inter_read_fn(&addr,&laddr,&op,&p1,&p2,&p3,fp);

		if (last_addr == -1) {
			bl[k].size = 1;
			bl[k].laddr = laddr;
			it = bl[k].it_block;
		}
		else {
			if (laddr != last_addr) {
				k++;
				bl[k].size = 1;
				bl[k].laddr = laddr;
				it = bl[k].it_block;
			}
			else {
				it = bl[k].it_block+bl[k].size;
				bl[k].size++;				
			}
		}
		
		it->address = addr;
		it->laddr = laddr;
		it->inst = op;
		it->p1 = p1;
		it->p2 = p2;
		it->p3 = p3;
		
		it++;

		last_addr = laddr;
	}
	printf("read %i insts %i\n",sz,k);
	

	qsort(bl,k,sizeof(IT_inst_block),IT_cmp);

	IT_instruction * its = new IT_instruction[sz];
	it = its;

	IT_inst_block * t = bl;
	j = 0;
	for(i=0;i<sz;i++) {
		its[i] = t->it_block[j++];
		its[i].address = i;
		if (j >= t->size) {
			j = 0;
			t++;
		}
	}

	
	for(i=0;i<sz;i++) {
		it = its+i;		
		if (it->inst == IOP_JMP || it->inst == IOP_JC) {
			dword targ = it->p1.base_value;
			//printf("jump to %X\n",targ);
			/* TODO replace this by a hash */
			for(j=0;j<sz;j++) {
				if ((its[j].laddr == targ)) {
					//printf("virt targ %X\n",its[j].address);
					it->p1.base_value = its[j].address;
					break;
				}
			}
		}
	}

	fp = fopen(infile,"wb");
	fwrite(its,sizeof(IT_instruction),sz,fp);
	fclose(fp);
	
}

/***************************************************************
 * ASCII print functions
 */

void inter_ascii_register(int reg, char * val) {
	
	switch(reg) {
		case EAX:
			sprintf(val,"EAX");
			break;
		case EBX:
			sprintf(val,"EBX");
			break;
		case ECX:
			sprintf(val,"ECX");
			break;
		case EDX:
			sprintf(val,"EDX");
			break;
		case ESP:
			sprintf(val,"ESP");
			break;
		case EBP:
			sprintf(val,"EBP");
			break;
		case EDI:
			sprintf(val,"EDI");
			break;
		case ESI:
			sprintf(val,"ESI");
			break;
	}
}

void inter_ascii_add_regref(IT_param * p, char * dest) {
	//[(R+disp2)+r1*mod+disp]
	char val[128],r1[128];
	if (p->rdisp) {
		inter_ascii_register(p->rdisp-1,val);
		inter_ascii_register(p->base_value,r1);
		sprintf(dest,"[(%s+%X)+%s*%X+%X]",val,p->disp2,r1,p->mod,p->disp);
	}
	else {
		//inter_ascii_register(p->r1,val);
		inter_ascii_register(p->base_value,r1);	
		if (p->mod) {
			sprintf(dest,"[%s*%X+%X]",r1,p->mod,p->disp);
		}
		else {
			sprintf(dest,"[%s+%X]",r1,p->disp);
		}
	}

	
	//printf("dest %s\n",dest);
}

int inter_ascii_add_param(IT_param * p, char * dest) {
	if (p->type == 0) {
		return 0;
	}
	char tmp[128];
	char type[128];
	char val[128];
	switch (p->type) {
		case I_IMMEDIATE:
			sprintf(type,"IMMEDIATE");
			sprintf(val,"%X",p->base_value);
			break;
		case I_REGISTER:
			sprintf(type,"REGISTER");
			switch(p->base_value) {
				case EAX:
					sprintf(val,"EAX");
					break;
				case EBX:
					sprintf(val,"EBX");
					break;
				case ECX:
					sprintf(val,"ECX");
					break;
				case EDX:
					sprintf(val,"EDX");
					break;
				case ESP:
					sprintf(val,"ESP");
					break;
				case EBP:
					sprintf(val,"EBP");
					break;
				case EDI:
					sprintf(val,"EDI");
					break;
				case ESI:
					sprintf(val,"ESI");
					break;
			}
			break;
		case I_REGREF:
			sprintf(type,"REGREF");
			val[0] = 0;
			inter_ascii_add_regref(p,val);
			break;
		case I_STATIC:
			sprintf(type,"STATIC");
			sprintf(val,"%X",p->base_value);
			break;
	}

	sprintf(tmp,"%s %s",type,val);
	int len = strlen(tmp);
	memcpy(dest,tmp,len+1);

	return len;
}

void inter_ascii_add_params(char * base, char * tmpdata, IT_param * p1, IT_param * p2, IT_param * p3) {
	int tmpidx;
	sprintf(tmpdata,base);
	tmpidx = strlen(tmpdata);
	tmpidx += inter_ascii_add_param(p1,tmpdata+tmpidx);
	tmpdata[tmpidx++] = ',';
	tmpdata[tmpidx++] = ' ';
	tmpidx += inter_ascii_add_param(p2,tmpdata+tmpidx);
	tmpdata[tmpidx++] = ',';
	tmpdata[tmpidx++] = ' ';
	tmpidx += inter_ascii_add_param(p3,tmpdata+tmpidx);

	tmpdata[tmpidx++] = ' ';
	tmpdata[tmpidx++] = '-';
	tmpdata[tmpidx++] = ' ';

	mem_full_ref mr;
	s_expr * ex;
	inter_param_translate(p1,&mr);
	series_parse_expression(&mr,&ex);
	series_to_readable(ex,tmpdata+tmpidx);
	tmpidx += strlen(tmpdata+tmpidx);
	tmpdata[tmpidx++] = ' ';
	tmpdata[tmpidx++] = '-';
	tmpdata[tmpidx++] = ' ';

	inter_param_translate(p2,&mr);
	series_parse_expression(&mr,&ex);
	series_to_readable(ex,tmpdata+tmpidx);
	tmpidx += strlen(tmpdata+tmpidx);

	tmpdata[tmpidx++] = 0;

	
}

void inter_file_to_ascii(char * inx, char * out) {
	char tmpdata[512];
	int tmpidx = 0;
	if (fp) fclose(fp);
	fp = 0;
	FILE * fin = fopen(inx,"rb");
	FILE * fout = fopen(out,"wb");

	if (!fin) {
		printf("Error acquiring ascii\n");
		return;
	}
	int op;
	IT_param p1,p2,p3;
	dword addr,vaddr;
	dword laddr = 0xFFFFFFFF;
	do {
		int rt = inter_read_fn(&addr,&vaddr,&op,&p1,&p2,&p3,fin);
		if (rt <= 0) {
			break;
		}
		if (addr == laddr && !op) continue;
		fprintf(fout,"%X %X: ",vaddr,addr);
		int zop = 0;
		log_debug("ASCII dump %X %X",addr,op);
		switch(op) {
			case IOP_CALL:
				if (p1.type == I_IMMEDIATE) {
					fprintf(fout,"CALL %X\n",p1.base_value);
				}
				break;
			case IOP_JMP:
				if (p1.type == I_IMMEDIATE) {
					fprintf(fout,"JMP %X\n",p1.base_value);
				}
				break;
			case IOP_EP:
				fprintf(fout,"EP %X\n",p1.base_value);
				break;
			case IOP_JC:
				fprintf(fout,"JC %X\n",p1.base_value);
				break;
			case IOP_RET:
				fprintf(fout,"RET\n");
				break;
			case IOP_MOV:
				inter_ascii_add_params("MOV ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case IOP_ADD:
				inter_ascii_add_params("ADD ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case IOP_SUB:
				inter_ascii_add_params("SUB ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case IOP_ACCESS:
				inter_ascii_add_params("ACCESS ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case IOP_XOR:
				inter_ascii_add_params("XOR ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case IOP_MUL:
				inter_ascii_add_params("MUL ",tmpdata,&p1,&p2,&p3);
				fprintf(fout,"%s\n",tmpdata);
				break;
			case 0:
				
				zop = 1;
				fprintf(fout,"NOP\n");
				break;
		}
		if (!zop) {
			//inter_read_fn(&addr,&op,&p1,&p2,&p3,fin);
		}
		laddr = addr;
	} while (!feof(fin));

	fclose(fin);
	fclose(fout);
}


