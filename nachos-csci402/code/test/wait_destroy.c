#include "syscall.h"

int lock, CV;

void wait_destroy(){
	int returnValue = 0, i = 0;	
	
	Printx("Thread 1 is destroying the lock\n",34,1);
	returnValue = DestroyLock(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to destroy the lock\n",38,1);
	}
	
	Printx("Thread 1 is waiting on a CV using a destroyed lock\n",53,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",38,1);
	}
	
	Printx("Thread 1 is destroying the CV\n",32,1);
	returnValue = DestroyCondition(CV);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to destroy the CV\n",38,1);
	}
	
	Printx("Thread 1 is waiting on a destroyed CV\n",40,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",38,1);
	}
	
	Exit(0);
}

int
main()
{
	char* nameLock = "lock";
	char* nameCV = "CV";
	int i = 0;
	
	lock = CreateLock(nameLock, 4);
	CV = CreateCondition(nameCV, 2);
    
    Printx("----------------------------------\n",36,1); 
    Printx("Beginning test on destroyed lock and CV\n",42,1);
    Printx("----------------------------------\n",36,1); 
    
    Fork(wait_destroy, "wait_destroy", 13);
        
    /* not reached */
}
