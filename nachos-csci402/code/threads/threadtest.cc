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

// General constants
#define MAX_NUM_TICKET_TAKERS 		5 
#define MIN_NUM_TICKET_TAKERS 		1 
#define MAX_NUM_VALETS 				5
#define MIN_NUM_VALETS 				1
#define MAX_NUM_CARS 				20
#define MIN_NUM_CARS 				5
// Valet Mananger constants
#define MAX_NUM_VALETS_ON_BENCH		2
#define	MIN_NUM_VEHICLES_WAITING	4
#define YIELD_DURATION				20
// Valet constants (not using an enum because array is also used to hold car indexes)
#define	WAITING_ON_BENCH			-1
#define	GOING_TO_BACK_ROOM			-2 
#define IN_BACK_ROOM				-3
#define IS_PARKING_CAR				-4
#define ON_BENCH_NOT_WAITING		-5

#define MIN_PARKING_DURATION		5
#define MAX_PARKING_DURATION		15

// Ticket Taker constants
#define NO_TICKET_AVAILABLE			-1
// Visitor constants
#define MIN_VISIT_DURATION			50

// Enum for Driver states
typedef enum { WAITING_TO_PARK, WAITING_FOR_LIMOS, PARKING_CAR, WAITING_TO_LEAVE } carStatus;

// General data
int numLimoDrivers = 0, numCarDrivers = 0, numValets = 0;
int numCars = 0, numVisitors = 0, numTicketTakers = 0;

int numCarsWaitingToPark = 0, numLimosWaitingToPark = 0;

// Valet Manager Data
Condition* valetManagerAlertCV; // used to keep the Valet Manager running until 
Lock* valetManagerAlertLock; // there are no more cars to park

// Valet Data
int valetCarNumber[MAX_NUM_VALETS]; // when parking, is the number of car being parked. If valet is in the back room, valetCarNumber[] = -2
Lock* valetCarNumberLock[MAX_NUM_VALETS]; // if valet is on bench but not parking a car, valetCarNumber[] = -1

Condition* valetStatusCV[MAX_NUM_VALETS];
Lock* valetStatusLock[MAX_NUM_VALETS];

Lock* valetLimoLineLock; // used to queue the waiting limos
Lock* valetCarLineLock; // used to queue the waiting cars

Lock* valetLimoParkingLock[MAX_NUM_VALETS]; // used to communicate with the drivers 
Condition* valetLimoParkingCV[MAX_NUM_VALETS]; // when actually parking their car

Lock* valetCarParkingLock[MAX_NUM_VALETS]; // used to communicate with the drivers 
Condition* valetCarParkingCV[MAX_NUM_VALETS]; // when actually parking their car

Condition* valetAlertCV[MAX_NUM_VALETS]; // used to keep the Valets running until 
Lock* valetAlertLock[MAX_NUM_VALETS]; // there are no more cars to park

// Driver Data
Lock* numCarsWaitingToParkLock; // lock on the global int numCarsWaitingToPark
Lock* numLimosWaitingToParkLock; // lock on the global int numLimosWaitingToPark 

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
	int carIndex = index/1000; // decode the index to determine which car we belong to
	int passengerIndex = index % 1000; // decode the index to determine which passenger position in the car we are
	
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
	visitDuration = (rand() % MIN_VISIT_DURATION) + MIN_VISIT_DURATION;
	for(int i = 0; i < visitDuration; i++){ currentThread->Yield(); }
	
	// Begin to get ready to leave the Museum
	printf("%s has left the Museum \n", 
			currentThread->getName());
}

// --------------------------------------------------
// Limo Driver function
// --------------------------------------------------
void LimoDriver(int index) {
	// Data
	carStatus status = WAITING_TO_PARK;
	int valetIndex = 0;
	
	while(true){
		if(status == WAITING_TO_PARK){ // waiting in line for the valet			

			// Wait in the line of limos to be parked
			valetLimoLineLock->Acquire(); // when acquired you are at the front of the line
			
			// TODO: acquire a lock on the CV for the car
			// broadcast on that lock to all passengers, telling them to leave
			// release the lock
			
			status = PARKING_CAR;
		}
		else if(status == PARKING_CAR){ // interacting with valet to park car and exchange keys and token
			for(int i = 0; i < numValets; i++){		
				valetCarNumberLock[i]->Acquire(); // acquire the locks around valetCarNumber[]
			}
			for(int i = 0; i < numValets; i++){
				if(valetCarNumber[i] == WAITING_ON_BENCH){ // if valet is sitting on the bench
					valetIndex = i;
					break;
				}
			}
			if(valetCarNumber[valetIndex] != WAITING_ON_BENCH){ // no valets are available
				for(int i = 0; i < numValets; i++){	
					valetCarNumberLock[i]->Release(); // release the locks around valetCarNumber[]
				}
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
			else { // found an available valet
				// TODO: Move to section where visitors actually leave car
				printf("%s has told his visitors to leave Car[%d] \n", 
						currentThread->getName(), index);
				printf("%s has parked Car[%d] at the Museum \n", 
						currentThread->getName(), index);
				valetLimoParkingLock[valetIndex]->Acquire(); // acquire the lock on the valet, so we are ready to interact with him
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				
				valetCarNumber[valetIndex] = index; // make the valet unavailable, and let him know which car is being parked
				
				for(int i = 0; i < numValets; i++){	
					valetCarNumberLock[i]->Release(); // release the locks around valetCarNumber[]
				}
				
				valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // wake up valet if he is sleeping
				printf("%s signalling valet \n", currentThread->getName());
				valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
				printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
									
				valetLimoParkingCV[valetIndex]->Wait(valetLimoParkingLock[valetIndex]); // wait for the valet to park the car
				printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
	
				valetLimoParkingLock[valetIndex]->Release(); // release the lock on the valet, now that the car is parked
				valetLimoLineLock->Release(); // allow the next driver in line to interact with the valets

				status = WAITING_TO_LEAVE; // visitors are inside the museum now
			}
		}	
		else if(status == WAITING_TO_LEAVE){
			break;
		}
		printf("%s looping \n", currentThread->getName());
	}
}

// --------------------------------------------------
// Car Driver function
// --------------------------------------------------
void CarDriver(int index) {
	// Data
	carStatus status = WAITING_TO_PARK;
	int valetIndex = 0, numLimosWaiting = 0;
	
	while(true){
		if(status == WAITING_TO_PARK){ // waiting in line for the valet			
		
			// Wait in the line of cars to be parked
			valetCarLineLock->Acquire(); // when acquired you are at the front of the line
			printf("%s at front of car line \n", currentThread->getName());
			// TODO: acquire a lock on the CV for the car
			// broadcast on that lock to all passengers, telling them to leave
			// release the lock
			
			status = WAITING_FOR_LIMOS;
		}
		else if(status == WAITING_FOR_LIMOS){ // at the front of the car line, but there are limos to be parked
			numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
			numLimosWaiting = numLimosWaitingToPark; // update our local copy of the variable
			numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			
			// Check to see if any limos are waiting to park
			if(numLimosWaiting == 0){ // no limos waiting
				printf("%s no limos left \n", currentThread->getName());
				status = PARKING_CAR;
			}
			else { // must wait for limos to be parked
				//printf("%s yielding \n", currentThread->getName());
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
		}
		else if(status == PARKING_CAR){ // interacting with valet to park car and exchange keys and token
			for(int i = 0; i < numValets; i++){		
				valetCarNumberLock[i]->Acquire(); // acquire the locks around valetCarNumber[]
			}
			for(int i = 0; i < numValets; i++){
				if(valetCarNumber[i] == WAITING_ON_BENCH){
					valetIndex = i;
					break;
				}
			}
			if(valetCarNumber[valetIndex] != WAITING_ON_BENCH){ // no valets are available
				for(int i = 0; i < numValets; i++){	
					valetCarNumberLock[i]->Release(); // release the locks around valetCarNumber[]
				}
			}
			else { // found an available valet
				//TODO: Move to the section where visitors actually get out
				printf("%s has told his visitors to leave Car[%d] \n", 
						currentThread->getName(), index);
				printf("%s has parked Car[%d] at the Museum \n", 
						currentThread->getName(), index);
				
				valetCarParkingLock[valetIndex]->Acquire(); // acquire the lock on the valet, so we are ready to interact with him					
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				valetCarNumber[valetIndex] = index; // make the valet unavailable, and give him the keys to the car being parked
				for(int i = 0; i < numValets; i++){	
					valetCarNumberLock[i]->Release(); // release the locks around valetCarNumber[]
				}
				
				valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // alert the valet that there is a car waiting to be parked
				printf("%s signalling valet \n", currentThread->getName());
				valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
				printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
										
				valetCarParkingCV[valetIndex]->Wait(valetCarParkingLock[valetIndex]); // wait for the valet to park the car
				printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
						
				valetCarParkingLock[valetIndex]->Release(); // release the lock on the valet, now that the car is parked
				valetCarLineLock->Release(); // allow the next driver in line to interact with the valets
				
				status = WAITING_TO_LEAVE; // visitors are inside the museum now
			}
		}	
		else if(status == WAITING_TO_LEAVE){
			break;
		}
		//printf("%s looping \n", currentThread->getName());
	}
}

// --------------------------------------------------
// Valet function
// --------------------------------------------------
void Valet(int index) {
	// Data
	int valetStatus = ON_BENCH_NOT_WAITING; // just a reminder that all valets need to be waiting before drivers can signal them
	int numCarsWaiting = 0, numLimosWaiting = 0;
	int parkingDuration = 0;
	bool sleepingOnBench = false;
	
	while(true){	
		valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
		valetStatus = valetCarNumber[index]; // read the updated status
		valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]

		valetStatusLock[index]->Acquire(); // acquire the lock on valetStatusCV[]

		if(valetStatus == GOING_TO_BACK_ROOM){ // if valet is sent to the back room
			printf("%s is going to the back room \n",
					currentThread->getName());
					
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
			valetCarNumber[index] = IN_BACK_ROOM; // update status
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
		}
		else if(valetStatus == IN_BACK_ROOM){ // if valet is in the back room
			valetStatusCV[index]->Wait(valetStatusLock[index]); // wait on a signal to be brought back to the bench
			printf("%s is coming out of the back room \n",
					currentThread->getName());
						
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
			valetCarNumber[index] = ON_BENCH_NOT_WAITING; // update status
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
		}
		else if(valetStatus == ON_BENCH_NOT_WAITING){
			// Confusing design - the valet needs to wait on a signal from a driver, so we change his 
			// state to WAITING_ON_BENCH just before having him wait (essentially sleep on the bench)
			// This is to prevent drivers from signalling a valet who isn't waiting
			
			// Wait to be signaled by a driver or the Valet Manager
			valetAlertLock[index]->Acquire(); // acquire the lock in valetAlertCV[]
			
			// Determine number of limos waiting to be parked
			numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
			numLimosWaiting = numLimosWaitingToPark; // update our copy of the variable
			numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			
			// Determine number of cars waiting to be parked
			numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
			numCarsWaiting = numCarsWaitingToPark; // update our copy of the variable
			numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark
			
			if((numCarsWaiting + numLimosWaiting) == 0){ // if there are no vehicles waiting
				if(!sleepingOnBench){
					printf("%s is going to sleep on the bench \n",
							currentThread->getName());
					sleepingOnBench = true; 
				}
			}
			
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
			valetCarNumber[index] = WAITING_ON_BENCH; // update status
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
			
			printf("%s waiting \n", currentThread->getName());
			valetAlertCV[index]->Wait(valetAlertLock[index]); // wait for a driver to signal saying there are still cars to be parked
			printf("%s signaled  \n", currentThread->getName());
			valetAlertLock[index]->Release(); // release the lock in valetAlertCV[]	
		}
		else if(valetStatus == WAITING_ON_BENCH){ 
			// Essentially a dummy state - the valet is only here while he is waiting on a signal from
			// a driver, who then changes his state to their index
		}
		else if(valetStatus == IS_PARKING_CAR){
			// Alert the valet manager that there are cars being parked
			valetManagerAlertLock->Acquire(); // acquire the lock in valetManagerAlertCV
			valetManagerAlertCV->Signal(valetManagerAlertLock); // signal to wake up the valet manager
			valetManagerAlertLock->Release(); // release the lock in valetManagerAlertCV
			
			valetStatusLock[index]->Release(); // release the lock on valetStatusCV[]
			
			// Parking the car should appear to take a random amount of time
			parkingDuration = (rand() % MAX_PARKING_DURATION) + MIN_PARKING_DURATION;
			for(int i = 0; i < parkingDuration; i++){ currentThread->Yield(); }
			
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
			valetCarNumber[index] = ON_BENCH_NOT_WAITING;
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
			
			valetStatusLock[index]->Acquire(); // acquire the lock on valetStatusCV[]
		}
		else { // valet is interacting with a driver
			if(sleepingOnBench){
				printf("%s has been woken up from the bench \n",
						currentThread->getName());
				sleepingOnBench = false;
			}
			if((valetStatus % 2) != 0){ // odd index means the vehicle is a limo
				valetLimoParkingLock[index]->Acquire(); // acquire the lock used to signal the driver
				printf("%s has received the keys from Limousine Driver[%d] for Car[%d] \n",
						currentThread->getName(), valetStatus, valetStatus);
						
				valetLimoParkingCV[index]->Signal(valetLimoParkingLock[index]); // signal the driver to let them know we have received their keys
				printf("%s has given Limousine Driver[%d] Parking Token[%d] for Car[%d] \n",
						currentThread->getName(), valetStatus, valetStatus, valetStatus);		
				valetLimoParkingLock[index]->Release(); // release the lock used to signal the driver
				
				numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
				numLimosWaitingToPark--; // update to remove the parked limo
				numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			}
			else { // even index means the vehicle is a car
				valetCarParkingLock[index]->Acquire(); // acquire the lock used to signal the driver
				printf("%s has received the keys from Car Driver[%d] for Car[%d] \n",
						currentThread->getName(), valetStatus, valetStatus);
				
				valetCarParkingCV[index]->Signal(valetCarParkingLock[index]); // signal the driver to let them know we have received their keys
				printf("%s has given Car Driver[%d] Parking Token[%d] for Car[%d] \n",
						currentThread->getName(), valetStatus, valetStatus, valetStatus);
				valetCarParkingLock[index]->Release(); // release the lock used to signal the driver
														
				numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
				numCarsWaitingToPark--; // update to remove the parked car
				numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark
			}
	
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]	
			valetCarNumber[index] = IS_PARKING_CAR;
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
			printf("%s is parking Car[%d] \n",
					currentThread->getName(), valetStatus);
		}	
		valetStatusLock[index]->Release(); // release the lock on valetStatusCV[]
		
		printf("%s looping \n", currentThread->getName());
	}
}

// --------------------------------------------------
// Valet Manager function
// --------------------------------------------------
void ValetManager(int index) {
	// Data
	int valetsOnBench = 0, benchValet = 0, backRoomValet = 0;
	int numTotalVehiclesWaiting = 0;
	
	while(true){
		// Check to see if 4 or more cars are waiting to be parked
		numCarsWaitingToParkLock->Acquire(); // acquire lock around global int numCarsWaitingToPark 
		numLimosWaitingToParkLock->Acquire(); // acquire lock around global int numLimosWaitingToPark
		
		numTotalVehiclesWaiting = numCarsWaitingToPark + // read globals to determine number of vehicles waiting
								  numLimosWaitingToPark;
									  
		if(numTotalVehiclesWaiting >= MIN_NUM_VEHICLES_WAITING){ // if 4 or more cars waiting
			for(int i = 0; i < numValets; i++){	
				valetCarNumberLock[i]->Acquire(); // acquire the lock around valetCarNumber[]
			}
			for(int i = 0; i < numValets; i++){ 
				if(valetCarNumber[i] == IN_BACK_ROOM){ // if valet is in the back room
					backRoomValet = i; // record position of first valet available in back room
					break;
				}
			}
			if(valetCarNumber[backRoomValet] == IN_BACK_ROOM){ // if at least one valet is in the back room
				valetCarNumber[backRoomValet] = ON_BENCH_NOT_WAITING; // change state of valet
				
				valetStatusLock[index]->Acquire(); // acquire the lock on valetStatusCV[]
				valetStatusCV[index]->Signal(valetStatusLock[index]);
				printf("%s has told Parking Valet[%d] to come out of the back room \n", 
						currentThread->getName(), backRoomValet);
				valetStatusLock[index]->Release(); // release the lock on valetStatusCV[]
			}
			for(int i = 0; i < numValets; i++){		
				valetCarNumberLock[i]->Release(); // release the lock around valetCarNumber[]
			}
		}	
		
		numCarsWaitingToParkLock->Release(); // release lock around global int numCarsWaitingToPark 
		numLimosWaitingToParkLock->Release(); // release lock around global int numLimosWaitingToPark
		
		for(int i = 0; i < 50; i++){ currentThread->Yield(); } // yield so we don't check too often
	
		// Check to see if more than 2 valets are sitting on bench	
		for(int i = 0; i < numValets; i++){	
			valetCarNumberLock[i]->Acquire(); // acquire the lock around valetCarNumber[]
		}
		for(int i = 0; i < numValets; i++){ 
			if(valetCarNumber[i] == WAITING_ON_BENCH){ // if valet is sitting on bench
				valetsOnBench++; // increment number of benched valets
				benchValet = i; // record position of last benched valet
			}
		}
		if(valetsOnBench > MAX_NUM_VALETS_ON_BENCH){
			if(valetCarNumber[benchValet] == WAITING_ON_BENCH){ // at least three valets are sitting on bench
				valetCarNumber[benchValet] = GOING_TO_BACK_ROOM; // change state of valet
				printf("%s has sent Parking Valet[%d] to the back room \n", 
						currentThread->getName(), benchValet);
				
				// If the valet is sleeping on the bench, we need to wake him up		
				valetAlertLock[benchValet]->Acquire(); // acquire the lock in valetAlertCV[]
				valetAlertCV[benchValet]->Signal(valetAlertLock[benchValet]); // wait for a driver to signal saying there are still cars to be parked
				valetAlertLock[benchValet]->Release(); // release the lock in valetAlertCV[]	
			}
		}		
		for(int i = 0; i < numValets; i++){		
			valetCarNumberLock[i]->Release(); // release the lock around valetCarNumber[]
		}
		
		for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
		
		//if(numTotalVehiclesWaiting < MIN_NUM_VEHICLES_WAITING){ // when we last checked, there weren't many cars left to park
			valetManagerAlertLock->Acquire(); // acquire the lock in valetManagerAlertCV
			valetManagerAlertCV->Wait(valetManagerAlertLock); // wait for a Valet to signal saying there are still cars to be parked
			valetManagerAlertLock->Release(); // release the lock in valetManagerAlertCV
		//}
		printf("%s looping \n", currentThread->getName());
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
	numCars = 5;
	
	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    
    // Initialize Valet Manager data
    buffer = new char[256];
	sprintf(buffer, "valetManagerAlertCV");
    valetManagerAlertCV = new Condition(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "valetManagerAlertLock");
	valetManagerAlertLock = new Lock(buffer);

	// Initialize Valet data		
	buffer = new char[256];
	sprintf(buffer, "numCarsWaitingToParkLock");
	numCarsWaitingToParkLock = new Lock(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "numCarsWaitingToParkLock");
	numLimosWaitingToParkLock = new Lock(buffer); 
	
	buffer = new char[256];
	sprintf(buffer, "valetLimoLineLock");
	valetLimoLineLock = new Lock(buffer);

	buffer = new char[256];
	sprintf(buffer, "valetCarLineLock");
	valetCarLineLock = new Lock(buffer);
	
	for(int i = 0; i < numValets; i++){
		valetCarNumber[i] = ON_BENCH_NOT_WAITING;  
		
		buffer = new char[256];
		sprintf(buffer, "valetCarNumberLock%d", i);
		valetCarNumberLock[i] = new Lock(buffer);
	
		buffer = new char[256];
		sprintf(buffer, "valetStatusCV%d", i);
		valetStatusCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetStatusLock%d", i);
		valetStatusLock[i] = new Lock(buffer);
	
		buffer = new char[256];
		sprintf(buffer, "valetAlertCV%d", i);
		valetAlertCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetAlertLock%d", i);		
		valetAlertLock[i] = new Lock(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetLimoParkingLock%d", i);	
		valetLimoParkingLock[i] = new Lock(buffer);  
		
		buffer = new char[256];
		sprintf(buffer, "valetLimoParkingCV%d", i);
		valetLimoParkingCV[i] = new Condition (buffer);

		buffer = new char[256];
		sprintf(buffer, "valetCarParkingLock%d", i);
		valetCarParkingLock[i] = new Lock(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetCarParkingCV%d", i);
		valetCarParkingCV[i] = new Condition(buffer);
	}
	
	// Initialize Driver data
	buffer = new char[256];
	sprintf(buffer, "numCarsWaitingToParkLock%d", 0);
	numCarsWaitingToParkLock = new Lock(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "numLimosWaitingToParkLock%d", 0);
	numLimosWaitingToParkLock = new Lock(buffer);
            
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
			driverIndex = 2*i; // even index
			sprintf(buffer, "Car Driver[%d]", driverIndex);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, driverIndex);
			numCarDrivers++;
			numCarsWaitingToPark++;
		}
		else {
			driverIndex = (2*i) + 1; // odd index
			sprintf(buffer, "Limo Driver[%d]", driverIndex);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, driverIndex);
			numLimoDrivers++;
			numLimosWaitingToPark++;
		}
	
/*   		// Create Visitors
   		numPassengers = (rand() % 4) + 2;
		for(int j = 0; j < numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", ((i*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, ((i*1000) + j));
			numVisitors++;
		}*/
		
	}
		
	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
	
	/*	// Create Ticket Takers
	for(int i = 0; i < numTicketTakers; i++){
		buffer = new char[256];
		sprintf(buffer, "Ticket Taker[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)TicketTaker, i);
	}*/
		
	// Create Valet Manager (only need 1)
   	buffer = new char[256];
   	sprintf(buffer, "Valet Manager");
	t = new Thread(buffer);
	t->Fork((VoidFunctionPtr)ValetManager, 0);	

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
