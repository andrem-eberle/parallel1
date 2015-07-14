#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pe.h"

#define BUF_SIZE 8192

byte ** ffbufs;

FILE * fptr;

pe_header * ex_header;
pe_opt_header32 * ex_opt_header;

dword sects_base;
pe_section_header ** sects;

pe_section_header * text;

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

	fptr = fopen(filename,"rb");
	if (!fptr) {
		return -1;
	}

	fseek(fptr, 0, SEEK_END);
	fsz = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
	ffbufs = (byte**) calloc(1,(fsz/BUF_SIZE)*sizeof(byte*));
	ffbufs[0] = (unsigned char *)malloc(BUF_SIZE);
	fread(ffbufs[0],1,BUF_SIZE,fptr);

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

	return 0;
}

dword get_entry_point() {
	return ex_opt_header->entry_point;
}

dword get_text_off(dword off) {
	return off+text->vaddr;
}

byte * get_text_section() {
	int i;
	byte * tbuf;
	for (i=0;i< ex_header->num_sections;i++) {
		if (!strcmp(sects[i]->name,".text")) {
			text = sects[i];
			printf("%X\n",sects[i]->vaddr);
			break;
		}
	}
	tbuf = malloc(sects[i]->raw_data_size);
	fseek(fptr,sects[i]->raw_data_ptr,SEEK_SET);
	fread(tbuf,1,sects[i]->raw_data_size,fptr);
	return tbuf;

}

void end_process() {
	free(sects);
	free(ffbufs);
	fclose(fptr);
}
