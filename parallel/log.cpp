#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#include "instset.h"
#include "memory.h"

#include "flux.h"

FILE * log_fp;

unsigned int dbg_addr;

extern function_table_entry * current_function;

void log_data(char * format,...) {
	va_list args;
	va_start (args, format);

	time_t  currtime;    
    char charTime[128] = {0};   
 
    time(&currtime);                                                     
    strftime(charTime,sizeof(charTime)-1,"%Y-%m-%d %H:%M:%S",localtime(&currtime)); 

	if (!log_fp) {
		log_fp = fopen("parallel.log","w");

	}
	if (format[strlen(format)-1] == '\n') {
		format[strlen(format)-1] = 0;
	}
	char debug[512], fr[512];
	vsprintf(fr,format,args);
	//sprintf(debug,"[DEBUG] [%s] %s\n",charTime,fr);
	sprintf(debug,"[LOG] [%s] [ADDR:%X] [VADDR:%X] [FN:%X] %s\n",charTime,dbg_addr,0x401000+dbg_addr,current_function ? current_function->src : 0,fr);

	fwrite(debug,strlen(debug),1,log_fp);
	fflush(log_fp);
}

char d_storage[32];

char * stringify_reg(int num) {
	switch (num) {
		case mod_reg_EAX:
			sprintf(d_storage,"EAX");
			break;
		case mod_reg_EBX:
			sprintf(d_storage,"EBX");
			break;
		case mod_reg_ECX:
			sprintf(d_storage,"ECX");
			break;
		case mod_reg_EDX:
			sprintf(d_storage,"EDX");
			break;
		case mod_reg_ESP:
			sprintf(d_storage,"ESP");
			break;
		case mod_reg_EBP:
			sprintf(d_storage,"EBP");
			break;
		case mod_reg_EDI:
			sprintf(d_storage,"EDI");
			break;
		case mod_reg_ESI:
			sprintf(d_storage,"ESI");
			break;
	}
	return d_storage;
}

char str[32];

char * stringify_hex(int val) {
	if (val < 0) {
		sprintf(str,"- %X",0xFFFFFFFF-val+1);
	}
	else {
		sprintf(str,"+ %X",val);
	}
	
	return str;
}

char memref[64];

unsigned char * dbg_arrbase;

dword dbg_array_to_va(unsigned char * arr) {
	return (dword)(arr-dbg_arrbase);
}

char * stringify_mem_ref(mem_full_ref * data) {
	switch (data->base.type) {
		case ID_TYPE_REG:
			sprintf(memref,"Type: REG, Value: (%s) %s",stringify_reg(data->base.arg),stringify_hex(data->disp));
			break;
		case ID_TYPE_IREG:
			sprintf(memref,"Type: IREG, Value: (%X) %s",(data->base.arg),stringify_hex(data->disp));
			break;
		case ID_TYPE_ABSOLUTE:
			sprintf(memref,"Type: ABS, Value: (%X)",(data->base.arg));
			break;
		case ID_TYPE_IMMEDIATE:
			sprintf(memref,"Type: IMM, Value: (%X)",(data->base.arg));
			break;
	}
	return memref;
}

