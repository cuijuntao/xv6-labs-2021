// 物理内存布局说明

// qemu -machine virt 的内存映射如下（参考 qemu 的 hw/riscv/virt.c）：
//
// 0x00001000 -- 启动 ROM，由 qemu 提供
// 0x02000000 -- CLINT（Core Local Interruptor，核心本地中断器）
// 0x0C000000 -- PLIC（Platform-Level Interrupt Controller，平台级中断控制器）
// 0x10000000 -- uart0 串口
// 0x10001000 -- virtio 硬盘
// 0x80000000 -- 启动 ROM 在机器模式下跳转到此地址
//                 -kernel 参数加载内核到这里
// 未使用的 RAM 位于 0x80000000 之后

// 内核使用物理内存的方式：
// 0x80000000 -- entry.S, 然后是内核文本段和数据段
// end -- 内核页面分配区起始
// PHYSTOP -- 内核使用的 RAM 末尾

// qemu 将 UART 寄存器映射到物理内存
#define UART0 0x10000000L
#define UART0_IRQ 10  // UART 中断号

// virtio mmio 接口
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1  // virtio 中断号

#ifdef LAB_NET
#define E1000_IRQ 33  // 网络设备中断号
#endif

// 核心本地中断器 (CLINT)，包含定时器
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid)) // 每个 hart 的定时器比较寄存器
#define CLINT_MTIME (CLINT + 0xBFF8) // 自开机以来的时钟周期数

// qemu 将平台级中断控制器 (PLIC) 映射在此地址
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)  // 中断优先级寄存器
#define PLIC_PENDING (PLIC + 0x1000) // 待处理中断寄存器
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100) // M 模式使能寄存器
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100) // S 模式使能寄存器
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000) // M 模式优先级
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000) // S 模式优先级
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000) // M 模式中断认领寄存器
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000) // S 模式中断认领寄存器

// 内核期望有 RAM 用于内核和用户页面
// 从物理地址 0x80000000 到 PHYSTOP
#define KERNBASE 0x80000000L
#define PHYSTOP (KERNBASE + 128*1024*1024) // 内核可用物理内存末尾

// 将 trampoline 页面映射到最高地址
// 用户态和内核态都使用同一页面
#define TRAMPOLINE (MAXVA - PGSIZE)

// 将内核栈映射在 trampoline 下方
// 每个栈周围都有保护页
#define KSTACK(p) (TRAMPOLINE - (p)*2*PGSIZE - 3*PGSIZE)

// 用户内存布局
// 从地址 0 开始：
//   text 段
//   原始数据和 bss
//   固定大小栈
//   可扩展堆
//   ...
//   USYSCALL (与内核共享)
//   TRAPFRAME (p->trapframe, trampoline 使用)
//   TRAMPOLINE (与内核使用同一页面)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
#ifdef LAB_PGTBL
#define USYSCALL (TRAPFRAME - PGSIZE)

struct usyscall {
  int pid;  // 进程 ID
};
#endif
