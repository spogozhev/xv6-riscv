#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Function to safely write data to a file descriptor.
// write() may write fewer bytes than requested,
// so we must write the remainder in a loop.
int write_all(int fd, const char *buf, int len) {
    int written = 0;
    while (written < len) {
        int n = write(fd, buf + written, len - written);
        if (n < 0) {
            printf("write_all: error\n");
            return -1;
        }
        written += n;
    }
    return written;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Use: pipe_wc argument1 argument2 ...\n");
        exit(1);
    }

    int p[2];
    // Creating an anonymous channel
    if (pipe(p) < 0) {
        printf("pipe_wc: Error creating channel\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        printf("pipe_wc: Error fork\n");
        exit(1);
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        
        // 1. Close the input (writing) end of the pipe; we don't need it. 
        // This is critical; otherwise, wc will wait for data and a deadlock will occur.
        close(p[1]); 

        // 2. Replace standard input (0) with the output end of the pipe
        close(0);          
        dup(p[0]);         
        close(p[0]);       

        // 3. Replace the process with the wc application
        char *exec_argv[] = {"/wc", 0};
        exec("/wc", exec_argv);
        
        // If exec returns control, an error has occurred.
        printf("pipe_wc: Error exec\n");
        exit(1);

    } else {
        // --- PARENT PROCESS ---
        
        // The parent process will only write, so we close the reading end.
        close(p[0]);

        // Pass all command-line arguments to the channel.
        for (int i = 1; i < argc; i++) {
            int len = strlen(argv[i]);
            
            // Record the argument itself (handling partial recording).
            if (write_all(p[1], argv[i], len) < 0) {
                exit(1);
            }
            
            if (write_all(p[1], "\n", 1) < 0) {
                exit(1);
            }
        }

        // Close the write end of the pipe. 
        // This sends an EOF (End-of-File) to the child process,
        // allowing wc to recognize that the data has ended and terminate.
        close(p[1]);

        // Waiting for the child process to complete.
        wait(0);
        
        // Terminating the parent process.
        exit(0);
    }
}