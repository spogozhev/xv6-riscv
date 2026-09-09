#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Use: test_sum number1 number2\n");
        exit(1);
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    int result = sum(a, b);

    // Вывод результата в пользовательском пространстве
    printf("User space: %d + %d = %d\n", a, b, result);
    
    exit(0);
}