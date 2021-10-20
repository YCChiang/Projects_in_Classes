#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>


#define BUFFSIZE 100

#define CONFIG_FILE "PDA_configuration.txt"

typedef struct node {
    int next_state;     // 能达到的状态
    char alphabet;      // 输入符号
    char in_stack;      // 入栈符号
    char out_stack;     // 出栈符号
    struct node *next;  // 单链表下一个节点
} NODE;

typedef struct pda {
    int state_number;           // 状态集的大小
    char **states;              // 状态集
    int start_state;            // 起始状态在状态集中的索引
    int *accept_states;         // 接受状态集，都是状态集中的索引
    int accept_state_number;    // 接受状态集的大小
    char *input_str;            // 需要识别的字符
    int input_len;              // 当前字符长度
    NODE** shift_array;         // 转移函数，每一个状态都有一个单链表，代表从此状态出发，输入符号能到达的状态
} PDA;

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
