#include "syscall.h"

int
main()
{
    char* nameCV = "CV";
    int CV = 0, i = 0;
    int returnValue = 0;
    
    Printx("Non-owner process is attempting to destroy a CV using an invalid index\n",73,1);
    returnValue = DestroyCondition(-1);
    if (returnValue == -1){
   		Printx("Non-owner not able to destroy CV\n",35,1);
    }
    
    Printx("Non-owner process is attempting to destroy a CV using an incorrect index\n",75,1);
    returnValue = DestroyCondition(-1);
    if (returnValue == -1){
   		Printx("Non-owner not able to destroy CV\n",35,1);
    }
    
    Printx("Non-owner process is attempting to destroy a CV that belongs to another process\n",82,1);
    returnValue = DestroyCondition(CV);
    if (returnValue == -1){
   		Printx("Non-owner not able to destroy CV\n",35,1);
    }
    
    
    Exit(0);
    /* not reached */
}
