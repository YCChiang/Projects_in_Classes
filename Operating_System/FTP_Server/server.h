#include <shadow.h>
#include <pwd.h>
#include <assert.h>

// #define _XOPEN_SOURCE
#define COMMAND_NUM 12
#define DEFAULT_DIR "/var/ftp"
#define ANON_USER "anonymous"
#define MAX_PATH 60

struct
{
    char *name;
    char *password;
    char cur_dir[MAX_PATH];
    int isSignIn;
    int control_socket;
    int data_socket;
    int data_fd;
} user;


void send_data(char *cmd, int remove);
int login();
int isSignIn();
void close_dataconn();
void init_info();
void FTP_Server();

void dostor(char *tokens[], int token_num);
void doretv(char *tokens[], int token_num);
void douser(char *tokens[], int token_num);
void dopass(char *tokens[], int token_num);
void disconnect(char *tokens[], int token_num);
void doport(char *tokens[], int token_num);
void dopasv(char *tokens[], int token_num);
void docwd(char *tokens[], int token_num);
void dolist(char *tokens[], int token_num);
void dopwd(char *tokens[], int token_num);

