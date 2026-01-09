//lab1-4
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long uint64;
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
// 递归查找文件的函数
// path: 当前要搜索的目录路径
// filename: 要查找的目标文件名
void find(char *path, char *filename) {
    char buf[512], *p; // buf用于存放拼接后的路径，p是指针
    int fd;            // 文件描述符
    struct dirent de;  // 目录条目结构
    struct stat st;    // 文件状态结构

    // 尝试打开当前路径
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取路径对应的文件状态信息
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

// 判断文件类型
    switch (st.type) {
    case T_FILE: // 如果是普通文件
        // 从完整路径中提取出最后的文件名部分
        for (p = path + strlen(path); p >= path && *p != '/'; p--)
            ;
        p++; // p现在指向文件名部分的起始位置

        // 比较提取出的文件名和目标文件名是否一致
        if (strcmp(p, filename) == 0) {
            printf("%s\n", path); // 如果匹配，打印完整路径
        }
        break;
        case T_DIR: // 如果是目录
        // 检查路径长度是否合法，防止缓冲区溢出
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("find: path too long\n");
            break;
        }
        // 将当前路径复制到缓冲区
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/'; // 在路径末尾添加一个 '/'

        // 读取目录中的每一个条目
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            // 跳过无效条目和特殊目录 "." 和 ".."
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            // 将当前目录条目的名称拼接到路径后面
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0; // 确保字符串以 null 结尾

            // 递归调用 find，查找新拼接的路径
            find(buf, filename);
        }
        break;
    }
    close(fd); // 关闭文件描述符
}

int main(int argc, char *argv[]) {
    // 参数检查：必须提供搜索路径和文件名
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }
    // 调用 find 函数开始查找
    find(argv[1], argv[2]);
    exit(0);
}