#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pe.h"
#include "phash.h"

#define BUF_SIZE 8192

byte ** ffbufs;

FILE * fptr;

pe_header * ex_header;
pe_opt_header32 * ex_opt_header;

dword sects_base;
pe_section_header ** sects;

pe_section_header * text;

phash * import_table;

pe_section_header * including_sect(dword vaddr) {
	int i;

	for(i=0;i<ex_header->num_sections;i++) {
		if (vaddr > sects[i]->vaddr && vaddr < sects[i]->vaddr+sects[i]->vsize) {
			return sects[i];
		}
	}
	return NULL;
}

int memiszero(void * mm, int size) {
	unsigned char * t = (unsigned char*)mm;
	unsigned char * b = t;
	while (t < (b+size)) {
		if (*t) return 0;
		t++;
	}
	return 1;
}

dword vaddr_to_faddr(pe_section_header * sect, dword vaddr) {
	return ((vaddr-sect->vaddr)+sect->raw_data_ptr);
}

char * table_get_name(dword vaddr) {
	pe_section_header * sect = including_sect(vaddr);
	if (sect == 0) {
		return "";
	}
	dword faddr = vaddr_to_faddr(sect,vaddr);

	return (char*)(ffbufs[0]+faddr);
}

void process_import_table() {
	pe_dir_entry * itable = (pe_dir_entry*)(&ex_opt_header->import_table);
	
	if (ex_opt_header->import_table == 0) {
		return;
	}
	printf("%X\n",ex_opt_header->image_base);
	pe_section_header* isect = including_sect(itable->addr);
	dword faddr = vaddr_to_faddr(isect,itable->addr);
	idt * direntry = (idt*)(ffbufs[0]+faddr);

	
	while (!memiszero(direntry,sizeof(idt))) {
		int idx = 0;
		char * libname = table_get_name(direntry->name_rva);
		printf("%s\n",table_get_name(direntry->name_rva));

		ilt * it;
		dword itentry = vaddr_to_faddr(isect,direntry->iat_rva);
		it = (ilt*)(ffbufs[0]+itentry);
		dword rvabase = direntry->iat_rva;

		while (it->name_idx) {
			it_inner_entry * e = (it_inner_entry*)malloc(sizeof(it_inner_entry));	
			dword evaddr = rvabase;
			e->vaddr = evaddr;
			e->libname = libname;
			e->idx = idx;
			e->name = table_get_name(it->name_idx+2);
			//printf("%X\n",ex_opt_header->image_base);
			//printf("%X %s\n",evaddr,e->name);
			ph_put(evaddr+ex_opt_header->image_base,(void*)(evaddr+ex_opt_header->image_base),e,import_table);
			it++;
			rvabase += 4;
			idx++;
		}
		direntry++;
	}
}

static char * _pe_inner_read(dword base, dword size) {
	dword f_idx = base / BUF_SIZE;
	dword t_idx = f_idx;
	dword rd = 0;
	dword rd_sz;
	dword rd_base;

	while (size) {
		rd_sz = size > (BUF_SIZE - base) ? (BUF_SIZE - base) : size;
		rd_base = base - (base % BUF_SIZE);
		if (!ffbufs[t_idx]) {
			fseek(fptr,base,SEEK_SET);
			ffbufs[t_idx] = (unsigned char *)malloc(BUF_SIZE);
			fread(ffbufs[t_idx],1,BUF_SIZE,fptr);
		}
		base += rd_sz;
		size -= rd_sz;
		t_idx++;
	}
}

int process_file(char * filename) {
	dword fsz;
	dword header_off;
	int i;

	import_table = ph_create(2,64,0);
	fptr = fopen(filename,"rb");
	if (!fptr) {
		printf("File not found: %s\n",filename);
		return -1;
	}

	fseek(fptr, 0, SEEK_END);
	fsz = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
	ffbufs = (byte**) calloc(1,(fsz/BUF_SIZE)*sizeof(byte*));
	//ffbufs[0] = (unsigned char *)malloc(BUF_SIZE);
	//fread(ffbufs[0],1,BUF_SIZE,fptr);
	ffbufs[0] = (unsigned char *)malloc(fsz);
	fread(ffbufs[0],1,fsz,fptr);

	memcpy(&header_off,ffbufs[0]+PE_SIG_BASE_OFF,sizeof(dword));
	
	/* first 4 bytes are a signature, ignore them */
	header_off += sizeof(dword);
	ex_header = (pe_header*) (ffbufs[0]+header_off);
	ex_opt_header = (pe_opt_header32*) (ffbufs[0]+header_off+sizeof(pe_header));
	sects_base = header_off+sizeof(pe_header)+sizeof(pe_opt_header32);

	sects = (pe_section_header **) malloc(ex_header->num_sections*sizeof(pe_section_header*));

	for (i=0;i < ex_header->num_sections ; i++) {
		sects[i] = (pe_section_header*)(ffbufs[0]+(sects_base+i*sizeof(pe_section_header)));
	}

	process_import_table();

	return 0;
}

dword get_entry_point() {
	//printf("%X\n",ex_opt_header->entry_point);
	return ex_opt_header->entry_point;
}

dword get_text_off(dword off) {
	return off-text->vaddr;
}

sect_info * get_text_section() {
	int i;
	sect_info * tex = (sect_info*)malloc(sizeof(sect_info));
	byte * tbuf;
	for (i=0;i< ex_header->num_sections;i++) {
		if (!strcmp(sects[i]->name,".text")) {
			text = sects[i];
			//printf("%X\n",sects[i]->vaddr);
			break;
		}
	}
	tbuf = (byte*)malloc(sects[i]->raw_data_size);
	fseek(fptr,sects[i]->raw_data_ptr,SEEK_SET);
	fread(tbuf,1,sects[i]->raw_data_size,fptr);
	tex->data = tbuf;
	tex->ep = get_text_off(get_entry_point());
	tex->size = sects[i]->raw_data_size;
	tex->va = sects[i]->vaddr;
	tex->vsize = sects[i]->vsize;
	//printf("%X %X %X\n",sects[i]->raw_data_ptr,tex->ep,sects[i]->raw_data_ptr+tex->ep-text->vaddr);
	return tex;

}

void end_process() {
	free(sects);
	free(ffbufs);
	fclose(fptr);
}

it_inner_entry * pe_get_call_target(dword vaddr) {
	return (it_inner_entry*)ph_get(vaddr,(void*)vaddr,import_table);
}

exec_engine pe_engine = {
	process_file,
	get_text_section,
	pe_get_call_target
};