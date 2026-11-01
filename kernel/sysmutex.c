// Системные вызовы мьютексов: sys_mutex, sys_mutex_lock, sys_mutex_unlock.
// Мьютекс виден в пользовательском пространстве как файловый дескриптор
// (FD_MUTEX в общей таблице открытых файлов процесса, см. task 1).
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "file.h"
#include "mutex.h"

// Создание мьютекса (аналог pipe): mutexalloc + fdalloc.
// Возвращает дескриптор или -1 при ошибке.
uint64
sys_mutex(void)
{
  struct file *f;
  int fd;

  if((f = mutexalloc()) == 0)
    return -1;

  // fdalloc находит свободный слот в p->ofile и увеличивает f->ref
  if((fd = fdalloc(f)) < 0){
    fileclose(f);
    return -1;
  }

  return fd;
}

// Захват мьютекса по дескриптору.
// Если мьютекс занят другим процессом - приостанавливаемся (sleeplock).
uint64
sys_mutex_lock(void)
{
  struct file *f;
  int fd;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  if(f->type != FD_MUTEX)
    return -1;

  acquiresleep(&f->mutex->lk);       // спим, пока мьютекс занят
  f->mutex->owner = myproc()->pid;   // запоминаем владельца под защитой lk

  printk("mutex_lock: pid %d acquired mutex %p\n", myproc()->pid, f->mutex);
  return 0;
}

// Освобождение мьютекса по дескриптору.
// Попытка освободить чужой (или свободный) мьютекс - ошибка.
uint64
sys_mutex_unlock(void)
{
  struct file *f;
  int fd;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  if(f->type != FD_MUTEX)
    return -1;

  if(f->mutex->owner != myproc()->pid){
    printk("mutex_unlock: pid %d refuses to unlock foreign mutex %p (owner %d)\n",
           myproc()->pid, f->mutex, f->mutex->owner);
    return -1;
  }

  f->mutex->owner = 0;
  releasesleep(&f->mutex->lk);

  printk("mutex_unlock: pid %d released mutex %p\n", myproc()->pid, f->mutex);
  return 0;
}
