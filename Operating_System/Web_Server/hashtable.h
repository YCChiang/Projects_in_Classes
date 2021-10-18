#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct content
{
    void *address;
    int length;
} content;

typedef struct hashpair
{
    char *file_name;
    content *file_content;
    struct hashpair *next;
#ifdef LFU
    int used_times;
#endif
#ifdef GD
    double H;
#endif
} hashpair;

typedef struct bucket
{
    hashpair *front;
    int total_size;
    int current_size;
    int replacedtime;
    int requesttime;
#ifdef GD
    double L;
#endif
    pthread_mutex_t mutex;    // mutex lock for the bucket
} bucket;

typedef struct hashtable
{
    bucket** buckets;
    int num_bucket;
} hashtable;

#ifdef LRU
//content* getContent_LRU(hashtable* table, bucket* current, char* file_name);
content* getContent_LRU(bucket* current, char* file_name);
void addItem_LRU(bucket* current, hashpair* item);
#endif

#ifdef LFU
//content* getContent_LFU(hashtable* table, bucket* current, char* file_name);
content* getContent_LFU(bucket* current, char* file_name);
void addItem_LFU(bucket* current, hashpair* item);
#endif

#ifdef GD
content *getContent_GD(bucket *current, char* file_name);
void addItem_GD(bucket* current, hashpair* item);
#endif

hashtable* create_hashtable(int num_bucket, int max_size);
void destroy_hashtable(hashtable* table);