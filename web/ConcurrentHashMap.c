#include "ConcurrentHashMap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static unsigned time33(const char* key, size_t len)
{
	unsigned hash = 0;
	int i;
	
	for(i = 0; i < len; i++)
	{
		hash = (hash << 5 + hash) + (unsigned)key[i];
	}

	return hash;
}

struct ConcurrentHashMap* newMap(int size)
{
	struct ConcurrentHashMap* map = (struct ConcurrentHashMap*)malloc
								(sizeof(struct ConcurrentHashMap));
	assert(map != NULL);

	map->table = (struct item_list*)malloc(size * sizeof(struct item_list));
	assert(map->table != NULL);

	int i;
	for(i = 0; i < size; i++)
	{
		SLIST_INIT(&map->table[i]);
	}

	map->size = size;
	
	map->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(map->mutex, NULL);

	map->hash = time33; 
}
void setHasher(struct ConcurrentHashMap* map, Hasher hash)
{
	assert(map != NULL);
	
	map->hash = hash;
}

void put(struct ConcurrentHashMap* map, const char* key
			, const char* value, time_t expire)
{
	assert(map != NULL);
	assert(key != NULL);
	assert(value != NULL);

	struct Item* item = (struct Item*)malloc(sizeof(struct Item));
	assert(item != NULL);
	
	item->key = strdup(key);
	item->value = strdup(value);
	item->expire = expire;

	unsigned hash = map->hash(key, strlen(key)) % map->size;
	
	pthread_mutex_lock(map->mutex);
	SLIST_INSERT_HEAD(&map->table[hash], item, entry);
	pthread_mutex_unlock(map->mutex);
}

void rm(struct ConcurrentHashMap* map, const char* key)
{
	assert(map != NULL);
	assert(key != NULL);

	struct Item* item;
	unsigned hash = map->hash(key, strlen(key)) % map->size;
	
	pthread_mutex_lock(map->mutex);
	SLIST_FOREACH(item, &map->table[hash], entry)
	{
		if(strcmp(item->key, key) == 0)
		{
			SLIST_REMOVE(&map->table[hash], item, Item, entry);
			break;
		}
	}
	pthread_mutex_unlock(map->mutex);

	if(item != NULL)
	{
		free(item->key);
		free(item->value);
		free(item);
	}
}
struct Item* get(struct ConcurrentHashMap* map, const char* key)
{
	assert(map != NULL);
	assert(key != NULL);

	struct Item* item;
	unsigned hash = map->hash(key, strlen(key)) % map->size;
	
	pthread_mutex_lock(map->mutex);
	SLIST_FOREACH(item, &map->table[hash], entry)
	{
		if(strcmp(item->key, key) == 0)
		{
			pthread_mutex_unlock(map->mutex);

			return item;
		}
	}
	pthread_mutex_unlock(map->mutex);

	return NULL;
}
