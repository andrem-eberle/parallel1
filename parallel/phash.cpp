#include "phash.h"
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "log.h"

FILE * __hlog_fp;

void hash_debug(char * format,...) {
	va_list args;
	va_start (args, format);

	time_t  currtime;    
	char charTime[128] = {0};   

	time(&currtime);                                                     
	strftime(charTime,sizeof(charTime)-1,"%Y-%m-%d %H:%M:%S",localtime(&currtime)); 

	if (!__hlog_fp) {
		__hlog_fp = fopen("hash.dump.log","w");
	}
	if (format[strlen(format)-1] == '\n') {
		format[strlen(format)-1] = 0;
	}
	char debug[512], fr[512];
	vsprintf(fr,format,args);
	//sprintf(debug,"[DEBUG] [%s] %s\n",charTime,fr);
	sprintf(debug,"[LOG] [%s] %s\n",charTime,fr);

	fwrite(debug,strlen(debug),1,__hlog_fp);
	fflush(__hlog_fp);
}


/* rehash function, taken from java implementation (HashMap) */
int __c_rehash(int h) {
   h ^= (h >> 20) ^ (h >> 12);
   return h ^ (h >> 7) ^ (h >> 4);
}


phash * ph_create(int bucket_size, uint32 st_size, uint32 hash_func(void *)) {
    if (st_size == 0)  {
        st_size = 2;
    }
    phash * hash = (phash*)malloc(sizeof(phash));
    hash->num_buckets = st_size;
    hash->buckets = (phash_bucket*)calloc(1,BUCKET_FULL_SIZE(st_size,bucket_size));
    hash->bucket_size = bucket_size;
    hash->seq_st = NULL;
    hash->seq_end = NULL;
    hash->hash_func = hash_func;
    hash->num_entries = 0;
    hash->dirty = 0;
    hash->cmp_func = NULL;
    hash->key_size = 0;

    return hash;   
}

uint32 ph_clear(phash * h) {
	
    h->num_entries = 0;
    h->seq_st = NULL;
    h->seq_end = NULL;
    phash_bucket * b = h->buckets;
    phash_bucket * bend = BUCKET_OFFSET(h,h->num_buckets);
	log_debug("cl st %X, %X, %i, %i",b,bend,h->num_buckets,BUCKET_SIZE(h->bucket_size));
    while (b < bend) {
        b->size = 0;   
		b = (phash_bucket*)(((unsigned char*)b)+BUCKET_SIZE(h->bucket_size));
    }

	return 0;
}

void ph_destroy(phash * h) {
    /* NOTE this behavior may cause problems if other threads have locked the mutex, use with care */
    MUTEX_LOCK(h->mutex);
    free(h->buckets);
    free(h);
}

uint32 ph_put(uint32 hash, void * key, void * data, phash * h) {
    uint32 rt =  ph_put_k(hash,key,data,h);
	return rt;
}

uint32 ph_put_k(uint32 hash, void * key, void * data, phash * h) {
    hash = __c_rehash(hash);
    MUTEX_LOCK(h->mutex);
    
    phash_bucket * bckt = BUCKET_OFFSET(h,key_to_idx(hash,h->num_buckets));
    phash_entry * t = bckt->entries;
    while (t < (bckt->entries+bckt->size) ) {
        if (t->key == hash) {
            if (!h->cmp_func || !key || (!h->cmp_func(t->skey,key,h))) {
                t->key = hash;
                t->data = data;
                MUTEX_UNLOCK(h->mutex);
                return 0;
            }
        }
        t++;
    }

    while (bckt->size >= h->bucket_size) {
        if (bckt->size >= C_HASH_MAX_SIZE) {
            MUTEX_UNLOCK(h->mutex);
            return C_HASH_MAX_SIZE_ERROR;
        }
		
        resize(h);
        bckt = BUCKET_OFFSET(h,key_to_idx(hash,h->num_buckets));
    }

    bckt->entries[bckt->size].key = hash;
    bckt->entries[bckt->size].data = data;
    bckt->entries[bckt->size].skey = key;
    bckt->entries[bckt->size].next = NULL;
    bckt->entries[bckt->size].prev = NULL;
    h->num_entries++;

    if (!h->dirty) {
        if (h->seq_st == NULL) {
            h->seq_end = h->seq_st = &(bckt->entries[bckt->size]);
        }
        else {
            h->seq_end->next = &(bckt->entries[bckt->size]);
            h->seq_end->next->prev = h->seq_end;
            h->seq_end = h->seq_end->next;
        }
    }
    bckt->size = bckt->size + 1;
    MUTEX_UNLOCK(h->mutex);
    
    return 0;
    
}

uint32 ph_put_h(void * key, void * data, phash * h) {
    return ph_put(h->hash_func(key),NULL,data,h);
}

void * ph_del_h(void * key, phash * h) {
    return ph_del(h->hash_func(key),NULL,h);    
}

void * ph_del(uint32 hash, void * key, phash * h) {
    return ph_del_k(hash,key,h,1);
}

void * ph_del_k(uint32 hash, void * key, phash * h, int rehash) {
    if (rehash) {
        hash = __c_rehash(hash);
    }
    MUTEX_LOCK(h->mutex);
log_debug("delk %X",h);
    phash_bucket * bckt = BUCKET_OFFSET(h,key_to_idx(hash,h->num_buckets));
    phash_entry * t = bckt->entries;
    phash_entry * r;
    while (t < (bckt->entries+bckt->size) ) {
        if (t->key == hash) {
            if (!h->cmp_func || !key || (!h->cmp_func(t->skey,key,h))) {
                void * data = t->data;

                if(t->next)t->next->prev = t->prev;
                if(t->prev)t->prev->next = t->next;

                if (h->seq_st == t) {
                    h->seq_st = t->next;
                }
                if (h->seq_end == t) {
                    h->seq_end = t->prev;
                }
                //memcpy(t,t+1,(bckt->entries+bckt->size-t-1));
                memset(t,0,sizeof(phash_entry));
                while ((t+1)<bckt->entries+bckt->size) {
                    *t = *(t+1);
                    t++;
                }
                bckt->size--;
                h->num_entries--;
				h->dirty = 1;

                MUTEX_UNLOCK(h->mutex);
                return data;
            }
        }
        t++;
    }
    MUTEX_UNLOCK(h->mutex);
    return NULL;
}   

void * ph_get(uint32 hash, void * key, phash * h) {
    void * rt = ph_get_k(hash,key,h);
	return rt;
}

void * ph_get_k(uint32 hash, void * key, phash * h) {
    hash = __c_rehash(hash);
    void * data;
   
    //log_debug("take mutex %X",&h->mutex);
    MUTEX_LOCK(h->mutex);
    phash_bucket * bckt = BUCKET_OFFSET(h,key_to_idx(hash,h->num_buckets));
    phash_entry * t = bckt->entries;
    while (t < (bckt->entries+bckt->size) ) {
        if (t->key == hash) {
            //log_debug("%X %X %X %X %X",h->cmp_func,key,h->cmp_func ? h->cmp_func(t->skey,key,h) : 0, t->skey,key);
            if (!h->cmp_func || !key || (!h->cmp_func(t->skey,key,h))) {
                data = t->data;
                MUTEX_UNLOCK(h->mutex);
                return t->data;
            }
        }
        t++;
    }
    MUTEX_UNLOCK(h->mutex);
    return NULL;
}

int ph_contains(uint32 hash, phash * h) {
    //MUTEX_LOCK(h->mutex);
    int assess = (ph_get(hash,NULL,h) != NULL);
    //MUTEX_UNLOCK(h->mutex);
    return assess;
}

void * ph_geth(void * key, phash * h) {
    return ph_get(h->hash_func(key),NULL,h);
}

uint32 key_to_idx(uint32 key, uint32 len) {
    return key & (len-1);
}

uint32 resize(phash * h) {
    phash_bucket * old_buckets = h->buckets;
    phash_bucket * tmp = old_buckets;
    uint32 new_len = 2*h->num_buckets;
	
    h->buckets = (phash_bucket*)calloc(1,BUCKET_FULL_SIZE(new_len,h->bucket_size));

    while (tmp < (BUCKET_OFFSETH(old_buckets,h->bucket_size,h->num_buckets)) ) {
        phash_entry * e = &tmp->entries[0];
        while (e < (tmp->entries+tmp->size)) {
            phash_bucket * bckt = BUCKET_OFFSET(h,key_to_idx(e->key,new_len));
            bckt->entries[bckt->size].key = e->key;
            bckt->entries[bckt->size].data = e->data;
            bckt->size = bckt->size + 1;
            e++;
        }
        tmp = BUCKET_INCREASE(tmp,h->bucket_size);		
    }
	
    h->num_buckets = new_len;
    h->dirty = 1;
    free(old_buckets);

	return 0;
}

uint32 ph_rebuild_list(phash * h) {
	uint32 i;
	if (h->num_entries == 0) {
		return 0;
	}
	
	phash_entry * e = NULL;
	phash_entry * e2;

	phash_bucket * b = h->buckets;
    phash_bucket * bend = BUCKET_OFFSET(h,h->num_buckets);

    while (b < bend) {
        for(i=0;i<b->size;i++) {
			if (!e) {				
				e = &b->entries[i];
				e->prev = 0;
				h->seq_st = e;                
			}
			else {
				e->next = &b->entries[i];
				e->next->prev = e;
				e2 = e;
				e = e->next;
			}
		} 
		b = (phash_bucket*)(((unsigned char*)b)+BUCKET_SIZE(h->bucket_size));
    }
	if (!e) {
		h->seq_end = e2;
	}
	else {
		h->seq_end = e;
	}

	h->dirty = 0;

	return 0;
}


/*
 * A hash function for strings. This algorithm is a classic from the literature.
 */
unsigned int str_djb2_hash(char *str) {
    unsigned int hash = 5381;
    int c;

	while (c = *str++) {
        hash = (hash * 33) ^ c;
	}

    return hash;
}