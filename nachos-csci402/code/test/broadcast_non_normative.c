#include "syscall.h"

int lock, CV;

void wait_norm_1(){ 
	int returnValue = 0;
	
	Printx("Thread 1 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 1 not able to acquire the lock\n",38,1);
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

void wait_norm_2(){ 
	int i = 0, returnValue = 0;
	
	/* Yield to allow other thread to acquire the lock and wait on the CV first. */
	for(i = 0; i < 100; i++){ Yield(); }
	
	Printx("Thread 2 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to acquire the lock\n",38,1);
	}
	
	Printx("Thread 2 is waiting on a signal\n",32,1);
	returnValue = Wait(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to wait on the CV\n",36,1);
	}
	
	Printx("Thread 2 has been signalled\n",30,1);
	
	Printx("Thread 2 is releasing the lock\n",31,1);
	returnValue = Release(lock);
	if(returnValue == -1){ 
		Printx("Thread 2 not able to release the lock\n",38,1);
	}
	
	Exit(0);
}

void broadcast_waiters_non_norm(){
	int returnValue = 0, i = 0;	
	
	/* Yield to allow other threads to acquire the lock and wait on the CV first. */
	for(i = 0; i < 300; i++){ Yield(); }
	
	Printx("Thread 3 is broadcasting before acquiring the lock\n",53,1);
	returnValue = Broadcast(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is acquiring the lock\n",31,1);
	returnValue = Acquire(lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to acquire the lock\n",38,1);
	}
	
	Printx("Thread 3 is broadcasting with an invalid CV index\n",52,1);
	returnValue = Broadcast(-1, lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is broadcasting with an invalid lock index\n",54,1);
	returnValue = Broadcast(CV, -1);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is broadcasting with a null CV index\n",48,1);
	returnValue = Broadcast((CV + 50), lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is broadcasting with a null lock index\n",50,1);
	returnValue = Broadcast(CV, (lock + 50));
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is broadcasting the waiters\n",39,1);
	returnValue = Broadcast(CV, lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to broadcast on the CV\n",43,1);
	}
	
	Printx("Thread 3 is releasing the lock\n",31,1);
	returnValue = Release(lock);
	if(returnValue == -1){ 
		Printx("Thread 3 not able to release the lock\n",38,1);
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
    
    Fork(wait_norm_1, "", 0);
    Fork(wait_norm_2, "", 0);
    Fork(broadcast_waiters_non_norm, "", 0);
        
    /* not reached */
}
