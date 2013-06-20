#include "syscall.h"
int
main()
{	int indexLock, indexCV, returnValue;
    char* name = "test1Lock";
    indexLock = CreateLock(name, 9);
    
    name = "test1CV";
    indexCV = CreateCondition(name, 7);
    
    returnValue = Acquire(indexLock);
    if(returnValue == 0){
	    returnValue = Wait(indexCV, indexLock);
	}    
    return 0;
}
