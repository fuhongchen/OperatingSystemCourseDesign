//lab1-2

typedef unsigned int uint;
#include "user/user.h"
int main() {
    // 定义两个管道：pipe1用于父写子读，pipe2用于子写父读
    int pipe1[2], pipe2[2];
    char buf[1]; // 用于读写的缓冲区
    pipe(pipe1);
    pipe(pipe2);
    if (fork() == 0) {
        // 子进程代码
        close(pipe1[1]); // 关闭父进程的写端
        close(pipe2[0]); // 关闭父进程的读端
        // 从父进程读取1个字节
        read(pipe1[0], buf, 1);
        printf("%d: received ping\n", getpid());
        close(pipe1[0]);
        // 向父进程写入1个字节作为回复
        write(pipe2[1], " ", 1);
        close(pipe2[1]);
        exit(0);
    } else {
        // 父进程代码
        close(pipe1[0]); // 关闭子进程的读端
        close(pipe2[1]); // 关闭子进程的写端
        // 向子进程写入1个字节
        write(pipe1[1], " ", 1);
        close(pipe1[1]);
        // 等待子进程的回复
        read(pipe2[0], buf, 1);
        printf("%d: received pong\n", getpid());
        close(pipe2[0]);
        exit(0);
    }}