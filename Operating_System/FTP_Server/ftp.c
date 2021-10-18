#include "ftp.h"

// 字符替换
void string_replace(char *str, int len, char c1, char c2) {
    int i;
    for(i=0; i<len; i++) {
        if(str[i] == c1) {
            str[i] = c2;
        }
    }
}

// 新建服务端socket
int create_server_socket(int port) {
    struct sockaddr_in server_addr;
    int socket_fd, val = 1;
    
    // Creating socket file descriptor 
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("ftp: creation"); 
        return -1; 
    } 

    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(port); 
       
    //2. 解决在close之后会有一个WAIT_TIME，导致bind失败的问题
    if((setsockopt(socket_fd, SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int))) < 0)
    {
        perror("ftp: set sockopt");
        return -1;
    }

    if (bind(socket_fd, (struct sockaddr*)&server_addr,  sizeof(server_addr))<0) 
    { 
        perror("ftp: bind"); 
        return -1; 
    } 
    if (listen(socket_fd, 5) < 0) 
    { 
        perror("ftp: listen");
        return -1;
    } 
    return socket_fd;
}

// 新建客户端socket
int create_client_socket(int client_port, int server_port, char *des_ipv4) {
    struct sockaddr_in server_addr, client_addr;
    int client_fd, val=1;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        perror("ftp: creation"); 
        return -1;
    }

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(client_port);

    //2. 解决在close之后会有一个WAIT_TIME，导致bind失败的问题
    if((setsockopt(client_fd, SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int))) == -1)
    {
        perror("ftp: set sockopt");
        return -1;
    }

    if( bind(client_fd,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        perror("ftp: bind"); 
        return -1;
    }

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(server_port); 

    // Convert des_ipv4  addresses from text to binary form 
    if(inet_pton(AF_INET, des_ipv4, &server_addr.sin_addr)<=0)  
    { 
        printf("ftp: Unknown host %s \n", des_ipv4); 
        return -1; 
    }
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    { 
        perror("ftp: connect\n"); 
        return -1; 
    } 
    return client_fd;
}

// 分割命令，提取每个命令字段
int tokenize_command(char *buff, char *tokens[]) {
    int token_count = 0, i=0, in_token = 0;
    int buf_lenth = strlen(buff);

    for(i=0; i<buf_lenth; i++) {
        switch (buff[i]) {
            // 命令字段结尾符号
            case ' ':
            case '\t':
            case '\n':
                buff[i] = '\0';
                in_token = 0;
                break;

            // 命令字段开始符号
            default:
                if (!in_token) {
                    tokens[token_count] = &buff[i];
                    token_count++;
                    in_token = 1;
                }
        }
    }
	tokens[token_count] = NULL;
    return token_count;
}

// 计算以\r\n结尾的行数
int count_lines(char *str, int str_lenth) {
    int i, count=0;
    int len = strlen(str);
    for(i = 0; i<len-1; ++i)
    {
        if(str[i] == '\r' && str[i+1]=='\n')
            ++count;
    }
    return count;
}

// 用\n\0代替\r\n分割数据
int split_data(char *buff, char **strs, int buf_lenth) {
    int line_count = 0, in_line = 0, i;
    for(i=0; i<buf_lenth-1; ++i) {
        if(buff[i] == '\r' && buff[i+1] == '\n') {
            buff[i++] = '\n';
            buff[i] = '\0';
            in_line = 0;
            continue;
        }
        if(!in_line) {
            strs[line_count] = &buff[i];
            ++line_count;
            in_line = 1;
        }
    }
    return line_count;
}

// 接收文件数据并写入file_fd指向的文件中
int recv_file(int scok, char *filename, int file_fd) {
    int recvbytes, file_size = 0, ret;
    char data_buff[DATA_BUF_SIZE+1];

    while ((recvbytes = recv(scok, data_buff, DATA_BUF_SIZE, 0)) > 0) {
        write(file_fd, data_buff, recvbytes);
        file_size += recvbytes;
    }
    close(file_fd);
    return file_size;
}

// 发送file_fd指向的文件
int send_file(int scok, char *filename, int file_fd) {
    char data_buff[DATA_BUF_SIZE+1] = {0};
    int file_size = 0, sendbytes, ret;

    while ((ret = read(file_fd, data_buff, DATA_BUF_SIZE)) > 0 ) {
        sendbytes = send(scok ,data_buff, ret, 0);
        if(sendbytes != -1)
            file_size += sendbytes;
    }
    close(file_fd);
    return file_size;
}

int find_substr(char *str, char *substr){
    int str_len = strlen(str);
    int substr_len = strlen(substr);
    int i=0, j=0, result;
    for (i=0; i<str_len-substr_len; i++) {
        if(str[i] == substr[0]) {
            result = 1;
            for (j=0; j<substr_len; j++) {
                if(str[i+j] != substr[j])
                    result = 0;
            }
            if(result)
                return 1;       
        }
    }
    return 0;
}