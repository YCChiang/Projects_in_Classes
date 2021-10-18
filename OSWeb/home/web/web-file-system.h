#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 32
#define FILE_TABLE_CONTENT "table-file-contents"
#define SYSTEM_FILE "System-file"
#define NOT_FOUND_FILE "The file NOT FOUND"

typedef struct file_info_t
{
    char name[MAX_NAME_LEN];
    off_t position;
    size_t length;
} file_info_t;

typedef struct content
{
    void *address;
    int length;
} content;

typedef struct hashpair
{
    file_info_t *file_info;
    char *file_name;
    struct hashpair *next;
} hashpair;

typedef struct bucket
{
    hashpair *front;
    pthread_mutex_t mutex;    // mutex lock for the bucket
} bucket;

typedef struct hashtable
{
    bucket** buckets;
    int num_bucket;
} hashtable;

typedef struct filesystem
{
    pthread_rwlock_t file_lock;
    pthread_rwlock_t table_lock;
    hashtable *table;
    int file_fd;
} filesystem;

void addItem(hashtable* table, hashpair* item);

hashtable* create_hashtable(int num_bucket);
void destroy_hashtable(hashtable* table);