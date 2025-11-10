#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int read_fd[2]){
    close(read_fd[1]);  //关闭传入管道的写通道，因为子进程只需要读管道的内容

    int new_read_fd[2];
    pipe(new_read_fd);  //创建新的管道
    int p;
    int state = read(read_fd[0], &p, sizeof(int)); //取管道中的第一个数，一定是素数

    if(state > 0){
        printf("prime %d\n", p); //如果第一个数能读到数据，那就是素数，打印

        int next_pid = fork();  //创建子进程
        if(next_pid > 0){   //父进程
            close(new_read_fd[0]); 
            for(;;){    //循环，读取传入管道中的所有数
                int n;
                int read_state = read(read_fd[0], &n, sizeof(int));
                if(read_state == 0) break;  //读取完跳出循环
                if(n%p != 0){   //如果符合素数算法，传入新的管道
                    write(new_read_fd[1], &n, sizeof(int));
                }
            }
            close(read_fd[0]);
            close(new_read_fd[1]);  //关闭此管道的写，让子进程能够读
        
            wait(0);    //此时我是父进程，等待子进程
        }else if(next_pid == 0){    //子进程继续调用此函数，开始递归孙进程
            close(read_fd[0]);
            primes(new_read_fd);
        
        }
    
    }else{  //如果传入的管道没有数据，则关闭所有管道的通道
        close(read_fd[0]);
        close(new_read_fd[0]);
        close(new_read_fd[1]);
    }

    exit(0);    //如果传入的管道为空，直接结束递归
}


int main(){
    int read_fd[2]; 
    pipe(read_fd);  //初始管道

    int pid = fork();

    if(pid < 0){
        printf("创建子进程失败\n");

    }else if(pid == 0){ //子进程
        primes(read_fd);
    
    }else{  //父进程
        close(read_fd[0]);
        for(int send_num=2; send_num<36; send_num++){   //把2~35传入管道
            write(read_fd[1], &send_num, sizeof(int));
        }       
        close(read_fd[1]);      //关闭管道写，使得子进程能够读取管道内容
        
        wait(0);    // 等待第一个子进程（以及它的所有后代）结束
    }

    exit(0);
}