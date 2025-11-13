#include "types.h"
#include "riscv.h"
#include "defs.h" //在这里引用了我们写的获取空闲内存和空闲进程数的函数
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"  //引入sysinfo结构体

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64
sys_trace(void)
{
  int mask; //掩码

  if(argint(0, &mask) < 0)  //从参数0获取值
    return -1;

  // 把获得掩码存进进程的这个结构体里面，方面在syscall.c里面调用
  struct proc *p = myproc();  //获取现在的进程
  p->trace_mask = mask;

  return 0;
}


uint64
sys_sysinfo(void) //收集有关正在运行的系统的信息
{
  struct sysinfo info;  //我们把空闲内存和空闲进程数添加到sysinfo结构体中
  info.freemem = freemem_amount();
  info.nproc = freeproc_amount();

  uint64 addr;  //地址指针
  if(argaddr(0, &addr) < 0) //接收用户空间传过来的虚拟地址
    return -1;

  struct proc *p = myproc();
  //p就是目标进程，在这里我们用目标进程的页表p->pagetable来接收用户空间传过来的虚拟地址addr，在这里我们把后续的数据开始写入
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)    //我们把info结构体里面的数据拷贝过去到页表中
    return -1;

  return 0;
}