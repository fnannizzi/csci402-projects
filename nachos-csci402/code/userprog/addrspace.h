// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

enum PageType 		{ CODE, DATA, STACK };
enum PageLocation 	{ SWAP_FILE, EXECUTABLE };

//----------------------------------------------------------------------
// PTTranslationEntry
// Class used to create page tables that inherits from TranslationEntry.
// added a page type and location, plus byte offsets for the executable
// and swap files.
//----------------------------------------------------------------------
class PTTranslationEntry: public TranslationEntry {
	public:
		PageType pageType;
		PageLocation pageLocation;
		
		void print(int);
		void setExecutableByteOffset(int offset){ executableByteOffset = offset; }
		int getExecutableByteOffset(){ return executableByteOffset; }
		void setSwapByteOffset(int offset){ swapByteOffset = offset; }
		int getSwapByteOffset(){ return swapByteOffset; }
	
	private:
		unsigned int executableByteOffset;	
		unsigned int swapByteOffset;	
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
										// initializing it with the program
										// stored in the file "executable"
    ~AddrSpace();	// De-allocate an address space

    void InitRegisters();	// Initialize user-level CPU registers,
							// before jumping to user code

    void SaveState();		// Save/restore address space-specific
    void RestoreState();	// info on a context switch
    Table fileTable;		// Table of openfiles
    /* Started editing */
    PTTranslationEntry *pageTable;	// Assume linear page table translation for now!
    Lock* makeNewPTLock;
    Lock* pageMapLock;
    int* pageNumbers; 		// used to track individual address space
    OpenFile* executableFile; 		// executable file should remain open
    
    int MakeNewPT();
    void DestroyStack(int i);
    int getID() { return id; } // return process ID
    void setID(int i) { id = i; } // set the process ID
    int getNumPages() { return numPages; } // return the number of pages needed for a process
    PTTranslationEntry getPageTableEntry(int entry) { return pageTable[entry]; } // get a specific page table entry
    /* Stopped editing */

 private:
 
    unsigned int numPages; // Number of pages in the virtual address space
    int id;	// added a process ID 
};

#endif // ADDRSPACE_H
