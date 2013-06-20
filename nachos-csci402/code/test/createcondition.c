#include "syscall.h"
int
main()
{
    char* name = "test1";
    int index = CreateCondition(name, 5);
    
    name = "test2";
    index = CreateCondition(name, 5);
    
    name = "test3";
    index = CreateCondition(name, 5);
    
    return 0;
}
