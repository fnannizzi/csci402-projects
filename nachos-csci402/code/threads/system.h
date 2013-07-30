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
#include "synch.h"

typedef struct Message {
	char* name;
	char* name2;
	char* request;
	int index, index2, index3;
	int ID, clientMachine, clientMailbox;
};

enum PageReplacementPolicy { FIFO, RAND };

//----------------------------------------------------------------------
// IPTTranslationEntry
// Class for IPT entries that inherits from TranslationEntry. Added a 
// processID to track the owner process of a page, and a timestamp for
// least-recently-used eviction.
//----------------------------------------------------------------------
class IPTTranslationEntry: public TranslationEntry {
	public:
		int processID;
		time_t timestamp;		
};

//----------------------------------------------------------------------
// InvertedPageTable
// Class that contains the IPT and some handy methods for operations on
// the IPT. 
//----------------------------------------------------------------------
class InvertedPageTable {
	public:
		IPTTranslationEntry* table;				// actual IPT
		
		InvertedPageTable();					// class constructor
		~InvertedPageTable();					// class destructor
		void print();							// print function for debugging
		int searchForEntry(int, int);			// finds a specified entry in IPT
		int searchForEmptyEntry();				// finds an empty entry in IPT
		void updateEntry(int, int, int, bool);	// updates an IPT entry 
		int chooseEvictionPageLRU();			// finds an eviction page using LRU method
		int chooseEvictionPageRandom();			// finds an eviction page randomly
		List * queue;
};


// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization, called before anything else
extern void Cleanup();	// Cleanup, called when Nachos is done.

extern Thread *currentThread;		// the thread holding the CPU
extern Thread *threadToBeDestroyed; // the thread that just finished
extern Scheduler *scheduler;		// the ready list
extern Interrupt *interrupt;		// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

// Added methods and objects for virtual memory
extern PageReplacementPolicy pageReplacementPolicy; // choose between FIFO or RAND replacement 
extern InvertedPageTable *ipt; 	// inverted page table used to track pages in memory	
extern Lock* iptLock;			// lock on IPT to prevent race conditions	
extern OpenFile* swapFile;		// swap file used to store dirty pages that have been evicted from memory
extern BitMap* swapFileMap;	// used to track open locations in swap file
extern int nextTLB;				// the next TLB location to write to
extern int ** swapArray;
extern int ** pageLocation;

// Networking
extern int numServers;
extern int IDer;

extern void PrintTLB();			// prints TLB entries for debugging
extern void InvalidateAllTLB();	// invalidate TLB entries on a context switch
extern int SearchForTLBEntry(int vpn); // find a specified TLB entry
extern void UpdateTLBEntry(int index, int paddr, int vaddr, bool dirtyBit, 
						   bool useBit, bool readOnlyBit, bool validBit);	// update an entry in the TLB					   
extern char* msgPrepare(Message* msg);
extern Message* decodeMessage(char* buf);

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
extern void Server();
extern void MessageHandler();
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
