// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
		queue->Append((void *)currentThread);	// so go to sleep
		currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) 
{
    name = debugName;
    free = true;
    ownerThread = NULL;
    queue = new List;
}

Lock::~Lock() 
{
	delete queue;
}

bool Lock::isHeldByCurrentThread(){
	if(ownerThread == currentThread) 
		return true;	
	else
		return false;
}

void Lock::Acquire() 
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	if(isHeldByCurrentThread()) // if I'm the lock owner
	{    
		printf("Thread %s already owns the lock %s it is attempting to acquire.\n", 	currentThread->getName(), getName());
	    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
	    return;
	}
	
	if(free) // if I'm not the owner and the lock is free
	{
		free = false;
		ownerThread = currentThread;
		//printf("Acquired by %i\n",currentThread->space);
	}
	else // if I'm not the owner and the lock is not free 		   
   	{
   		//printf("Thread %s is attempting to acquire lock %s, but the lock is not free and thread is not owner.\n", currentThread->getName(), getName());
		queue->Append((void *)currentThread);	// append to list of waiting threads
		currentThread->Sleep();			// go to sleep
    }
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
    //printf("Thread %s is exiting Acquire() for lock %s, but the lock is not free and thread is not owner.\n", currentThread->getName(), getName());
}

void Lock::Release() 
{
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	if(!isHeldByCurrentThread()) // if I'm not the lock owner
	{    
	    printf("Thread %s does not own the lock %s it is attempting to release.\n", currentThread->getName(), getName());
	    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
	    return;
	}
	
	if(!queue->IsEmpty()) // if I'm the owner and there are threads waiting for the lock
	{
		//printf("Released by %i\n",currentThread->space);
		Thread *thread = (Thread *)queue->Remove();
    	if (thread != NULL)	   // make thread ready, consuming the V immediately
		{
			ownerThread = thread;
			scheduler->ReadyToRun(thread);
		}
	}
	
	else // if I'm the owner and there are no threads waiting for the lock		   
   	{
   		//	printf("Released by %i\n",currentThread->space);
   		free = true; // make lock available
   		ownerThread = NULL; // clear lock ownership
    }
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

Condition::Condition(char* debugName) 
{ 
	name = debugName;
	queue = new List;
	waitingLock = NULL;
}

Condition::~Condition() 
{
	delete queue;
}

void Condition::Wait(Lock* conditionLock) 
{ 
	//ASSERT(FALSE); 
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	
	if(conditionLock == NULL) // programmer has made a mistake if this occurs
	{
		fprintf(stderr, "ERROR: %s received a null pointer to conditionLock.\n", getName());
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	else if(waitingLock == NULL) // if first thread calling Wait on this condition variable
	{
		waitingLock = conditionLock; // associate the lock with this condition variable
	}
	else if(waitingLock != conditionLock) // if lock passed does not match the lock already associated
	{
		fprintf(stderr, "ERROR: %s received a lock that does not match the associated lock.\n", getName());
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	
	conditionLock->Release(); // current thread has completed critical section
	queue->Append((void *)currentThread);	// append to list of waiting threads
	currentThread->Sleep(); // put current thread to sleep until signaled
	conditionLock->Acquire(); // upon waking up, reenter the critical section
	
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Condition::Signal(Lock* conditionLock) 
{ 
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
	
	if(conditionLock == NULL) // conditionLock shouldn't be null
	{
		fprintf(stderr, "ERROR: %s received a null pointer to conditionLock.\n", getName());
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	
	if(queue->IsEmpty()) // if no threads waiting in wait queue
	{
		//printf("Thread %s signaled lock %s, which has no waiting threads.\n", currentThread->getName(), getName());
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;
	}
	else if(waitingLock != conditionLock) // if lock passed by signaler doesn't match lock from waiters
	{
		fprintf(stderr, "ERROR: %s received a lock that does not match the associated lock.\n", getName());
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return;	
	}
	
	Thread *thread = (Thread *)queue->Remove(); // remove one thread from wait queue
    if (thread != NULL)	
	{
		scheduler->ReadyToRun(thread); // add to ready queue
		//signaledprintf("Added thread %s to the ready queue\n", thread->getName());
	}
	
	if(queue->IsEmpty()) // if no more waiting threads
	{
		waitingLock = NULL; // clear relationship with lock
	}
	
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Condition::Broadcast(Lock* conditionLock) 
{	
	while(!queue->IsEmpty()) // for all threads waiting in queue
	{
		Signal(conditionLock); // signal next thread in wait queue
	}
}
