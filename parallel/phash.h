#ifndef _PHASH_H_
#define _PHASH_H_

/**
 * Generic hash functions. 
 * NOTE there is some problem with this implementation, poor hashing functions
 * may cause the table to grow too quickly, which must be fixed.
 * TODO change buckets to sorted array and implement binary search
 */

#define uint32 unsigned int

#define BUCKET_ST_SIZE          (sizeof(uint32))
#define BUCKET_FULL_SIZE(s,b)   (s*(BUCKET_ST_SIZE + (b)*sizeof(phash_entry)))
#define BUCKET_SIZE(b)          (BUCKET_ST_SIZE+(b)*sizeof(phash_entry))
#define BUCKET_INCREASE(t,b)    (phash_bucket*)((unsigned char *)t+BUCKET_SIZE(b))

#define BUCKET_OFFSET(h, o)     (phash_bucket*)((unsigned char *)h->buckets+((o)*BUCKET_SIZE(h->bucket_size)))
#define BUCKET_OFFSETH(hs, hb, o) (phash_bucket*)((unsigned char *)hs+((o)*BUCKET_SIZE(hb)))

#define C_HASH_KEY_TYPE_PTR             1
#define C_HASH_KEY_TYPE_UINT32          2

// 32 mb for now
#define C_HASH_MAX_SIZE 33554432

#define C_HASH_MAX_SIZE_ERROR -1

#define MUTEX_LOCK(a)
#define MUTEX_UNLOCK(a)

#ifdef WIN32
#pragma warning(disable: 4200)
#endif

typedef struct _phash_entry_serial {
    uint32 key;
    unsigned char data[];
} phash_entry_serial;

typedef struct _phash_entry {
	uint32 key;
    void * skey;
	void * data;
    struct _phash_entry * next;
    struct _phash_entry * prev;
} phash_entry;

typedef struct _phash_bucket {
	uint32 size;
	phash_entry entries[];	
} phash_bucket;


typedef struct _phash {
    uint32 bucket_size;
    uint32 num_buckets;
    uint32 num_entries;
    phash_bucket * buckets;
    uint32 (*hash_func)(void *);
    phash_entry * seq_st;
    phash_entry * seq_end;
    int (*cmp_func)(void *, void *, void *);
    uint32 key_size;
    uint32 key_type;
    int dirty;
} phash;

#define P_HASH_REMOVE_IT(it,h) \
    int tobreak = (it == h->seq_end); \
    phash_entry * tmp = (it)->next; \
    ph_del_k((it)->key,NULL,h,0); \
    if (tobreak) { \
        break; \
    } \
    it = tmp; \
    continue;
    
#define P_HASH_ITERATE_L(it,h,code) \
    LOCK_MUTEX(h->mutex); \
    { \
    if (h->dirty) { \
		ph_rebuild_list(h); \
	} \
    phash_entry * it = h->seq_st; \
    if (it) { \
        for(;;) { \
            code \
            if (it == h->seq_end) { \
                break; \
            } \
            it = it->next; \
        } \
    }\
    } \
    UNLOCK_MUTEX(h->mutex);


#define P_HASH_ITERATE(it,h,code) \
    {\
    if (h->dirty) { \
		ph_rebuild_list(h); \
	} \
    phash_entry * it = h->seq_st; \
    if (it) { \
        for(;;) { \
            code \
            if (it == h->seq_end) { \
                break; \
            } \
            it = it->next; \
        } \
    }\
    } \

#define P_HASH_CONTINUE(it,h) \
	if (it == h->seq_end) { \
		break; \
	} \
	it = it->next; \
	continue;


unsigned int ph_put(unsigned int hash, void* key, void * data, phash* h);
void *ph_get(unsigned int hash, void* key, phash* h);
void * ph_del(uint32 hash, void * key, phash * h);
void * ph_del_k(uint32 hash, void * key, phash * h, int rehash);
void * ph_get_k(uint32 hash, void * key, phash * h);

//void * ph_remove_k(uint32 hash, void * key, phash * h, int rehash);

phash * ph_create(int bucket_size, uint32 st_size, uint32 hash_func(void *));
void ph_destroy(phash * h);

uint32 key_to_idx(uint32 hash, uint32 len);
uint32 resize(phash * h);
uint32 ph_clear(phash * h);
void * ph_del_t(uint32 hash, void* key, phash * h, int rehash);

uint32 ph_rebuild_list(phash * h);
uint32 ph_put_k(uint32 hash, void * key, void * data, phash * h);

unsigned int str_djb2_hash(char *str);

#define STRING_HASH(s) str_djb2_hash(s)



#endif