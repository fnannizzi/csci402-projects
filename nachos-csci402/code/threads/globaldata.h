// globaldata.h 
// Francesca Nannizzi

#include "system.h"
#include "synch.h"
#include <cstdlib>

// --------------------------------------------------
// Define global variables here
// --------------------------------------------------
#ifndef GLOBAL_DATA

// General constants
#define MAX_NUM_TICKET_TAKERS 		5 
#define MIN_NUM_TICKET_TAKERS 		1 
#define MAX_NUM_VALETS 				5
#define MIN_NUM_VALETS 				1
#define MAX_NUM_CARS 				20
#define MIN_NUM_CARS 				5
#define MAX_NUM_DRIVERS				20
#define MIN_NUM_DRIVERS				5

// Valet Mananger constants
#define MAX_NUM_VALETS_ON_BENCH		2
#define	MIN_NUM_VEHICLES_WAITING	4
#define YIELD_DURATION				20
// Valet constants (not using an enum because array is also used to hold car indexes, which are positive numbers)
#define	WAITING_ON_BENCH			-1
#define	GOING_TO_BACK_ROOM			-2 
#define IN_BACK_ROOM				-3
#define IS_PARKING_CAR				-4
#define ON_BENCH_NOT_WAITING		-5
#define MIN_PARKING_DURATION		5
#define MAX_PARKING_DURATION		15
#define MIN_RETURNING_CAR_DURATION	5
#define MAX_RETURNING_CAR_DURATION	15
// Ticket Taker constants
#define NO_TICKET_AVAILABLE			-1
#define NOT_WAITING					-2
#define DRIVER_MULTIPLIER			1000
// Visitor constants
#define MIN_VISIT_DURATION			50
// Driver constants
#define TOKEN_MULTIPLIER			1000
#define CAR							0
#define LIMO						1

// Enum for Driver states
typedef enum { WAITING_TO_PARK, 
			   SEARCHING_FOR_VALET,
			   WAITING_TO_TELL_PASSENGERS_TO_EXIT,
			   WAITING_FOR_PASSENGERS_TO_EXIT,
			   WAITING_FOR_LIMOS, 
			   PARKING_CAR, 
			   IN_MUSEUM, 
			   WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM,
			   WAITING_IN_TOKEN_LINE,
			   RETURNING_TOKEN,
			   TELL_PASSENGERS_TO_ENTER_CAR,
			   LEAVING_MUSEUM,
			   QUIT
			  } driverState;



// General data
int numLimoDrivers = 0, numCarDrivers = 0, numValets = 0;
int numCars = 0, numVisitors = 0, numTicketTakers = 0;
int numCarsWaitingToPark = 0, numLimosWaitingToPark = 0;

// Test case data
bool museumOpen; // used in test cases to close museum so that only parking and leaving occurs
bool onlyPark; // used in test cases to show only the parking (not reclaiming of car) simulation and quit
bool onlyTickets; // used in test cases to show only the Ticket Taker interaction

// Valet Manager Data
Condition* valetManagerAlertCV; // used to keep the Valet Manager running until 
Lock* valetManagerAlertLock; // there are no more cars to park

// Valet Data
int valetCarNumber[MAX_NUM_VALETS]; // when parking, is the number of car being parked. If valet is in the back room, valetCarNumber[] = -2
Lock* valetCarNumberLock[MAX_NUM_VALETS]; // if valet is on bench but not parking a car, valetCarNumber[] = -1

Condition* valetStatusCV[MAX_NUM_VALETS]; // used to communicate between the Valet Manager and the Valets
Lock* valetStatusLock[MAX_NUM_VALETS]; // so Valet Manager can signal Valets when needed

Lock* valetLimoLineLock; // used to queue the waiting limos
Lock* valetCarLineLock; // used to queue the waiting cars

Lock* valetLimoParkingLock[MAX_NUM_VALETS]; // used to communicate with the drivers 
Condition* valetLimoParkingCV[MAX_NUM_VALETS]; // when actually parking their car

Lock* valetCarParkingLock[MAX_NUM_VALETS]; // used to communicate with the drivers 
Condition* valetCarParkingCV[MAX_NUM_VALETS]; // when actually parking their car

Condition* valetAlertCV[MAX_NUM_VALETS]; // used to keep the Valets running until 
Lock* valetAlertLock[MAX_NUM_VALETS]; // there are no more cars to park

Condition* valetTokenReturnCV[MAX_NUM_VALETS]; // used to signal between driver and valet
Lock* valetTokenReturnLock[MAX_NUM_VALETS]; // during token/key/tip exchange

Lock* valetTokenReturnLineLock; // a lock to manage the line of drivers waiting to return their tokens

Lock* valetTokenExchangeLock[MAX_NUM_VALETS]; // prevents the valet from parking the car until he hands off the token

// Driver Data
int vehicleType[MAX_NUM_CARS]; // holds data on whether vehicle is a car or limo

Lock* numCarsWaitingToParkLock; // lock on the global int numCarsWaitingToPark
Lock* numLimosWaitingToParkLock; // lock on the global int numLimosWaitingToPark 

Condition* driverPassengerCV[MAX_NUM_CARS]; // manages communication between drivers and passengers
Lock* driverPassengerLock[MAX_NUM_CARS]; // allowing driver to wait for everyone to leave/enter car
int passengerCount[MAX_NUM_CARS]; // tracks number of passengers remaining in car

int totalPassengers[MAX_NUM_CARS]; // holds total number of passengers per car, only accessed at beginning

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
// Initialize data function
// --------------------------------------------------
void InitializeData(int cars, int valets, int ticketTakers){
    numValets = valets;
	numCars = cars; 
	numTicketTakers = ticketTakers;
    char *buffer; // Used to name threads
    museumOpen = true; // museum should be open by default
    onlyPark = false; // don't run parking only simulation by default
    
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
	
	buffer = new char[256];
	sprintf(buffer, "valetTokenReturnLineLock");	
	valetTokenReturnLineLock = new Lock(buffer);
	
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
		
		buffer = new char[256];
		sprintf(buffer, "valetTokenExchangeLock%d", i);
		valetTokenExchangeLock[i] = new Lock(buffer);

		buffer = new char[256];
		sprintf(buffer, "valetTokenReturnCV%d", i);		
		valetTokenReturnCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetTokenReturnLock%d", i);
		valetTokenReturnLock[i] = new Lock(buffer);
	}
	
	// Initialize Driver data
	buffer = new char[256];
	sprintf(buffer, "numCarsWaitingToParkLock%d", 0);
	numCarsWaitingToParkLock = new Lock(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "numLimosWaitingToParkLock%d", 0);
	numLimosWaitingToParkLock = new Lock(buffer);
	
	for(int i = 0; i < numCars; i++){
		buffer = new char[256];
		sprintf(buffer, "driverPassengerCV%d", i);
		driverPassengerCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "driverPassengerLock%d", i);		 
		driverPassengerLock[i] = new Lock(buffer);
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
}

// --------------------------------------------------
// Delete data function, used when running
// consective tests
// --------------------------------------------------
void DeleteData(){
/*    
    // Delete Valet Manager data
    delete valetManagerAlertCV;
	delete valetManagerAlertLock;

	// Initialize Valet data		
	delete numCarsWaitingToParkLock;
	delete numLimosWaitingToParkLock; 
	delete valetLimoLineLock;
	delete valetCarLineLock;	
	delete valetTokenReturnLineLock;
	
	for(int i = 0; i < numValets; i++){
		valetCarNumber[i] = ON_BENCH_NOT_WAITING;  
	
		delete valetCarNumberLock[i];
		delete valetStatusCV[i];
		delete valetStatusLock[i];
		delete valetAlertCV[i];	
		delete valetAlertLock[i];
		
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
		
		buffer = new char[256];
		sprintf(buffer, "valetTokenExchangeLock%d", i);
		valetTokenExchangeLock[i] = new Lock(buffer);

		buffer = new char[256];
		sprintf(buffer, "valetTokenReturnCV%d", i);		
		valetTokenReturnCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "valetTokenReturnLock%d", i);
		valetTokenReturnLock[i] = new Lock(buffer);
	}
	
	// Initialize Driver data
	buffer = new char[256];
	sprintf(buffer, "numCarsWaitingToParkLock%d", 0);
	numCarsWaitingToParkLock = new Lock(buffer);
	
	buffer = new char[256];
	sprintf(buffer, "numLimosWaitingToParkLock%d", 0);
	numLimosWaitingToParkLock = new Lock(buffer);
	
	for(int i = 0; i < numCars; i++){
		buffer = new char[256];
		sprintf(buffer, "driverPassengerCV%d", i);
		driverPassengerCV[i] = new Condition(buffer);
		
		buffer = new char[256];
		sprintf(buffer, "driverPassengerLock%d", i);		 
		driverPassengerLock[i] = new Lock(buffer);
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
	}*/
}

// --------------------------------------------------
// Print menu function
// --------------------------------------------------
int PrintMenu(){
	int testChoice = 0;
	
	printf("Choose an option:\n");
	printf("	1) Run a system test\n ");
	printf("	2) As long as limousines keep arriving, regular cars never get parked\n");
	printf("	3) If only limousines OR cars arrive, no limousine or car has to wait forever to get parked\n");
	printf("	4) If no Valets are available, vehicles don't get parked\n");
	printf("	5) The Valet Manager performs their job properly\n");
	printf("	6) Cars/Limousines are never parked until all Visitors have exited the car\n");
	printf("	7) No Visitor enters the Museum until the Ticket Taker has informed them that their ticket is acceptable\n");
	printf("	8) Valets do not retrieve a car until they receive the parking token from the Driver\n");
	printf("	9) Valets don't release a car to a Driver until the driver gives them a tip\n");
	printf("	10) Drivers don't leave until all occupants are in the car\n");
	printf("	11) Drivers don't give their parking token to a Valet until all the occupants of their car have left the Museum\n");
	
	while(true){
		scanf("%d", &testChoice);
		if((testChoice > 11) || (testChoice < 1)){ printf("Please enter a number between %d and %d: ", 1, 11); }
		else { break; }
	}
	if(testChoice == 1){
		printf("Enter the number of cars in the simulation: ");
		while(true){
			scanf("%d", &numCars);
			if((numCars > MAX_NUM_CARS) || (numCars < MIN_NUM_CARS)){ printf("The number of cars must be between %d and %d. Please enter a different number: ", MIN_NUM_CARS, MAX_NUM_CARS); }
			else { break; }
		}
		printf("Enter the number of Parking Valets in the simulation: ");
		while(true){
			scanf("%d", &numValets);
			if((numValets > MAX_NUM_VALETS) || (numCars < MIN_NUM_VALETS)){ printf("The number of Parking Valets must be between %d and %d. Please enter a different number: ", MIN_NUM_VALETS, MAX_NUM_VALETS); }
			else { break; }
		}
		printf("Enter the number of Ticket Takers in the simulation: ");
		while(true){
			scanf("%d", &numTicketTakers);
			if((numTicketTakers > MAX_NUM_TICKET_TAKERS) || (numCars < MIN_NUM_TICKET_TAKERS)){ printf("The number of Ticket Takers must be between %d and %d. Please enter a different number: ", MIN_NUM_TICKET_TAKERS, MAX_NUM_TICKET_TAKERS); }
			else { break; }
		}
	}	
	printf("\nInput received.\n");
	printf("-------------------------------------\n");
	printf("Beginning museum parking simulation. \n");
	printf("-------------------------------------\n");
	return testChoice;
}

void PrintData(){
	printf("Number of Limousine Drivers = [%d]\n", numLimoDrivers);
	printf("Number of Car Drivers = [%d]\n", numCarDrivers);
	printf("Number of Parking Valets = [%d]\n", numValets);
	printf("Number of Visitors = [%d]\n", numVisitors);
	printf("Number of Ticket Takers = [%d]\n", numTicketTakers);
	printf("Number of Cars = [%d]\n", numCars);
}

#endif // GLOBAL_DATA
