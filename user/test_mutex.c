// Тест мьютексов (задача 2): два процесса печатают посимвольно
// свои аргументы командной строки в формате
//   pid: arg i, char 'c'
// Сначала без синхронизации (строки перемешиваются),
// затем с захватом мьютекса на каждую строку вывода.
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Печать аргументов. Если mtx >= 0, каждая строка защищена мьютексом.
void
print_args(int use_mutex, int mtx, int argc, char *argv[])
{
  int i, j;

  for(i = 1; i < argc; i++){
    for(j = 0; argv[i][j] != '\0'; j++){
      if(use_mutex)
        mutex_lock(mtx);
      printf("%d: arg %d, char '%c'\n", getpid(), i, argv[i][j]);
      if(use_mutex)
        mutex_unlock(mtx);
    }
  }
}

int
main(int argc, char *argv[])
{
  int mtx, pid;

  if(argc < 2){
    fprintf(2, "usage: test_mutex arg1 [arg2 ...]\n");
    exit(1);
  }

  // Мьютекс создается ДО fork: общий предок создает его,
  // оба процесса пользуются им через унаследованный дескриптор
  // (fork увеличивает счетчик ссылок через filedup).
  if((mtx = mutex()) < 0){
    fprintf(2, "test_mutex: cannot create mutex\n");
    exit(1);
  }

  // Мьютекс - не файл и не канал: read/write/fstat должны давать ошибку
  char buf[8];
  printf("read on mutex:   %d (expect -1)\n", read(mtx, buf, sizeof(buf)));
  printf("write on mutex:  %d (expect -1)\n", write(mtx, "x", 1));
  struct stat st;
  printf("fstat on mutex:  %d (expect -1)\n", fstat(mtx, &st));

  pid = fork();
  if(pid < 0){
    fprintf(2, "test_mutex: fork failed\n");
    exit(1);
  }

  printf("--- stage 1: no sync (lines may interleave) ---\n");
  print_args(0, mtx, argc, argv);

  printf("--- stage 2: mutex-protected (whole lines only) ---\n");
  print_args(1, mtx, argc, argv);

  // exit закроет дескрипторы сам (fileclose), но закроем и явно:
  // последний закрывший уничтожает мьютекс (mutexclose).
  close(mtx);
  exit(0);
}
