#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define COMMAND_NUM 16
#define CHOOSE_PORT rand()%3000+49152

int control_socket, data_socket;
int control_port, data_fd; // control port
int mode;

struct {
    char *name;
    char *discrption;
} commands[COMMAND_NUM] =  {
    {"?", "Print local help information"},
    {"bye", " Terminate ftp session and exit"},
    {"cd", "Change the working directory."},
    {"close", "Terminate ftp session and exit"},
    {"delete", "Delete remote file"},
    {"dir", "List contents of remote directory"},
    {"disconnect", "Terminate ftp session"},
    {"get", "Retrieve the file and store it on the local machine"},
    {"help", "Print local help information"},
    {"ls", "List contents of remote directory"},
    {"open", "Connect to remote ftp"},
    {"passive", "Enter passive transfer mode"},
    {"put", "Store a local file on the remote machine"},
    {"pwd", " Print working directory on remote machine"},    
    {"quit", "Terminate ftp session and exit"},    
    {"sendport", "toggle use of PORT cmd for each data connection"}
};

void get(char *tokens[], int token_num);
void put(char *tokens[], int token_num);
void get_path(char *tokens[], int token_num);
void list_contents(char *tokens[], int token_num);
void change_dir(char *tokens[], int token_num);
void connect_server(char *tokens[], int token_num);
void passive(char *tokens[], int token_num);
void quit(char *tokens[], int token_num);
void disconnect(char *tokens[], int token_num);
void help(char *tokens[], int token_num);
void delete_file(char *tokens[], int token_num);
void sendport(char *tokens[], int token_num);

int login(char des_ipv4[]);
void doport();
void dopasv();
void close_dataconn();
void recv_data();