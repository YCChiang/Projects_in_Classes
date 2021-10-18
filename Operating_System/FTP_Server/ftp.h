#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define DATA_BUF_SIZE 2048
#define CMD_BUF_SIZE 100
#define NUM_TOKENS (CMD_BUF_SIZE/3 + 1)
#define COMMAND_PORT 21
#define DATA_PORT 20
#define PASV_MODE 1
#define PORT_MODE 2

typedef void(*function)(char*[], int);

int create_server_socket(int port);
int create_client_socket(int client_port, int server_port, char *des_ipv4);
int tokenize_command(char *buff, char *tokens[]);
void string_replace(char *str, int len, char c1, char c2);
int split_data(char *buff, char **strs, int buf_lenth);
int count_lines(char *str, int str_lenth);
int send_file(int scok, char *filename, int file_fd);
int recv_file(int scok, char *filename, int file_fd);
int find_substr(char *str, char *substr);
