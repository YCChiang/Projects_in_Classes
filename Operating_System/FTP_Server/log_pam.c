/*
    此验证用户代码未完成，差一个与pam通信的自定义pam_conv结构
    没有此结构，只能直接调用默认通信结构，只能命令行输入验证密码
    无法直接通过程序传入密码验证
*/
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <stdio.h>

// 定义一个pam_conv结构,用于与pam通信
static struct pam_conv conv = {
misc_conv, NULL
};

int pam_check(char *name, char *pass) {
    pam_handle_t *pamh=NULL;
    int retval;
    const char *user="nobody";
    const char *s1=NULL;
    
    if(name == NULL)
        return -1;
    
    user = name;
    
    printf("user: %s\n",user);

    retval = 0;
    
    //调用pamtest配置文件
    retval = pam_start("pamtest", user, &conv, &pamh);
    
    if (retval == PAM_SUCCESS)    
        //进行auth类型认证
        retval = pam_authenticate(pamh, 0); 
    else {
        //如果认证出错,pam_strerror将输出错误信息.
        printf("pam_authenticate(): %d\n",retval);
        s1=pam_strerror( pamh, retval);
        printf("%s\n",s1);
    }

    if (retval == PAM_SUCCESS)    
        //进行account类型认证
        retval = pam_acct_mgmt(pamh, 0); 
    else {
        printf("pam_acct_mgmt() : %d\n",retval);
        s1 = pam_strerror(pamh, retval);
        printf("%s\n",s1);
    }
    
    if (retval == PAM_SUCCESS) {
        fprintf(stdout, "Authenticated\n");
    } else {
        fprintf(stdout, "Not Authenticated\n");
    }
    
    // 关闭PAM
    if (pam_end(pamh,retval) != PAM_SUCCESS) { 
        pamh = NULL;
        fprintf(stderr, "pamtest0: failed to release authenticator\n");
        exit(1);
    }
    
    return ( retval == PAM_SUCCESS ? 0:1 );
}