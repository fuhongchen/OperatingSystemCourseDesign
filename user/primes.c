//lab1-3
typedef unsigned int uint;
#include "user/user.h"
// 声明 primes 函数为不返回的函数，避免编译器警告
void primes(int) __attribute__((noreturn));
void primes(int read_fd) {
    int base_prime;
    // 从父进程读取第一个数，它一定是素数
    if (read(read_fd, &base_prime, sizeof(int)) == 0) {
        close(read_fd);
        exit(0); // 没有数字了，结束进程
        }
    printf("prime %d\n", base_prime);   // 打印当前进程发现的素数
    int p[2];
    pipe(p);
    int num;
    if (fork() == 0) {
        // 子进程：关闭不必要的端口，递归调用 primes
        close(read_fd);
        close(p[1]);
        primes(p[0]); // 子进程从管道读取数据
    } else { // 父进程：关闭不必要的读端
        close(p[0]);
        // 读取父进程传来的剩余数字，筛除 base_prime 的倍数
        while (read(read_fd, &num, sizeof(int)) > 0) {
            if (num % base_prime != 0) {
                // 将未被筛除的数字写入管道，传递给子进程
                write(p[1], &num, sizeof(int));
            }
        }
        // 关闭所有端口，通知子进程结束
        close(read_fd);
        close(p[1]);
        // 等待子进程结束
        wait(0);
        exit(0);
    }
}
int main() {
    int p[2];
    pipe(p);

    if (fork() == 0) {
        // 子进程：关闭写端，开始筛法
        close(p[1]);
        primes(p[0]);
    } else {
        // 父进程：关闭读端，生成初始数字 2-50
        close(p[0]);
        for (int i = 2; i <= 50; i++) {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]); // 关闭写端，表示数据发送完毕
        wait(0);     // 等待所有子进程结束
        exit(0);
    }
    return 0;
}