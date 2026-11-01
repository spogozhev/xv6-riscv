#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include "types.h"
#include "sleeplock.h"

// Объект-мьютекс в ядре: выделяется через kalloc,
// освобождается через kfree (см. mutex.c)
struct mutex_obj {
  struct sleeplock lk;  // sleep-блокировка: занявший ее процесс приостанавливается
  int owner;            // pid процесса-владельца, 0 если мьютекс свободен
};

#endif
