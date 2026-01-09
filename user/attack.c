#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PGSIZE 4096
// 判断字符是否属于 randstring 使用的字符集 "./abcdef"
static int
is_valid_secret_char(char c)
{
  const char *set = "./abcdef";
  for (int i = 0; set[i]; i++) {
    if (set[i] == c) return 1;
  }
  return 0;
}
int
main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  for (;;) {
    char *p = (char*)sbrk(PGSIZE);// 申请一页（sbrk 返回申请前的地址）
    if (p == (char*)-1) {
      // 申请失败，退出
      exit(1);
    }
    char *cand = p + 32;// secret 存放在 page + 32 的位置
    // 检查 cand[0..6] 都是合法字符，并且 cand[7] == '\0'
    int ok = 1;
    for (int i = 0; i < 7; i++) {
      // 必须为打印集合内的字符
      if (!is_valid_secret_char(cand[i])) {
        ok = 0;
        break;
      }
    }
    if (ok && cand[7] == '\0') {
      // 发现秘密：把包含终止符的 8 字节写到 fd=2
      write(2, cand, 8);
      exit(0);
    }
    // 否则继续分配下一页，重试
  }
  exit(1);
}