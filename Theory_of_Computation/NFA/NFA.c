#include "NFC.h"


/*打印NFA所有设置参数*/
void print_NFA(NFA* nfa){
    int i;
    NODE* shift_func;
    printf("state number:%d\nstates:", nfa->state_number);
    for(i=0; i<nfa->state_number; i++)
        printf("%s,",nfa->states[i]);
    printf("\nstart state:%s\naccept state number:%d\naccept state(s):",
        nfa->states[nfa->start_state],
        nfa->accept_state_number);
    for(i=0; i<nfa->accept_state_number; i++)
        printf("%s,",nfa->states[nfa->accept_states[i]]);
    printf("\nshift_function:\n");
    for(i=0; i<nfa->state_number; i++) {
        shift_func = nfa->shift_array[i];
        while (shift_func)
        {
            printf("%s->%s:%c\n",nfa->states[i], nfa->states[shift_func->next_state], shift_func->alphabet);
            shift_func = shift_func->next;
        }       
    }
}

/**
 * @brief		获得输入状态的在状态集里的编号
 * @param[in]	state   字符串      输入状态
 * @param[in]   states  字符串数值  状态集
 * @param[in]   number  整数        状态集长度
 * @retval		>=0		成功
 * @retval		-1	    错误
 */
int get_idx(char* state,char** states, int number){
    int i;
    for(i=0; i<number; i++) {
        if(strcmp(state, states[i]) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief		获得输入字符串中所有用逗号分隔的子串
 * @param[in]	input   字符串      里面的子串由逗号分隔
 * @param[in]   number  整数        状态集长度
 * @retval		数组地址    成功
 * @retval		NULL       错误
 */
char** get_elements(char* input,int number) {
    int i=0, j=0, k=0;
    char** strs = (char**)malloc(sizeof(char*)*number);
    char* temp = NULL;
    while (input[i]!='\0')
    {
        if(input[i]==',') {
            temp = (char*)malloc(sizeof(char)*(i-j+1));
            memcpy(temp, input+j, (i-j));
            temp[i-j] = '\0';
            strs[k++] = temp;
            j = i+1;
        }
        i++;
    } 
    return strs;   
}

/**
 * @brief		获得输入文件中NFA的设定数据
 * @param[in]	fd      文件指针    已经打开的文件的指针
 * @param[in]   number  整数        状态集长度
 * @retval		1       成功
 * @retval		0       错误
 */
int read_info(FILE *fd, NFA* nfa) {
    char *ret, **str_array, buff[BUFFSIZE];
    int len, i, j, k, temp, result = 1;
    NODE *node, *next;
    ret = fgets(buff, BUFFSIZE, fd);
    while (ret!=NULL && result)
    {
        if (strcmp(buff, "state_number:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                nfa->state_number = atoi(buff);
            }
            else 
                result = 0;
        }
        else if (strcmp(buff, "accept_state_number:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                nfa->accept_state_number = atoi(buff);
            }
            else {
                printf("accept_state_number has no vlaue!\n");
                result = 0;
            }                
        }
        else if(strcmp(buff, "states:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                nfa->states = get_elements(buff, nfa->state_number);
            }
            else 
                result = 0;
        }        
        else if (strcmp(buff, "start_state:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                temp = get_idx(buff, nfa->states, nfa->state_number);
                if(temp!=-1) {
                    nfa->start_state = temp;
                }                    
                else {
                    printf("The start state %s is not in the states array!", buff);
                    result = 0;
                }
            }
            else 
                result = 0;
        }
        else if (strcmp(buff, "accept_state(s):\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                str_array = get_elements(buff, nfa->accept_state_number);
                nfa->accept_states = (int*)malloc(sizeof(int)*nfa->accept_state_number);
                k = 0;
                for(i=0; i<nfa->accept_state_number; i++) {
                    temp = get_idx(str_array[i], nfa->states, nfa->state_number);
                    if(temp!=-1)
                        nfa->accept_states[k++] = temp;
                    else {
                        printf("The state %s is not in the states array!", str_array[i]);
                        result = 0;
                        break;
                    } 
                }
            }
            else 
                result = 0;
        }
        else if (strcmp(buff, "shift_function:\n") == 0) {
            nfa->shift_array = (NODE**)malloc(sizeof(NODE*)*nfa->state_number);
            for(i=0; i<nfa->state_number; i++) {
                nfa->shift_array[i] = NULL;
            }
            ret = fgets(buff, BUFFSIZE, fd);    // skip the line: current_state,node_state,input_alphabet
            ret = fgets(buff, BUFFSIZE, fd);    
            while (ret!=NULL) {
                str_array = get_elements(buff, 3);
                i = get_idx(str_array[0], nfa->states, nfa->state_number);
                j = get_idx(str_array[1], nfa->states, nfa->state_number);
                node = (NODE*)malloc(sizeof(NODE));
                node->next_state = j;
                if(strcmp(str_array[2], "epsilon")==0) {
                    node->alphabet = '\0';
                }
                else
                    node->alphabet = str_array[2][0];
                next = nfa->shift_array[i];
                node->next = next;
                nfa->shift_array[i] = node;                
                ret = fgets(buff, BUFFSIZE, fd);
            }
        }
        ret = fgets(buff, BUFFSIZE, fd);
    }
    return result;
}

/**
 * @brief	    初始化NFA的设定数据
 * @param[in]	input_file  字符串  文件名字
 * @retval		nfa地址 成功
 * @retval		NULL    错误
 */
NFA* init_NFA(char* input_file) {
    FILE *fd = NULL;
    NFA* nfa=NULL;
    if((fd = fopen(input_file, "r")) != NULL)
    {
        nfa = (NFA*)malloc(sizeof(NFA));
        if(!read_info(fd, nfa)) {
            free(nfa);
            nfa = NULL;
        }
        fclose(fd);
    }
    return nfa;
}

void print_stack(int *state_stack, int number) {
    int i;
    for(i=0; i<number; i++) {
        printf("%d\t", state_stack[i]);
    }
    printf("\n");
}

/**
 * @brief	    合并两个状态数组
 * @param[in]	current_stack   整数数组    目前的状态数组
 * @param[in]   state_stack     整数数组    新输入的需要被合并的状态数组
 * @param[in]   number          整数        状态数组长度
 */
void merge_stake(int *current_stack,int *state_stack, int number) {
    int i;
    for(i=0; i<number; i++) {
        if(current_stack[i] || state_stack[i])
            current_stack[i] = 1;
    }
}

/**
 * @brief		状态转移函数
 * @param[in]	current_state   整数            当前输入串所在的状态
 * @param[in]   input           字符            输入字符
 * @param[in]   nfa             NFA结构体指针   NFA的设定
 * @retval		数组地址        成功
 */
int* shift_function(int current_state, char input, NFA* nfa){
    int i,j=0;
    NODE* shift_func = nfa->shift_array[current_state];
    int *state_stack, *temp_stack;
    state_stack = (int *)malloc(sizeof(int)*nfa->state_number);
    memset(state_stack, 0, sizeof(int)*nfa->state_number);
    while (shift_func)
    {
        if(shift_func->alphabet == input) {
            state_stack[shift_func->next_state] = 1;
            temp_stack = shift_function(shift_func->next_state, '\0', nfa);
            merge_stake(state_stack, temp_stack, nfa->state_number);
            free(temp_stack);
        }
        shift_func = shift_func->next;
    }
    return state_stack;
}

/**
 * @brief		获得输入字符串中所有用逗号分隔的子串
 * @param[in]	input_str   字符串          需要
 * @param[in]   nfa         NFA结构体指针   NFA的设定
 * @retval		1    成功
 * @retval		0    错误
 */
int is_accepted(char *input_str, NFA *nfa) {
    int *current_stack, *state_stack, *prev_stack;
    int i, j;    
    prev_stack = (int*)malloc(sizeof(int)*nfa->state_number);
    memset(prev_stack, 0, sizeof(int)*nfa->state_number);
    prev_stack[nfa->start_state] = 1;

    printf("input");
    for(i=0; i<nfa->state_number; i++)
        printf("\t%s",nfa->states[i]);
    printf("\n\t");

    print_stack(prev_stack, nfa->state_number);
    for(i=0; i<strlen(input_str); i++) {        
        current_stack = (int*)malloc(sizeof(int)*nfa->state_number);
        memset(current_stack, 0, sizeof(int)*nfa->state_number);
        for(j=0; j<nfa->state_number; j++) {
            if(prev_stack[j]) {
                state_stack = shift_function(j, input_str[i], nfa);
                merge_stake(current_stack, state_stack, nfa->state_number);
            }
        }
        free(prev_stack);
        prev_stack = current_stack;
        printf("%c\t", input_str[i]);
        print_stack(prev_stack, nfa->state_number);
    }
    for(j=0; j<nfa->accept_state_number; j++) {
        i = nfa->accept_states[j];
        if(prev_stack[i]) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]){
    char *input_file = "NFA_configuration.txt";
    char *input_str = "01011110";
    char c;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
        switch (c)
        {
            case 'h':
                printf("%s", help_msg);
                abort();
            case 's':
                input_str = optarg;
                break;
            case 'f':
                input_file = optarg;
                break;
            case '?':
                abort();
            default:
                abort();
        }
    }
    printf("NFA configuration file: %s\nThe string: %s\n", input_file, input_str);
    NFA* nfa = init_NFA(input_file);
    if(nfa) {
        if(is_accepted(input_str, nfa))
            printf("%s is accpted by the NFA!", input_str);
        else
            printf("%s is not accpted by the NFA!", input_str);

        free(nfa);
    }
    else
        printf("%s: Cannot read the file or wrong data format in the file\n", input_file);
    return 0;
}