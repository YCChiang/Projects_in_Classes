#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

static struct option long_options[] =
{  
    {"help", optional_argument, NULL, 'h'},
    {"string",  optional_argument, NULL,'s'}
};

char *optstr = "h:s:";

char *help_msg = "This program uses PDA to recognize string.\n"\
    "Usage:\n\tpda.exe [options]\n\n"\
    "General Options:\n"\
    "\t-h, --help\t\tShow help\n"\
    "\t-s, string\t\tThe string to be recognized.\n";

int is_accepted(char *input_str) {
    int i, len, stack_ptr, state; 
    char *stack;
    len = strlen(input_str);
    stack = (char*)malloc(sizeof(char)*(len+1));    // add a space for $ in buttom of stack
    memset(stack, 1, sizeof(char)*(len+1));
    stack[0] = '$';
    state = 2;
    stack_ptr = 1;
    for(i=0; i<len; i++) {
        switch (input_str[i])
        {
        case 'a':
            break;
        case 'b':
            if(state == 2) {
                state = 3;
                stack[stack_ptr++] = 'b';
            }else if(state == 3) {
                stack[stack_ptr++] = 'b';
            }else if (state == 4) {
                stack_ptr--;
                if(stack_ptr == 1) {
                    state = 2;
                }
            }            
            break;
        case 'c':
            if(state == 2) {
                state = 4;
                stack[stack_ptr++] = 'c';
            }else if(state == 3) {
                stack_ptr--;
                if(stack_ptr == 1) {
                    state = 2;
                }
            }else if (state == 4) {
                stack[stack_ptr++] = 'c';             
            }
            break;
        default:
            printf("Wrong input %c", input_str[i]);
            exit(0);
        }
    }
    if(stack_ptr == 1 && stack[0] == '$') {
        return 1;
    }
    return 0;
}


int main(int argc, char **argv) {
    char *input_str = "aabbaacc";
    char c;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
        switch (c)
        {
            case 'h':
                printf("%s", help_msg);
                abort();
                break;
            case 's':
                input_str = optarg;
                break;
            case '?':
                abort();
            default:
                abort();
        }
    }
    printf("The string: %s\n", input_str);
    if(is_accepted(input_str))
        printf("%s is accpted by the NFA!\n", input_str);
    else
        printf("%s is not accpted by the NFA!\n", input_str);
    return 0;
}