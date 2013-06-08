// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//  Additions to test synchronization methods
//	Simple test cases for the threads assignment.

//#include "copyright.h"
//#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include <cstdlib>
#endif

#ifndef MUSEUM_PARKING
// --------------------------------------------------
// Natural History Museum Parking Simulation
// --------------------------------------------------

// --------------------------------------------------
// Define global variables here
// --------------------------------------------------

#define MAX_NUM_TICKET_TAKERS 		5 
#define MIN_NUM_TICKET_TAKERS 		1 
#define MAX_NUM_VALETS 				5
#define MIN_NUM_VALETS 				1
#define MAX_NUM_CARS 				20
#define MIN_NUM_CARS 				5
#define MAX_NUM_VALETS_ON_BENCH		2
#define	MIN_NUM_CARS_WAITING		4
#define	ON_BENCH					-1
#define	IN_BACK_ROOM				-2 
#define NO_TICKET_AVAILABLE			-1

int numLimoDrivers = 0, numCarDrivers = 0, numValets = 0;
int numCars = 0, numVisitors = 0, numTicketTakers = 0;

// Valet Manager Data
Condition* valetManagerAlertCV; // used to keep the Valet Manager running until 
Lock* valetManagerAlertLock; // there are no more cars to park

// Valet Data
int valetCarNumber[MAX_NUM_VALETS]; // when parking, is the number of car being parked. If valet is in the back room, valetCarNumber[] = -2
Lock* valetCarNumberLock; // if valet is on bench but not parking a car, valetCarNumber[] = -1

Condition* valetAlertCV[MAX_NUM_VALETS]; // used to keep the Valets running until 
Lock* valetAlertLock[MAX_NUM_VALETS]; // there are no more cars to park

// Ticket Taker Data
int ticketLineLength[MAX_NUM_TICKET_TAKERS]; // if no ticket taker, line length = -1
Lock* ticketLineLengthLock; // if someone is at ticket taker, line length = 1

Condition* ticketTakerCV[MAX_NUM_TICKET_TAKERS]; // used to have the Visitor wait while the ticket is processed
Lock* ticketTakerLock[MAX_NUM_TICKET_TAKERS]; // and to signal them when it is accepted

int ticketVisitorNumber[MAX_NUM_TICKET_TAKERS]; // if no ticket is being offered to the Ticket Taker, ticketVisitorNumber[] = -1
Lock* ticketVisitorNumberLock[MAX_NUM_TICKET_TAKERS]; // if a ticket is being offered by a Visitor, ticketVisitorNumber[] = Visitor's index (>= 0)

Lock* ticketLineLock[MAX_NUM_TICKET_TAKERS]; // prevents Visitors from reaching the Ticket Taker out of order

Condition* ticketTakerAlertCV[MAX_NUM_TICKET_TAKERS]; // used to signal the Ticket Taker when a Visitor is ready
Lock* ticketTakerAlertLock[MAX_NUM_TICKET_TAKERS]; // and to put the Ticket Taker to sleep when there is no one to serve


// --------------------------------------------------
// Print menu function
// --------------------------------------------------
void PrintMenu(){
	printf("---------------------------------\n");
	printf("---------------------------------\n");
	printf("Beginning museum parking simulation. \n");
}

// --------------------------------------------------
// Visitor function
// --------------------------------------------------
void Visitor(int index) {
	// Data
	int lineIndex = 0, visitDuration = 0;
	
	// Begin Ticket Taker interaction
	// Choose a Ticket Taker to line up for
	ticketLineLengthLock->Acquire(); // acquire the lock around ticketLineLength[]
	for(int i = (lineIndex + 1); i < numTicketTakers; i++){ // find the Ticket Taker with the shortest line 
		if(ticketLineLength[i] >= 0){ // check to see if the Ticket Taker is available
			if(ticketLineLength[lineIndex] > ticketLineLength[i]){ // if a shorter line length is found
				lineIndex = i; // update the lineIndex
			}
		}
	}
	ticketLineLength[lineIndex]++; // increment the length of line, because the Visitor is now standing in it
	ticketLineLengthLock->Release(); // release the lock around ticketLineLength[]
	
	// Wait in line to give the Ticket Taker a ticket
	printf("%s is waiting for Ticket Taker[%d] \n", 
			currentThread->getName(), lineIndex);
			
	// When ticketLineLock[lineIndex] is acquired, the visitor is at the front of the line
	ticketLineLock[lineIndex]->Acquire();
	
	// When ticketTakerLock[lineIndex] is acquired, the visitor is ready to interact with the Ticket Taker
	ticketTakerLock[lineIndex]->Acquire(); // acquire the lock on the ticketTakerCV
	
	ticketTakerAlertLock[lineIndex]->Acquire(); // acquire the lock in ticketTakerAlertCV
	ticketTakerAlertCV[lineIndex]->Signal(ticketTakerAlertLock[lineIndex]); // alert the Ticket Taker that there is a visitor to serve
	
	ticketVisitorNumberLock[lineIndex]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
	ticketVisitorNumber[lineIndex] = index; // update ticketVisitorNumber[] with my unique index, which serves as my ticket
	ticketVisitorNumberLock[lineIndex]->Release(); // release the lock around ticketVisitorNumber[index]

	ticketTakerAlertLock[lineIndex]->Release(); // release the lock after we have made our ticket available
	
	// Visitor has made their ticket available to the Ticket Taker
	printf("%s has given their ticket to TicketTaker[%d] \n", 
			currentThread->getName(), lineIndex);
			
	// Wait for the Ticket Taker to approve the ticket
	ticketTakerCV[lineIndex]->Wait(ticketTakerLock[lineIndex]); // wait until the Ticket Taker is available
	ticketTakerLock[lineIndex]->Release(); // the Visitor's ticket was accepted
	
	printf("%s has entered the Museum \n", 
			currentThread->getName());
	
	ticketLineLock[lineIndex]->Release(); // release the lock, allowing the next thread to interact with the Ticket Taker
	
	// Begin Museum visit
	visitDuration = (rand() % 50) + 50;
	for(int i = 0; i < visitDuration; i++){
		currentThread->Yield();
	}
	
	// Begin to get ready to leave the Museum
	printf("%s has left the Museum \n", 
			currentThread->getName());
}

// --------------------------------------------------
// Limo Driver function
// --------------------------------------------------
void LimoDriver(int index) {
	
}

// --------------------------------------------------
// Car Driver function
// --------------------------------------------------
void CarDriver(int index) {

}

// --------------------------------------------------
// Valet function
// --------------------------------------------------
void Valet(int index) {
	while(true){
		
	
		valetAlertLock[index]->Acquire(); // acquire the lock in valetAlertCV[]
		valetAlertCV[index]->Wait(valetAlertLock[index]); // wait for a driver to signal saying there are still cars to be parked
		valetAlertLock[index]->Release(); // release the lock in valetAlertCV[]
	}
}

// --------------------------------------------------
// Valet Manager function
// --------------------------------------------------
void ValetManager(int index) {
	while(true){
		// Data
		int valetsOnBench = 0, lastValet = 0;
	
		// check to see if 4 or more cars are waiting to be parked
		for(int i = 0; i < 100; i++){
			currentThread->Yield();
		}
		
		// if 4 or more cars waiting
			//printf("%s has told Parking Valet[%d] to come out of the back room", 
			//		currentThread->getName(), lastValet);
	
		// check to see if more than 2 valets are sitting on bench	
		valetCarNumberLock->Acquire(); // acquire the lock around valetCarNumber[]
		for(int i = 0; i < numValets; i++){ 
			if(valetCarNumber[i] == ON_BENCH){ // if valet is sitting on bench
				valetsOnBench++; // increment number of benched valets
				lastValet = i; // record position of last benched valet
			}
		}
		if(valetsOnBench > MAX_NUM_VALETS_ON_BENCH){
			// send one valet to back room
			printf("%s has sent Parking Valet[%d] to the back room", 
					currentThread->getName(), lastValet);
		}
		
		for(int i = 0; i < 100; i++){
			currentThread->Yield();
		}
		
		valetManagerAlertLock->Acquire(); // acquire the lock in valetManagerAlertCV
		valetManagerAlertCV->Wait(valetManagerAlertLock); // wait for a Valet to signal saying there are still cars to be parked
		valetManagerAlertLock->Release(); // release the lock in valetManagerAlertCV
	}
}

// --------------------------------------------------
// Ticket Taker function
// --------------------------------------------------
void TicketTaker(int index) {
	while(true){
		// Ticket Taker must acquire the lock in ticketTakerCV[index] before checking the ticket
		ticketTakerLock[index]->Acquire(); // acquire the lock in tickerTakerCV[index]
		ticketVisitorNumberLock[index]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
	
		if(ticketVisitorNumber[index] >= 0){ // the Visitor at the front of my line is offering a ticket
			printf("%s has received a ticket from Visitor[%d] \n", 
					currentThread->getName(), ticketVisitorNumber[index]);
			ticketTakerCV[index]->Signal(ticketTakerLock[index]); // we are taking their ticket, so we should tell them not to wait 
	
			printf("%s has accepted a ticket from Visitor[%d] \n", 
					currentThread->getName(), ticketVisitorNumber[index]);
			ticketVisitorNumber[index] = NO_TICKET_AVAILABLE; // update to show that the ticket transaction is over
			
			ticketLineLengthLock->Acquire(); // acquire the lock around ticketLineLength[]
			ticketLineLength[index]--; // decrement the length of line, because the Visitor is leaving
			ticketLineLengthLock->Release(); // release the lock around ticketLineLength[]
		}
		
		ticketVisitorNumberLock[index]->Release(); // release the lock around ticketVisitorNumber[index]
		ticketTakerLock[index]->Release(); // release the lock in tickerTakerCV[index]
		
		ticketTakerAlertLock[index]->Acquire(); // acquire the lock in ticketTakerAlertCV
		ticketTakerAlertCV[index]->Wait(ticketTakerAlertLock[index]); // wait for a Visitor to arrive
		ticketTakerAlertLock[index]->Release(); // release the lock in ticketTakerAlertCV
	}
}	

// --------------------------------------------------
// Main thread
// --------------------------------------------------
void MuseumParkingSimulation(){
	
	PrintMenu();	
		
	numValets = 5;
	numTicketTakers = 2;
	numCars = 2;
	
	Thread *t; // Used to fork threads    
    char * buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
        
    // Initialize Valet Manager data
    buffer = new char[256];
	sprintf(buffer, "valetManagerAlertCV");
    valetManagerAlertCV = new Condition(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "valetManagerAlertLock");
	valetManagerAlertLock = new Lock(buffer);

	// Initialize Valet data
	buffer = new char[256];
	sprintf(buffer, "valetCarNumberLock");
	valetCarNumberLock = new Lock(buffer);
	
	for(int i = 0; i < numValets; i++){
		valetCarNumber[i] = ON_BENCH; 
	
		buffer = new char[256];
		sprintf(buffer, "valetAlertCV%d", i);
		valetAlertCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetAlertLock%d", i);		
		valetAlertLock[i] = new Lock(buffer);
	}
            
 	// Initialize Ticket Taker data
    ticketLineLengthLock = new Lock("ticketLineLengthLock");    
        
    for(int i = 0; i < numTicketTakers; i++){
		buffer = new char[256];
		sprintf(buffer, "ticketTakerLock%d", i);
		ticketTakerLock[i] = new Lock(buffer);

		buffer = new char[256];
		sprintf(buffer, "ticketTakerCV%d", i);
		ticketTakerCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "ticketVisitorNumberLock%d", i);
		ticketVisitorNumberLock[i] = new Lock(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "ticketLineLock%d", i);
		ticketLineLock[i] = new Lock(buffer);		
		
		buffer = new char[256];
		sprintf(buffer, "ticketTakerAlertCV%d", i);
		ticketTakerAlertCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "ticketTakerAlertLock%d", i);
		ticketTakerAlertLock[i] = new Lock(buffer);
		
		ticketVisitorNumber[i] = NO_TICKET_AVAILABLE;
	}
	
	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = rand() % 2;
		buffer = new char[256];
		if(limoOrCar == 0){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else {
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	
   		// Create Visitors
   		numPassengers = (rand() % 4) + 2;
		for(int j = 0; j < numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", numVisitors);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, numVisitors);
			numVisitors++;
		}
	}	

	// Create Ticket Takers
	for(int i = 0; i < numTicketTakers; i++){
		buffer = new char[256];
		sprintf(buffer, "Ticket Taker[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)TicketTaker, i);
	}
    
    // Create Valets
/*    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
    
    // Create Valet Manager (only need 1)
    buffer = new char[256];
    sprintf(buffer, "Valet Manager");
	t = new Thread(buffer);
	t->Fork((VoidFunctionPtr)ValetManager, 0);
*/	
	printf("Number of Limousine Drivers = [%d]\n", numLimoDrivers);
	printf("Number of Car Drivers = [%d]\n", numCarDrivers);
	printf("Number of Parking Valets = [%d]\n", numValets);
	printf("Number of Visitors = [%d]\n", numVisitors);
	printf("Number of Ticket Takers = [%d]\n", numTicketTakers);
	printf("Number of Cars = [%d]\n", numCars);
}



#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(), t1_l1.getName());
    t1_s3.P();
    
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(), t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(), t1_l1.getName());
    t1_l1.Acquire(); // TODO: fix me

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {
	printf("%s: is waiting for t2 to be ready to try to acquire the lock %s\n",currentThread->getName(), t1_l1.getName());
    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock
	printf("%s: is aware that t2 is ready \n",currentThread->getName());
    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
    printf("%s: Has completed t4_signaller()\n",currentThread->getName());
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

	printf("---------------------------------\n");
    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);
    
    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ ){
		t1_done.P();
   	}
   	
     // Test 2

	printf("---------------------------------\n");
    printf("Starting Test 2.  Note that it is an error if thread t2_t2 completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

	printf("---------------------------------\n");
    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t3_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

	printf("---------------------------------\n");
    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t4_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

	printf("---------------------------------\n");
    printf("Starting Test 5.  Note that it is an error if thread t5_t1 completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which) {
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest() {
    //DEBUG('t', "Entering SimpleTest");

    //Thread *t = new Thread("forked thread");

    //t->Fork(SimpleThread, 1);
    //SimpleThread(0);
    //TestSuite();
    MuseumParkingSimulation();
}
