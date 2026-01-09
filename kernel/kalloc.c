// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

//超级页管理结构

// 预留的超级页数量，设 8 个
#define N_SUPERPAGES 8 
struct {
  struct spinlock lock;
  struct run *freelist;
  char *super_start; // 记录超级页内存区域的起始地址
} skmem;
/*
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}*/

// 释放一个 2MB 超级页
void
superfree(char *pa)
{
  // 动态计算超级页内存池的结束地址
  char *super_end = skmem.super_start + N_SUPERPAGES * SUPERPGSIZE; 
  
  if(((uint64)pa % SUPERPGSIZE) != 0 || pa < skmem.super_start || pa >= super_end)
    panic("superfree");

  struct run *r;
  acquire(&skmem.lock);
  r = (struct run*)pa;
  r->next = skmem.freelist;
  skmem.freelist = r;
  release(&skmem.lock);
}
// 分配一个 2MB 超级页
char*
superalloc(void)
{
  acquire(&skmem.lock);
  struct run *r = skmem.freelist;
  if(r)
    skmem.freelist = r->next;
  release(&skmem.lock);
  
  // 必须是 2MB 对齐的地址
  if(r && ((uint64)r % SUPERPGSIZE) != 0)
      panic("superalloc not 2MB aligned");

  // 分配到的内存不需要清零，因为 uvmalloc 会进行清零
  return (char*)r;
}
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&skmem.lock, "superkmem");
  extern char end[];
  uint64 pa;
  // 确定超级页内存范围（减少预留后）
  uint64 pa_super_start = SUPERPGROUNDUP((uint64)end); 
  skmem.super_start = (char*)pa_super_start;
  uint64 pa_super_end = pa_super_start + N_SUPERPAGES * SUPERPGSIZE;
  if (pa_super_end > PHYSTOP)
     panic("kinit: superpages out of memory");
  // 初始化超级页空闲列表
  for (char *p = (char*)pa_super_start; p < (char*)pa_super_end; p += SUPERPGSIZE) {
    superfree(p);
  }
  // 释放 4KB 内存块
  uint64 pa_4k_start = PGROUNDUP((uint64)end);
  int cnt_4k = 0; // 统计释放的 4KB 页数量（调试用）
  // 分两段释放 4KB 页：[pa_4k_start, pa_super_start) 和 [pa_super_end, PHYSTOP)
  // 第一段：end 到超级页开始前
  for (pa = pa_4k_start; pa < pa_super_start; pa += PGSIZE) {
    kfree((char*)pa);
    cnt_4k++;
  }
  // 第二段：超级页结束后到 PHYSTOP
  for (pa = pa_super_end; pa < PHYSTOP; pa += PGSIZE) {
    kfree((char*)pa);
    cnt_4k++;
  }
  if (cnt_4k == 0) {
    panic("kinit: no 4KB pages freed");
  }
}


void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}



