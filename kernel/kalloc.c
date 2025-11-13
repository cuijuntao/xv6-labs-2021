// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"



//这份代码的核心是将所有空闲的物理内存页组织成了一个单向链表 （每个页大小为4096字节）
//然后我们就可以通过遍历链表来获取有多少空闲内存

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {  //链表的节点
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist; //这是一个全局的指针，它始终指向这个空闲链表的头节点。kmem.freelist
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


uint64 freemem_amount(void){  //获取空闲内存
  struct run *r;  //定义头结点
  uint64 count = 0;

  // 我们需要获取锁来安全地遍历链表，
  // 防止在遍历过程中有其他CPU正在修改它。
  acquire(&kmem.lock);

  r = kmem.freelist;
  while (r)
  {
    count++;        // 每有一个节点，就计数一次
    r = r->next;    // 移动到下一个节点
  }
  release(&kmem.lock);
  
  // 每个节点代表一个页面，所以总字节数是 页面数 * 页面大小 PGSIZE就是页面大小4096
  return count*PGSIZE;
}
