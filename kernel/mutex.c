#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "proc.h"
#include "mutex.h"
#include "defs.h"

// Выделение мьютекса (аналог pipealloc):
// открытый файл из ftable + динамический sleeplock.
// Возвращает открытый файл (FD_MUTEX) или 0 при ошибке.
struct file *
mutexalloc(void)
{
  struct file *f;
  struct mutex_obj *m;

  if((f = filealloc()) == 0)
    return 0;

  if((m = (struct mutex_obj *)kalloc()) == 0){
    fileclose(f);
    return 0;
  }

  f->type = FD_MUTEX;
  f->readable = 1;
  f->writable = 1;
  f->mutex = m;

  initsleeplock(&m->lk, "mutex");
  m->owner = 0;              // мьютекс изначально свободен

  printk("mutexalloc: pid %d kalloc mutex %p\n", myproc()->pid, m);
  return f;
}

// Уничтожение мьютекса: вызывается из fileclose,
// когда счетчик ссылок на открытый файл упал до 0.
void
mutexclose(struct mutex_obj *m)
{
  printk("mutexclose: pid %d kfree mutex %p\n", myproc()->pid, m);
  kfree(m);
}
