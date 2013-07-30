#include "syscall.h"

int lock, CV;

void wait_norm(){ 
	int returnValue = 0;
	
	Printx("Thread 1 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to acquire the lock\n",38,1);
	}
	
	Printx("Thread 1 is waiting on a signal\n",32,1);
	/* If we let Thread 1 actually wait on the CV, we won't get to see the destroying action! */
/*	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 has been signalled\n",30,1);
	
	Printx("Thread 1 is releasing the lock\n",31,1);
	returnValue = Release(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to release the lock\n",38,1);
	}
*/	
	Exit(0);
}

void signal_waiter_destroy(){
	int returnValue = 0, i = 0;	
	
	/* Yield to allow other thread to acquire the lock and wait on the CV first. */
	for(i = 0; i < 100; i++){ Yield(); }
	
	Printx("Thread 2 is destroying the lock\n",34,1);
	returnValue = DestroyLock(lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to destroy the lock\n",38,1);
	}
	
	Printx("Thread 2 is signalling the waiter using a destroyed lock\n",59,1);
	returnValue = Signal(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to signal on the CV\n",38,1);
	}
	
	Printx("Thread 2 is destroying the CV\n",32,1);
	returnValue = DestroyCondition(CV);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to destroy the CV\n",38,1);
	}
	
	Printx("Thread 2 is signalling the waiter using a destroyed CV\n",57,1);
	returnValue = Signal(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to signal on the CV\n",40,1);
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
    
    Fork(wait_norm, "", 0);
    Fork(signal_waiter_destroy, "", 0);
        
    /* not reached */
}
