#include "parallel.h"
#include "phash.h"

#include "syscall_table.h"

extern exec_engine * l_engine;

it_inner_entry * is_syscall(dword addr) {
	it_inner_entry * ite = l_engine->get_call_target(addr);
	if (!ite) {
		return 0;
	}
	printf("ctarg %s %s\n",ite->name ,ite->libname);
	if (strcmp(ite->libname,"MSVCR90.dll") && strcmp(ite->libname,"KERNEL32.dll")) {
		return 0;
	}

	return ite;	
}

phash * syscall_table;

void init_syscall_table() {
	series_fn_info * fn_info;
	syscall_table = ph_create(2,8,0);

	/* for now we hard-wire the syscalls, in the future we may
		run the series recognizer on kernel libs to generate
		the table */

	fn_info = (series_fn_info*)calloc(1,sizeof(series_fn_info));
	fn_info->ret.flags = RET_FLAG_NEW;	
	fn_info->stack_size = 4;
	ph_put(STRING_HASH("malloc"),"malloc",fn_info,syscall_table);

}

series_fn_info * syscall_get_info(dword addr) {

	it_inner_entry * ite = is_syscall(addr);
	if (!ite) {
		return NULL;
	}
	log_debug("Syscall found: %s",ite->name);
	return (series_fn_info*)ph_get(STRING_HASH(ite->name),ite->name,syscall_table);
}