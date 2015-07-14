#ifndef _LOG_H_
#define _LOG_H_

struct _mem_full_ref;
extern unsigned char * dbg_arrbase;

void log_data(char * format,...);
#define log_debug(f,...) log_data(f,__VA_ARGS__)
char * stringify_reg(int num);
char * stringify_hex(int val);
char * stringify_mem_ref(struct _mem_full_ref * data);
unsigned int dbg_array_to_va(unsigned char * arr);


#endif