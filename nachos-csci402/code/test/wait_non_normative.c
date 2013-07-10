#include "syscall.h"

int lock, CV;

void wait_non_norm(){ 
	int returnValue = 0;
	
	Printx("Thread 1 is waiting before acquiring the lock\n",48,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to acquire the lock\n",38,1);
	}
	
	Printx("Thread 1 is waiting with an invalid CV index\n",47,1);
	returnValue = Wait(-1, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 is waiting with an invalid lock index\n",49,1);
	returnValue = Wait(CV, -1);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 is waiting with a null CV index\n",43,1);
	returnValue = Wait((CV + 50), lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 2 is waiting with a null lock index\n",45,1);
	returnValue = Wait(CV, (lock + 50));
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 is waiting on a signal\n",32,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 1 has been signalled\n",30,1);
	
	Printx("Thread 1 is releasing the lock\n",31,1);
	returnValue = Release(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to release the lock\n",38,1);
	}
	
	Exit(0);
}

void signal_waiter_norm(){
	int returnValue = 0, i = 0;	
	
	/* Yield to allow other thread to acquire the lock and wait on the CV first. */
	for(i = 0; i < 200; i++){ Yield(); }
	
	Printx("Thread 2 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to acquire the lock\n",38,1);
	}
	
	Printx("Thread 2 is signalling the waiter\n",34,1);
	returnValue = Signal(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to signal on the CV\n",38,1);
	}
	
	Printx("Thread 2 is releasing the lock\n",31,1);
	returnValue = Release(lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to release the lock\n",38,1);
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
    Printx("Beginning non-normative test\n",31,1);
    Printx("----------------------------------\n",36,1); 
    
    Fork(wait_non_norm, "wait_non_normative", 19);
    Fork(signal_waiter_norm, "signal_waiter_normative", 24);
        
    /* not reached */
}
