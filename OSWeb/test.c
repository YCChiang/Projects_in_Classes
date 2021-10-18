#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// make bash command
char *command = "echo Hello WikiStack";
int main() {
    FILE *p;
    p = popen(command, "r");
    if (!p) {
        fprintf(stderr, "Error opening pipe.\n");
        return 1;
    }
    char string[100];
    printf("%s\n", command);
    while(fgets(string, 100, p)) {
        printf("%s\n", string);
    }
    if (pclose(p) == -1) {
        fprintf(stderr, " Error!\n");
        return 1;
    }
    return 0;
}
