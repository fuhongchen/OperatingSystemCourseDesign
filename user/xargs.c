// lab1-5
typedef unsigned int uint;

#include "user/user.h" 
#include "kernel/param.h" 

int main(int argc, char *argv[]) {
    char buf[512];          // 缓冲区，用于临时存储从输入读取的每个参数
    char *args[MAXARG];     // 参数指针数组，用于传递给 exec 系统调用
    int n;                  // 读取操作的返回值，表示读取的字节数
    int i = 0;              // 通用循环计数器
    int arg_count;          // 当前参数数组中的参数数量
    char c;                 // 用于逐个读取字符的变量
    int pos = 0;            // 当前在 buf 缓冲区中的写入位置

    // 参数检查
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [initial-args...]\n");
        exit(1); // 报错并异常退出
    }

    // 初始化参数列表，复制原始参数
    for (arg_count = 0; arg_count < argc - 1; arg_count++) {
        args[arg_count] = argv[arg_count + 1];
    }
     // 主循环：从标准输入逐个字符读取数据
    while ((n = read(0, &c, 1)) > 0) {
        // 检查当前字符是否是参数分隔符（空格或换行符）
        if (c == ' ' || c == '\n') {
            // 如果缓冲区中有内容，说明这是一个完整的参数
            if (pos > 0) {
                buf[pos] = '\0'; // 在字符串末尾添加终止符
                
                // 防止参数数组溢出
                if (arg_count >= MAXARG - 1) {
                    fprintf(2, "xargs: too many arguments\n");
                    exit(1);
                }
                
                // 为新参数动态分配内存并复制内容
                args[arg_count] = malloc(strlen(buf) + 1);
                strcpy(args[arg_count], buf);
                arg_count++; // 参数计数器增加
                args[arg_count] = 0; // exec要求参数列表以NULL指针结尾
                pos = 0; // 重置缓冲区位置，准备接收下一个参数
            } // end if (pos > 0)
            // 如果是换行符，执行命令
            if (c == '\n') {
                // 检查是否有新参数加入，有才执行
                if (arg_count > argc - 1) {
                    // 创建子进程来执行命令
                    if (fork() == 0) {
                        // 子进程：执行目标命令
                        exec(args[0], args);
                        // 如果exec执行失败则报错并异常退出
                        fprintf(2, "xargs: exec %s failed\n", args[0]);
                        exit(1);
                    } else {
                        // 父进程：等待子进程执行完毕
                        wait(0);
                    }
                } //end if (arg_count > argc - 1)
                               // 重置参数列表：释放本轮添加的参数内存，保留原始参数
                for (i = argc - 1; i < arg_count; i++) {
                    free(args[i]); // 释放动态分配的内存
                }
                // 重置参数计数器，只保留原始参数
                arg_count = argc - 1;
            } //end if (c == '\n')
        }// end  if (c == ' ' || c == '\n')
       else {
            // 普通字符（非分隔符），添加到缓冲区
            if (pos < sizeof(buf) - 1) {
                buf[pos++] = c; // 存储字符并移动位置指针
            }
        }
    } //end while
    // 处理最后一行输入（如果输入不是以换行符结尾）
    if (pos > 0) {
        buf[pos] = '\0'; // 添加字符串终止符
        
        // 检查参数数组容量
        if (arg_count >= MAXARG - 1) {
            fprintf(2, "xargs: too many arguments\n");
            exit(1);
        }
        
        // 添加最后一个参数
        args[arg_count] = malloc(strlen(buf) + 1);
        strcpy(args[arg_count], buf);
        arg_count++;
        args[arg_count] = 0; // 确保以NULL结尾
        
        // 创建子进程执行命令
        if (fork() == 0) {
            exec(args[0], args);
            fprintf(2, "xargs: exec %s failed\n", args[0]);
            exit(1);
        } else {
            wait(0); // 父进程等待子进程
        }
    }
    exit(0);
}