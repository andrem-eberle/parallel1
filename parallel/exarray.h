#ifndef _EXARRAY_
#define _EXARRAY_

#include <stdlib.h>
#include <string.h>
#include "log.h"

#define EA_BASE_SIZE	2

typedef struct _ex_array {
	void * arr;
	unsigned int fsize;
	unsigned int size;
	unsigned int dsize;
} ex_array;

inline void ea_init_array(ex_array * arr, unsigned int dsize, unsigned int ssize = EA_BASE_SIZE) {
	arr->arr = malloc(ssize*dsize);
	arr->dsize = dsize;
	arr->size = 0;
	arr->fsize = ssize;
}

inline void ea_resize(ex_array * arr) {
	void * tmp = arr->arr;
	arr->fsize = arr->fsize * EA_BASE_SIZE;
	arr->arr = malloc(arr->fsize*arr->dsize);
	memcpy(arr->arr,tmp,arr->size*arr->dsize);
}

inline void ea_check_arr(ex_array * arr) {
	if (arr->size == arr->fsize) {
		ea_resize(arr);
	}	
}

#define ea_get_element(type,ea,i) (((type)ea.arr)[i])

inline void ea_add_element(ex_array * arr, unsigned int el) {
	ea_check_arr(arr);
	unsigned char * base = (unsigned char*)arr->arr;
	*(unsigned int*)(base+(arr->size*arr->dsize)) = el;
	arr->size = arr->size + 1;
}

/* the pointer may change when the array resizes, so don't reuse it */
inline void * ea_get_ptr(ex_array * arr) {
	unsigned char * base = (unsigned char*)arr->arr;
	return base+(arr->size*arr->dsize);
}

inline void ea_remove_element(ex_array * ar, unsigned int el) {
	unsigned int i;
	unsigned char * base = (unsigned char*)ar->arr;
	log_debug("remove %i %i %i %i dsize=%i",ar->dsize*el,ar->size,(ar->size-1)*(ar->dsize),el,ar->dsize);
	for(i = ar->dsize*el; i < (ar->size-1)*(ar->dsize); i++) {
		base[i] = base[i+ar->dsize];
	}
	(ar->size)--;
}

inline void ea_add_element_p(ex_array * arr, void * el) {
	ea_add_element(arr,(unsigned int) el);
}

inline void ea_guarantee_size(ex_array * arr, unsigned int size) {
	while (arr->size+size >= arr->fsize) {
		ea_resize(arr);
	}
}

inline void ea_clone_array(ex_array * dest, ex_array *src) {
	memcpy(dest,src,sizeof(ex_array));
	dest->arr = malloc(dest->fsize*dest->dsize);
	memcpy(dest->arr,src->arr,dest->size*dest->dsize);
}

inline void ea_clear(ex_array * arr) {
	//free(arr->arr);
	arr->size = 0;
}

inline void ea_add_element_pc(ex_array * arr, void * el) {
	ea_check_arr(arr);
	unsigned char * base = (unsigned char*)arr->arr;
	memcpy(base+(arr->size*arr->dsize),el,arr->dsize);
	arr->size = arr->size + 1;

}

inline void ea_destroy_array( ex_array * arr) {
	free(arr->arr);
}

#endif