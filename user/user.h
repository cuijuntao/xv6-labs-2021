struct stat;
struct rtcdate;

// system calls 这部分声明的函数是用户程序与操作系统内核 (Kernel) 沟通的唯一桥梁。
//当用户程序调用这些函数时，它会触发一个特殊的硬件中断，使得CPU从用户态切换到内核态，由内核来执行相应的特权操作
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**); // 用一个新程序替换当前进程的内存映像
int open(const char*, int);
int mknod(const char*, short, short); // 创建一个设备文件。
int unlink(const char*);    // 从文件系统中删除一个名字（及文件本身，如果没有其他链接）。
int fstat(int fd, struct stat*);    // 通过文件描述符获取文件状态。
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*); // 更改当前工作目录。
int dup(int);   // 复制一个文件描述符。
int getpid(void);
char* sbrk(int);    // 增加或减少程序的堆内存空间。
int sleep(int); // 让进程暂停指定的“ticks”数。
int uptime(void);   // 获取系统自启动以来的“ticks”数。

int trace(int);

// ulib.c
//这部分声明的函数是在用户空间实现的，它们不直接与内核交互（虽然它们内部可能会调用系统调用）。它们是标准的C语言库函数的简化版，为用户程序提供了便利。
int stat(const char*, struct stat*); // 这是一个库函数“外壳”。它内部会调用 open, fstat, close 系统调用来通过路径名获取文件状态。
char* strcpy(char*, const char*); // 复制字符串。
void *memmove(void*, const void*, int); // 移动内存区域（可以处理重叠区域）。
char* strchr(const char*, char c); // 在字符串中查找一个字符。
int strcmp(const char*, const char*); // 比较两个字符串。
void fprintf(int, const char*, ...); // 向指定文件描述符打印格式化字符串。
void printf(const char*, ...); // 向标准输出（fd=1）打印格式化字符串。它内部会调用 fprintf(1, ...)。
char* gets(char*, int max); // 从标准输入读取一行。
uint strlen(const char*); // 计算字符串长度。
void* memset(void*, int, uint); // 用一个字节填充内存区域。
void* malloc(uint); // 在堆上分配指定大小的内存。它内部会调用 sbrk 系统调用。
void free(void*); // 释放由 malloc 分配的内存。
int atoi(const char*); // 将字符串转换为整数。
int memcmp(const void *, const void *, uint); // 比较两个内存区域。
void *memcpy(void *, const void *, uint); // 复制内存区域（不能处理重叠区域）。
