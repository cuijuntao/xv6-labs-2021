#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 vaddr;
  int len;
  uint64 uaddr; //传进来的用户地址
  if(argaddr(0, &vaddr) < 0)  //接收传进来的第一个参数，是一个地址，下面同理
    return -1;
  if(argint(1, &len) < 0) //接收要检查的页面数
    return -1;
  if(argaddr(2, &uaddr) < 0)
    return -1;
  if(len > 32 || len < 0){  //如果传进来的页面数超过了范围
    return -1;
  }
  struct proc *p = myproc();  //获取当前进程
  int bitmask = 0;  //临时缓冲区，存放输出位掩码

  for(int i=0; i<len; i++){
    uint64 va = vaddr + i * PGSIZE; //计算当前页的起始虚拟地址

    // 【新增】必须防止 va 越界，否则 walk 会 panic
    if(va >= MAXVA)
      return 0;

    pte_t *pte = walk(p->pagetable, va, 0); //通过walk函数查找页表项

    if(pte!=0 && (*pte & PTE_V) && (*pte & PTE_A)){ //查看pte是否有效和被访问过
      bitmask = bitmask | (1<<i); //如果被访问过，将bitmask的第i位置1

      *pte = *pte & ~PTE_A; //然后清零  此时的PTE_A一定是1，取反为0，也就是把*pte的对应PTE_A置0
    }
  }

  //调用copyout，将数据从内核态传回用户态
  if(copyout(p->pagetable, uaddr, (char *)&bitmask, sizeof(bitmask)) < 0)
    return -1;

  return 0;
}
#endif

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
