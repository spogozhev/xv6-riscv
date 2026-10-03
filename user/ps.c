#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int lim = 10; // Начальный размер буфера
    struct procinfo *plist = 0;
    int count;

    // Цикл для динамического увеличения буфера
    while(1) {
        plist = (struct procinfo*)malloc(lim * sizeof(struct procinfo));
        if(plist == 0) {
            fprintf(2, "ps: malloc failed\n");
            exit(1);
        }
        
        count = ps_listinfo(plist, lim);
        
        if(count < 0) {
            fprintf(2, "ps: ps_listinfo failed\n");
            free(plist);
            exit(1);
        }
        
        // Если count <= lim, значит все процессы успешно скопированы
        if(count <= lim) {
            break;
        }
        
        // Иначе увеличиваем лимит и пробуем снова
        free(plist);
        lim = count; 
    }

    // Вывод заголовка таблицы
    printf("PID\tSTATE\tPPID\tNAME\n");
    
    for(int i = 0; i < count; i++) {
        char *state_str;
        // Преобразуем числовое состояние в строку
        switch(plist[i].state) {
            case 0: state_str = "UNUSED"; break;
            case 1: state_str = "USED"; break;
            case 2: state_str = "SLEEPING"; break;
            case 3: state_str = "RUNNABLE"; break;
            case 4: state_str = "RUNNING"; break;
            case 5: state_str = "ZOMBIE"; break;
            default: state_str = "UNKNOWN"; break;
        }
        
        printf("%d\t%s\t%d\t%s\n", 
               plist[i].pid, 
               state_str, 
               plist[i].ppid, 
               plist[i].name);
    }

    free(plist);
    exit(0);
}