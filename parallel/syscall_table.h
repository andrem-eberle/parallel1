#ifndef _SYSCALL_TABLE_H_
#define _SYSCALL_TABLE_H_

#include "series.h"

void init_syscall_table();

it_inner_entry * is_syscall(dword addr);
series_fn_info * syscall_get_info(dword addr);

#endif