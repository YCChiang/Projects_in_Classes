#include "ftp.h"
#include "client.h"

// get远程文件
void get(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes, reinput_num, file_fd;
    long int file_size;
    double sec, speed;
    char *reinput_tokens[4], cmd_buff[CMD_BUF_SIZE+1];
    char *local_file = NULL, *remote_file = NULL;
    char *cmd_line = NULL, *cmd_line2 = NULL;
    struct timeval t1, t2;

    // 命令行中没有指定远程文件名，要求用户输入文件名
    if(token_num < 2) {
        cmd_line = readline("(remote-file) ");
        reinput_num = tokenize_command(cmd_line, reinput_tokens);
        if(reinput_num == 1) {
            remote_file = reinput_tokens[0];
            cmd_line2 = readline("(local-file) ");
            reinput_num = tokenize_command(cmd_line, reinput_tokens);
            if(reinput_num > 0) 
                local_file = reinput_tokens[0];
        }            
        else {
            remote_file = reinput_tokens[0];
            local_file = reinput_tokens[1];
        }
    }
    else {
        if(token_num > 2)
            local_file = tokens[2];
        else
            local_file = tokens[1];        
        remote_file = tokens[1];
    }
    printf("local: %s remote: %s\n", local_file, remote_file);
    
    // 判断当前客户选择的模式，并进行建立新数据socket
    if(mode == PORT_MODE) {
        doport();
    }
    else {
        dopasv();
    }
    
    sprintf(cmd_buff, "RETR %s\r\n", remote_file);
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);

    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);
    
    // 只有当数据socket成功建立和服务端准备好了发送数据时才接受文件并写新建的文件中
    if(data_socket != -1) {
        if(find_substr(cmd_buff, "150")) {
            file_fd = open(local_file, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR);

            gettimeofday(&t1, NULL);
            file_size = recv_file(data_socket, local_file, file_fd);
            gettimeofday(&t2, NULL);
            close_dataconn();

            recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
            cmd_buff[recvbytes-2] = '\0';
            printf("%s\n", cmd_buff);
            
            sec = t2.tv_usec - t1.tv_usec;
            speed = file_size/sec;
            printf("%ld bytes received in %.2f secs (%.4f MB/s)\n", file_size, sec/1000000, speed);            
        }
        else
            close_dataconn();
    }  

    if(cmd_line != NULL)
        free(cmd_line);
    if(cmd_line2 != NULL)
        free(cmd_line2);    
}

// 上传本地文件
void put(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes, reinput_num, file_fd;
    long int file_size;
    double sec, speed;
    char *reinput_tokens[4], cmd_buff[CMD_BUF_SIZE+1];
    char *local_file = NULL, *remote_file = NULL;
    char *cmd_line = NULL, *cmd_line2 = NULL;
    struct timeval t1, t2;

    // 命令行中没有指定本地文件名，要求用户输入文件名
    if(token_num < 2) {
        cmd_line = readline("(local-file) ");
        reinput_num = tokenize_command(cmd_line, reinput_tokens);
        if(reinput_num == 1) {
            local_file = reinput_tokens[0];
            cmd_line2 = readline("(remote-file) ");
            reinput_num = tokenize_command(cmd_line, reinput_tokens);
            if(reinput_num > 0) 
                remote_file = reinput_tokens[0];
        }            
        else {
            remote_file = reinput_tokens[1];
            local_file = reinput_tokens[0];
        }
    }
    else {
        if(token_num > 2)
            remote_file = tokens[2];
        else
            remote_file = tokens[1];        
        local_file = tokens[1];        
    }

    printf("local: %s remote: %s\n", local_file, remote_file);
    
    if((file_fd = open(local_file, O_RDONLY, S_IRUSR))<0) {
        printf("local: %s: No such file or directory\n", local_file);
    }
    else {
        // 判断当前客户选择的模式，并进行建立新数据socket
        if(mode == PORT_MODE) {
            doport();
        }
        else {
            dopasv();
        }

        sprintf(cmd_buff, "STOR %s\r\n", remote_file);
        send(control_socket, cmd_buff, strlen(cmd_buff), 0);

        recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        cmd_buff[recvbytes-2] = '\0';
        printf("%s\n", cmd_buff);

        // 只有当数据socket成功建立和服务端准备好了接受数据时，才发送文件
        if(data_socket != -1) {
            if(find_substr(cmd_buff, "150")) {                
                gettimeofday(&t1, NULL);
                file_size = send_file(data_socket, local_file, file_fd);
                gettimeofday(&t2, NULL);               
                close_dataconn(); 

                recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
                cmd_buff[recvbytes-2] = '\0';
                printf("%s\n", cmd_buff);
                

                sec = t2.tv_usec - t1.tv_usec;
                speed = file_size/sec;
                printf("%ld bytes sent in %.2f secs (%.4f MB/s)\n", file_size, sec/1000000, speed);                
            }
        }
    }        
    
    if(cmd_line != NULL)
        free(cmd_line);
    if(cmd_line2 != NULL)
        free(cmd_line2);
}


// 获得当前远程的文件夹路径
void get_path(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes;
    char cmd_buff[CMD_BUF_SIZE+1];
    sprintf(cmd_buff, "PWD\r\n");
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);
    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);
}

// 获得当前远程文件夹下的所有文件信息
void list_contents(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes;
    char cmd_buff[CMD_BUF_SIZE+1];

    // 判断当前客户选择的模式，并进行建立新数据socket
    if(mode == PORT_MODE) {
        doport();
    }
    else {
        dopasv();
    }

    if(token_num > 1) {
        sprintf(cmd_buff, "LIST %s\r\n", tokens[1]);
    }
    else {
        sprintf(cmd_buff, "LIST\r\n");
    }
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);

    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);

    recv_data();
    close_dataconn();

    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);        
}

// 改变远程文件夹路径
void change_dir(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes;
    char cmd_buff[CMD_BUF_SIZE+1];
    char *cmd_line;
    if(token_num < 2) {
        cmd_line = readline("Remote directory ");
        sprintf(cmd_buff, "CWD %s\r\n", cmd_line);
        free(cmd_line);
    }
    else {
        sprintf(cmd_buff, "CWD %s\r\n", tokens[1]);
    }
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);
    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);
}


// 帮助手册
void help(char *tokens[], int token_num) {
    int i=0, j=0;
    if(token_num == 1) {
        for(i=0; i<COMMAND_NUM; ++i) {
            printf("%s\t", commands[i].name);
            if((i+1)%4 == 0)
                 printf("\n");
        }
        printf("\n");
    }
    else {
        for(i=1; i<token_num; ++i) {
            for(j=0; j<COMMAND_NUM; j++) {
                if(strcmp(tokens[i], commands[j].name) == 0) {
                    printf("%s\t\t%s\n", commands[j].name, commands[j].discrption);
                    break;
                }
            }
            if(j == COMMAND_NUM) {
                printf("Invalid help command %s.\n", tokens[i]);
            }
        }
    }
}

// 建立与服务端的控制连接
void connect_server(char *tokens[], int token_num) {
    char des_ipv4[20];
    if(token_num < 2) {
        char *cmd_line = readline("To ");
        strcpy(des_ipv4, cmd_line);
        free(cmd_line);
    }
    else {
        strcpy(des_ipv4, tokens[1]);
    }
    login(des_ipv4);
}

// 删除指定远程文件
void delete_file(char *tokens[], int token_num) {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes, reinput_num;
    char *reinput_tokens[4], cmd_buff[CMD_BUF_SIZE+1];
    char *remote_file = NULL, *cmd_line = NULL;

    // 命令行中没有指定远程文件名，要求用户输入文件名
    if(token_num < 2) {
        cmd_line = readline("(remote-file) ");
        reinput_num = tokenize_command(cmd_line, reinput_tokens);
        if(reinput_num > 0) {
            remote_file = reinput_tokens[0];
        }
    }
    else {    
        remote_file = tokens[1];
    }
    sprintf(cmd_buff, "DELE %s\r\n", remote_file);
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);

    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-2] = '\0';
    printf("%s\n", cmd_buff);
}


// 开启或关闭被动模式
void passive(char *tokens[], int token_num) {
    if(mode != PASV_MODE) {
        mode = PASV_MODE;
        printf("Passive mode on.\n");
    }
    else {
        mode = PORT_MODE;
        printf("Passive mode off.\n");
    }
}

// 开启或关闭主动模式
void sendport(char *tokens[], int token_num) {
    if(mode != PORT_MODE) {
        mode = PORT_MODE;
        printf("Use of PORT cmds on.\n");
    }
    else {
        mode = PASV_MODE;
        printf("Use of PORT cmds off.\n");
    }
}

// 断开与服务端的控制连接
void disconnect(char *tokens[], int token_num) {
    int recvbytes;
    char cmd_buff[CMD_BUF_SIZE+1]; 
    if(control_socket != -1) {
        sprintf(cmd_buff, "QUIT\r\n");
        send(control_socket, cmd_buff, strlen(cmd_buff), 0);
        recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        cmd_buff[recvbytes-2] = '\0';
        printf("%s\n", cmd_buff);
        close(control_socket);
        control_socket = -1;
    }        
    if(data_socket != -1) {
        close(data_socket);
        data_socket = -1;
    }        
}

// 退出客户端进程
void quit(char *tokens[], int token_num) {   
    disconnect(tokens, token_num);
    exit(0);
}

// 命令关键字对应的功能函数
static function cmd_func[COMMAND_NUM] = {
    help, quit, change_dir, disconnect, delete_file, list_contents,
    disconnect, get, help, list_contents, connect_server, passive,
    put, get_path, quit, sendport
};

// 在主动模式下，建立与服务端的数据连接
void doport() {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int recvbytes, addrlen;
    char cmd_buff[CMD_BUF_SIZE+1]; 
    char *local_ip;
    unsigned int p = CHOOSE_PORT;
    struct sockaddr_in cli_addr;
    addrlen = sizeof(cli_addr);
    data_fd = create_server_socket(p);
    if(data_fd != -1) {
        getsockname(control_socket, (struct sockaddr *)&cli_addr, (socklen_t*)&addrlen);
        local_ip = inet_ntoa(cli_addr.sin_addr);
        string_replace(local_ip, sizeof(local_ip), '.', ',');
        sprintf(cmd_buff, "PORT %s,%u,%u\r\n", local_ip, (p >> 8) & 255, p & 255);
        send(control_socket, cmd_buff, strlen(cmd_buff), 0);

        if ((data_socket = accept(data_fd, (struct sockaddr*)&cli_addr,  (socklen_t*)&addrlen))<0) { 
            perror("accept error"); 
        }
        else {
            recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
            cmd_buff[recvbytes-2] = '\0';
            printf("%s\n", cmd_buff);
        }
    }
}

// 接受服务端发送的数据，并进行处理后显示在客户端
void recv_data() {
    char data_buff[DATA_BUF_SIZE+1];
    char **data = NULL;
    int i, total_lines, recvbytes;

    while((recvbytes = recv(data_socket, data_buff, DATA_BUF_SIZE, 0)) > 0) {
        data = (char **)malloc(sizeof(char *)*(count_lines(data_buff, recvbytes)+1));
        total_lines = split_data(data_buff, data, recvbytes);
        for(i=0; i<total_lines; ++i)
            printf("%s", data[i]);
    }
}

// 在被动模式下，与服务端建立数据连接
void dopasv() {
    if(control_socket == -1) {
        printf("Not connected.\n");
        return;
    }
    int cli_port, len, recvbytes; 
    char *begin, *end;
    char cmd_buff[CMD_BUF_SIZE+1];
    char addr_info[25] = {0};
    unsigned int a1, a2, a3, a4, p1, p2;

    sprintf(cmd_buff, "PASV\r\n");
    send(control_socket, cmd_buff, strlen(cmd_buff), 0);

    recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
    cmd_buff[recvbytes-1] = '\0';
    printf("%s\n", cmd_buff);

    begin = strchr(cmd_buff, '(');
    end = strchr(cmd_buff, ')');
    len = end - begin;
    strncpy(addr_info, begin+1, len-1);

    // 判断接受到的ip地址和端口是否正确
    if (sscanf(addr_info, "%u,%u,%u,%u,%u,%u",
               &a1, &a2, &a3, &a4, &p1, &p2) != 6 ||
        a1 > 255 || a2 > 255 || a3 > 255 || a4 > 255 ||
        p1 > 255 || p2 > 255 || (a1|a2|a3|a4) == 0 ||
        (p1 | p2) == 0) {

    }
    else {
        sprintf(addr_info, "%u.%u.%u.%u", a1, a2, a3, a4);
        cli_port = (p1 << 8) | p2;
        data_socket = create_client_socket(DATA_PORT, cli_port, addr_info);
    }
}

// 登录模块
int login(char des_ipv4[]) {
    int recvbytes;
    char cmd_buff[CMD_BUF_SIZE+1];
    char *cmd_line;    
    control_port = CHOOSE_PORT;
    control_socket = create_client_socket(control_port, COMMAND_PORT,des_ipv4);
    if(control_socket > 0) {
        printf("Connected to %s.\n", des_ipv4);
        recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        cmd_buff[recvbytes-2] = '\0';
        printf("%s\n", cmd_buff);

        sprintf(cmd_buff, "User (%s): ", des_ipv4);
        cmd_line = readline(cmd_buff);
        sprintf(cmd_buff, "USER %s\r\n", cmd_line);
        free(cmd_line);
        send(control_socket, cmd_buff, strlen(cmd_buff), 0);
        recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        cmd_buff[recvbytes-2] = '\0';
        printf("%s\n", cmd_buff);

        cmd_line = getpass("Password:");
        sprintf(cmd_buff, "PASS %s\r\n", cmd_line);
        free(cmd_line);
        send(control_socket, cmd_buff, strlen(cmd_buff), 0);
        recvbytes = recv(control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        cmd_buff[recvbytes-2] = '\0';
        printf("%s\n", cmd_buff);
    }
}

// 关闭数据连接
void close_dataconn() {
    if(data_fd != -1){
        close(data_fd);
        data_fd = -1;
    }
    if(data_socket != -1) {
        close(data_socket);
        data_socket = -1;
    }
}

// 初始化全局变量
void init_info() {
    mode = PORT_MODE;
    control_socket = -1;
    data_socket = -1;
}

int main(int argc, char const *argv[]) {
    char *tokens[NUM_TOKENS];
    char *cmd_line;
    char des_ipv4[20];
    int token_num = 0, i=0;
    srand(time(0));
    init_info();
    if(argc < 3) {     
        if(argc == 2) {
            strcpy(des_ipv4, argv[1]);
            login(des_ipv4); 
        }
    }
    else
    {
        printf("Too many arguments!\n");
        exit(0);
    }
    // signal(SIGINT, SIG_IGN);
    while (1)
    {
        token_num = 0;
        cmd_line = readline("ftp > ");  
        if(strlen(cmd_line) > 0) {
            add_history(cmd_line);
            token_num = tokenize_command(cmd_line, tokens);
        }

        if(token_num != 0) {        
            for(i=0; i<COMMAND_NUM; ++i) {
                if(strcmp(tokens[0], commands[i].name) == 0) {
                    cmd_func[i](tokens, token_num);
                    break;
                }
            }
            if(i == COMMAND_NUM) {
                printf("Invalid!\n");
            }
        }
        free(cmd_line);
    }
    return 0; 
}


