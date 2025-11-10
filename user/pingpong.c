#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define Message 16

int main(){

    int first[2];   //父进程->子进程
    int second[2];  //子进程->父进程
    char buf[Message];

    pipe(first);    //创建两个管道
    pipe(second);

    int pid = fork();   //创建子进程
    if(pid < 0){
        close(first[0]);
        close(first[1]);    
        close(second[0]);  
        close(second[1]); 
        exit(1);       //异常退出

    }else if(pid == 0){   //子进程
        close(first[1]);    
        close(second[0]);
        read(first[0], buf, Message);
        if(buf[0] == 'a'){
            printf("%d: received ping\n", getpid());    //注意打印换行符
        }else{
            printf("子进程接收失败\n");    //注意打印换行符
        }
        close(first[0]);

        write(second[1], "b", Message);
        close(second[1]);   //关闭second管道的写入端,让父进程的read不再阻塞

    
    }else{  //父进程
        close(first[0]);    //关闭first管道的读取端
        close(second[1]);   //关闭second管道的写入端
        write(first[1], "a", Message);
        close(first[1]);    //关闭写入端，让子进程的read不再阻塞

        read(second[0], buf, Message);
        if(buf[0] == 'b'){
            printf("%d: received pong\n", getpid());    //注意打印换行符
        }else{
            printf("父进程接收失败\n");    //注意打印换行符
        }
        close(second[0]);

    }


    exit(0);
}