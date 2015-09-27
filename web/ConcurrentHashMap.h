#ifndef CONCURRENT_HASH_MAP_H
#define CONCURRENT_HASH_MAP_H

#define DEBUG

#include "queue.h"
#include <pthread.h>
#include <time.h>

struct Item
{
	char* key;
	char* value;
	time_t expire;
	SLIST_ENTRY(Item) entry;	
};

SLIST_HEAD(item_list, Item);

typedef unsigned (*Hasher)(const char* key, size_t len);

struct ConcurrentHashMap
{
	struct item_list* table;
	int size;
	pthread_mutex_t* mutex;

	Hasher hash;
};

struct ConcurrentHashMap* newMap(int size);
void setHasher(struct ConcurrentHashMap* map, Hasher hash); 
void put(struct ConcurrentHashMap* map, const char* key
			, const char* value, time_t expire);
void rm(struct ConcurrentHashMap* map, const char* key);
struct Item* get(struct ConcurrentHashMap* map, const char* key);

#endif
