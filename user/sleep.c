// lab1-1
typedef unsigned int uint;
//头文件中可能使用了uint，不先定义运行‘make qemu’的时候会报错
#include "user/user.h"
int main(int argc, char *argv[]) {
    int ticks = atoi(argv[1]);
    sleep(ticks);//调用系统调用进入睡眠
    exit(0);//正常退出
}