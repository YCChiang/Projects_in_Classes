#include "server.h"
#include "ftp.h"

// 执行命令之后，获得返回结果并发送给客户端
void send_data(char *cmd, int remove) {
    FILE *fp = NULL;
    char data[100] = {'0'};
    char result[DATA_BUF_SIZE+1] = {0};
    int data_len;

    // 切换到用户指定文件夹
    chdir(user.cur_dir);

    // 执行命令
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen error\n");
        return ;
    }
    // 获得返回结果
    while (fgets(data, sizeof(data)-2, fp) != NULL)
    {
        if(remove > 0) {
            remove--;
            continue;
        }
        data_len = strlen(data);
        data[data_len-1] = '\r';
        data[data_len] = '\n';
        data[data_len+1] = '\0';

        // 如果积累的数据大于数据缓存，发送数据清空数据缓存
        if(strlen(result)+data_len+2 > DATA_BUF_SIZE) {
            send(user.data_socket, result, DATA_BUF_SIZE, 0);
            result[0] = '\0';
        }
        strcat(result, data);
    }
    pclose(fp);
}


// 登录验证，
int login() {
    char cmd_buff[CMD_BUF_SIZE+1];
    // char salt[512]={0};
    // struct spwd *sp = getspnam(user.name);
    // struct passwd *pw;
    // if(sp == NULL)
    // {
    //     printf("get spentry error\n");
    //     sprintf(cmd_buff, "530 Login incorrect.\r\n");
    //     send(user.control_socket, cmd_buff, strlen(cmd_buff), 0); 
    // }
    // else {
    //     get_salt(salt,sp->sp_pwdp);
    //     if(strcmp(sp->sp_pwdp, (char*)crypt(user.password, salt)) == 0) 
    //     {
    //         printf("yes\n");
    //         pw = getpwnam(user.name);
    //         assert(pw);
    //         strcpy(user.cur_dir, pw->pw_dir);
    //         setuid(pw->pw_uid);
    //         user.isSignIn = 1;
    //         sprintf(cmd_buff, "230 Login successful.\r\n");
    //         send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);  
    //     }
    //     else
    //     {
    //         printf("no\n");
    //         sprintf(cmd_buff, "530 Login incorrect.\r\n");
    //         send(user.control_socket, cmd_buff, strlen(cmd_buff), 0); 
    //     }
    // }
    user.isSignIn = 1;
    sprintf(cmd_buff, "230 Login successful.\r\n");
    send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);  
}

// 判断用户是否登录。未登录则发送错误并请求用户登录
int isSignIn() {
    if(user.isSignIn== 0) {
        char cmd_buff[CMD_BUF_SIZE+1];
        sprintf(cmd_buff, "530 Please login with USER and PASS.\r\n");
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        return 0;
    }
    return 1;
}

// 接受客户端发送文件数据
void dostor(char *tokens[], int token_num) {
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1];
        char data_buff[DATA_BUF_SIZE+1] = {0};
        int file_fd;
        
        // 未开启数据传输通道，请求客户重新建立数据连接
        if(user.data_socket == -1){
            sprintf(cmd_buff, "425 Use PORT or PASV first.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
        else{
            sprintf(cmd_buff, "150 Ok to send data.\r\n");            
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);

            chdir(user.cur_dir);
            file_fd = open(tokens[1], O_CREAT | O_WRONLY, S_IRUSR | S_IRWXU);
            recv_file(user.data_socket, tokens[1], file_fd);
            close_dataconn();

            sprintf(cmd_buff, "226 Transfer complete.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);            
        }
    }
}

// 发送文件给客户端
void doretv(char *tokens[], int token_num) {
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1];
        char data_buff[DATA_BUF_SIZE+1] = {0};
        int file_fd;

        // 未开启数据传输通道，请求客户重新建立数据连接
        if(user.data_socket == -1){
            sprintf(cmd_buff, "425 Use PORT or PASV first.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
        else{            
            chdir(user.cur_dir);
            // 文件无法打开，发送错误给客户端并关闭数据连接
            if((file_fd = open(tokens[1], O_RDONLY, S_IRUSR)) < 0) {
                sprintf(cmd_buff, "550 Failed to open file.\r\n");
                send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
                close_dataconn();
            }
            else {
                sprintf(cmd_buff, "150 Opening BINARY mode data connection for %s.\r\n", tokens[1]);
                send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);  

                send_file(user.data_socket, tokens[1], file_fd);
                close_dataconn();
                    
                sprintf(cmd_buff, "226 Transfer complete.\r\n");
                send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
            }
        }
    }
}

// 接收用户名称并写入全局变量中
void douser(char *tokens[], int token_num) {
    int recvbytes, sendbytes; 
    char cmd_buff[CMD_BUF_SIZE+1];
    user.name = (char *)malloc(sizeof(strlen(tokens[1]+1)));
    strcpy(user.name, tokens[1]);
    sprintf(cmd_buff, "331 Please specify the password.\r\n");
    send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
}


// 接收用户密码并写入全局变量中
void dopass(char *tokens[], int token_num) {
    char cmd_buff[CMD_BUF_SIZE+1];
    user.password = (char *)malloc(sizeof(strlen(tokens[1]+1)));
    strcpy(user.password, tokens[1]);
    if(strcmp(user.name, ANON_USER) != 0)
        login();
    else {
        user.isSignIn = 1;
        sprintf(cmd_buff, "230 Login successful.\r\n");
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    }
}

// 断开与客户端的控制连接并接受此子进程
void disconnect(char *tokens[], int token_num){
    char buffer[CMD_BUF_SIZE+1];
    sprintf(buffer, "221 Goodbye.\r\n");
    send(user.control_socket, buffer, strlen(buffer), 0);        
    if(user.control_socket != -1) {
        close(user.control_socket);
        user.control_socket = -1;
    }
    exit(0);
}

// 与客户端用主动模式建立数据连接
void doport(char *tokens[], int token_num) {
    int ser_port;
    struct sockaddr_in cli_addr;
    int addrlen;
    int recvbytes, sendbytes; 
    char cmd_buff[CMD_BUF_SIZE+1];
    char ipv4[INET_ADDRSTRLEN] = {0};
    unsigned int a1, a2, a3, a4, p1, p2;

    // 判断接受到的ip地址和端口是否正确
    if (sscanf(tokens[1], "%u,%u,%u,%u,%u,%u",
               &a1, &a2, &a3, &a4, &p1, &p2) != 6 ||
        a1 > 255 || a2 > 255 || a3 > 255 || a4 > 255 ||
        p1 > 255 || p2 > 255 || (a1|a2|a3|a4) == 0 ||
        (p1 | p2) == 0) {
        sprintf(cmd_buff, "501 Syntax error in IP address\r\n");
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    }
    else {
        sprintf(ipv4, "%u.%u.%u.%u", a1, a2, a3, a4);
        ser_port = (p1 << 8) | p2;
        user.data_socket = create_client_socket(DATA_PORT, ser_port, ipv4);
        if(user.data_socket != -1) {
            sprintf(cmd_buff,"200 PORT command successful. Consider using PASV.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
            user.mode = PORT_MODE;
        }
    }
}

// 与客户端用被动模式建立数据连接
void dopasv(char *tokens[], int token_num) {
    char cmd_buff[CMD_BUF_SIZE+1];
    unsigned int p = rand()%3976 + 1024;
    struct sockaddr_in cli_addr;
    int addrlen = sizeof(cli_addr);
    char *local_ip;

    user.data_fd = create_server_socket(p);
    if(user.data_fd == -1) {
        // 数据连接建立错误，通知客户端
        sprintf(cmd_buff, "421 Unable to open a passive connection.\r\n");
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    }
    else {
        // 获取与客户端连接的本机ip地址
        getsockname(user.control_socket, (struct sockaddr *)&cli_addr, (socklen_t*)&addrlen);
        local_ip = inet_ntoa(cli_addr.sin_addr);

        string_replace(local_ip, INET_ADDRSTRLEN, '.', ',');
        sprintf(cmd_buff, "227 Entering Passive Mode (%s,%u,%u).\r\n", local_ip, (p >> 8) & 255, p & 255);
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);

        if ((user.data_socket = accept(user.data_fd, (struct sockaddr*)&cli_addr,  (socklen_t*)&addrlen))<0) { 
            perror("accept error"); 
        }
    }    
}

// 改变当前用户文件夹路径
void docwd(char *tokens[], int token_num) {    
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1] = {0};
        char path[MAX_PATH] = {0};
        char *p;
        switch(tokens[1][0]) {
            case '/':
                strcpy(path, tokens[1]);
                break;
            case '.':
                strcpy(path, user.cur_dir);
                if(tokens[1][1] == '.') {
                    p = strrchr(path, '/');
                    if(p!=path)
                        *p = '\0';
                    else 
                        *(p+1) = '\0';
                }
                break;
            case '~':
                strcpy(path, DEFAULT_DIR);
                break;
            default :
                strcpy(path, user.cur_dir);
                strcat(path, "/");
                strcat(path, tokens[1]);
                break;
        }

        if(access(path, F_OK) == 0) {
            strcpy(user.cur_dir, path);
            sprintf(cmd_buff, "250 Directory successfully changed.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
        else {
            // 如果文件夹不存在或没有权限， 发送错误给客户端
            sprintf(cmd_buff, "550 Can't change directory to %s\r\n", path);
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
    }
}

// 获得当前用户文件夹路径下所有文件信息
void dolist(char *tokens[], int token_num) {
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1];
        char data_buff[DATA_BUF_SIZE+1] = {0};
        int i = 0;
        if(user.data_socket == -1){
            sprintf(cmd_buff, "425 Use PORT or PASV first.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
        else{
            sprintf(cmd_buff, "150 Here comes the directory listing.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);  

            sprintf(cmd_buff, "ls -l -n");
            for(i=1; i<token_num; ++i) {
                strcat(cmd_buff, " ");
                strcat(cmd_buff, tokens[i]);
            }
            send_data(cmd_buff, 1);
            close_dataconn();
            
            sprintf(cmd_buff, "226 Directory send OK.\r\n");
            send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
        }
    }
}

// 确认登录后，获取当前
void dopwd(char *tokens[], int token_num) {
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1];
        char path[MAX_PATH];
        sprintf(cmd_buff, "257 \"%s\" is the current directory\r\n", user.cur_dir);
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    }
}

// 确认登录后，删除单个文件
void dodelete(char *tokens[], int token_num) {
    if(isSignIn()) {
        char cmd_buff[CMD_BUF_SIZE+1];
        chdir(user.cur_dir);
        if(remove(tokens[1]) == 0)
            sprintf(cmd_buff, "250 Delete operation successful.\r\n");
        else
            sprintf(cmd_buff, "550 Delete operation failed.\r\n");
        send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    }
}

// FTP协议中功能关键字
static char commands[COMMAND_NUM][5] = {
    "CWD", "PWD", "LIST", "USER", "PASS", "PASV", "PORT", "QUIT", "RETR", "STOR", "DELE"
};

// 关键字对应调用的功能函数
static function cmd_func[COMMAND_NUM] = {
    docwd, dopwd, dolist, douser, dopass, dopasv, doport,disconnect, doretv, dostor, dodelete
};

// 关闭数据传输嵌套字
void close_dataconn(){
    if(user.data_socket != -1) {
        close(user.data_socket);
        user.data_socket = -1;
    }
    if(user.data_fd != -1) {
        close(user.data_fd);
        user.data_fd= -1;
    }
}

// 初始化全局变量
void init_info() {
    user.isSignIn = 0;
    user.data_socket = -1;
    user.name = NULL;
    user.password = NULL;
    user.data_fd = -1;
    strcpy(user.cur_dir, DEFAULT_DIR);
}

void FTP_Server() {
    int recvbytes, sendbytes; 
    char cmd_buff[CMD_BUF_SIZE+1];
    char *tokens[CMD_BUF_SIZE/3+1];
    int token_num = 0, i=0;
    sprintf(cmd_buff, "220 (myFTP 1.0)\r\n");
    sendbytes = send(user.control_socket, cmd_buff, strlen(cmd_buff), 0);
    while (1)
    {
        if(user.control_socket == -1)
            break;
        
        recvbytes = recv(user.control_socket, cmd_buff, CMD_BUF_SIZE, 0);
        if(recvbytes < 0)
            perror("recive error\n");
        else
        {
            cmd_buff[recvbytes-2] = '\0';
            printf("%s\n", cmd_buff);
            token_num = 0;
            token_num = tokenize_command(cmd_buff, tokens);     
            
            for(i=0; i<COMMAND_NUM; ++i) {
                if(strcmp(tokens[0], commands[i]) == 0) {
                    cmd_func[i](tokens, token_num);
                    break;
                }
            }
        }
    }   
}

int main() {
    int command_fd;
    struct sockaddr_in cli_addr;
    int addrlen;
    pid_t pid;
    command_fd = create_server_socket(COMMAND_PORT);
    
    while (command_fd > 0)
    {
        addrlen = sizeof(cli_addr);
        if ((user.control_socket = accept(command_fd, (struct sockaddr*)&cli_addr,  (socklen_t*)&addrlen))<0) { 
            perror("accept error"); 
        }
        else {
            if((pid = fork()) < 0){
                perror("fork");
            }
            else {
                if(pid != 0) {
                    init_info();
                    FTP_Server();
                }         
            }
        } 
    }
    return 0;
}