#include "hashtable.h"

#define NUMTHREADS 8
#define HASHCOUNT 300

double ave_time = 0;
pthread_mutex_t mutex; 

static inline unsigned long hashString(char *str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

hashtable* create_hashtable(int num_bucket, int max_size)
{
    hashtable* table = (hashtable*)malloc(sizeof(hashtable));
    int i;
    bucket* temp;
    
    if(table==NULL)
        return NULL;    
    table->buckets = (bucket**)malloc(num_bucket*sizeof(bucket*));
    if(table->buckets == NULL)
    {
        free(table);
        return NULL;
    }
    for(i=0; i<num_bucket; i++)
    {
        temp = (bucket*)malloc(sizeof(bucket));
        temp->front = NULL;
        temp->current_size = 0;
        temp->replacedtime = 0;
        temp->requesttime = 0;
        temp->total_size = max_size;
    #ifdef GD
        temp->L = 0;
    #endif
        pthread_mutex_init(&(temp->mutex), NULL);
        table->buckets[i] = temp;
    }
    table->num_bucket = num_bucket;
    return table;
}

void destroy_hashtable(hashtable* table)
{
    int i;
    if(table == NULL)
        return;
    hashpair *next, *pair;
    
    for(i=0; i<table->num_bucket; i++)
    {
        pair = table->buckets[i]->front;
        while (pair)
        {
            next = pair->next;
            free(pair->file_name);
            free(pair->file_content->address);
            free(pair->file_content);
            free(pair);
            pair = next;
        }
        pthread_mutex_destroy(&(table->buckets[i]->mutex));
        free(table->buckets[i]);
    }
    free(table->buckets);
    free(table);
}

#ifdef LRU
void addItem_LRU(bucket* current, hashpair* item)
{
    item->next = current->front;
    current->front = item;
    current->current_size += item->file_content->length;
}
#endif

#ifdef LFU
void addItem_LFU(bucket* current, hashpair* item)
{
    hashpair *pre = NULL, *pair = current->front;
    while (pair)
    {
        if(pair->used_times < item->used_times)
            break;
        pre = pair;
        pair = pair->next;
    }
    if(pre)
    {
        pre->next = item;
        item->next = pair;
    }
    else
    {
        item->next = current->front;
        current->front = item;
    }
    current->current_size += item->file_content->length;
}
#endif

#ifdef GD
void addItem_GD(bucket* current, hashpair* item)
{
    hashpair *pre = NULL, *pair = current->front;
    while (pair)
    {
        if(pair->H < item->H)
            break;      
        pre = pair;
        pair = pair->next;
    }
    if(pre)
    {
        pre->next = item;
        item->next = pair;
    }
    else
    {
        item->next = current->front;
        current->front = item;
    }
    current->current_size += item->file_content->length;
}
#endif

void *thread_func(void* data)
{
    int hash, i, name, base;
    char *file_name;
    hashtable *table = (hashtable*)data;
    struct timeval t1, t2;
    double time = 0;
    base = 1;
    for(i=0; i<10000; i++)
    { 
        file_name = (char*)malloc(sizeof(char)*40);
        name = rand()%1000+base;
        if(i%10 == 0)
            name += rand()%1000 + 100;
        if(i%1000 == 0)
            base += 500;
        sprintf(file_name, "Crawler4jDate/%d.html", name);
        hash = hashString(file_name)%table->num_bucket;
        //printf("\n%s: %d\n", file_name, hash);
        gettimeofday(&t1, NULL);
    #ifdef LRU
        getContent_LRU(table->buckets[hash], file_name); 
    #endif
    #ifdef GD
        getContent_GD(table->buckets[hash], file_name);  
    #endif      
    #ifdef LFU
        getContent_LFU(table->buckets[hash], file_name);  
    #endif 
        gettimeofday(&t2, NULL);
        time += (t2.tv_usec - t1.tv_usec)/1000 + (t2.tv_sec - t1.tv_sec)*1000;
    }
    pthread_mutex_lock(&mutex);
    ave_time += time / 10000;
    pthread_mutex_unlock(&mutex);
}

int main(void)
{
    hashtable* table = create_hashtable(HASHCOUNT, 1024*1024*4);
    int i, request = 0, replaced = 0;
    pthread_t threads[NUMTHREADS];
    srand((unsigned)time(NULL));

    pthread_mutex_init(&mutex, NULL);
    for(i=0;i<NUMTHREADS; i++)
        pthread_create(&threads[i], NULL, thread_func, (void*)table);
    
    for(i=0; i<NUMTHREADS; i++)
        pthread_join(threads[i], NULL);
    
    for(i=0; i<HASHCOUNT; i++)
    {
        request += table->buckets[i]->requesttime;
        replaced += table->buckets[i]->replacedtime;
    }
    printf("%d/%d = hit ratio: %f\naverage time: %fms\n", replaced, request, 1-((double)replaced/(double)request), ave_time/HASHCOUNT);
    pthread_mutex_destroy(&mutex);
}
