#include "syscall.h"

int
main()
{
    char* nameCV = "CV";
    int CV = 0, i = 0;
    int returnValue = 0;
    
    Printx("Owner process is creating a new CV\n",37,1);
    CV = CreateCondition(nameCV,2);
    if (CV == -1){
   		Printx("Owner process not able to create CV\n",38,1);
    }
    
    Printx("Owner process is destroying the CV\n",37,1);
    returnValue = DestroyCondition(CV);
    if (returnValue == -1){
   		Printx("Owner not able to destroy CV\n",31,1);
    }
    
    Printx("Owner process is attempting to destroy the CV again\n",54,1);
    returnValue = DestroyCondition(CV);
    if (returnValue == -1){
   		Printx("Owner not able to destroy CV\n",31,1);
    }
    
    
    Exit(0);
    /* not reached */
}
