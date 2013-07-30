#include "syscall.h"

int
main()
{
	int i = 0, returnValue = 0;
	int lock = 0, CV = 0;
	char* nameCV = "CV";
    
   	Printx("Non-owner process is signalling on a CV belonging to another process\n",71,1);
	returnValue = Signal(CV, lock);
	if(returnValue == -1){ 
		Printx("Non-owner not able to signal on the CV\n",41,1);
	}
	
	CV = CreateCondition(nameCV, 2);
	Printx("Non-owner process created a new CV\n",37,1);
	
	Printx("Non-owner process is signalling on a lock belonging to another process\n",73,1);
	returnValue = Signal(CV, lock);
	if(returnValue == -1){ 
		Printx("Non-owner not able to signal on the CV\n",41,1);
	}
        
    /* not reached */
}
