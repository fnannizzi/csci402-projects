// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include <stdio.h>
#include <iostream>

using namespace std;

typedef struct {
	bool isToBeDeleted, isDeleted;
	Lock* lock;
	AddrSpace* processOwner;
} LockX;

/* Constructor-style function */
LockX* constructLockX(char* name) {
	LockX* lX = new LockX;
	Lock* l = new Lock(name);
	lX->lock = l;
	lX->processOwner = currentThread->space;
	lX->isToBeDeleted = false;
	lX->isDeleted = false;
	return lX;
}

typedef struct {
	bool isToBeDeleted, isDeleted;
	AddrSpace *processOwner;
	Condition* condition;
} ConditionX;

/* Constructor-style function */
ConditionX* constructConditionX(char* name){
	ConditionX* cX = new ConditionX;
	Condition* c = new Condition(name);
	cX->condition = c;
	cX->processOwner = currentThread->space;
	cX->isToBeDeleted = false;
	cX->isDeleted = false;
	return cX;
}

typedef struct {
	int vaddr;
	int stack;
} KT;

typedef struct {
	int childProcesses;
	AddrSpace* parent;
	int spaceID;
	//ChildProcess* childProcessTable;
} Process;

/* Constructor-style function */
Process* constructProcess(int id) {
	Process* p = new Process;
	p->spaceID = id;
	return p;
}

// Constants to hold the maximum number of locks and CVs
#define MAX_LOCKS 		100
#define MAX_CONDITIONS 	500
#define MAX_PROCESSES 	5

// Global tables to hold locks, CVs, and processes    
LockX* lockTable[MAX_LOCKS];
ConditionX* conditionTable[MAX_CONDITIONS];
Process* processTable[MAX_PROCESSES];
int lockTableIndex; // perhaps should not be global?

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

/*Started editing here*/
void Kernel_Thread(int ktint) {
	KT* kt = (KT*)ktint;
	machine->WriteRegister(PCReg,kt->vaddr);
	machine->WriteRegister(NextPCReg,(kt->vaddr)+4);
	currentThread->space->RestoreState();
	//stack code
	machine->WriteRegister(StackReg,kt->stack);
	machine->Run();
}

void Fork_Syscall(int i) { /*Not sure about the parameter, how do we make into a *func type*/
	/*Need to implement this in Part 2*/
	int vaddr = machine->ReadRegister(4);
	Thread* t = new Thread("thread"); // can we name our threads different things?
	t->space = currentThread->space;
	
	//Update process table
	/*int processID = currentThread->space->getID();
	processTable[processID]->threadCount++;*/
	
	//stack code
	int stack = t->space->MakeNewPT();
	KT* kt = new KT;
	kt->vaddr = vaddr; // TODO
	kt->stack = stack; // TODO
	t->Fork(Kernel_Thread,(int)kt);
}

void Yield_Syscall(int i){} // TODO

void ExecThread(){} // TODO 

void Exec_Syscall(int i ) { /*Not sure about the parameter, need to convert to char* type*/
	/*Need to implement this in Part 2*/
	int vaddr = machine->ReadRegister(4);
	//Convert to physical address and read contents from here
	//Use filesystem->Open
	//Store openfile pointer
	//Create new addressspace for executable
	/*AddrSpace* addrspace = new AddrSpace(Openfile pointer);*/
	Thread * t = new Thread("thread");
	/*t->space = addrspace;*/
	//Update process table
	/*machine->WriteRegister(Register(2),space->getID());*/
	//t->Fork(Exec_Thread,(int) kt);
}

void Exit_Syscall(int i) {/*Not sure about the parameter*/
    currentThread->Finish();
}

/*Lock code*/

int Acquire_Syscall(int index) {
 if ((index >= MAX_LOCKS) || (index < 0)) {
  	printf("Invalid Lock requested. Must be between 0 and %i\n", MAX_LOCKS);
  	return -1;
  	 
  }
  if (lockTable[index] == NULL) {
  	printf("%s","Lock was never initialized\n");
  	return -1;
  }
/*   if (!lockTable[index]->lock->isHeldByCurrentThread()) {
  	printf("Thread %s does not own the lock it is attempting to release\n",currentThread->getName());
  	return -1;
  }*/
  if (lockTable[index]->processOwner != currentThread->space) {
  	printf("%s","Lock is not owned by current process\n");
  	return -1;
  }
  if (lockTable[index]->isToBeDeleted) {
  	printf("%s", "Lock has already been set to be deleted\n");
  	return -1;
  }
  if (lockTable[index]->isDeleted) {
  	printf("%s", "Lock has already been deleted\n");
  	return -1;
  }
  
  lockTable[index]->lock->Acquire();
  return 0;
}

int Release_Syscall(int index) {
	if ((index >= MAX_LOCKS) || (index < 0)) {
  	printf("Invalid Lock requested. Must be between 0 and %i\n", MAX_LOCKS);
  	return -1;
  	 
  }
  if (lockTable[index] == NULL) {
  	printf("%s","Lock was never initialized\n");
  	return -1;
  }
  if (!lockTable[index]->lock->isHeldByCurrentThread())
  {
  	printf("Thread %s does not own the lock it is attempting to release\n",currentThread->getName());
  	return -1;
  }
  if (lockTable[index]->processOwner != currentThread->space) {
  	printf("%s","Lock is not owned by current process\n");
  	return -1;
  }
  if (lockTable[index]->isToBeDeleted) {
  	printf("%s", "Lock has already been set to be deleted\n");
  	return -1;
  }
  if (lockTable[index]->isDeleted) {
  	printf("%s", "Lock has already been deleted\n");
  	return -1;
  }


  lockTable[index]->lock->Release();
  return 0;
}

int Wait_Syscall(int iCV, int iLock) {
	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){
		fprintf(stderr, "Invalid conditionTable index %d, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){
		fprintf(stderr, "Invalid lockTable index %d, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){
		fprintf(stderr, "No Condition initialized at index %d, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){
		fprintf(stderr, "No Lock initialized at index %d, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if((conditionTable[iCV]->isToBeDeleted) || (conditionTable[iCV]->isDeleted)){
		fprintf(stderr, "CV at index %d marked for deletion, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((lockTable[iLock]->isToBeDeleted) || (lockTable[iLock]->isDeleted)){
		fprintf(stderr, "Lock at index %d marked for deletion, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){
		fprintf(stderr, "CV at index %d not owned by current process, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){
		fprintf(stderr, "Lock at index %d not owned by current process, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){
		fprintf(stderr, "Lock at index %d not owned by current thread, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		printf("Thread %s is waiting on CV %s at index %d \n",
				currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Wait(lockTable[iLock]->lock);
	}
	return 0;
}

int Signal_Syscall(int iCV, int iLock) { // TODO delete CV if no one is waiting on it and isToBeDeleted
	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){
		fprintf(stderr, "Invalid conditionTable index %d, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){
		fprintf(stderr, "Invalid lockTable index %d, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){
		fprintf(stderr, "No Condition initialized at index %d, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){
		fprintf(stderr, "No Lock initialized at index %d, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if((conditionTable[iCV]->isToBeDeleted) || (conditionTable[iCV]->isDeleted)){
		fprintf(stderr, "CV at index %d marked for deletion, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((lockTable[iLock]->isToBeDeleted) || (lockTable[iLock]->isDeleted)){
		fprintf(stderr, "Lock at index %d marked for deletion, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){
		fprintf(stderr, "CV at index %d not owned by current process, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){
		fprintf(stderr, "Lock at index %d not owned by current process, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){
		fprintf(stderr, "Lock at index %d not owned by current thread, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		printf("Thread %s is signalling on CV %s at index %d \n",
				currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Signal(lockTable[iLock]->lock);
	}
	return 0;
}

int Broadcast_Syscall(int iCV, int iLock) { // delete CV if no one is waiting on it and isToBeDeleted
	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){
		fprintf(stderr, "Invalid conditionTable index %d, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){
		fprintf(stderr, "Invalid lockTable index %d, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){
		fprintf(stderr, "No Condition initialized at index %d, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){
		fprintf(stderr, "No Lock initialized at index %d, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if((conditionTable[iCV]->isToBeDeleted) || (conditionTable[iCV]->isDeleted)){
		fprintf(stderr, "CV at index %d marked for deletion, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((lockTable[iLock]->isToBeDeleted) || (lockTable[iLock]->isDeleted)){
		fprintf(stderr, "Lock at index %d marked for deletion, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){
		fprintf(stderr, "CV at index %d not owned by current process, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){
		fprintf(stderr, "Lock at index %d not owned by current process, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){
		fprintf(stderr, "Lock at index %d not owned by current thread, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		printf("Thread %s is broadcasting on CV %s at index %d \n",
				currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Broadcast(lockTable[iLock]->lock);
	}
	return 0;
}

int CreateLock_Syscall(unsigned int vaddr, int len) {
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return -1;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to CreateLock\n");
	delete buf;
	return -1;
    }

    buf[len]='\0';
    
    lockTableIndex = -1;
    for (int i = 0; i < MAX_LOCKS; i++)
    {
    	if (lockTable[i] == NULL){
    		lockTableIndex = i;
    		break;
    	}
    }
    if (lockTableIndex == -1) {
    	printf("%s","There are no available spaces to CreateLock\n");
    	delete buf;
    	return -1;
    }
 
    lockTable[lockTableIndex] = constructLockX(buf);
    
    if (lockTable[lockTableIndex] != NULL)
  	printf("Created Lock %s at position %i\n",
  			lockTable[lockTableIndex]->lock->getName(), lockTableIndex);
    delete[] buf;
    return lockTableIndex;
}

int DestroyLock_Syscall(int index){
  if ((index >= MAX_LOCKS) || (index < 0)) {
  	printf("Invalid Lock requested. Must be between 0 and %i\n",MAX_LOCKS);
  	return -1;
  	 
  }
  if (lockTable[index] == NULL) {
  	printf("%s","Lock was never initialized\n");
  	return -1;
  }
  /*if (lockTable[index]->isToBeDeleted == true) {
  	printf("%s","Lock is already set to be deleted\n");
  	return -1;
  }*/
  if (lockTable[index]->isDeleted) {
  	printf("%s","Lock has already been deleted\n");
  	return -1;
  }
  lockTable[index]->isDeleted = true;
  if (lockTable[index]->isDeleted)
 	printf("Lock at index %i in the table was successfully destroyed\n",lockTableIndex);
  return 0;
}

int CreateCondition_Syscall(unsigned int vaddr, int len){
    char *buf = new char[len+1];	// Kernel buffer to put the name in
	// Make sure memory was allocated for the name
    if (!buf) {
		printf("%s","Can't allocate kernel buffer in CreateCondition \n");
		return -1;
    }
	// Validate name
    if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to CreateCondition \n");
		delete[] buf;
		return -1;
    }
	// Append a null character to the name
    buf[len]='\0';
	
	int conditionTableIndex = -1;
		
	for(int i = 0; i < MAX_CONDITIONS; i++){
		if(conditionTable[i] == NULL){
			conditionTableIndex = i;
			break;
		}
	}
	
	if(conditionTableIndex == -1){
		fprintf(stderr, "Thread %s: Condition not created, max number of conditions reached \n",
				currentThread->getName());
		return -1;
	}
	
	conditionTable[conditionTableIndex] = constructConditionX(buf);
	printf("Thread %s: Created Condition %s at position %d \n", 
			currentThread->getName(), conditionTable[conditionTableIndex]->condition->getName(), conditionTableIndex);
	delete[] buf;
	return conditionTableIndex;	
}

int DestroyCondition_Syscall(int index){
  if((index >= MAX_CONDITIONS) || (index < 0)) {
  	printf("Thread %s: Invalid index in DestroyCondition. Must be between 0 and %d \n",
  			currentThread->getName(), (MAX_CONDITIONS-1));
  	return -1;
  }
  if(conditionTable[index] == NULL) {
  	printf("Thread %s: Condition at index %d cannot be deleted because it was never initialized\n",
  			currentThread->getName(), index);
  	return -1;
  }
  if(conditionTable[index]->isDeleted) {
  	printf("Thread %s: Condition at index %d has already been deleted\n",
  			currentThread->getName(), index);
  	return -1;
  }
  
  // Condition can be deleted
  conditionTable[index]->isToBeDeleted = true;
  printf("Thread %s: Successfully destroyed Condition at index %d \n",
  		 currentThread->getName(), index);
  return 0;
}

void Printx_Syscall(unsigned int vaddr, int len, int id) {
    char *buf;		// Kernel buffer for output
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
		    printf("%s","Bad pointer passed to to write: data not written\n");
		    delete[] buf;
		    return;
		}
    }

    for (int ii=0; ii<len; ii++) {
		printf("%c",buf[ii]);
    }

    delete[] buf;
}

/*Finished editing here*/

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
	/*Started editing here*/
	    case SC_Fork:
	    	DEBUG('a', "Fork syscall.\n");
	    	Fork_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Yield:
	    	DEBUG('a', "Yield syscall.\n");
	    	Yield_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Exec:
	    	DEBUG('a', "Exec syscall.\n");
	    	Exec_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Exit:
	    	DEBUG('a', "Exit syscall.\n");
	    	Exit_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Acquire:
	    	DEBUG('a', "Acquire syscall.\n");
	    	rv = Acquire_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Release:
	    	DEBUG('a', "Release syscall.\n");
	    	rv = Release_Syscall(machine->ReadRegister(4));
	    	break;
	    case SC_Wait:
	    	DEBUG('a', "Wait syscall.\n");
	    	rv = Wait_Syscall(machine->ReadRegister(4),
	    				 machine->ReadRegister(5));
	    	break;
	    case SC_Signal:
	    	DEBUG('a', "Signal syscall.\n");
	    	rv = Signal_Syscall(machine->ReadRegister(4),
	    				   machine->ReadRegister(5));
	    	break;
	    case SC_Broadcast:
	    	DEBUG('a', "Broadcast syscall.\n");
	    	rv = Broadcast_Syscall(machine->ReadRegister(4),
	    					  machine->ReadRegister(5));
	    	break;
	    case SC_CreateLock:
	    	DEBUG('a', "CreateLock syscall.\n");
	    	rv = CreateLock_Syscall(machine->ReadRegister(4),
	    							machine->ReadRegister(5));
	    	break;
	    	case SC_DestroyLock:
	    	DEBUG('a', "DestroyLock syscall.\n");
	    	rv = DestroyLock_Syscall(machine->ReadRegister(4));
	    	break;
	   case SC_CreateCondition:
	    	DEBUG('a', "CreateCondition syscall.\n");
	    	rv = CreateCondition_Syscall(machine->ReadRegister(4),
	    								 machine->ReadRegister(5));
	    	break;
	   case SC_DestroyCondition:
	    	DEBUG('a', "DestroyCondition syscall.\n");
	    	rv = DestroyCondition_Syscall(machine->ReadRegister(4));
	    	break;
	   case SC_Printx:
	    	DEBUG('a', "Printx syscall.\n");
	    	Printx_Syscall(machine->ReadRegister(4), 
	    				   machine->ReadRegister(5),
			      		   machine->ReadRegister(6));
	    	break;
	/*Finished editing here*/

	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
