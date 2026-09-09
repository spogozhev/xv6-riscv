#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Use: %s [a|b]\n", argv[0]);
        exit(1);
    }

    int child_pid = fork();

    if (child_pid < 0) {
        printf("Error fork\n");
        exit(1);
    }

    if (child_pid == 0) {
        pause(100); 
        exit(1); 
    } else {

        printf("Parent PID: %d, Child PID: %d\n", getpid(), child_pid);

        if (argv[1][0] == 'a') {
            int status;
            int wpid = wait(&status); 
            printf("Variant (a): PID %d finished, return code %d\n", wpid, status);
        } 
        else if (argv[1][0] == 'b') {
            kill(child_pid); 
            
            int status;
            int wpid = wait(&status);
            printf("Vatiant (b): PID %d finished, return code %d\n", wpid, status);
        } 
        else {
            printf("Invalid argument. Use 'a' or 'b'.\n");
        }

        exit(0);
    }
}