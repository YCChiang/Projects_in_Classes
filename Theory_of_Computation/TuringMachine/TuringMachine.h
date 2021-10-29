#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define BUFFSIZE 100

#define CONFIG_FILE "TM_configuration.txt"

#define BLANK 0
#define CROSS_OFF 24
#define ALL 7

typedef struct node {
    char input_alphabet;    // 输入符号
    char tape_alphabet;     // 带子符号
    char move;              // 移动方向
    int next_state;         // 能达到的状态
    struct node *next;      // 单链表下一个节点
} NODE;

typedef struct tm {
    int state_number;           // 状态集的大小
    char **states;              // 状态集
    int start_state;            // 起始状态在状态集中的索引
    int *accept_states;         // 接受状态集，都是状态集中的索引
    int accept_state_number;    // 接受状态集的大小
    NODE** shift_array;         // 转移函数，每一个状态都有一个单链表，代表从此状态出发，输入符号能到达的状态
} TuringMachine;

static struct option long_options[] =
{  
    {"help", optional_argument, NULL, 'h'},
    {"file", optional_argument, NULL, 'f'},
    {"string",  optional_argument, NULL,'s'}
};

char *optstr = "h:s:f:";

char *help_msg = "This program uses Turing Machine to recognize string.\n"\
    "Usage:\n\tTuringMachine.exe [options]\n\n"\
    "General Options:\n"\
    "\t-h, --help\t\tShow help\n"\
    "\t-s, string\t\tThe string to be recognized.\n"\
    "\t-f, -file\t\tThe Turing Machine configuration file.\n\n";
