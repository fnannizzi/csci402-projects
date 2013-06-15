#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    CreateLock(1);
    /*b = CreateLock(1);*/
    /* not reached */
}
