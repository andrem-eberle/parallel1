#ifndef _PE_H_
#define _PE_H_

#include "parallel.h"

#define PE_SIG_BASE_OFF		0x3c

typedef struct _pe_header {
	word machine;
	word num_sections;
	dword timestamp;
	dword symbol_table;
	dword num_symbols;
	word optheader_size;
	word characteristics;
} pe_header;


#pragma pack(push,1)

typedef struct _pe_dir_entry {
	dword addr;
	dword size;
} pe_dir_entry;

typedef struct _pe_opt_header32 {
	word magic;
	byte maj_linker_version;
	byte min_linker_version;
	dword size_of_code;
	dword size_of_initdata;
	dword size_of_uninitdata;
	dword entry_point;
	dword code_base;
	dword data_base;
	dword image_base;
	dword sect_align;
	dword file_align;
	word maj_os_ver;
	word min_os_ver;
	word maj_img_ver;
	word min_img_ver;
	word maj_subsys_ver;
	word min_subsys_ver;
	dword win32_ver;
	dword size_of_image;
	dword size_of_headers;
	dword checksum;
	word subsys;
	word dll_charac;
	dword size_of_stack_res;
	dword size_of_stack_commit;
	dword size_of_heap_res;
	dword size_of_heap_commit;
	dword loader_flags;
	dword num_rvas;
	long long export_table;
	long long import_table;
	long long resource_table;
	long long exception_table;
	long long cert_table;
	long long base_reloc_table;
	long long debug;
	long long arch;
	long long global_ptr;
	long long tls_table;
	long long load_cfg_table;
	long long bound_import;
	long long iat;
	long long delay_descriptor;
	long long clr_header;
	long long reserved; /* always zero */


} pe_opt_header32;



typedef struct _pe_section_header {
	char name[8];
	dword vsize;
	dword vaddr;
	dword raw_data_size;
	dword raw_data_ptr;
	dword reloc_ptr;
	dword ln_ptr;
	word reloc_num;
	word ln_num;
	dword characteristics;

} pe_section_header;

typedef struct _import_directory_table {
	dword lookup_rva;	
	dword tstamp;
	dword fwd_chain;
	dword name_rva;
	dword iat_rva;
} idt;

#define PE_IS_ORDINAL(i)	(i&0x80000000)
#define PE_NAME_IDX(i)		(i&0x7FFFFFFF)

typedef struct _import_lookup_table {
	dword name_idx;
} ilt;


#pragma warning( disable : 4200 )
typedef struct _hint_name_table {
	unsigned short hint;
	char name[];
} hnt;

#pragma pack(pop)


extern exec_engine pe_engine;

#endif