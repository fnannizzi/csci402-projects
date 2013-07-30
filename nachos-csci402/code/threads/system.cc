// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include <sstream>

#define NumPhysPages	32 //Added this too
#define NumSwapPages	10000

//----------------------------------------------------------------------
// InvertedPageTable::InvertedPageTable
// Constructor for IPT class, initializes important data to be null or
// invalid
//----------------------------------------------------------------------
InvertedPageTable::InvertedPageTable(){
	table = new IPTTranslationEntry[NumPhysPages];
	queue = new List();
	for (int i = 0; i < NumPhysPages; i++){
		table[i].virtualPage = NULL;
    	table[i].physicalPage = NULL;
		table[i].valid = FALSE;
	}
}

//----------------------------------------------------------------------
// InvertedPageTable::~InvertedPageTable
// Destructor for IPT class, just needs to delete the table. 
//----------------------------------------------------------------------
InvertedPageTable::~InvertedPageTable(){
	delete table;
}

//----------------------------------------------------------------------
// InvertedPageTable::print
// Prints the IPT entries for debugging purposes.
//----------------------------------------------------------------------
void InvertedPageTable::print(){
    for(int i = 0; i < NumPhysPages; i++){
    	DEBUG('a', "IPT entry: %d\n", i);
    	DEBUG('a', "	ProcessID: %d\n", table[i].processID);
    	DEBUG('a', "	Virtual page: %d\n", table[i].virtualPage);
		DEBUG('a', "	Physical page: %d\n", table[i].physicalPage);
		DEBUG('a', "	Valid: %d\n", table[i].valid);
		DEBUG('a', "	Dirty: %d\n", table[i].dirty);
    }
}

//----------------------------------------------------------------------
// InvertedPageTable::searchForEntry
// Find an entry in the IPT by matching the virtual page number and 
// process ID, entry must also be valid.
//----------------------------------------------------------------------
int InvertedPageTable::searchForEntry(int vpn, int processID){
	int index = -1;
	for(int i = 0; i < NumPhysPages; i++){
		if(table[i].virtualPage == vpn){
		//printf("Process comparison: %i vs %i\n",table[i].processID,processID);		
			if(table[i].processID == processID){
				if(table[i].valid){
					index = i;
					break;
				}
			}
		}
	}
	
	return index;
}

//----------------------------------------------------------------------
// InvertedPageTable::searchForEmptyEntry
// Find an empty entry in the IPT if one is available. Otherwise, return
// -1 and make HandlePageFault() evict a page. 
//----------------------------------------------------------------------
int InvertedPageTable::searchForEmptyEntry(){
	int index = -1;
	for(int i = 0; i < NumPhysPages; i++){
		if(!table[i].valid){	
			index = i;
			break;
		}
	}
	
	return index;
}

//----------------------------------------------------------------------
// InvertedPageTable::updateEntry
// Update an IPT entry with the given data. 
//----------------------------------------------------------------------
void InvertedPageTable::updateEntry(int index, int vaddr, int process, bool validBit){
	table[index].virtualPage = vaddr;
	table[index].physicalPage = index;
	table[index].processID = process;
	table[index].valid = validBit;
	table[index].dirty = FALSE;
	//(2)
	table[index].use = TRUE;
	table[index].readOnly = FALSE;
	queue->Append((void*)index);
}

//----------------------------------------------------------------------
// InvertedPageTable::chooseEvictionPageLRU
// Choose a page to evict from the IPT based on when it was last used.
//----------------------------------------------------------------------
int InvertedPageTable::chooseEvictionPageLRU(){
	/*time_t oldestTime = table[0].timestamp;
	int oldestIndex = 0;
	for(int i = 0; i < NumPhysPages; i++){
		if(table[i].timestamp < oldestTime){ // page is older
			oldestTime = table[i].timestamp;
			oldestIndex = i;
		}
	}
	return oldestIndex;*/
	int index = (int)queue->Remove();
	return index;
}

//----------------------------------------------------------------------
// InvertedPageTable::chooseEvictionPageRandom
// Choose a page to evict from the IPT randomly.
//----------------------------------------------------------------------
int InvertedPageTable::chooseEvictionPageRandom(){
	srand(time(NULL));
	//(1)
	int randomIndex = -1;
	while (randomIndex == -1)
	{
		randomIndex = rand() % NumPhysPages;
		if (table[randomIndex].use)
			randomIndex = -1;
	}
	return randomIndex;
}

//----------------------------------------------------------------------
// msgPrepare
// Create a char* msg from a Message object
//----------------------------------------------------------------------
char* msgPrepare(Message* msg){
	stringstream ss;
	ss << msg->request[0];
	ss << msg->request[1];
	ss << ' ';
	ss << msg->name;
	ss << ' ';
	ss << msg->name2;
	ss << ' ';
	ss << msg->index;
	ss << ' ';
	ss << msg->index2; 
	ss << ' ';
	ss <<  msg->index3;
	ss << ' ';
	ss << msg->ID;
	ss << ' ';
	ss << msg->clientMachine;
	ss << ' ';
	ss << msg->clientMailbox;
	char* d = new char[MaxMailSize];
	strcpy(d, ss.str().c_str());
	return d;
}

//----------------------------------------------------------------------
// decodeMessage
// Create a message object from a char*
//----------------------------------------------------------------------
Message* decodeMessage(char* buf){
	Message* message = new Message;
	message->request = new char[2];
	message->name = new char[20];
	message->name2 = new char[20];
	stringstream ss;
	ss << buf;
	ss >> message->request;
	ss >> message->name;
	ss >> message->name2;
	ss >> message->index;
	ss >> message->index2;
	ss >> message->index3;
	ss >> message->ID;
	ss >> message->clientMachine;
	ss >> message->clientMailbox;
	return message;
}

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  	// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;				// performance metrics
Timer *timer;					// the hardware timer device for invoking context switches
InvertedPageTable *ipt; 		// inverted page table used to track pages in memory
OpenFile* swapFile;				// swap file that stores dirty pages which have been evicted from memory
BitMap* swapFileMap;			// used to track open pages in swap file
Lock* iptLock;					// lock on IPT to prevent race conditions
int nextTLB;					// next entry of the TLB to be written to
int ** swapArray;
int ** pageLocation;
PageReplacementPolicy pageReplacementPolicy; // choose between FIFO or RAND replacement

// Networking
int numServers;
int IDer;

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
BitMap * pageMap;
#endif

#ifdef NETWORK
PostOffice *postOffice;
#endif



// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;
    
    bool isServer = FALSE, setNetname = FALSE;
    IDer = 0;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    setNetname = TRUE;
	    argCount = 2;
	}
	if(!strcmp(*argv, "-nS")){
    	ASSERT(argc > 1);
        numServers = atoi(*(argv + 1));
    	ASSERT(numServers > 0);
    	ASSERT(numServers < 6);
        printf("numServers: %d\n", numServers);
    }
	if (!strcmp(*argv, "-SERVER")) {
		isServer = TRUE;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);
	
    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    DEBUG('a', "USER_PROGRAM flags set \n");
    machine = new Machine(debugUserProg);	// this must come first
    pageMap = new BitMap(NumPhysPages);	// Need to make sure this is updated
    
    // Initialize the TLB
	for (int i = 0; i < TLBSize; i++){
    	machine->tlb[i].virtualPage = NULL;
    	machine->tlb[i].physicalPage = NULL;
		machine->tlb[i].valid = FALSE;
	}
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
    DEBUG('a', "FILESYS flags set \n");
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
	DEBUG('a', "NETWORK flags set \n");
	
	if(isServer && setNetname){
		Thread* t = new Thread("server");
		t->Fork((VoidFunctionPtr)Server, 1);
		MessageHandler();
	}
#endif

	// Added virtual memory objects 
	ipt = new InvertedPageTable();
	iptLock = new Lock("iptLock");
	swapFileMap = new BitMap(NumSwapPages);
	nextTLB = 0;
	swapArray = new int*[10];
	pageLocation = new int*[10];
	for (int a = 0; a < 10; a++)
	{
		swapArray[a] = new int[1000];
		pageLocation[a] = new int[1000];
		for (int b = 0; b < 1000; b++)
		{
			pageLocation[a][b] = 0;
		}
	}

	// Create and open the swap file
	bool success = fileSystem->Create("swap_file", (PageSize*NumSwapPages));
    ASSERT(success); // if we can't create the swap file, abort
    swapFile = fileSystem->Open("swap_file");
    ASSERT(swapFile); // if Open returns null, abort

}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    if(ipt != NULL){
    	delete ipt;
    	delete iptLock;
    }
    
    Exit(0);
}

