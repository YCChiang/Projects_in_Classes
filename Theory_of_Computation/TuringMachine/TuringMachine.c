#include "TuringMachine.h"

/*打印TuringMachine所有设置参数*/
void print_TuringMachine(TuringMachine* tm){
    int i;
    NODE* shift_func;
    printf("state number:%d\nstates:", tm->state_number);
    for(i=0; i<tm->state_number; i++)
        printf("%s,",tm->states[i]);
    printf("\nstart state:%s\naccept state number:%d\naccept state(s):",
        tm->states[tm->start_state],
        tm->accept_state_number);
    for(i=0; i<tm->accept_state_number; i++)
        printf("%s,",tm->states[tm->accept_states[i]]);
    printf("\nshift_function:\n");
    for(i=0; i<tm->state_number; i++) {
        shift_func = tm->shift_array[i];
        while (shift_func)
        {

            printf("%s->%s:",tm->states[i], tm->states[shift_func->next_state]);
            if(shift_func->input_alphabet < 32)
                printf("%d", shift_func->input_alphabet);
            else
                printf("%c", shift_func->input_alphabet);
            printf("->");
            if(shift_func->tape_alphabet < 32)
                printf("%d", shift_func->tape_alphabet);
            else
                printf("%c", shift_func->tape_alphabet);
            printf(",%c\n", shift_func->move);
            shift_func = shift_func->next;
        }
    }
}

/*释放TuringMachine空间*/
void free_TuringMachine(TuringMachine* tm) {
    int i;
    NODE *prev, *next;
    for(i=0; i<tm->state_number; i++)
        free(tm->states[i]);
    free(tm->accept_states);
    for(i=0; i<tm->state_number; i++) {
        prev = tm->shift_array[i];
        next = prev->next;
        while (next) {
            free(prev);
            prev = next;
            next = prev->next;
        }
        free(prev);
    }
    free(tm->shift_array);
    free(tm);
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
        temp = NULL;
        if(input[i]==',') {
            if(i != j) {
                temp = (char*)malloc(sizeof(char)*(i-j+1));
                memcpy(temp, input+j, (i-j));
                temp[i-j] = '\0';
            }
            strs[k++] = temp;
            j = i+1;
        }
        i++;
    } 
    return strs;   
}

/**
 * @brief		处理文件中的字符串
 * @param[in]	str     字符串    读取到的字符串
 * @retval		char    字符串对应的保留字符
 */
char get_alphabet(char *str){
    if(str == NULL) 
        return '\0';
    if(strcmp(str, "blank")==0)
        return BLANK;
    else if(strcmp(str, "cross_off")==0)
        return CROSS_OFF;
    else if(strcmp(str, "all")==0)
        return ALL;
    else
        return str[0];
}

/**
 * @brief		获得输入文件中TuringMachine的设定数据
 * @param[in]	fd      文件指针    已经打开的文件的指针
 * @param[in]   number  整数        状态集长度
 * @retval		1       成功
 * @retval		0       错误
 */
int read_info(FILE *fd, TuringMachine* tm) {
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
                tm->state_number = atoi(buff);
            }
            else 
                result = 0;
        }
        else if(strcmp(buff, "states:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                tm->states = get_elements(buff, tm->state_number);
            }
            else 
                result = 0;
        } 
        else if (strcmp(buff, "accept_state_number:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                tm->accept_state_number = atoi(buff);
            }
            else {
                printf("accept_state_number has no vlaue!\n");
                result = 0;
            }                
        }       
        else if (strcmp(buff, "start_state:\n") == 0) {
            if(fgets(buff, BUFFSIZE, fd) != NULL) {
                len = strlen(buff);
                buff[len-1] = '\0';
                temp = get_idx(buff, tm->states, tm->state_number);
                if(temp!=-1) {
                    tm->start_state = temp;
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
                str_array = get_elements(buff, tm->accept_state_number);
                tm->accept_states = (int*)malloc(sizeof(int)*tm->accept_state_number);
                k = 0;
                for(i=0; i<tm->accept_state_number; i++) {
                    temp = get_idx(str_array[i], tm->states, tm->state_number);
                    if(temp!=-1)
                        tm->accept_states[k++] = temp;
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
            tm->shift_array = (NODE**)malloc(sizeof(NODE*)*tm->state_number);
            for(i=0; i<tm->state_number; i++) {
                tm->shift_array[i] = NULL;
            }
            ret = fgets(buff, BUFFSIZE, fd);    // current_state,next_state,input_alphabet,tape_alphabet,move
            ret = fgets(buff, BUFFSIZE, fd);  
            while (ret!=NULL) {
                str_array = get_elements(buff, 5);
                i = get_idx(str_array[0], tm->states, tm->state_number);
                j = get_idx(str_array[1], tm->states, tm->state_number);
                node = (NODE*)malloc(sizeof(NODE));
                node->next_state = j;
                node->input_alphabet = get_alphabet(str_array[2]);
                node->tape_alphabet = get_alphabet(str_array[3]);
                node->move = get_alphabet(str_array[4]);
                next = tm->shift_array[i];
                node->next = next;
                tm->shift_array[i] = node;                
                ret = fgets(buff, BUFFSIZE, fd);
            }
        }
        ret = fgets(buff, BUFFSIZE, fd);
    }
    return result;
}

/**
 * @brief	    初始化TuringMachine的设定数据
 * @param[in]	input_file  字符串  文件名字
 * @retval		tm地址 成功
 * @retval		NULL    错误
 */
TuringMachine* init_TuringMachine(char* input_file) {
    FILE *fd = NULL;
    TuringMachine* tm=NULL;
    if((fd = fopen(input_file, "r")) != NULL)
    {
        tm = (TuringMachine*)malloc(sizeof(TuringMachine));
        if(!read_info(fd, tm)) {
            free(tm);
            tm = NULL;
        }
        fclose(fd);
    }
    else
        printf("Cannot open the %s file!", input_file);
    return tm;
}

/**
 * @brief	    判断输入字符是否是保留字符
 * @param[in]	c   字符
 * @retval		1   是保留字符
 * @retval		0   不是保留字符
 */
int is_reserved_alphabet(char c) {
    int ret = 0;
    switch (c)
    {
    case BLANK:
        ret = 1;
        break;
    case CROSS_OFF:
        ret = 1;
        break;
    case ALL:
        ret = 1;
        break;
    case '+':
        ret = 1;
        break;
    case '=':
        ret= 1;
        break;
    default:
        break;
    }
    return ret;
}

/**
 * @brief		状态转移函数
 * @param[in]	current_state   整数            当前输入串所在的状态
 * @param[in]   input           字符            输入字符
 * @param[in]   input_idx       整数            输入字符的索引
 * @param[in]   tm             TuringMachine结构体指针   TuringMachine的设定
 * @retval		数组地址        成功
 */
int shift_function(int current_state,char* input_str, int *input_idx, TuringMachine* tm){
    int i, next_state = -1, need_change = 0;
    NODE *shift_func, *tmp;

    shift_func = tm->shift_array[current_state];
    tmp = NULL;

    while (shift_func)
    {
        if(shift_func->input_alphabet == ALL) {
            if(is_reserved_alphabet(input_str[*input_idx])){
                shift_func =shift_func->next;
                continue;
            }
            need_change = 1;
        }
        else {
            if(shift_func->input_alphabet == input_str[*input_idx]) {
                need_change = 1;
            }
        }
        if(need_change) {
            next_state = shift_func->next_state;
            if(shift_func->tape_alphabet != '\0') {
                input_str[*input_idx] = shift_func->tape_alphabet;
            }
            switch (shift_func->move)
            {
            case 'R':
                (*input_idx)++;
                break;
            case 'L':
                (*input_idx)--;
                break;
            }
            break;
        }
        shift_func = shift_func->next;
    }

    return next_state;
}

/**
 * @brief		获得输入字符串中所有用逗号分隔的子串
 * @param[in]	input_str   字符串          需要
 * @param[in]   tm         TuringMachine结构体指针   TuringMachine的设定
 * @retval		1    成功
 * @retval		0    错误
 */
int is_accepted(char *input_str, TuringMachine *tm) {
    int i, input_idx, current_state;    
    char *input;

    i = strlen(input_str);

    // copy the string to a new space and add # in front of the string
    input = (char *)malloc(sizeof(char)*(i+2));
    input[0] = '#';
    memcpy(input+1, input_str, sizeof(char)*(i+1));
    current_state = tm->start_state;
    input_idx = 0;
    
    while (current_state >= 0)
    {
        current_state = shift_function(current_state, input, &input_idx, tm);
        for (i=0; i<tm->accept_state_number; i++) {
            if(current_state == tm->accept_states[i]) {
                return 1;
            }
        }
    }
    return 0;
}


int main(int argc, char **argv) {
    char *input_file = CONFIG_FILE;
    char *input_str = "aa+bbaa=ccaabb";
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
    printf("TuringMachine configuration file: %s\nThe string: %s\n", input_file, input_str);
    TuringMachine *tm = init_TuringMachine(input_file);
    if(tm) {
        // print_TuringMachine(tm);
        if(is_accepted(input_str, tm))
            printf("%s is accpted by the Turing Machine!\n", input_str);
        else
            printf("%s is not accpted by the Turing Machine!\n", input_str);
        free_TuringMachine(tm);
    }
    else
        printf("%s: Cannot read the file or wrong data format in the file\n", input_file);
    return 0;
}