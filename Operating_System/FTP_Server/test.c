#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>

int get_local_ip(char *ips) {
        struct ifaddrs *ifAddrStruct;
        void *tmpAddrPtr=NULL;
        char ip[INET_ADDRSTRLEN];
        int n = 0;
        getifaddrs(&ifAddrStruct);
        while (ifAddrStruct != NULL) {
            if (ifAddrStruct->ifa_addr->sa_family==AF_INET) {
                tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
                printf("%s IP Address:%s\n", ifAddrStruct->ifa_name, ip);
                if (strcmp(ip, "127.0.0.1") != 0) {
                    if (n == 0){
                        strcpy(ips, ip);
                    }
                }
            }
            ifAddrStruct=ifAddrStruct->ifa_next;
        }
        //free ifaddrs
        freeifaddrs(ifAddrStruct);
        return 0;
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


int main()
{
    struct passwd *pw;
    uid_t uid;
    char *login;

    uid = getuid();
    assert(uid != -1);
    printf("UID is %d\n", uid);

    login = getlogin();
    assert(login != NULL);
    printf("User is %s\n", login);

    pw = getpwuid(uid);
    assert(pw != NULL);
    printf("\n\n\n");
    printf("getpwuid:\n");
    printf("name = %s\nuid = %d\ngid = %d\nhome = %s\nshell = %s\n", pw->pw_name, pw->pw_uid, pw->pw_gid, pw->pw_dir, pw->pw_shell);

    pw = getpwnam("test");
    assert(pw);
    printf("\n\n\n");
    printf("getpwnam(root):\n");
    printf("name = %s\nuid = %d\ngid = %d\nhome = %s\nshell = %s\npassword=%s\n", 
        pw->pw_name, pw->pw_uid, pw->pw_gid, pw->pw_dir, pw->pw_shell, pw->pw_passwd);
    return 0;
}