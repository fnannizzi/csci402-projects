// p2.cc 
// Francesca Nannizzi

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "p2functions.h"
#include <cstdlib>

#ifndef MUSEUM_PARKING	

// --------------------------------------------------
// Test 1 - Run a system test
// --------------------------------------------------
void test1(){ 
	InitializeData(numCars, numValets, numTicketTakers);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car

	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
	
	for(int i = 0; i < 50; i++){ currentThread->Yield(); } // yield so we don't check too often

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = rand() % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = (rand() % 4) + 2;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	
  		// Create Visitors
		for(int j = 1; j <= numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", (((i + 1)*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, (((i + 1)*1000) + j));
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
		
	// Create Valet Manager (only need 1)
   	buffer = new char[256];
   	sprintf(buffer, "Valet Manager");
	t = new Thread(buffer);
	t->Fork((VoidFunctionPtr)ValetManager, 0);	
	
	PrintData();
}

// --------------------------------------------------
// Test 2 - As long as limousines keep arriving, 
// regular cars never get parked (-rs 0, 1, 2)
// --------------------------------------------------
void test2(){ 

	InitializeData(14, 1, 0); // Initialize data with correct numbers of cars, valets, and ticket takers
	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    int numLimos = 12; // number of limos arriving
    museumOpen = false;
    onlyPark = true;
    
    // Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
    
	// Create Limo Drivers
	for(int i = 0; i < numLimos; i++){
		buffer = new char[256];
		numPassengers = 0;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
   		vehicleType[i] = LIMO;
   			
		sprintf(buffer, "Limo Driver[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)LimoDriver, i);
		numLimoDrivers++;
	}
	
	// Create Car Drivers
	for(int i = numLimos; i < numCars; i++){
		buffer = new char[256];
		numPassengers = 0;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
   		vehicleType[i] = CAR;
   		
		sprintf(buffer, "Car Driver[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)CarDriver, i);
		numCarDrivers++;
	}
	PrintData();
}

// --------------------------------------------------
// Test 3 - If only limousines OR cars arrive, 
// no limousine or car has to wait forever to 
// get parked (-rs 0, 1, 2)
// --------------------------------------------------
void test3(){ 
	InitializeData(10, 1, 0); // Initialize data with correct numbers of cars, valets, and ticket takers
	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    int numLimos = 5; // number of limos arriving
    museumOpen = false;
    onlyPark = true;
    
    // Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
    
	// Create Limo Drivers
	for(int i = 0; i < numLimos; i++){
		buffer = new char[256];
		numPassengers = 0;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
   		vehicleType[i] = LIMO;
   			
		sprintf(buffer, "Limo Driver[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)LimoDriver, i);
		numLimoDrivers++;
	}
	
	// Create Car Drivers
	for(int i = numLimos; i < numCars; i++){
		buffer = new char[256];
		numPassengers = 0;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
   		vehicleType[i] = CAR;
   		
		sprintf(buffer, "Car Driver[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)CarDriver, i);
		numCarDrivers++;
	}
	PrintData();
}

// --------------------------------------------------
// Test 4 - If no Valets are available, vehicles 
// don't get parked TODO (-rs 4, 6)
// --------------------------------------------------
void test4(){ 
	InitializeData(5, 1, 0); // Initialize data with correct numbers of cars, valets, and ticket takers
	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    museumOpen = false;
    onlyPark = true;
    
    // Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		valetStatusRegister[i] = IN_BACK_ROOM; // make the valet unavailable
		t->Fork((VoidFunctionPtr)Valet, i);
	}
    
	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	}
	
	printf("Signalling the valet\n");
	// Signal a bunch of times to put the valet back in the waiting state, where he would normally start
	for(int i = 0; i < numValets; i++){
		valetStatusLock[i]->Acquire(); // acquire the lock on valetStatusCV[]
		valetStatusCV[i]->Signal(valetStatusLock[i]); // have the valet come out of the back room
		valetStatusLock[i]->Release(); // release the lock on valetStatusCV[]
	}
	for(int i = 0; i < numValets; i++){
		valetStatusLock[i]->Acquire(); // acquire the lock on valetStatusCV[]
		valetStatusCV[i]->Signal(valetStatusLock[i]); // have the valet come out of the back room
		valetStatusLock[i]->Release(); // release the lock on valetStatusCV[]
	}
	for(int i = 0; i < numValets; i++){
		valetStatusLock[i]->Acquire(); // acquire the lock on valetStatusCV[]
		valetStatusCV[i]->Signal(valetStatusLock[i]); // have the valet come out of the back room
		valetStatusLock[i]->Release(); // release the lock on valetStatusCV[]
	}
	
}

// --------------------------------------------------
// Test 5 - The Valet Manager performs their job 
// properly
// --------------------------------------------------
void test5(){ 
	InitializeData(20, 1, 0);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    museumOpen = false; // close the museum to run a parking simulation
    onlyPark = true;

	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		valetStatusRegister[i] = ON_BENCH;
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		if(i == 10){
			for(int j = 0; j < YIELD_DURATION; j++){ currentThread->Yield(); } // yield before adding more cars
		}
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	}
		
	// Create Valet Manager (only need 1)
   	buffer = new char[256];
   	sprintf(buffer, "Valet Manager");
	t = new Thread(buffer);
	t->Fork((VoidFunctionPtr)ValetManager, 0);	
	
	PrintData();
}

// --------------------------------------------------
// Test 6 - Cars/Limousines are never parked until 
// all Visitors have exited the car
// --------------------------------------------------
void test6(){ 
	InitializeData(5, 1, 0);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    museumOpen = false; // 
    onlyPark = true; 

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = 2;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	
  		// Create Visitors
		for(int j = 1; j <= numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", (((i + 1)*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, (((i + 1)*1000) + j));
			numVisitors++;
		}
	}
		
	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
	
	PrintData();
}

// --------------------------------------------------
// Test 7 - No Visitor enters the Museum until the 
// Ticket Taker has informed them that their ticket 
// is acceptable
// --------------------------------------------------
void test7(){ 
	InitializeData(5, 1, 5);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    onlyTickets = true;
    onlyPark = false;
    museumOpen = true;

	// Create Ticket Takers
	for(int i = 0; i < numTicketTakers; i++){
		buffer = new char[256];
		sprintf(buffer, "Ticket Taker[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)TicketTaker, i);
	}

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = 1;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
	
  		// Create Visitors
		for(int j = 1; j <= numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", (((i + 1)*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, (((i + 1)*1000) + j));
			numVisitors++;
		}
	}
	
	
	
	PrintData();
}

// --------------------------------------------------
// Test 8 - Valets do not retrieve a car until they 
// receive the parking token from the Driver 
// --------------------------------------------------
void test8(){
	
	InitializeData(5, 1, 0);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    museumOpen = false;
    onlyPark = true;

	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	}
		

	
	PrintData();
}

// --------------------------------------------------
// Test 9 - Valets don't release a car to a Driver 
// until the driver gives them a tip
// --------------------------------------------------
void test9(){ 
	
	InitializeData(5, 1, 0);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
    museumOpen = false;
    onlyPark = true;


	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = 2;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	}
		
	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
	
	PrintData();
}

// --------------------------------------------------
// Test 10 - Drivers don't leave until all occupants 
// are in the car
// --------------------------------------------------
void test10(){ 
	
	InitializeData(5, 1, 0);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
	museumOpen = false;

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = 2;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	
  		// Create Visitors
		for(int j = 1; j <= numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", (((i + 1)*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, (((i + 1)*1000) + j));
			numVisitors++;
		}
	}
		
	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}	
	
	PrintData();
}

// --------------------------------------------------
// Test 11 - Drivers don't give their parking token 
// to a Valet until all the occupants of their car 
// have left the Museum
// --------------------------------------------------
void test11(){ 
	InitializeData(5, 1, 5);	

	Thread *t; // Used to fork threads    
    char *buffer; // Used to name threads
    int limoOrCar = 0; // used to randomly generate Limo or Car Drivers
    int numPassengers = 0; // used to randomly generate the number of passengers per car or limo
    int driverIndex = 0; // used to create unique indices for each driver, odd for limo, even for car
	museumOpen = true;
	onlyPark = false;

	// Create Drivers
	for(int i = 0; i < numCars; i++){
		limoOrCar = i % 2;
		buffer = new char[256];
		vehicleType[i] = limoOrCar;
		numPassengers = 2;
   		totalPassengers[i] = numPassengers;
   		passengerCount[i] = numPassengers;
		
		if(limoOrCar == CAR){
			sprintf(buffer, "Car Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)CarDriver, i);
			numCarDrivers++;
		}
		else { // limoOrCar == LIMO
			sprintf(buffer, "Limo Driver[%d]", i);
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)LimoDriver, i);
			numLimoDrivers++;
		}
	
  		// Create Visitors
		for(int j = 1; j <= numPassengers; j++){
			buffer = new char[256];
			sprintf(buffer, "Visitor[%d]", (((i + 1)*1000) + j));
			t = new Thread(buffer);
			t->Fork((VoidFunctionPtr)Visitor, (((i + 1)*1000) + j));
			numVisitors++;
		}
	}
		
	// Create Valets
    for(int i = 0; i < numValets; i++){
		buffer = new char[256];
		sprintf(buffer, "Parking Valet[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)Valet, i);
	}
	
	// Create Ticket Takers
	for(int i = 0; i < numTicketTakers; i++){
		buffer = new char[256];
		sprintf(buffer, "Ticket Taker[%d]", i);
		t = new Thread(buffer);
		t->Fork((VoidFunctionPtr)TicketTaker, i);
	}
	
	PrintData();
}

// --------------------------------------------------
// Main thread // must be run with an argument to -P2
// --------------------------------------------------
void Problem2(char* choice){
	
	int testChoice = 0;
	int argChoice = atoi(choice);
	if((argChoice > 0) && (argChoice < 12)){
		testChoice = argChoice;
	}
	else {
		testChoice = PrintMenu();
	}	
		
	if(testChoice == 1){ test1(); } // Run a system test
	else if(testChoice == 2){ test2(); } // As long as limousines keep arriving, regular cars never get parked
	else if(testChoice == 3){ test3(); } // If only limousines OR cars arrive, no limousine or car has to wait forever to get parked
	else if(testChoice == 4){ test4(); } // If no Valets are available, vehicles don't get parked
	else if(testChoice == 5){ test5(); } // The Valet Manager performs their job properly
	else if(testChoice == 6){ test6(); } // Cars/Limousines are never parked until all Visitors have exited the car
	else if(testChoice == 7){ test7(); } // No Visitor enters the Museum until the Ticket Taker has informed them that their ticket is acceptable
	else if(testChoice == 8){ test8(); } // Valets do not retrieve a car until they receive the parking token from the Driver
	else if(testChoice == 9){ test9(); } // Valets don't release a car to a Driver until the driver gives them a tip
	else if(testChoice == 10){ test10(); } // Drivers don't leave until all occupants are in the car
	else if(testChoice == 11){ test11(); } // Drivers don't give their parking token to a Valet until all the occupants of their car have left the Museum
	else { printf("ERROR: Invalid choice. \n"); }
	
}
#endif
