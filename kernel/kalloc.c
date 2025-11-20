// 物理内存分配器，用于用户进程、内核栈、页表页和管道缓冲区。
// 分配整页（4096 字节）的物理内存。

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // 内核数据段结束后的第一个地址
                   // 在 kernel.ld 中定义

// 链表节点，表示一个空闲内存页
struct run {
  struct run *next;
};

// 内核内存管理结构
struct {
  struct spinlock lock;  // 保护 freelist 的自旋锁
  struct run *freelist;  // 空闲物理页链表头
} kmem;

// 初始化内存分配器
void
kinit()
{
  initlock(&kmem.lock, "kmem");   // 初始化自旋锁
  freerange(end, (void*)PHYSTOP); // 将内核之后到物理内存末尾的内存加入空闲链表
}

// 将 [pa_start, pa_end) 范围内的物理内存加入空闲链表
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  // 将起始地址向上取整到页边界
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);  // 将每页加入空闲链表
}

// 释放一页物理内存 pa
// pa 必须是通过 kalloc 分配的页地址
// kinit 初始化时也会调用 kfree
void
kfree(void *pa)
{
  struct run *r;

  // 检查地址是否对齐并在有效范围内
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 用垃圾数据填充该页，用于捕捉悬空引用
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);    // 获取锁
  r->next = kmem.freelist; // 插入到空闲链表头
  kmem.freelist = r;
  release(&kmem.lock);    // 释放锁
}

// 分配一页 4096 字节的物理内存
// 返回内核可用的指针，分配失败返回 0
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);      // 获取锁
  r = kmem.freelist;        // 取链表头的空闲页
  if(r)
    kmem.freelist = r->next; // 更新链表头
  release(&kmem.lock);       // 释放锁

  if(r)
    memset((char*)r, 5, PGSIZE); // 用垃圾数据填充已分配页
  return (void*)r;               // 返回页地址
}
