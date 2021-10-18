#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct file_info_t
{
    char name[32];
    off_t position;
    size_t length;
} file_info_t;

int main()
{
    int i;
    file_info_t files[5] =
    {
        {"file1", 100, 2000},
        {"file2", 200, 3000},
        {"file3", 300, 4000},
        {"file4", 400, 5000}
    };
    file_info_t temp;
    FILE *file = fopen("table.txt", "w+");
    if(file == NULL)
    {
        printf("error open file!");
        return -1;
    }
    for(i=0; i<5; i++)
    {
        fwrite(&files[i], sizeof(file_info_t), 1, file);
        printf("write name:%s position:%ld length:%lu\n", files[i].name, files[i].position, files[i].length);
    }
    fclose(file);
    file = fopen("table.txt", "r");
    if(file == NULL)
    {
        printf("error open file!");
        return -1;
    }
    while (fread(&temp, sizeof(file_info_t), 1, file))
    {
        printf("name:%s position:%ld length:%lu\n", temp.name, temp.position, temp.length);
    }
    fclose(file);
    return 0;
}