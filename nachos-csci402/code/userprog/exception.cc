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
#include <ctime>
#include <sstream>

using namespace std;

// Struct containing lock data, used in lockTable
typedef struct {
	bool isToBeDeleted, isDeleted;
	Lock* lock;
	AddrSpace* processOwner;
} LockX;

// Constructor-style function for LockX
LockX* constructLockX(char* name) {
	LockX* lX = new LockX;
	Lock* l = new Lock(name);
	lX->lock = l;
	lX->processOwner = currentThread->space;
	lX->isToBeDeleted = false;
	lX->isDeleted = false;
	return lX;
}

// Struct containing condition data, used in conditionTable
typedef struct {
	bool isToBeDeleted, isDeleted;
	AddrSpace *processOwner;
	Condition* condition;
} ConditionX;

// Constructor-style function for ConditionX
ConditionX* constructConditionX(char* name){
	ConditionX* cX = new ConditionX;
	Condition* c = new Condition(name);
	cX->condition = c;
	cX->processOwner = currentThread->space;
	cX->isToBeDeleted = false;
	cX->isDeleted = false;
	return cX;
}

// Struct containing kernel thread data 
typedef struct {
	int vaddr, stack;
} KT;

// Struct containing process data, used in processTable
typedef struct {
	int threadCount, spaceID;
	AddrSpace* processOwner;
} Process;

// Constructor-style function for Process
Process* constructProcess(int id, AddrSpace* space) {
	Process* p = new Process;
	p->threadCount = 1;
	p->spaceID = id;
	p->processOwner = space;
	return p;
}

// Constants to hold the maximum number of locks, CVs, and processes
#define MAX_LOCKS 		500
#define MAX_CONDITIONS 	500
#define MAX_PROCESSES 	5

#define MSG_LENGTH		40

// Global tables to hold locks, CVs, and processes    
LockX* lockTable[MAX_LOCKS];
ConditionX* conditionTable[MAX_CONDITIONS];
Process* processTable[MAX_PROCESSES];
int numProcesses = 0;
int numProcessesExited = -1;
Lock* forkLock = new Lock("forkLock");
Lock* execLock = new Lock("execLock");
Lock* conditionLock = new Lock("conditionLock");
Lock* processLock[MAX_PROCESSES];
Condition* processCV[MAX_PROCESSES];
int mailboxCount = 0;

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
    	//printf("copyin: %s, %d\n", buf, n);
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

    if (!buf) { return; }

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
		if ((id = currentThread->space->fileTable.Put(f)) == -1 ){
		    delete f;
		}
		return id;
    }
    else {
		return -1;
	}
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

    if ( id == ConsoleInput) { return; }
    
    if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
    } 
    else {
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
    } 
    else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	   		f->Write(buf, len);
		} 
		else {
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

    if ( id == ConsoleOutput) { return -1; }
     
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
    } 
    else {
		if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    	len = f->Read(buf, len);
	    	if ( len > 0 ) {
	        	//Read something from the file. Put into user's address space
  	        	if ( copyout(vaddr, len, buf) == -1 ) {
		    		printf("%s","Bad pointer passed to Read: data not copied\n");
				}
	    	}
		} 
		else {
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
    } 
    else {
    	printf("%s","Tried to close an unopen file\n");
    }
}

// Started editing here
void Kernel_Thread(int ktint) {
	KT* kt = (KT*)ktint; // decode the pointer to a kernel thread object
	machine->WriteRegister(PCReg,kt->vaddr); // write the PCReg with the virtual address of the kernel thread
	machine->WriteRegister(NextPCReg,(kt->vaddr)+4); // write the NextPCReg with the next virtual address of the kernel thread
	currentThread->space->RestoreState(); // call RestoreState()
	machine->WriteRegister(StackReg,kt->stack); // write the StackReg with the kernel thread stack
	machine->Run(); // call machine->Run()
}

void Fork_Syscall(int vaddr, int name, int len) { // Not sure about the parameter, how do we make into a *func type
	/*Need to implement this in Part 2*/
	forkLock->Acquire();
	char *buf = new char[len+1];	// Kernel buffer to put the name in
	// Make sure memory was allocated for the name
    if (!buf) {
		printf("%s","Can't allocate kernel buffer in CreateCondition \n");
		forkLock->Release();
		return;
    }
	// Validate name
    if( copyin(name,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to CreateCondition \n");
		delete[] buf;
		forkLock->Release();
		return;
    }
	// Append a null character to the name
    buf[len]='\0';
    if ((vaddr < 0) || ((vaddr % 4) != 0)) {
    	printf("Invalid virtual address\n");
    	forkLock->Release();
    	return;
    }
	
	Thread* t = new Thread(buf); // create a new thread object
	t->space = currentThread->space; // assign the new thread to the current address space
	t->mailbox = mailboxCount++;
	//Update process table 
	int processID = currentThread->space->getID(); // get the processID of the current space
	processTable[processID]->threadCount++; // increment threadCount to reflect the new thread
	int stack = t->space->MakeNewPT(); // make a new page table (stack) for the new thread
	t->id = stack; //added
	KT* kt = new KT; // create a kernel thread object
	kt->vaddr = vaddr; // set the vaddr for the kernel thread object
	kt->stack = stack; // set the stack for the kernel thread object
	forkLock->Release(); // release the forkLock
	t->Fork(Kernel_Thread,(int)kt); // fork the kernel thread
}

void Yield_Syscall(int i){ currentThread->Yield(); }

void Exec_Thread(int i){
	currentThread->space->InitRegisters(); // initialize the registers
	currentThread->space->RestoreState(); // call RestoreState()
	machine->Run(); // call machine->Run()
}  

int Exec_Syscall(int vaddr, int len) {
    execLock->Acquire(); // acquire the lock on exec

    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file

    if (!buf) {
		printf("%s","Can't allocate kernel buffer in Exec\n");
		execLock->Release();
		return -1;
    } 
    if ((vaddr < 0) || ((vaddr % 4) != 0)) {
    	printf("Invalid virtual address\n");
    	execLock->Release();
    	return -1;	
    }
    if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to Exec\n");
		delete[] buf;
		execLock->Release();
		return -1;
    }

    buf[len]='\0';
    
    f = fileSystem->Open(buf); // Converts to physical address and reads contents and stores openfile pointer
    delete[] buf;
    if (f==NULL) {
    	printf("Unable to open file\n");
    	execLock->Release();
    	return -1;
    }
    AddrSpace * addrspace = new AddrSpace(f); //Creates new address space for executable
    numProcesses++;
    addrspace->setID(numProcesses); // give the new address space an ID
    Thread * t = new Thread("threadexec"); //Creates a new thread
    t->space = addrspace; //Allocates space created to thread space
    t->mailbox = mailboxCount++;
    //delete f;
    
    processTable[numProcesses] = constructProcess(numProcesses,addrspace); // create a new process object
    processLock[numProcesses] = new Lock("processLock"); // create a new process lock
    processCV[numProcesses] = new Condition("processCV"); // create a new process CV
    processLock[numProcesses]->Acquire(); // acquire the process lock
    //processTable[numProcesses]->threadCount++; // increment the threadcount to reflect the main thread
    processLock[numProcesses]->Release(); // release the process lock
    //numProcesses++; // increment numProcesses to reflect the new process

    //machine->WriteRegister(2,addrspace->getID()); // Writes spaceID to register 2
    execLock->Release(); // release the lock on exec
    t->Fork(Exec_Thread,0); // fork an exec_thread
    return addrspace->getID(); // return the address space's ID
}

void deleteStack(){ 
	for(int i = 0; i < 8; i++){
		//printf("Clearing page %d \n", currentThread->space->pageNumbers[i] );
		pageMap->Clear(currentThread->space->pageNumbers[i]); // Clear the pageMap 
	}		
}

void Exit_Syscall(int i) {
	printf("Exit value: %d \n", i);
	//currentThread->Finish(); // allow the current thread to exit
	int id = currentThread->space->getID();
	if (id == 0)
	{
		execLock->Acquire();
		numProcessesExited++;
		if (numProcessesExited == numProcesses)
		{
			execLock->Release();
			interrupt->Halt();
		}
		else
		{
			execLock->Release();
			currentThread->Finish();
		}
	}
	else
	{
		execLock->Acquire();
		processLock[id]->Acquire();
		processTable[id]->threadCount--;
		if (processTable[id]->threadCount == 0)
		{
			numProcessesExited++;
			if (numProcessesExited == numProcesses)
			{
				processLock[id]->Release();
				execLock->Release();
				interrupt->Halt();
			}
			else
			{
				processLock[id]->Release();
				execLock->Release();
				currentThread->Finish();
			}
		}
		else
		{
			processLock[id]->Release();
			execLock->Release();
			currentThread->Finish();
		}
	}
	

	/*int index = currentThread->space->getID(); // get address space/process ID
	processLock[index]->Acquire(); // acquire lock on the process table
	processTable[index]->threadCount--; // decrement threadcount to show this thread is exiting
	if(currentThread->isMain()){ // if the current thread is the main thread of its process
		if(processTable[index]->threadCount > 0){ // if the thread count is greater than 0
			processCV[index]->Wait(processLock[index]); // wait for the child threads to exit
		}
		delete processTable[index]->processOwner; // delete address space 
		delete processTable[index]; // delete the process table entry	
	}
	else { // current thread is a child thread
		if(processTable[index]->threadCount == 0){ // if the threat count is 0
			processCV[index]->Signal(processLock[index]); // signal your main thread
		}
			currentThread->space->DestroyStack(currentThread->id);
	}
	processLock[index]->Release(); // release the process table lock
	currentThread->Finish(); // allow the current thread to exit*/
}

/*Lock code*/
int Acquire_Syscall(int index) {

#ifdef NETWORK
    
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'A';
    msg->request[1] = 'Q';
    msg->name = new char[20];
    msg->name = "Acquire";
    msg->index = index; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
   	postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
   	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;
   
#else

 	if ((index >= MAX_LOCKS) || (index < 0)) { // lock index is invalid
  		printf("Invalid Lock requested. Must be between 0 and %i\n", MAX_LOCKS);
  		return -1;
  	}
  	if (lockTable[index] == NULL) { // lock at index is null
  		printf("%s","Lock was never initialized\n");
  		return -1;
  	}
  	if(lockTable[index]->processOwner != currentThread->space) { // lock not owned by process
  		printf("%s","Lock is not owned by current process\n");
  		return -1;
  	}
  	if (lockTable[index]->lock->isHeldByCurrentThread()) { // lock not held by current thread
  		printf("Thread %s already owns the lock it is attempting to acquire.\n",currentThread->getName());
  		return -1;
  	}
  	if (lockTable[index]->isDeleted) { // lock has already been deleted
  		printf("%s", "Lock has already been deleted\n");
  		return -1;
  	}
  	if (lockTable[index]->isToBeDeleted) { // lock is marked for deletion
  		printf("%s", "Lock has already been set to be deleted\n");
  		delete lockTable[index]->lock; // delete the lock
  		lockTable[index]->isDeleted = true; // set isDeleted to true
  		printf("Lock has now been deleted\n");
  		return -1;
  	}
  
  	lockTable[index]->lock->Acquire(); // acquire the lock
  	return index;

#endif 
}

int Release_Syscall(int index) {

#ifdef NETWORK
    
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'R';
    msg->request[1] = 'L';
    msg->name = new char[20];
    msg->name = "Release";
    msg->index = index; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
   	postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
   	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;

#else

	if ((index >= MAX_LOCKS) || (index < 0)) { // lock index is invalid
  		printf("Invalid Lock requested. Must be between 0 and %i\n", MAX_LOCKS);
  		return -1;
 	}
  	if (lockTable[index] == NULL) { // lock at index is null
  		printf("%s","Lock was never initialized\n");
  		return -1;
  	}
  	if (!lockTable[index]->lock->isHeldByCurrentThread()){ // lock is not held by current thread
  		printf("Thread %s does not own the lock it is attempting to release\n",currentThread->getName());
  		return -1;
  	}
 	if (lockTable[index]->processOwner != currentThread->space) { // lock not owned by process
  		printf("%s","Lock is not owned by current process\n");
  		return -1;
  	}
  	if (lockTable[index]->isDeleted) { // lock is already deleted
  		printf("%s", "Lock has already been deleted\n");
  		return -1;
  	}
  	if (lockTable[index]->isToBeDeleted) { // lock is marked for deletion
  		printf("%s", "Lock will now be deleted\n");
  		lockTable[index]->lock->Release(); // release the lock
  		delete lockTable[index]->lock; // delete the lock
  		lockTable[index]->isDeleted = true; // set deleted to true
  		printf("Lock has been deleted\n");
  		return index;
  	}
	
  	lockTable[index]->lock->Release(); // release the lock
  	return index;

#endif
}

int Wait_Syscall(int iCV, int iLock) {
#ifdef NETWORK
  
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'W';
    msg->request[1] = 'T';
    msg->name = new char[20];
    msg->name = "Wait";
    msg->index = iCV; //Maybe need to rename
    msg->index2 = iLock;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;

#else

	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){ // CV index is invalid
		printf("Invalid conditionTable index %d, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){ // lock index is invalid
		printf( "Invalid lockTable index %d, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){ // CV at index is null
		printf( "No Condition initialized at index %d, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){ // lock at index is null
		printf( "No Lock initialized at index %d, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if((conditionTable[iCV]->isToBeDeleted) || (conditionTable[iCV]->isDeleted)){ // CV is marked for deletion
		printf( "CV at index %d marked for deletion, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((lockTable[iLock]->isToBeDeleted) || (lockTable[iLock]->isDeleted)){ // lock is marked for deletion
		printf( "Lock at index %d marked for deletion, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){ // CV not owned by current process
		printf( "CV at index %d not owned by current process, cannot allow %s to wait \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){ // lock not owned by current process
		printf( "Lock at index %d not owned by current process, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){ // lock not owned by current thread
		printf( "Lock at index %d not owned by current thread, cannot allow %s to wait \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		//printf("Thread %s is waiting on CV %s at index %d \n",
		//		currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Wait(lockTable[iLock]->lock); // wait on the CV
	}
	return 0;
	
#endif
}

int Signal_Syscall(int iCV, int iLock) {
#ifdef NETWORK
  
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'S';
    msg->request[1] = 'G';
    msg->name = new char[20];
    msg->name = "Signal";
    msg->index = iCV; //Maybe need to rename
    msg->index2 = iLock;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;

#else

	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){ // invalid CV index
		printf( "Invalid conditionTable index %d, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){ // invalid lock index
		printf( "Invalid lockTable index %d, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){ // CV at index is null
		printf( "No Condition initialized at index %d, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){ // lock at index is null
		printf( "No Lock initialized at index %d, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->isDeleted){ // CV is already deleted
		printf( "CV at index %d deleted, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->isDeleted){ // lock is already deleted
		printf( "Lock at index %d deleted, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){ // CV not owned by current process
		printf( "CV at index %d not owned by current process, cannot allow %s to signal \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){ // lock not owned by current process
		printf( "Lock at index %d not owned by current process, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){ // lock not held by current thread
		printf( "Lock at index %d not owned by current thread, cannot allow %s to signal \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		//printf("Thread %s is signalling on CV %s at index %d \n",
		//		currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Signal(lockTable[iLock]->lock); // signal on the CV
		if(conditionTable[iCV]->isToBeDeleted){ // if the CV is marked to be deleted
			if(conditionTable[iCV]->condition->isQueueEmpty()){ // if there is no one waiting on the CV
				conditionTable[iCV]->isDeleted = TRUE; // set to be deleted
				delete conditionTable[iCV]->condition; // delete the CV
			}
		}
	}
	return 0;
	
#endif
}

int Broadcast_Syscall(int iCV, int iLock) { // delete CV if no one is waiting on it and isToBeDeleted
#ifdef NETWORK
  
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'B';
    msg->request[1] = 'C';
    msg->name = new char[20];
    msg->name = "Broadcast";
    msg->index = iCV; //Maybe need to rename
    msg->index2 = iLock;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;

#else	

	if((iCV < 0) || (iCV >= MAX_CONDITIONS)){ // invalid CV index
		printf( "Invalid conditionTable index %d, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((iLock < 0) || (iLock >= MAX_LOCKS)){ // invalid lock index
		printf( "Invalid lockTable index %d, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV] == NULL){ // CV at index is null
		printf( "No Condition initialized at index %d, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock] == NULL){ // lock at index is null
		printf( "No Lock initialized at index %d, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->isDeleted){ // CV is deleted
		printf( "CV at index %d marked for deletion, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if((lockTable[iLock]->isToBeDeleted) || (lockTable[iLock]->isDeleted)){ // lock is marked for deletion
		printf( "Lock at index %d marked for deletion, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(conditionTable[iCV]->processOwner != currentThread->space){ // CV not owned by current process
		printf( "CV at index %d not owned by current process, cannot allow %s to broadcast \n", 
				iCV, currentThread->getName());
		return -1;
	}
	if(lockTable[iLock]->processOwner != currentThread->space){ // lock not owned by current process
		printf( "Lock at index %d not owned by current process, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	if(!(lockTable[iLock]->lock->isHeldByCurrentThread())){ // lock not held by current thread
		printf( "Lock at index %d not owned by current thread, cannot allow %s to broadcast \n", 
				iLock, currentThread->getName());
		return -1;
	}
	else {
		//printf("Thread %s is broadcasting on CV %s at index %d \n",
		//		currentThread->getName(), conditionTable[iCV]->condition->getName(), iCV);
		conditionTable[iCV]->condition->Broadcast(lockTable[iLock]->lock); // broadcast on the CV
		if(conditionTable[iCV]->isToBeDeleted){ // if the CV is marked for deletion
			if(conditionTable[iCV]->condition->isQueueEmpty()){ // if no one is waiting on the CV
				conditionTable[iCV]->isDeleted = TRUE; // set isDeleted to true
				delete conditionTable[iCV]->condition;	// delete the CV
			}
		}
	}
	return 0;
	
#endif
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
    
#ifdef NETWORK
    
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message * msg = new Message;
    msg->name = new char[20];
    strcpy(msg->name, buf);
    msg->request = new char[2];
    msg->request[0] = 'C';
    msg->request[1] = 'L';
    msg->index = len; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
 	Message* receivedMsg = decodeMessage(receivedData); 
    returnValue = receivedMsg->index;
    
/*    delete buf;
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;
   
#else
    
    int lockTableIndex = -1; // create index
    for (int i = 0; i < MAX_LOCKS; i++){ 
    	if (lockTable[i] == NULL){ // if no lock at index
    		lockTableIndex = i; // record index
    		break;
    	}
    }
    
    if (lockTableIndex == -1) { // no indexes found
    	printf("%s","There are no available spaces to CreateLock\n");
    	delete buf;
    	return -1;
    }
 
    lockTable[lockTableIndex] = constructLockX(buf); // make a new lock
    delete[] buf;
    return lockTableIndex;
    
#endif
}

int DestroyLock_Syscall(int index){
#ifdef NETWORK
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message * msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'D';
    msg->request[1] = 'L';
    msg->name = new char[20];
    msg->name = "DestroyLock";
    msg->index = index; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData); 
    returnValue = receivedMsg->index;
    
/*    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;*/
    
    return returnValue;

#else
	
	if ((index >= MAX_LOCKS) || (index < 0)) { // invalid lock index
  		printf("Invalid Lock requested. Must be between 0 and %i\n",MAX_LOCKS);
  		return -1;
  	}
  	if (lockTable[index] == NULL) { // lock at index is null
  		printf("%s","Lock was never initialized\n");
  		return -1;
  	}
  	if (lockTable[index]->processOwner != currentThread->space){ // lock not owned by process
  		printf("%s","Lock is not owned by current process\n");
  		return -1;
  	}	
  	if (lockTable[index]->lock->isHeldByCurrentThread()) { // lock owned by current thread
  		printf("Lock is owned by current thread so cannot be destroyed\n");
  		return -1;
  	}
 	if (lockTable[index]->isToBeDeleted == true) { // lock is marked for deletion
  		printf("%s","Lock is already set to be deleted\n"); 
  		return -1;
  	}
  	if (lockTable[index]->isDeleted) { // lock is already deleted
  		printf("%s","Lock has already been deleted\n"); 
  		return -1;
  	}
  	lockTable[index]->isToBeDeleted = true;  // set isToBeDeleted to true
  	printf("Lock at index %i in the table is set to be destroyed\n",index);
 	
  	return index;

#endif
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
    
#ifdef NETWORK    
    
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->name = new char[20];
    strcpy(msg->name, buf);
    msg->request = new char[2];
    msg->request[0] = 'C';
    msg->request[1] = 'C';
    msg->index = len; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
  	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData); 
    returnValue = receivedMsg->index;  
/*    
	delete buf;
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;
    
#else
	
	int conditionTableIndex = -1;
	conditionLock->Acquire(); // acquire the condition lock
	for(int i = 0; i < MAX_CONDITIONS; i++){
		if(conditionTable[i] == NULL){ // empty space in table
			conditionTableIndex = i; // record empty index
			break;
		}
	}
	
	if(conditionTableIndex == -1){ // no empty spaces found
		printf( "Thread %s: Condition not created, max number of conditions reached \n",
				currentThread->getName());
		return -1;
	}
	
	conditionTable[conditionTableIndex] = constructConditionX(buf); // make a new condition
	//printf("Thread %s: Created Condition %s at position %d \n", 
	//		currentThread->getName(), conditionTable[conditionTableIndex]->condition->getName(), conditionTableIndex);
	conditionLock->Release(); // release the condition lock
	delete[] buf;
	return conditionTableIndex;

#endif	
}

int DestroyCondition_Syscall(int index){
#ifdef NETWORK
      
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message * msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'D';
    msg->request[1] = 'C';
    msg->name = new char[20];
    msg->name = "DestroyCV";
    msg->index = index; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
/*    
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;
    
#else

	if((index >= MAX_CONDITIONS) || (index < 0)) { // invalid CV index
  		printf("Thread %s: Invalid index in DestroyCondition. Must be between 0 and %d \n",
  				currentThread->getName(), (MAX_CONDITIONS-1));
  		return -1;
  	}
  	if(conditionTable[index] == NULL) { // CV at index is null
  		printf("Thread %s: Condition at index %d cannot be deleted because it was never initialized\n",
  				currentThread->getName(), index);
  		return -1;
  	}	
  	if(conditionTable[index]->processOwner != currentThread->space){ // CV not owned by current process
  		printf( "Thread %s: Cannot delete Condition at index %d because it is not owned by current process \n", 
				currentThread->getName(), index);
		return -1;
  	}
  	if(conditionTable[index]->isDeleted) { // CV is already deleted
  		printf("Thread %s: Condition at index %d has already been deleted\n",
  				currentThread->getName(), index);
  		return -1;
  	}
  
  	// Condition can be deleted
  	conditionTable[index]->isToBeDeleted = true; // set isToBeDeleted to true
  	if(conditionTable[index]->condition->isQueueEmpty()){ // no one waiting on CV
  		conditionTable[index]->isDeleted = true; // set isDeleted to true
  		delete conditionTable[index]->condition; // delete condition
  	}
  	//printf("Thread %s: Successfully destroyed Condition at index %d \n",
  	//		 currentThread->getName(), index);
  	return 0;

#endif
}

int CreateMV_Syscall(unsigned int vaddr, int len, int len2) {
#ifdef NETWORK
	char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return -1;

    if( copyin(vaddr,len,buf) == -1 ) {
		printf("%s","Bad pointer passed to CreateLock\n");
		delete buf;
		return -1;
    } 

    buf[len]='\0';
    
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message * msg = new Message;
    msg->name = new char[20];
    strcpy(msg->name, buf);
    msg->request = new char[2];
    msg->request[0] = 'C';
    msg->request[1] = 'M';
    msg->index = len; //Maybe need to rename
    msg->index2 = len2; //Array length
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
/*    
    delete buf;
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;

#else
	return -1;
#endif
}

int DestroyMV_Syscall(int mv) {
#ifdef NETWORK
	PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'D';
    msg->request[1] = 'M';
    msg->name = new char[20];
    msg->name = "DestroyMV";
    msg->index = mv; //Maybe need to rename
    msg->index2 = -1;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    returnValue = receivedMsg->index;
/*    
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;
#else
    return -1;
#endif
}

int GetMV_Syscall(int mv, int index) {
#ifdef NETWORK
	PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message* msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'G';
    msg->request[1] = 'M';
    msg->name = new char[20];
    msg->name = "GetMV";
    msg->index = mv; //Maybe need to rename
    msg->index2 = index;
    msg->index3 = -1;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
/*    
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;
#else
    return -1;
#endif
}

int SetMV_Syscall(int mv, int index, int value) {
#ifdef NETWORK
    PacketHeader packetHeaderSend, packetHeaderReceive;
    MailHeader mailHeaderSend,mailHeaderReceive;
    char* receivedData = new char[MaxMailSize];
    int returnValue;
    
    packetHeaderSend.to = 0;    
    mailHeaderSend.to = 0;
    mailHeaderSend.from = currentThread->mailbox;
    
    Message * msg = new Message;
    msg->request = new char[2];
    msg->request[0] = 'S';
    msg->request[1] = 'M';
    msg->name = new char[20];
    msg->name = "SetMV";
    msg->index = mv; //Maybe need to rename
    msg->index2 = index;
    msg->index3 = value;
    
	char* msgstringstream = msgPrepare(msg);
    
   	mailHeaderSend.length = MSG_LENGTH;
   	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
    postOffice->Receive(currentThread->mailbox,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
	Message* receivedMsg = decodeMessage(receivedData);
    returnValue = receivedMsg->index;
    returnValue = receivedMsg->index;
/*    
    delete receivedData;
    delete msg->request;
    delete msg->name;
    delete msg;
    delete msgstringstream;
    delete receivedMsg->request;
    delete receivedMsg->name;
    delete receivedMsg;
*/    
    return returnValue;
#else
    return -1;
#endif
}

void Printx_Syscall(unsigned int vaddr, int len, int value) {
    char *buf;		// Kernel buffer for output
    
    if ( !(buf = new char[len]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return;
    } 
    else {
        if ( copyin(vaddr,len,buf) == -1 ) {
		    printf("%s","Bad pointer passed to to write: data not written\n");
		    delete[] buf;
		    return;
		}
    }

    int count = 0;
    int output;
    for (int ii=0; ii<len; ii++) {
    	if (buf[ii] == '%'){
    		if ((ii+1) < len){
    			if (buf[ii+1] == 'd'){
    				if (count == 0){
    					output = value / 10000000;
    					printf("%d",output);
    					value = value % 10000000;
    				}
    				if (count == 1){
    					output = value / 10000;
    					printf("%d",output);
    					value = value % 10000;
    				}
    				if (count == 2){
    					output = value / 100;
    					printf("%d",output);
    					value = value % 100;
    				}
    				if (count == 3){
    					output = value;
    					printf("%d",output);
    				}
    				count++;
    				ii++;
    			}
    			if (buf[ii+1] == 's'){
    				printf("%s",currentThread->getName());
    				ii++;
    			}
    		}
    	}
    	else {
			printf("%c",buf[ii]);
        }
	}
    delete[] buf;
}

/*Finished editing here*/
void HandlePageFault()
{	

    int vpn = machine->ReadRegister(BadVAddrReg) / PageSize;
  //IntStatus oldLevel = interrupt->SetLevel(IntOff);  
  
	// acquire lock on IPT
	//printf("%i reached here\n",currentThread->space);
	iptLock->Acquire();
	
	// update timestamps in IPT from TLB
	/*int updateIndex;
	for(int i = 0; i < TLBSize; i++){
		//if (machine->tlb[i].valid)
		//{
			updateIndex = machine->tlb[i].physicalPage;
			ipt->table[updateIndex].timestamp = time(NULL);
		//}
	}*/
	
	// read virtual address from register

    
    // check IPT
    int iptIndex = ipt->searchForEntry(vpn, currentThread->space->getID());
    if(iptIndex == -1){ // entry not in IPT
    	DEBUG('a', "Entry not in IPT.\n");
	    	
	    // check for empty pages in IPT
	    iptIndex = ipt->searchForEmptyEntry();
    	if(iptIndex == -1){ // no empty entries found
    		DEBUG('a', "No empty entries in IPT.\n");
    		//interrupt->Halt();
    		
    		// pick a page to evict from IPT
    		int evictionPage = -1;
    		if(pageReplacementPolicy == RAND){
    			//(1) in method
    			evictionPage = ipt->chooseEvictionPageRandom();
    		}
    		else {
	    		evictionPage = ipt->chooseEvictionPageLRU();
	    	}
    		
    		// check to see if eviction page is in TLB
    		int evictionPageInTLB = SearchForTLBEntry(ipt->table[evictionPage].virtualPage);
    		if(evictionPageInTLB != -1){ // eviction page is in the TLB
    			// set eviction page to invalid and propagate dirty bit back
    			IntStatus oldLevel2 = interrupt->SetLevel(IntOff);
    			ipt->table[evictionPage].dirty = machine->tlb[evictionPageInTLB].dirty;
    			machine->tlb[evictionPageInTLB].valid = FALSE;
    			(void) interrupt->SetLevel(oldLevel2);
    		}
    		if(ipt->table[evictionPage].dirty){ // eviction page is dirty
    			// copy to swap file 
    			int swapFileIndex = swapFileMap->Find();
    			if(swapFileIndex == -1){
    				fprintf(stderr, "Swap file is full.\n");
    				interrupt->Halt();	
    			}
    			
    			//1.
    			 //currentThread->space->pageTable[ipt->table[evictionPage].virtualPage].setSwapByteOffset(swapFileIndex*PageSize);
    			swapArray[ipt->table[evictionPage].processID][ipt->table[evictionPage].virtualPage] = swapFileIndex*PageSize;
    			
    			//2.
    			// swapFile->WriteAt(&(machine->mainMemory[evictionPage*PageSize]), PageSize, currentThread->space->pageTable[ipt->table[evictionPage].virtualPage].getSwapByteOffset());
    			swapFile->WriteAt(&(machine->mainMemory[evictionPage*PageSize]), PageSize, (swapFileIndex*PageSize));
    			
    			// update page location in process page table
    			
    			//3. 
    			//currentThread->space->pageTable[ipt->table[evictionPage].virtualPage].pageLocation = SWAP_FILE;
    			pageLocation[ipt->table[evictionPage].processID][ipt->table[evictionPage].virtualPage] = 1;
    			
    			//4. 
    			/*currentThread->space->pageTable[ipt->table[evictionPage].virtualPage].valid = FALSE;
    			currentThread->space->pageTable[ipt->table[evictionPage].virtualPage].dirty = FALSE;*/
    			
    		}
    		// the page is now evicted
    		iptIndex = evictionPage;
    		
    	}
    	
    	// copy the needed virtual page into the empty slot in memory from either the swap file or executable
    	
    	//5. 
    	//if(currentThread->space->pageTable[vpn].pageLocation == EXECUTABLE){
    	if (pageLocation[currentThread->space->getID()][vpn] == 0) {
    		DEBUG('a', "Read page from executable.\n");
	    	
	    	//6. 
	    	//currentThread->space->executableFile->ReadAt(&(machine->mainMemory[iptIndex*PageSize]), PageSize, currentThread->space->pageTable[vpn].getExecutableByteOffset());
	    	currentThread->space->executableFile->ReadAt(&(machine->mainMemory[iptIndex*PageSize]), PageSize, 40 + (vpn*PageSize));
    	}
    	//7. 
    	//else if(currentThread->space->pageTable[vpn].pageLocation == SWAP_FILE){
	else if (pageLocation[currentThread->space->getID()][vpn] == 1) {
	
	   	 //8. 
	   	 //swapFile->ReadAt(&(machine->mainMemory[iptIndex*PageSize]), PageSize, currentThread->space->pageTable[vpn].getSwapByteOffset());
	   	 swapFile->ReadAt(&(machine->mainMemory[iptIndex*PageSize]), PageSize, swapArray[currentThread->space->getID()][vpn]);
    		
    		//swapFileMap->Clear(currentThread->space->pageTable[vpn].getSwapByteOffset() / PageSize);
    	}
	else if (pageLocation[currentThread->space->getID()][vpn] == 1) {
		for (int i = 0; i < PageSize; i++)
			machine->mainMemory[(iptIndex*PageSize)+i] = '0';
	}
    	DEBUG('a', "Page loaded into memory.\n");
    	// update the valid in process page table
    	currentThread->space->pageTable[vpn].valid = TRUE;
    	currentThread->space->pageTable[vpn].dirty = FALSE;
    	
    	// update IPT with new entry
    	//(2) (within method)
    	ipt->updateEntry(iptIndex, 
    					 vpn, 
    				     currentThread->space->getID(), 
    				     TRUE);
    	
    }
    
IntStatus oldLevel = interrupt->SetLevel(IntOff);

    
    // if TLB entry to be replaced is valid and dirty, propagate dirty bit to IPT
   	if(machine->tlb[nextTLB].valid){
   		if(machine->tlb[nextTLB].dirty){
    		int tlbIndex = ipt->searchForEntry(machine->tlb[nextTLB].virtualPage, 
   													    currentThread->space->getID());
   			ipt->table[tlbIndex].dirty = machine->tlb[nextTLB].dirty;	
   		}
   	}
   	
   	//(3)
   	int useIndex = ipt->searchForEntry(machine->tlb[nextTLB].virtualPage, currentThread->space->getID());
   	ipt->table[useIndex].use = FALSE;
   
   	
   	// replace latest entry in TLB
   	UpdateTLBEntry(nextTLB,
   				   ipt->table[iptIndex].physicalPage, 
   				   ipt->table[iptIndex].virtualPage,
   				   ipt->table[iptIndex].dirty, FALSE, FALSE, TRUE);
   	
   	// increment nextTLB
   	nextTLB = (nextTLB + 1) % TLBSize;
   	
   	// print out TLB
	PrintTLB();
	

	(void) interrupt->SetLevel(oldLevel);
	
	  iptLock->Release();	
	// print out IPT
	//ipt->print();
	/*printf("MAIN MEMORY\n");
	for(int i = 0; i < PageSize; i++){
		if(!machine->mainMemory[(iptIndex*PageSize) + i]){
			printf("%d NULL\n", i);
		}
		else {
			printf("%s", machine->mainMemory[(iptIndex*PageSize) + i]);	
		}
	}
	printf("\n");*/
	
	// release lock on IPT
		
   	DEBUG('a', "PageFaultException handled.\n");
}


void ExceptionHandler(ExceptionType which) {
      //IntStatus oldLevel = interrupt->SetLevel(IntOff);
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
				Create_Syscall(machine->ReadRegister(4), 
					  		   machine->ReadRegister(5));
				break;
	    	case SC_Open:
				DEBUG('a', "Open syscall.\n");
				rv = Open_Syscall(machine->ReadRegister(4), 
						  		  machine->ReadRegister(5));
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
	    		Fork_Syscall(machine->ReadRegister(4),
	    				 	 machine->ReadRegister(5),
	    				 	 machine->ReadRegister(6));
	    		break;
	    	case SC_Yield:
	    		DEBUG('a', "Yield syscall.\n");
	    		Yield_Syscall(machine->ReadRegister(4));
	    		break;
	    	case SC_Exec:
	    		DEBUG('a', "Exec syscall.\n");
	    		rv = Exec_Syscall(machine->ReadRegister(4),
	    					 	  machine->ReadRegister(5));
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
		    case SC_CreateMV:
		    	DEBUG('a', "CreateMV syscall.\n");
		    	rv = CreateMV_Syscall(machine->ReadRegister(4),
		    						  machine->ReadRegister(5),
		    						  machine->ReadRegister(6));
		    	break;
		   
		   case SC_DestroyMV:
		    	DEBUG('a', "DestroyMV syscall.\n");
		    	rv = DestroyMV_Syscall(machine->ReadRegister(4));
		    	break;
		   
		   case SC_GetMV:
		    	DEBUG('a', "GetMV syscall.\n");
		    	rv = GetMV_Syscall(machine->ReadRegister(4),
		    					   machine->ReadRegister(5));
		    	break;
		   
		   case SC_SetMV:
		    	DEBUG('a', "SetMV syscall.\n");
		    	rv = SetMV_Syscall(machine->ReadRegister(4),
		    					   machine->ReadRegister(5),
		    					   machine->ReadRegister(6));
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
		DEBUG('a', "PC incremented, at 0x%x\n", machine->registers[PCReg]);
		
		return;
	}
	else if(which == PageFaultException){
		DEBUG('a', "PageFaultException occurred.\n");
		HandlePageFault();
	}
    else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
   // (void) interrupt->SetLevel(oldLevel);
}
