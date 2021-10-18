#include "hashtable.h"

void showTable(hashtable *table)
{
    int i;
    hashpair *pre, *current;
    for(i=0; i<table->num_bucket; i++)
    {
        current = table->buckets[i]->front;
        if(current)
        {
            printf("%d: ", i);        
            while (current)
            {
            #ifdef GD
                printf("%s %f %d\t", current->file_name, current->H, current->file_content->length);
            #endif 
                pre = current;
                current = current->next;
            }
            printf("\n");
        }
    }
}

#ifdef LRU
content* getContent_LRU(bucket *current, char* file_name)
{
    int file_fd, i, len;
    content* file_content = NULL;
    pthread_mutex_lock(&current->mutex);
    hashpair *pair = current->front, *pre = NULL, *rear;  
    current->requesttime++;
    while (pair)
    {
        // if the file exist, move it to the front
        if(strcmp(pair->file_name, file_name) == 0)
        {
            if(pre != NULL)
            {
                pre->next = pair->next;
                pair->next = current->front;
                current->front = pair;                
            }
            file_content = pair->file_content;
            break;
        }
        pre = pair;
        pair = pair->next;
    } 

    // if the file doesn't exist, add into bucket
    if(pair == NULL)
    {
        if(( file_fd = open(file_name,O_RDONLY)) > -1) { /* 打开指定的文件名*/
            pair = (hashpair*)malloc(sizeof(hashpair));
            len = lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/

            // if the bucket is full, delete the file in rear
            while ((current->current_size + len) > current->total_size)
            {
                rear = current->front;
                pre = NULL;
                while (rear->next)
                {
                    pre = rear;
                    rear = rear->next;         
                }
                current->replacedtime++;
                current->current_size -= rear->file_content->length;
                free(rear->file_name);
                free(rear->file_content->address);
                free(rear->file_content);
                free(rear);
                pre->next = NULL;                
            }
            lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/
            file_content = (content*)malloc(sizeof(content));
            file_content->address = (char*)malloc(sizeof(char)*(len+1));
            file_content->length = read(file_fd, file_content->address, len);
            pair->file_content = file_content;
            pair->file_name = file_name;
            close(file_fd);
            addItem_LRU(current, pair);
        }
    }
    pthread_mutex_unlock(&current->mutex);
    return file_content;
}
#endif

#ifdef LFU
content* getContent_LFU(bucket *current, char* file_name)
{
    int file_fd, len, i;
    content* file_content;
    pthread_mutex_lock(&current->mutex);
    hashpair *pair = current->front, *pre = NULL, *rear; 
    current->requesttime++;   
    while (pair)
    {
        // if the file exist, move it to the front
        if(strcmp(pair->file_name, file_name) == 0)
        {
            pair->used_times++;
            if(pre != NULL)
            {
                pre->next = pair->next;
                current->current_size -= pair->file_content->length;
                addItem_LFU(current, pair);
            }
            file_content = pair->file_content;
            break;
        }
        pre = pair;
        pair = pair->next;
    } 

    // if the file doesn't exist, add into bucket
    if(pair == NULL)
    {
        if(( file_fd = open(file_name,O_RDONLY)) > -1) { /* 打开指定的文件名*/
            pair = (hashpair*)malloc(sizeof(hashpair));
            len = lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/
            // if the bucket is full, delete the file in rear
            if((current->current_size + len) > current->total_size)
            {
                rear = current->front;
                pre = NULL;
                while (rear->next)
                {
                    pre = rear;
                    rear = rear->next;                
                }
                current->current_size -= rear->file_content->length;
                current->replacedtime++;
                free(rear->file_name);
                free(rear->file_content->address);
                free(rear->file_content);
                free(rear);
                pre->next = NULL;
            }
            lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/
            file_content = (content*)malloc(sizeof(content));
            file_content->address = (char*)malloc(sizeof(char)*(len+1));
            file_content->length = read(file_fd, file_content->address, len);
            pair->file_content = file_content;
            pair->file_name = file_name;
            pair->used_times = 1;
            close(file_fd);
            addItem_LFU(current, pair);
        }
    }
    pthread_mutex_unlock(&current->mutex);
    return file_content;
}
#endif

content *getContent_MQ(bucket *current, char* file_name)
{
    
    return NULL;
}

#ifdef GD
content *getContent_GD(bucket *current, char* file_name)
{
    int file_fd, len, i;
    content* file_content;
    pthread_mutex_lock(&current->mutex);
    hashpair *pair = current->front, *pre = NULL, *rear;    
    current->requesttime++;
    while (pair)
    {
        // if the file exist, move it to the front
        if(strcmp(pair->file_name, file_name) == 0)
        {
            pair->H = current->L + 1/(double)pair->file_content->length;
            if(pre != NULL)
            {
                pre->next = pair->next;
                current->current_size -= pair->file_content->length;
                addItem_GD(current, pair);
            }
            file_content = pair->file_content;
            break;
        }
        pre = pair;
        pair = pair->next;
    } 

    // if the file doesn't exist, add into bucket
    if(pair == NULL)
    {
        if(( file_fd = open(file_name,O_RDONLY)) > -1) { /* 打开指定的文件名*/
            pair = (hashpair*)malloc(sizeof(hashpair));
            len = lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/
            // if the bucket is full, delete the file in rear
            if((current->current_size + len) > current->total_size)
            {
                rear = current->front;
                pre = NULL;
                while (rear->next)
                {
                    pre = rear;
                    rear = rear->next;                
                }
                current->current_size -= rear->file_content->length;
                current->L = rear->H;
                current->replacedtime++;
                free(rear->file_name);
                free(rear->file_content->address);
                free(rear->file_content);
                free(rear);
                pre->next = NULL;
            }
            lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/
            file_content = (content*)malloc(sizeof(content));
            file_content->address = (char*)malloc(sizeof(char)*(len+1));
            file_content->length = read(file_fd, file_content->address, len);
            pair->file_content = file_content;
            pair->file_name = file_name;            
            pair->H = current->L + 1/(double)file_content->length;
            close(file_fd);
            addItem_GD(current, pair);
        }
    }
    pthread_mutex_unlock(&current->mutex);
    return file_content;
}
#endif