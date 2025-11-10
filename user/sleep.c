#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc, char *argv[]){

    if(argc != 2){   //如果用户忘记传递参数或者传入多参数，sleep打印一条错误信息
        fprintf(2, "Usage: sleep <ticks>\n");   //Usage："用法是"
        exit(1);        
    }
    // 使用 atoi 函数将字符串形式的命令行参数 (argv[1]) 转换为整数。
    int ticks = atoi(argv[1]);

    // 调用 sleep 系统调用，sleep 函数的实现在内核中，这里只是通过用户库调用它
    sleep(ticks);

    exit(0);
}