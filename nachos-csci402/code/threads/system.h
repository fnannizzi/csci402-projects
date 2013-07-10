// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "translate.h"
#include "../filesys/openfile.h"

class IPTTranslationEntry: public TranslationEntry {
	public:
		int processID;
		time_t timestamp;		
};

class InvertedPageTable {
	public:
		IPTTranslationEntry* table;
		
		InvertedPageTable();
		~InvertedPageTable();	
		void print();			
		int searchForEntry(int, int);
		int searchForEmptyEntry();
		void updateEntry(int, int, int, bool);
		int chooseEvictionPage();
};


// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

// Added methods and objects for virtual memory
extern InvertedPageTable *ipt; 
extern OpenFile* swapFile;
extern int nextTLB;
extern void printTLB();
extern void invalidateAllTLB();
extern int searchForTLBEntry(int vpn, int processID);
extern void updateTLBEntry(int index, int paddr, int vaddr, bool dirtyBit, 
						   bool useBit, bool readOnlyBit, bool validBit);						   

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
extern BitMap* pageMap;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
