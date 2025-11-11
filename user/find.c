#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"  //导入dirent

// 从 ls.c 借鉴的辅助函数：从完整路径中提取文件名
// 例如：从 "a/b/c.txt" 提取 "c.txt"
char* get_fmtname(char *path)
{
    char *p;
    // Find first character after last slash. 寻找最后一个 '/'
    for(p=path+strlen(path); p >= path && *p != '/'; p--);

    p++;  // p 现在指向最后一个'/'之后的字符，即文件名的开头

    //返回文件的文件名
    return p;
}



void find(char *path, const char* filename){

    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 1. 打开文件/目录
    if((fd = open(path, 0)) < 0){ //如果打开失败
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 2. 获取文件/目录的元数据 (metadata)
    if(fstat(fd, &st) < 0){ // 将一个打开文件的信息放入 *st
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:  //如果是文件 case 2:
        if(strcmp(filename, get_fmtname(path)) == 0){   //文件和目标文件进行匹配，如果找到就打印路径
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        //检查路径长度，防止缓冲区溢出
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
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

            //!!!!!!重要，不要在"."和".."文件目录递归，不然会无限递归
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) 
                continue;

            //构造子项的完整路径
            memmove(p, de.name, DIRSIZ);  // 将目录项的名字追加到 "dir/" 后面
            p[DIRSIZ] = 0;  // 添加字符串结束符
            // 现在 buf 可能是 "dir/file1"

            /*  find 和 ls的区别，find在此处获取子项元数据，因为下面会递归，在新的find里面会获取子项的元数据
            //获取子项的元数据
            if(stat(buf, &st) < 0){ //注意stat和fstat，一个是子项一个是目录
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            */

            // **递归调用**：对这个新的完整路径继续调用 find
            find(buf, filename);
        }
        break;  

    default:
        break;
    }

    close(fd);

}

//参考ls.c 与之不同的是find 命令需要两个参数：要搜索的目录和要查找的文件名
int main(int argc, char *argv[]){

    if(argc != 3){  //如果用户传入的参数格式不正确，告知正确的格式
        fprintf(2, "Usage: find <directory> <filename>\n");
        exit(1); //错误退出
    }

    find(argv[1], argv[2]); //传入目录和文件名

    exit(0);
}

