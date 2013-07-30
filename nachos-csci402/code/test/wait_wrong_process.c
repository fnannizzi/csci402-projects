#include "syscall.h"

int
main()
{
	int i = 0, returnValue = 0;
	int lock = 0, CV = 0;
	char* nameCV = "CV";
    
   	Printx("Non-owner process is waiting on a CV belonging to another process\n",68,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Non-owner not able to wait on the CV\n",39,1);
	}
	
	CV = CreateCondition(nameCV, 2);
	Printx("Non-owner process created a new CV\n",37,1);
	
	Printx("Non-owner process is waiting on a lock belonging to another process\n",70,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Non-owner not able to wait on the CV\n",39,1);
	}
        
    /* not reached */
}
