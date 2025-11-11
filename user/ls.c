#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path) //这个函数的作用是从一个可能很长的路径中，只提取出最后的文件名部分，并将其格式化为固定的长度。
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash. 寻找最后一个 '/'
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;  // p 现在指向最后一个'/'之后的字符，即文件名的开头

  // Return blank-padded name.  处理文件名过长的情况
  if(strlen(p) >= DIRSIZ)
    return p;

  //格式化文件名（用空格填充）
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));

  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // 1. 打开文件/目录
  if((fd = open(path, 0)) < 0){ //如果打开失败
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }
  // 2. 获取文件/目录的元数据 (metadata)
  if(fstat(fd, &st) < 0){ // 将一个打开文件的信息放入 *st
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){   // 3. 根据类型进行不同处理
  case T_FILE:  //如果是文件 2
    //打印格式化的文件名、类型、inode号、大小
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR: // 如果是目录 1
    //检查路径长度，防止缓冲区溢出
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    //构造用于读取目录项的完整路径
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/'; //在路径末尾加上斜杠，例如 "dir" -> "dir/"

    //循环读取目录中的每一项
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)  //inum为0表示这个目录项是空闲的,跳过此目录
        continue;
      //构造子项的完整路径
      memmove(p, de.name, DIRSIZ);  // 将目录项的名字追加到 "dir/" 后面
      p[DIRSIZ] = 0;  // 添加字符串结束符
      // 现在 buf 可能是 "dir/file1"

      //获取子项的元数据
      if(stat(buf, &st) < 0){ //注意stat和fstat，一个是子项一个是目录
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      //打印子项的信息
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
