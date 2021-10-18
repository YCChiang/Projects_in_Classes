#include "web-file-system.h"

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

hashpair *getItemByName(hashtable* table, char *file_name)
{
    int hash = hashString(file_name)%table->num_bucket;
    hashpair *pair = table->buckets[hash]->front, *pre = NULL;
    pthread_mutex_lock(&(table->buckets[hash]->mutex));
    while (pair)
    {
        if(strcmp(pair->file_info->name, file_name) == 0)
            break;
        pre = pair;
        pair = pair->next;
    }
    pthread_mutex_unlock(&(table->buckets[hash]->mutex));
    return pair;
}

void addItem(hashtable* table, hashpair* item)
{
    int hash = hashString(item->file_info->name)%table->num_bucket;
    pthread_mutex_lock(&(table->buckets[hash]->mutex));
    item->next = table->buckets[hash]->front;
    table->buckets[hash]->front = item;
    pthread_mutex_unlock(&(table->buckets[hash]->mutex));
}

hashtable* create_hashtable(int num_bucket)
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
            free(pair->file_info);
            free(pair);
            pair = next;
        }
        pthread_mutex_destroy(&(table->buckets[i]->mutex));
        free(table->buckets[i]);
    }
    free(table->buckets);
    free(table);
}

content *read_file(char *file_name, filesystem *sys)
{
    hashpair *pair = getItemByName(sys->table, file_name);
    content *file_content = NULL;
    file_info_t *file_info;
    int ret;
    if(pair)
    {
        file_info = pair->file_info;
        file_content = (content*)malloc(sizeof(content));
        file_content->address = (char*)malloc(sizeof(char)*(file_info->length+1));
        file_content->length = file_info->length;
        pthread_rwlock_rdlock(&sys->file_lock);
        ret = pread(sys->file_fd, file_content->address, file_info->length, file_info->position);
        pthread_rwlock_unlock(&sys->file_lock);
        if(ret < 0)
            printf("read %s file failed!\n", file_name);
    }
    else
    {
        printf("The %s file doesn't exist in the web file filesystem!\n", file_name);
    }    
    return file_content;
}

void store_fileinfo(filesystem *sys, file_info_t *file_info)
{
    FILE *file;
    pthread_rwlock_wrlock(&sys->table_lock);
    file = fopen(FILE_TABLE_CONTENT, "ab+");
    fwrite(file_info, sizeof(file_info_t), 1, file);
    fclose(file);
    pthread_rwlock_unlock(&sys->table_lock);
}

int write_file(char *file_name, filesystem *sys)
{
    int file_fd, len, ret;
    file_info_t *temp;
    hashpair *item;
    char *file_content;
    if(getItemByName(sys->table, file_name))
    {
        printf("ERROR: the %s file already exist in the web file filesystem\n", file_name);
        return -1;
    }
    file_fd = open(file_name, O_RDONLY);
    if(file_fd < -1)
    {
        printf("ERROR: Can't open the %s file\n", file_name);
        return -1;
    }
    len = lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/
    lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/
    temp = (file_info_t*)malloc(sizeof(file_info_t));
    file_content = (char*)malloc(sizeof(char)*(len+1));
    len = read(file_fd, file_content, len);
    close(file_fd);
    if(len <= -1)
    {
        printf("ERROR: Can't read the %s file\n", file_name);
        return -1;
    }
    temp->length = len;
    pthread_rwlock_wrlock(&sys->file_lock);
    temp->position = lseek(sys->file_fd, (off_t)0, SEEK_END);
    ret = pwrite(sys->file_fd, file_content, temp->length, temp->position);
    pwrite(file_fd, "\n", 1, temp->position+ret);
    pthread_rwlock_unlock(&sys->file_lock);
    if(ret < 0)
        printf("write %s file failed!\n", file_name);
    strcpy(temp->name, file_name);
    store_fileinfo(sys, temp);
    item = (hashpair*)malloc(sizeof(hashpair));
    item->file_info = temp;
    item->next = NULL;
    addItem(sys->table, item);
    printf("Added %s file to the web file filesystem!\n", file_name);
    return 0;
}

void load_table(filesystem *sys)
{
    FILE *file;
    file_info_t temp, *file_info;
    hashpair *pair;
    sys->table = create_hashtable(1000);
    pthread_rwlock_rdlock(&sys->table_lock);
    file = fopen(FILE_TABLE_CONTENT, "r");
    if(file == NULL)
    {
        file = fopen(FILE_TABLE_CONTENT, "w+");
    }
    else
    {
        while (fread(&temp, sizeof(file_info_t), 1, file))
        {
            printf("name:%s position:%ld length:%lu\n", temp.name, temp.position, temp.length);
            pair = (hashpair*)malloc(sizeof(hashpair));
            file_info = (file_info_t*)malloc(sizeof(file_info_t));
            file_info->length = temp.length;
            file_info->position = temp.position;
            strcpy(file_info->name, temp.name);
            pair->file_info = file_info;
            pair->next = NULL;
            addItem(sys->table, pair);
        }
    }       
    fclose(file);
    pthread_rwlock_unlock(&sys->table_lock);
}

filesystem *open_filesystem()
{
    filesystem *sys = (filesystem*)malloc(sizeof(filesystem));
    
    pthread_rwlock_init(&sys->file_lock, NULL);
    pthread_rwlock_init(&sys->table_lock, NULL);
    sys->file_fd = open(SYSTEM_FILE, O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
    load_table(sys);

    return sys;
}

void close_filesystem(filesystem *sys)
{
    pthread_rwlock_destroy(&sys->file_lock);
    pthread_rwlock_destroy(&sys->table_lock);
    destroy_hashtable(sys->table);
    close(sys->file_fd);
    free(sys);
}

int main()
{
    filesystem *sys = open_filesystem();
    char *buffer;
    buffer = (char*)malloc(sizeof(char)*MAX_NAME_LEN);
    sprintf(buffer, "3.html");
    int ret = write_file(buffer, sys);
    printf("%d\n", ret);

    buffer = (char*)malloc(sizeof(char)*MAX_NAME_LEN);
    sprintf(buffer, "2.html");
    ret = write_file(buffer, sys);
    printf("%d\n", ret);

    buffer = (char*)malloc(sizeof(char)*MAX_NAME_LEN);
    sprintf(buffer, "2.html");
    content *temp = read_file(buffer, sys);
    if(temp)
        printf("%d: %s\n", temp->length, (char*)temp->address);
    close_filesystem(sys);
    return 0;
}