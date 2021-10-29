#include "PDA.h"

/*打印PDA所有设置参数*/
void print_PDA(PDA* pda){
    int i;
    NODE* shift_func;
    printf("state number:%d\nstates:", pda->state_number);
    for(i=0; i<pda->state_number; i++)
        printf("%s,",pda->states[i]);
    printf("\nstart state:%s\naccept state number:%d\naccept state(s):",
        pda->states[pda->start_state],
        pda->accept_state_number);
    for(i=0; i<pda->accept_state_number; i++)
        printf("%s,",pda->states[pda->accept_states[i]]);
    printf("\nshift_function:\n");
    for(i=0; i<pda->state_number; i++) {
        shift_func = pda->shift_array[i];
        while (shift_func)
        {
            printf("%s->%s:%c,%c->%c\n",pda->states[i], pda->states[shift_func->next_state], shift_func->alphabet, shift_func->out_stack,shift_func->in_stack);
            shift_func = shift_func->next;
        }       
    }
}

/*释放PDA空间*/
void free_PDA(PDA* pda) {
    int i;
    NODE *prev, *next;
    for(i=0; i<pda->state_number; i++) {
        if(pda->states[i])
            free(pda->states[i]);
    }

    if(pda->accept_states)
        free(pda->accept_states);
    for(i=0; i<pda->state_number; i++) {
        prev = pda->shift_array[i];
        if(prev) {
            next = prev->next;
            while (next) {
                free(prev);
                prev = next;
                next = prev->next;
            }
            free(prev);
        }        
    }
    if(pda->shift_array)
        free(pda->shift_array);
    free(pda);
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

char get_alphabet(char *str){
    if(strcmp(str, "epsilon")==0) 
        return '\0';
    
    else
        return str[0];
}

/**
 * @brief		获得输入文件中PDA的设定数据
 * @param[in]	fd      文件指针    已经打开的文件的指针
 * @param[in]   number  整数        状态集长度
 * @retval		1       成功
 * @retval		0       错误
 */
int read_info(FILE *fd, PDA* pda) {
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
                pda->state_number = atoi(buff);
            }
            else 
                result = 0;
        }
        else if (strcmp(buff, "accept_state_number:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                pda->accept_state_number = atoi(buff);
            }
            else {
                printf("accept_state_number has no vlaue!\n");
                result = 0;
            }                
        }
        else if(strcmp(buff, "states:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                pda->states = get_elements(buff, pda->state_number);
            }
            else 
                result = 0;
        }        
        else if (strcmp(buff, "start_state:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                temp = get_idx(buff, pda->states, pda->state_number);
                if(temp!=-1) {
                    pda->start_state = temp;
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
                str_array = get_elements(buff, pda->accept_state_number);
                pda->accept_states = (int*)malloc(sizeof(int)*pda->accept_state_number);
                k = 0;
                for(i=0; i<pda->accept_state_number; i++) {
                    temp = get_idx(str_array[i], pda->states, pda->state_number);
                    if(temp!=-1)
                        pda->accept_states[k++] = temp;
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
            pda->shift_array = (NODE**)malloc(sizeof(NODE*)*pda->state_number);
            for(i=0; i<pda->state_number; i++) {
                pda->shift_array[i] = NULL;
            }
            ret = fgets(buff, BUFFSIZE, fd);    // skip the line: current_state,next_state,input_alphabet,out_stack,in_stack
            ret = fgets(buff, BUFFSIZE, fd);  
            while (ret!=NULL) {
                str_array = get_elements(buff, 5);
                i = get_idx(str_array[0], pda->states, pda->state_number);
                j = get_idx(str_array[1], pda->states, pda->state_number);
                node = (NODE*)malloc(sizeof(NODE));
                node->next_state = j;
                node->alphabet = get_alphabet(str_array[2]);
                node->out_stack = get_alphabet(str_array[3]);
                node->in_stack = get_alphabet(str_array[4]);                
                next = pda->shift_array[i];
                node->next = next;
                pda->shift_array[i] = node;                
                ret = fgets(buff, BUFFSIZE, fd);
            }
        }
        ret = fgets(buff, BUFFSIZE, fd);
    }
    return result;
}

/**
 * @brief	    初始化PDA的设定数据
 * @param[in]	input_file  字符串  文件名字
 * @retval		pda地址 成功
 * @retval		NULL    错误
 */
PDA* init_PDA(char* input_file) {
    FILE *fd = NULL;
    PDA* pda=NULL;
    if((fd = fopen(input_file, "r")) != NULL)
    {
        pda = (PDA*)malloc(sizeof(PDA));
        if(!read_info(fd, pda)) {
            free_PDA(pda);
            pda = NULL;
        }
        fclose(fd);
    }
    else
        printf("Cannot open the %s file!", input_file);
    return pda;
}

/**
 * @brief		状态转移函数
 * @param[in]	current_state   整数            当前输入串所在的状态
 * @param[in]   input           字符            输入字符
 * @param[in]   input_idx       整数            输入字符的索引
 * @param[in]   stack           字符数组        字符堆栈
 * @param[in]   stack_idx       整数            字符堆栈索引
 * @param[in]   pda             PDA结构体指针   PDA的设定
 * @retval		数组地址        成功
 */
int shift_function(int current_state, int input_idx, char input,
    char *stack, int stack_idx, PDA* pda){
    int i,j=0, tmp_idx;
    int result = 0, is_shift = 0;

    // add a space for $ in buttom of stack
    char *tmp_stack = (char*)malloc(sizeof(char)*(pda->input_len+1));

    // 当全部字符已经输入，开始判断是否可以接收此字符串
    if(input_idx == pda->input_len) {
        for(j=0; j<pda->accept_state_number; j++) {
            i = pda->accept_states[j];
            if(i==current_state && stack_idx==0) {
                result = 1;
            }
        }
    }
    else {
        NODE* shift_func = pda->shift_array[current_state];
        while (shift_func)
        {
            if(shift_func->alphabet == input) {
                if (shift_func->out_stack == '\0' || shift_func->out_stack == stack[stack_idx-1])
                {
                    tmp_idx = stack_idx;
                    memcpy(tmp_stack, stack, sizeof(char)*(pda->input_len+1));
                    if(shift_func->out_stack == stack[stack_idx-1]) {
                        tmp_idx--;
                    }
                    is_shift = 1;
                    if(shift_func->in_stack != '\0') {
                        tmp_stack[tmp_idx++] = shift_func->in_stack;
                    }
                    //输入epsilon
                    result = shift_function(shift_func->next_state, input_idx, '\0', tmp_stack, tmp_idx, pda);
                    // 已经有一个分支被接受，无需再计算其他分支
                    if(result)  break;
                    
                    //输入下一个字符
                    result = shift_function(shift_func->next_state, input_idx+1, pda->input_str[input_idx+1], tmp_stack, tmp_idx, pda);
                    // 已经有一个分支被接受，无需再计算其他分支
                    if(result)  break;
                }
            }
            shift_func = shift_func->next;
        }
        // 如果此次输入字符没有产生任何状态转移，则拒绝该分支
        if(!is_shift) result = 0;
    }
    return result;
}

/**
 * @brief		获得输入字符串中所有用逗号分隔的子串
 * @param[in]	input_str   字符串          需要
 * @param[in]   pda         PDA结构体指针   PDA的设定
 * @retval		1    成功
 * @retval		0    错误
 */
int is_accepted(char *input_str, PDA *pda) {
    int i, j, len, stack_idx;    
    char *stack;
    
    pda->input_str = input_str;
    pda->input_len = strlen(input_str);

    // add a space for $ in buttom of stack
    stack = (char*)malloc(sizeof(char)*(len+1));
    memset(stack, 0, sizeof(char)*(len+1));
    stack_idx = 0;

    return shift_function(pda->start_state, 0, '\0', stack, stack_idx, pda);
}

int main(int argc, char **argv) {
    char *input_file = CONFIG_FILE;
    char *input_str = "aabbaacc";
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
                printf("%s", help_msg);
                abort();
            default:
                abort();
        }
    }
    printf("PDA configuration file: %s\nThe string: %s\n", input_file, input_str);
    PDA *pda = init_PDA(input_file);
    if(pda) {
        // print_PDA(pda);
        if(is_accepted(input_str, pda))
            printf("%s is accpted by the PDA!\n", input_str);
        else
            printf("%s is not accpted by the PDA!\n", input_str);
        free_PDA(pda);
    }
    else
        printf("%s: Cannot read the file or wrong data format in the file\n", input_file);
    return 0;
}