// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// PTTranslationEntry::print
// Prints a page table entry, useful for debugging.
//----------------------------------------------------------------------
void PTTranslationEntry::print(int i){
	DEBUG('a', "PT entry: %d\n", i);
	DEBUG('a', "	Virtual page: %d\n", virtualPage);
	DEBUG('a', "	Physical page: %d\n", physicalPage);
	DEBUG('a', "	Valid: %d\n", valid);
	DEBUG('a', "	Dirty: %d\n", dirty);
    DEBUG('a', "	PageType: %d\n", pageType);
    DEBUG('a', "	PageLocation: %d\n", pageLocation);
    DEBUG('a', "	ExecutableByteOffset: %d\n", executableByteOffset);
    DEBUG('a', "	SwapByteOffset: %d\n", swapByteOffset);
}


//----------------------------------------------------------------------
// PrintTLB
// Prints all entries of the TLB, useful for debugging.
//----------------------------------------------------------------------
void PrintTLB(){
    for(int i = 0; i < TLBSize; i++){
    	DEBUG('a', "TLB entry: %d\n", i);
    	DEBUG('a', "	Virtual page: %d\n", machine->tlb[i].virtualPage);
		DEBUG('a', "	Physical page: %d\n",machine->tlb[i].physicalPage);
		DEBUG('a', "	Valid: %d\n", machine->tlb[i].valid);
		DEBUG('a', "	Dirty: %d\n", machine->tlb[i].dirty);
    }
}


//----------------------------------------------------------------------
// InvalidateAllTLB
// Called on a context switch, used to invalidate all TLB entries
//----------------------------------------------------------------------
void InvalidateAllTLB(){
	//iptLock->Acquire();
	//printf("CONTEXT SWITCH\n");
    for(int i = 0; i < TLBSize; i++){
    	// propagate dirty bits to IPT
    	if(machine->tlb[i].valid){
    		ipt->table[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
    	}
    	// invalidate every entry in TLB
    	machine->tlb[i].valid = FALSE;
    }
    //iptLock->Release();
}

//----------------------------------------------------------------------
// SearchForTLBEntry
// Returns the index of a valid TLB entry that matches the given virtual
// address and process ID.
//----------------------------------------------------------------------
int SearchForTLBEntry(int vpn){
	int index = -1;
	for(int i = 0; i < TLBSize; i++){
		if(machine->tlb[i].virtualPage == vpn){		
			if(machine->tlb[i].valid){
				index = i;
				break;
			}
		}
	}
	return index;
}

//----------------------------------------------------------------------
// UpdateTLBEntry
// Update an entry in the TLB with the given data.
//----------------------------------------------------------------------
void UpdateTLBEntry(int index, int paddr, int vaddr, bool dirtyBit, 
					bool useBit, bool readOnlyBit, bool validBit){
					
   	machine->tlb[index].physicalPage = paddr;
   	machine->tlb[index].virtualPage = vaddr;
   	machine->tlb[index].dirty = dirtyBit;
   	machine->tlb[index].use = useBit;
   	machine->tlb[index].readOnly = readOnlyBit;
   	machine->tlb[index].valid = validBit;
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {

    NoffHeader noffH;
    unsigned int i, size;
    int availablePage;
    makeNewPTLock = new Lock("makeNewPTLock");
    pageMapLock = new Lock("pageMapLock");
    executableFile = executable;

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executableFile->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC)){
    	SwapHeader(&noffH);
    }
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    
    // we need to increase the size to leave room for the stack
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize); // we need to increase the size to leave room for the stack
    printf("Num Pages: %i\n",numPages);
    size = numPages * PageSize;

    //ASSERT(numPages <= NumPhysPages);		// check we're not trying to run anything too big --
											// at least until we have virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    
	// first, set up the translation 
    makeNewPTLock->Acquire();
    pageNumbers = new int[numPages];
    pageTable = new PTTranslationEntry[numPages];
    
    for (i = 0; i < numPages; i++) {
    	DEBUG('a', "Creating page %d\n", i);
    	/*availablePage = pageMap->Find(); // find a virtual address 
    	if (availablePage == -1){	
    		printf("There is no more space for the stack\n");
    		break;
    	}*/
		//pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		//pageTable[i].physicalPage = availablePage;//availablePage;
		//pageNumbers[i] = availablePage;
		pageTable[i].virtualPage = i;
		pageTable[i].valid = FALSE; // cannot set as valid until page is "loaded" into memory
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on a separate page, we could set its pages to be read-only
    	pageTable[i].pageLocation = EXECUTABLE; // most pages are located in the executable at this point
		pageTable[i].setSwapByteOffset(-1); // we haven't ever written to the swap file, so the byte offset is invalid
		
    	//if ((i*PageSize) < (unsigned int)noffH.code.size){
    		pageTable[i].setExecutableByteOffset(noffH.code.inFileAddr+(i*PageSize));
    	//	pageTable[i].pageType = CODE;
    		DEBUG('a', "Initializing code page %d at 0x%x \n", i, noffH.code.inFileAddr+(i*PageSize));
   		 	//executableFile->ReadAt(&(machine->mainMemory[availablePage*PageSize]),PageSize,noffH.code.inFileAddr+(i*PageSize));
    	//}
    	//else if(((i*PageSize) > (unsigned int)noffH.code.size) && ((i*PageSize) < (unsigned int)(noffH.code.size + noffH.initData.size))){
    	//	pageTable[i].setExecutableByteOffset(noffH.initData.inFileAddr+(i*PageSize));
    	//	pageTable[i].pageType = DATA;
    	//	DEBUG('a', "Initializing data page %d at 0x%x \n", i, noffH.initData.inFileAddr+(i*PageSize));
  	 		//executable->ReadAt(&(machine->mainMemory[availablePage*PageSize]),PageSize,noffH.initData.inFileAddr+(i*PageSize));
  	 	//}
  	 	//else {
  	 	//	pageTable[i].setExecutableByteOffset(noffH.initData.inFileAddr+(i*PageSize));
    	//	pageTable[i].pageType = DATA;
    	//	DEBUG('a', "Initializing other page %d at 0x%x \n", i, noffH.initData.inFileAddr+(i*PageSize));
  	 	//}
    }

    makeNewPTLock->Release();
    DEBUG('a', "Finished creating page table\n");
    // print out PT
	for(i = 0; i < numPages; i++){
		pageTable[i].print(i);
	}
   	 //DEBUG('a', "Debugging break\n");
   	 //interrupt->Halt();
    
	// zero out the entire address space, to zero the unitialized data segment and the stack segment
    /* bzero(machine->mainMemory, size);

	// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]), noffH.code.size, noffH.code.inFileAddr);
	
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]), noffH.initData.size, noffH.initData.inFileAddr);
    }*/
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
	// TODO: check to see if next thread is from same process
    InvalidateAllTLB(); // invalidate every entry in TLB
    //machine->pageTable = pageTable;
    //machine->pageTableSize = numPages;
}

//----------------------------------------------------------------------
// AddrSpace::MakeNewPT()
// 
//----------------------------------------------------------------------

int AddrSpace::MakeNewPT()
{
    unsigned int i;
    int availablePage;
    makeNewPTLock->Acquire(); // acquire lock 
    PTTranslationEntry * pageTable2 = new PTTranslationEntry[numPages+8]; // create a new page table with 8 extra pages
    for (i = 0; i < numPages; i++)
    {
    	pageTable2[i].virtualPage = pageTable[i].virtualPage;	// for now, virtual page # = phys page #
		pageTable2[i].pageLocation = pageTable[i].pageLocation;
		pageTable2[i].pageType = pageTable[i].pageType;
		pageTable2[i].valid = pageTable[i].valid;
		pageTable2[i].use = pageTable[i].use;
		pageTable2[i].dirty = pageTable[i].dirty;
		pageTable2[i].readOnly = pageTable[i].readOnly; 
		pageTable2[i].setExecutableByteOffset(pageTable[i].getExecutableByteOffset());
		pageTable2[i].setSwapByteOffset(pageTable[i].getSwapByteOffset());
    }
    
    pageNumbers = new int[8];
    for (i = numPages; i < (numPages + 8); i++)
    {
    	pageTable2[i].virtualPage = i;
    	//pageTable2[i].pageLocation = STACK_SPACE;
    	pageTable2[i].pageType = STACK;
    	pageTable2[i].valid = FALSE;
    	pageTable2[i].use = FALSE;
    	pageTable2[i].dirty = FALSE;
    	pageTable2[i].readOnly = FALSE;
    	pageTable2[i].setExecutableByteOffset(-1);
    	pageTable2[i].setSwapByteOffset(-1);
    	
    	pageLocation[currentThread->space->getID()][i] = 2;
    	
    	
    	/*availablePage = pageMap->Find();
    	if (availablePage == -1)
    	{
    		printf("There is no more space for the stack\n");
    		return -1;
    	}
    	pageTable2[i].virtualPage = i;	// for now, virtual page # = phys page #
		//pageTable2[i].physicalPage = availablePage;
		pageTable2[i].valid = FALSE; // cannot set as valid until page is "loaded" into memory
		pageTable2[i].use = FALSE;
		pageTable2[i].dirty = FALSE;
		pageTable2[i].readOnly = FALSE; 
		//pageNumbers[i - numPages] = availablePage;
		*/
    }
    delete [] pageTable; // free up the memory from the old page table
    pageTable = pageTable2; // point to the new page table
    numPages+=8; // update numPages
    //machine->pageTable = pageTable; // point the machine to the new page table
    //machine->pageTableSize = numPages; // update the machine's pageTableSize variable
    int loc = (numPages*PageSize) - 16; // record the new starting point in a local variable
    makeNewPTLock->Release(); // release lock
    return loc;   // return the new starting point
}
void AddrSpace::DestroyStack(int i)
{
	makeNewPTLock->Acquire();
	i+=16;
	i/=PageSize;
	i-=8;
	for (int a = i; a < (i+8); a++)
	{
		if (a > 0)
			pageMap->Clear(pageTable[a].physicalPage);
	}
	makeNewPTLock->Release();
	
}
