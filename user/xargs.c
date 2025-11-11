#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

//make grade
//echo hello too | xargs echo bye

// mkdir a
// echo hello > a/b
// mkdir c
// echo hello > c/b
// echo hello > b
// find . b | xargs grep hello
//sh < xargstest.sh



#define Message 512

// xargs 就是为了“将一个命令的输出，转成另一个命令的参数”,而不是一个命令的标准输出， 转为另一个命令的标准输入
int main(int argc, char* argv[]){
    // sleep(100);  //本意是find执行的比较慢，可能xargs后面的指令执行了，但是find还没输出，导致没有标准输入，等待一会即刻
    //但是由于不知道标准输入之前的指令具体会花费多久，这里手动的sleep并不好，进行了下面的优化

    //把xargs 后面的第一个参数作为接下来新的命令
    char* command = argv[1];
    char* new_argv[MAXARG]; //MAXARG 是exec能传入的最大参数数量
    int arg_index = 0;
    //首先把xargs自带的参数（除了xargs）放入新的参数列表,包括后面的命令
    //比如 echo hello\nhello | xargs echo bye，new_argv[]里面应该是echo bye
    for(int i=1; i<argc; i++){
        new_argv[arg_index++] = argv[i];
    }

    char buf[Message];  //存放标准化输入的一行
    //--------------------------sleep优化------------------------------------------
    int buf_size = read(0, buf, sizeof(buf));
    int buf_idx = 0;
    //在这里循环读取标准输入，知道find进程和xargs之间的管道为空
    while (buf_size > 0)
    {
        buf_idx += buf_size;
        buf_size = read(0, &buf[buf_idx], Message);
    }
    buf_size = buf_idx; //这里要把buf_idx重新赋值给buf_size，因为上面循环完之后，buf_size=0，buf_idx里面才是真正的buf大小
    //------------------------------------------------------------------------
    char* p = buf;  //指针指向数组头地址

    for(int i=0; i<buf_size; i++){
        if(buf[i] == '\n'){
            buf[i] = 0; // 将换行符替换为字符串结束符
            new_argv[arg_index] = p;
            new_argv[arg_index+1] = 0;    //exec 需要一个以NULL结尾的参数列表

            int pid = fork();
            if(pid < 0){
                fprintf(2, "创建子进程失败\n");
                exit(1);
            }else if(pid == 0){
                //子进程执行exec
                exec(command, new_argv);

                fprintf(2, "exec %s 失败\n", command);
                exit(1);
            }else{
                wait(0);    //父进程等待子进程执行结束
                p = &buf[i+1];  //指针移动到标准输入换行符的下一个位置
            }
        }
    }
    
    if (*p != '\0') {   //防止xargs前面进程的标准输出没有输出字符串结束符而卡死
        new_argv[arg_index] = p;
        new_argv[arg_index + 1] = 0;

        if (fork() == 0) {
            exec(command, new_argv);
            fprintf(2, "exec %s failed\n", command);
            exit(1);
        }
        wait(0);
    }   

    exit(0);
}