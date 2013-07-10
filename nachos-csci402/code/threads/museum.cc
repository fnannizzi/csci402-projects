#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include <stdlib.h>
#include <time.h>

// General constants
#define RETURN_ERROR				-1
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
// Valet constants 
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
#define EMPTY						-1
#define TIP							-5000

int * limoReturnLineLock;
int * carLineLock;
int * carReturnLineLock;
int ** carLock;
int ** passengerCountLock;
int ** carStatusLock;
int ** valetLock;
int ** valetStatusLock;
int* limoReturnLineLengthLock;
int * carLineLengthLock;
int * carReturnLineLengthLock;
int * limoReturnLineCondition;
int * carLineCondition;
int * carReturnLineCondition;
int ** carCondition;
int ** valetCondition;

int * valetManagerLock;
int * valetManagerCondition;

int ** ticketTakerLock;
int ** ticketLineLock;
int ** ticketLineLengthLock;
int * lineCheckLock; 
int ** ticketLineCondition;
int ** ticketTakerCondition;

int ** valetLineLock;
int ** valetLineCondition; 
int * valetLineCheckLock;
int ** valetLineLengthLock;
int ** valetLineLengthCondition;
int * limoLineCheckLock;
int ** limoLineLock;
int ** limoLineCondition;
int ** limoLineLengthLock;
int** visitorLock;
int ** visitorCondition;

int ** backRoomLock;
int ** backRoomCondition;


int ticketLineLength[MAX_NUM_TICKET_TAKERS];
int ticketLineVisitor[MAX_NUM_TICKET_TAKERS];
int ticketDriver[MAX_NUM_TICKET_TAKERS];
int limoReturnLineLength;
int carLineLength;
int carReturnLineLength;
int carCount;
int passengerCount[MAX_NUM_DRIVERS];
int passengerMax[MAX_NUM_DRIVERS];
int carStatus[MAX_NUM_DRIVERS];
int ticketTakers;
int parkingValets;
int cars;
int visitors;

int carValet[MAX_NUM_DRIVERS];
int frontOfLimoLine;
int frontOfCarLine;
int frontOfLimoReturnLine;
int frontOfCarReturnLine;
int valetCheck;

int valetLineLength[MAX_NUM_VALETS];
int limoLineLength[MAX_NUM_VALETS];
int valetLineLimo[MAX_NUM_VALETS];
int valetLineCar[MAX_NUM_VALETS];
int driverIsPresent[MAX_NUM_DRIVERS];

int carLastOneOut[MAX_NUM_DRIVERS];

int valetManagerPresent;
int valetManagerCount;
int valetStatus[MAX_NUM_VALETS]; //-1 if available, -2 if in back, 0 is busy
int valetPriority[MAX_NUM_VALETS];
int valetReturn[MAX_NUM_VALETS];




void
CarDriver(int i)
{
	int car = i / 2000; //car numbers are found by dividing parameter by 2000
	int id = i % 1000; //id numbers are found by modding parameter by 1000
	int valetNumber = -1;
	int returnValue = 0;
	carLastOneOut[i] = car;
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	driverIsPresent[car] = 1; //for visitors to hold off on signaling driver if not initialized yet
returnValue = Acquire(	visitorLock[car]); 
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Broadcast(	visitorCondition[car] ,visitorLock[car]); //Signaling visitors car is ready
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	while (true)
	{
	//Arriving at Museum
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 0)) 
	{
	Printx("%s has parked Car[%d] at the Museum\n",36,car*10000000);
returnValue = Acquire(	carStatusLock[car]); //used to determine stages
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Broadcast(	carCondition[car] ,carLock[car]); //signals visitors to leave car
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
	Printx("%s has told his visitors to leave Car[%d]\n",42,car*10000000);	
	}
	if ((passengerCount[car] == 0) && (carStatus[car] == 1))
	{
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int shortestLength = 999;
	int shortestIndex = 0;
	for (int a = 0; a < parkingValets; a++) //checks for available valet line
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
	//waits here if valet is busy
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
	//goes ahead if valet available
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 0; //will tell valet what to do with car
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given their keys to ParkingValet[%d] for Car[%d]\n",56,((shortestIndex*10000000) + (car*10000)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has received ParkingToken[%d] from ParkingValet[%d] for Car[%d]\n",67,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	
	//Valet Parking Car
	
	
	
	//Visit Museum - Should reside in the if statement and not in another if statement
	//TicketTakerInteraction
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	shortestLength = 999;
	shortestIndex = 0;
	for (int a = 0; a < ticketTakers; a++)
	{
	if (ticketLineLength[a] < shortestLength) //checks for shortest ticket line
	{
	shortestIndex = a;
	shortestLength = ticketLineLength[a];
	}
	}
returnValue = Release(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if (ticketLineLength[shortestIndex] > 0)
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]); //waits if ticket taker busy
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	ticketLineCondition[shortestIndex] ,ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]); //goes ahead if not busy
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineVisitor[shortestIndex] = i; //lets ticket taker know who he is
returnValue = Release(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s is waiting for TicketTaker[%d]\n",34,shortestIndex*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given their ticket to TicketTaker[%d]\n",45,shortestIndex*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has entered the Museum\n",26,0);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	
	//Visiting Museum
	int r = (rand() % MIN_VISIT_DURATION) + MIN_VISIT_DURATION;
	for (int j = 0; j < r; j++){		
		Yield();
	}
	Printx("%s has left the Museum\n", 23,0);
	
	
	}
	
	//Valet Returning Car
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 2))
	{
	Printx("%s has been notified that all their Visitors have left the Museum for Car[%d]\n",78,car*10000000);
returnValue = Acquire(	carStatusLock[car]); //Similar to car parking
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int shortestLength = 999;
	int shortestIndex = 0;
	for (int a = 0; a < parkingValets; a++)
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	valetLock[shortestIndex]); //Again all of this is similar to car parking
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 1;
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given ParkingToken[%d] to ParkingValet[%d] for Car[%d]\n",62,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given a tip to ParkingValet[%d] for Car[%d]\n",51,((shortestIndex*10000000)+(car*10000)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has received their keys from ParkingValet[%d] for Car[%d]\n",61,((car*10000000)+(car*10000)));
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
	Printx("%s has told his visitors to get into Car[%d]\n",45,car*10000000);
	}
	if ((passengerCount[car] == 0) && (carStatus[car] == 3))
	{
	Printx("%s has left the Museum in Car[%d]\n",34,car*10000000);
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
}

void
LimoDriver(int i) //This is simply a repeat of Car Driver without any museum visiting or ticket taker interaction
{
	int car = i / 2000; 
	int id = i % 1000;
	int valetNumber = -1;
	int returnValue = 0;
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	driverIsPresent[car] = 1;
returnValue = Acquire(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Broadcast(	visitorCondition[car] ,visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	while (true)
	{
	//Arriving at Museum
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 0)) 
	{
	
	Printx("%s has parked Car[%d] at the Museum\n",36,car*10000000);
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
	Printx("%s has told his visitors to leave Car[%d]\n",42,car*10000000);	
	}
	
	//Valet Parking Car
	else if ((passengerCount[car] == 0) && (carStatus[car] == 1))
	{
returnValue = Acquire(	carStatusLock[car]); //Only difference here is there is a limo line for limo priority
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int shortestLength = 999;
	int shortestIndex = 0;
	
	for (int a = 0; a < parkingValets; a++)
	{
	if (limoLineLength[a] < shortestLength)
	{
	shortestIndex = a;
	shortestLength = limoLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if ((limoLineLength[shortestIndex]+valetLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[shortestIndex]++;
returnValue = Release(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	limoLineCondition[shortestIndex] ,limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
returnValue = Acquire(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[shortestIndex]++;
returnValue = Release(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 2; //tells valet he is parking a limo
returnValue = Release(	limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given their keys to ParkingValet[%d] for Car[%d]\n",56,((shortestIndex*10000000)+(car*10000)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has received ParkingToken[%d] from ParkingValet[%d] for Car[%d]\n",67,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	
	//Valet Returning Car
	//Exactly the same as car driver interaction with valet
	else if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 2))
	{
	Printx("%s has been notified that all their Visitors have left the Museum for Car[%d]\n",78,car*10000000);
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int shortestLength = 999;
	int shortestIndex = 0;
	for (int a = 0; a < parkingValets; a++)
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 1;
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given ParkingToken[%d] to ParkingValet[%d] for Car[%d]\n",62,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given a tip to ParkingValet[%d] for Car[%d]\n",51,(shortestIndex*10000000)+(car*10000));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has received their keys from ParkingValet[%d] for Car[%d]\n",61,((car*10000000)+(car*10000)));
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); }
	Printx("%s has told his visitors to get into Car[%d]\n",45,car*10000000);
	}
	else if ((passengerCount[car] == 0) && (carStatus[car] == 3))
	{
	Printx("%s has left the Museum in Car[%d]\n",34,car*10000000);
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	
	}	
	
}

void Valet(int i)
{
int returnValue = 0;
returnValue = Acquire(	valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	while (true)
	{
	if (valetStatus[i] == 22) //Used for Test 7 to simulate a back room. Otherwise never used
	{
	valetStatus[i] = -2;
	limoLineLength[i] = 999;
returnValue = Acquire(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Wait(	backRoomCondition[i] ,backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	valetStatus[i] = -1;
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[i] = 0;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s is coming out of the back room\n",34,0);
returnValue = Release(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	valetStatus[i] = 0; //Means he is busy unless does not deal with a car
	if ((limoLineLength[i] > 0) && (valetReturn[i] == 2) && (valetStatus[i] > -2))
	{
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0) //Can determine if car is a limo or car
	limo = 1;
	car/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	if (limo)
	Printx("%s has received the keys from LimousineDriver[%d] for Car[%d]\n",62,((driver*10000000)+(valetLineCar[i]*10000)));
	else
	Printx("%s has received the keys from CarDriver[%d] for Car[%d]\n",56,((driver*10000000)+(car*10000)));
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	if (limo)
	Printx("%s has given LimousineDriver[%d] ParkingToken[%d] for Car[%d]\n",62,((driver*10000000)+(car*10000)+(car*100)));
	else
	Printx("%s has given CarDriver[%d] ParkingToken[%d] for Car[%d]\n",56,((driver*10000000)+(car*10000)+(car*100)));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s is parking Car[%d]\n",22,car*10000000);
	int r = (rand() % 16) + 5;
	for (int y = 0; y < r; y++)
	Yield();
returnValue = Acquire(	valetLineCheckLock); //Decremeting line length
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[i]--;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	
	else if ((valetLineLength[i] > 0) && (valetReturn[i] == 0) && (valetStatus[i] > -2))
	{
	//Same thing but for car parking
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0)
	limo = 1;
	car/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	if (limo)
	Printx("%s has received the keys from LimousineDriver[%d] for Car[%d]\n",62,((driver*10000000)+(car*10000)));
	else
	Printx("%s has received the keys from CarDriver[%d] for Car[%d]\n",56,((driver*10000000)+(car*10000)));
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	if (limo)
	Printx("%s has given LimousineDriver[%d] ParkingToken[%d] for Car[%d]\n",62,((driver*10000000)+(car*10000)+(car*100)));
	else
	Printx("%s has given CarDriver[%d] ParkingToken[%d] for Car[%d]\n",56,((driver*10000000)+(car*10000)+(car*100)));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s is parking Car[%d]\n",22,car*10000000);
	int r = (rand() % 16) + 5;
	for (int y = 0; y < 20; y++)
	Yield();
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[i]--;
returnValue = Release(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else if ((valetLineLength[i] > 0) && (valetReturn[i] == 1) && (valetStatus[i] > -2))
	{
	//Similar to parking car with different print statements
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0)
	limo = 1;
	limo/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	if (limo)
	Printx("%s has received ParkingToken[%d] from LimousineDriver[%d] for Car[%d]\n",70,((car*10000000)+(driver*10000)+(car*100)));
	else
	Printx("%s has received ParkingToken[%d] from CarDriver[%d] for Car[%d]\n",64,((car*10000000)+(driver*10000)+(car*100)));
	int r = (rand() % 16) + 5;
	for (int y = 0; y < r; y++)
	Yield();
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	if (limo)
	Printx("%s has given LimousineDriver[%d] Car[%d]\n",41,((driver*10000000)+(car*10000)));
	else
	Printx("%s has given CarDriver[%d] Car[%d]\n",35,((driver*10000000)+(car*10000)));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	if (limo)
	Printx("%s has received tip from LimousineDriver[%d] for Car[%d]\n",57,((driver*10000000)+(car*10000)));
	else
	Printx("%s has received tip from CarDriver[%d] for Car[%d]\n",51,((driver*10000000)+(car*10000)));	
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	if (limo)
	Printx("%s has returned keys to LimousineDriver[%d] for Car[%d]\n",56,((driver*10000000)+(car*10000)));
	else
	Printx("%s has returned keys to CarDriver[%d] for Car[%d]\n",50,((driver*10000000)+(car*10000)));
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	valetLineLock[i]);	
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineLength[i]--;
returnValue = Release(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else if (valetReturn[i] == -2)
	{
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	valetStatus[i] = -2;
	Printx("%s is going to the back room\n",29,0);
returnValue = Acquire(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Wait(	backRoomCondition[i] ,backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	valetStatus[i] = -1;
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[i] = 0;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s is coming out of the back room\n",34,0);
returnValue = Release(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	if (valetStatus[i] != -2)
	{
	valetStatus[i] = -1;
	Printx("%s is going to sleep on the bench\n",34,0);
	valetManagerCount++;
	if ((valetManagerPresent == 1) && ((valetManagerCount % 7) == 6))
	{
returnValue = Acquire(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}	
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has been woken up from the bench\n",36,0);
	
	}
	} 
	
}


void
TicketTaker(int i)
{
int returnValue = 0;
returnValue = Acquire(	ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	while (true)
	{
	//Once signaled by next person in line, they will interact with visitor or car driver
	if (ticketLineLength[i] > 0)
	{
	int visitor = ticketLineVisitor[i] % 1000;
returnValue = Signal(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	if (ticketDriver[i] == 1)
	Printx("%s has received a ticket from CarDriver[%d]\n",44,visitor*10000000);
	else
	Printx("%s has received a ticket from Visitor[%d]\n",42,visitor*10000000);
returnValue = Signal(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	if (ticketDriver[i] == 1)
	Printx("%s has accepted a ticket from CarDriver[%d]\n",44,visitor*10000000);
	else
	Printx("%s has accepted a ticket from Visitor[%d]\n", 42,visitor*10000000);
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	ticketLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineLength[i]--;
	ticketDriver[i] = 0;
returnValue = Release(	ticketLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (ticketLineLength[i] > 0) //signals next person in line
	{
returnValue = Acquire(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	ticketLineCondition[i] ,ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Release(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
}

void
Visitor(int i)
{
	int car = i / 2000;
	int id = i % 1000;
	int returnValue = 0;
	if (driverIsPresent[car] == 0) //This solved the race condition when drivers were not initialized yet and visitors were trying to signal them
	{
returnValue = Acquire(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Wait(	visitorCondition[car] ,visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	
	//Arriving at Museum
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	passengerCount[car]+=1;
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (passengerCount[car] == passengerMax[car])
returnValue = Signal(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	carCondition[car] ,carLock[car]); //Will wait for broadcast from driver
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has been told to exit Car[%d]\n",33,car*10000000);
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	passengerCount[car]-=1; //Decrementing passengers in car, MV for car drivers
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s has exited Car[%d]\n", 22,car*10000000);
	if (passengerCount[car] == 0)
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); //If last one out, tells driver
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	}
returnValue = Release(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }

	//TicketTakerInteraction
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int shortestLength = 999;
	int shortestIndex = 0;
	for (int a = 0; a < ticketTakers; a++) //Will find shortest line
	{
	if (ticketLineLength[a] < shortestLength)
	{
	shortestIndex = a;
	shortestLength = ticketLineLength[a];
	}
	}
returnValue = Release(	lineCheckLock); //All the same as the car driver
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Acquire(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	if (ticketLineLength[shortestIndex] > 0)
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]); 
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Wait(	ticketLineCondition[shortestIndex] ,ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
	else
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
returnValue = Acquire(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	ticketLineVisitor[shortestIndex] = i;
returnValue = Release(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s is waiting for TicketTaker[%d]\n",34,shortestIndex*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has given their ticket to TicketTaker[%d]\n",45,shortestIndex*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has entered the Museum\n",26,0);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	
	//Visiting Museum
	int r = (r % 51) + 50;
	for (int j = 0; j < r; j++)
	Yield();
	Printx("%s has left the Museum\n",23,0);
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	passengerCount[car]+=1;
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (passengerCount[car] == passengerMax[car])
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); //Last one back tells driver all are out of the museum
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has been told to get into Car[%d]\n",37,car*10000000);
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	passengerCount[car]-=1;
	Printx("%s has gotten into Car[%d] and is waiting to leave\n",51,car*10000000);
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if (passengerCount[car] == 0)
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); //Last one in signals driver all passengers in car
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	}
returnValue = Release(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
}


void ValetManager()
{
	int returnValue = 0;
	valetManagerPresent = 1; //Avoids race condition in which valets message valetmanager when not initialized
returnValue = Acquire(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Wait(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	while (true)
	{
	valetManagerPresent = 0;
	
	int valetBench = 0;
	int check = 0;
	for (int a = 0; a < parkingValets; a++)
	{
	if (valetStatus[a] == -1)
	valetBench++;
	if (valetBench >= 2)
	{
	check = 1;
	Printx("%s has detected two Parking Valets on the bench\n",48,0);
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	int index;
	for (int v = 0; v < parkingValets; v++)
	{
	if ((limoLineLength[v] + valetLineLength[v]) == 0)
	{
	index = v;
returnValue = Acquire(	limoLineLengthLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[v] = 999;
returnValue = Release(	limoLineLengthLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	break;
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	if ((limoLineLength[index] + valetLineLength[index]) == 999)
	{
returnValue = Acquire(	valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	valetLineCar[index] = -1;
	valetReturn[index] = -2;
returnValue = Signal(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Wait(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	Printx("%s has sent Parking Valet[%d] to the back room\n",47,index*10000000);
returnValue = Signal(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
returnValue = Release(	valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	}
	else
	{
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Acquire(	limoLineLengthLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
	limoLineLength[index] = 0;
returnValue = Release(	limoLineLengthLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	Printx("%s sees more cars out so ParkingValet[%d] is needed\n",52,index*10000000);
	}
	
	}
	if (check == 1)
	break;
	
	}
	carCount = 0;
	for (int a = 0; a < parkingValets; a++)
	{
	if ((valetStatus[a] > -2) && (valetReturn[a] != -2))
	{
	carCount+=(valetLineLength[a] + limoLineLength[a]);	
	}
	}
	if (carCount >= 4)
	{
	Printx("%s has detected four (or more cars) waiting to be parked\n",57,0);
	for (int v = 0; v < parkingValets; v++)
	{
	if (valetStatus[v] == -2)
	{
returnValue = Acquire(	backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); }
returnValue = Signal(	backRoomCondition[v] ,backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); }
	Printx("%s has told ParkingValet[%d] to come out of the back room\n",58,v*10000000);
returnValue = Release(	backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); }
	break;
	}
	}
	}
	valetManagerPresent = 1;
returnValue = Wait(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); }
	}
}

void 
Init(int c, int v, int t) //This function initializes all lock arrays and condition arrays
{
	int i;
	char* name;
	
	limoReturnLineLock = CreateLock("limoReturnLineLock");
carLineLock = CreateLock("carLineLock");
carReturnLineLock = CreateLock("carReturnLineLock");
carLock = CreateLock* [20];
passengerCountLock = CreateLock* [20];
carStatusLock = CreateLock* [20];
valetLock = CreateLock* [5];
valetStatusLock = CreateLock* [5];
limoReturnLineLengthLock = CreateLock("limoReturnLineLengthLock");
carLineLengthLock = CreateLock("carLineLengthLock");
carReturnLineLengthLock = CreateLock("carReturnLineLengthLock");
limoReturnLineCondition = CreateCondition("limoReturnLineCondition");
carLineCondition = CreateCondition("carLineCondition");
carReturnLineCondition = CreateCondition("carReturnLineCondition");
carCondition = CreateCondition* [20];
valetCondition = CreateCondition* [5];

valetManagerLock = CreateLock("valetManagerLock");
valetManagerCondition = CreateCondition("valetManagerCondition");

ticketTakerLock = CreateLock* [5];
ticketLineLock = CreateLock* [5];
ticketLineLengthLock = CreateLock* [5];
lineCheckLock = CreateLock("lineCheckLock"); 
ticketLineCondition = CreateCondition* [5];
ticketTakerCondition = CreateCondition* [5];

valetLineLock = CreateLock* [5];
valetLineCondition = CreateCondition* [5]; 
valetLineCheckLock = CreateLock("valetLineCheckLock");
valetLineLengthLock = CreateLock* [5];
valetLineLengthCondition = CreateCondition* [5];
limoLineCheckLock = CreateLock("limoLineCheckLock");
limoLineLock = CreateLock* [5];
limoLineCondition = CreateCondition* [5];
limoLineLengthLock = CreateLock* [5];
visitorLock = CreateLock* [20];
visitorCondition = CreateCondition* [20];

backRoomLock = CreateLock* [5];
backRoomCondition = CreateCondition* [5];

for( i = 0; i <5; i++) {
	ticketLineLength[i] = 0;
	ticketLineVisitor[i] = -1;
	ticketDriver[i] = 0;
	valetLineLength[i] = 0;
	limoLineLength[i] = 0;
	valetLineLimo[i] =0;
	valetLineCar[i] = 0;
	valetStatus[i] = 0; //-1 if available, -2 if in back, 0 is busy
	valetPriority[i] = 0;
	valetReturn[i] = 0;
}

limoReturnLineLength = 0;
carLineLength = 0;
carReturnLineLength = 0;
carCount = 0;
ticketTakers = -1;
parkingValets = -1;
cars = -1;
visitors = -1;


frontOfLimoLine = -1;
frontOfCarLine = -1;
frontOfLimoReturnLine = -1;
frontOfCarReturnLine = -1;
valetCheck = 0;

valetManagerPresent = 0;
valetManagerCount = 0;
	
	cars = c;
	parkingValets = v;
	ticketTakers = t;
	for (i = 0; i < cars; i++)
	{
	passengerCount[i] = 0;
	passengerMax[i] = 0;
	carStatus[i] = 0;
	carValet[i] = -1;
	carLastOneOut[i] = 0;
	driverIsPresent[i] = 0;
	}
	
	for (i = 0; i < cars; i++)
	{
	name = new char [30];
	sprintf(name,"Car[%d]Lock",i);
	carLock[i] = CreateLock(name);
	}
	
	for (i = 0; i < cars; i++)
	{
	name = new char [30];
	sprintf(name,"CarStatus[%d]Lock",i);
	carStatusLock[i] = CreateLock(name);
	}
	
	for (i = 0; i < cars; i++)
	{
	name = new char [30];
	sprintf(name,"Car[%d]Condition",i);
	carCondition[i] = CreateCondition(name);
	}
	
	for (i = 0; i < cars; i++)
	{
	name = new char [30];
	sprintf(name,"PassengerCount[%d]Lock",i);
	passengerCountLock[i] = CreateLock(name);
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [30];
	sprintf(name,"Valet[%d]Lock",i);
	valetLock[i] = CreateLock(name);
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [30];
	sprintf(name,"ValetStatus[%d]Lock",i);
	valetStatusLock[i] = CreateLock(name);
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [30];
	sprintf(name,"Valet[%d]Condition",i);
	valetCondition[i] = CreateCondition(name);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [30];
	sprintf(name,"TicketTaker[%d]Lock",i);
	ticketTakerLock[i] = CreateLock(name);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [30];
	sprintf(name,"TicketLine[%d]Lock",i);
	ticketLineLock[i] = CreateLock(name);
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [30];
	sprintf(name,"TicketLineLength[%d]Lock",i);
	ticketLineLengthLock[i] = CreateLock(name);
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [40];
	sprintf(name,"TicketLine[%d]Condition",i);
	ticketLineCondition[i] = CreateCondition(name);
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [40];
	sprintf(name,"TicketTaker[%d]Condition",i);
	ticketTakerCondition[i] = CreateCondition(name);
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"valetLineLength[%d]Lock",i);
	valetLineLengthLock[i] = CreateLock(name);
	}	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"valetLine[%d]Lock",i);
	valetLineLock[i] = CreateLock(name);
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"valetLineLength[%d]Condition",i);
	valetLineLengthCondition[i] = CreateCondition(name);
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"valetLine[%d]Condition",i);
	valetLineCondition[i] = CreateCondition(name);
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"limoLineLength[%d]Lock",i);
	limoLineLengthLock[i] = CreateLock(name);
	}	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"limoLine[%d]Lock",i);
	limoLineLock[i] = CreateLock(name);
	}	
	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"limoLine[%d]Condition",i);
	limoLineCondition[i] = CreateCondition(name);
	}	
	
	for (i = 0; i < cars; i++)
	{
	name = new char [40];
	sprintf(name,"vistor[%d]Lock",i);
	visitorLock[i] = CreateLock(name);
	}	
	
	
	for (i = 0; i < cars; i++)
	{
	name = new char [40];
	sprintf(name,"visitor[%d]Condition",i);
	visitorCondition[i] = CreateCondition(name);
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"backRoom[%d]Condition",i);
	backRoomCondition[i] = CreateCondition(name);
	}	

	for (i = 0; i < parkingValets; i++)
	{
	name = new char [40];
	sprintf(name,"backRoomLock[%d]",i);
	backRoomLock[i] = CreateLock(name);
	}	
}

void
SystemTest() //Main test function
{
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	ticketTakers = -1;
	parkingValets = -1;
	cars = -1;
	while (ticketTakers == -1)
	{
	Printx("Enter number of Ticket Takers (1-5): ",37,0);
	scanf("%d", &ticketTakers);
	if ((ticketTakers > 5) || (ticketTakers < 1)) 
	{
	Printx("Number of Ticket Takers must be between 1 and 5\n",48,0);
	ticketTakers = -1;
	}
	}
	while (parkingValets == -1)
	{
	Printx("Enter number of Parking Valets (1-5): ",38,0);
	scanf("%d", &parkingValets);
	if ((parkingValets > 5) || (parkingValets < 1)) 
	{
	Printx("Number of Parking Valets must be between 1 and 5\n",49,0);
	parkingValets = -1;
	}
	}
	while (cars == -1)
	{
	Printx("Enter number of Cars (5-20): ",29,0);
	scanf("%d", &cars);
	if ((cars > 20) || (cars < 5))
	{
	printf("Number of Cars must be between 5 and 20\n",40,0);
	cars = -1;
	}
	}
	
	Init(cars,parkingValets,ticketTakers);
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [20];
	sprintf(name,"ParkingValet[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Valet,i);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [20];
	sprintf(name,"TicketTaker[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)TicketTaker,i);
	}
	
	j = 0;
	int pj;
	int limoCounter;
	int carCounter;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2; //multiplied by 2 so eventually threads can determine if limo or car and id
	int flip = rand() % 2; //randomly generates car or limo
	passengerMax[i] = (rand() % 5) + 1; //randomly generates passengers
	limoCounter = 0;
	carCounter = 0;
	if (flip == 1)
	{
	carCounter++;
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	limoCounter++;
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	name = new char [30];
	sprintf(name,"ValetManager");
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)ValetManager,0);
	
	Printx("Number of LimousineDrivers = [%d]\n",34,limoCounter*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,carCounter*10000000);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);
	
}

void Test2() //Initialized with 4 limos, 1 car, and 1 valet
{
	Thread * t;
	char * name;
	int i;
	int j;
	int r;	
	int c;
	
	Init(5,1,1);
	
	j=0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = 0;
	if (i == 2)
	flip = 1;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [20];
	sprintf(name,"ParkingValet[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Valet,i);
	}
	Printx("Number of LimousineDrivers = [%d]\n",34,40000000);
	Printx("Number of CarDrivers = [%d]\n",28,10000000);
	Printx("Number of ParkingValets = [%d]\n",31,10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,0);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);


}

void Test3() //Initialized with 3 limos with 2 passengers each but no valets
{
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	
	Init(3,1,1);
	
	j = 0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = rand() % 2;
	flip = 0;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
		Printx("Number of LimousineDrivers = [%d]\n",34,30000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,0);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,0);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}

void Test4() //Initialized with 1 limo of 4 passengers and 1 ticket taker
{
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	
	Init(1,1,1);
	
	j = 0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = 0;
	passengerMax[i] = 4;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [20];
	sprintf(name,"TicketTaker[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)TicketTaker,i);
	}
	
	Printx("Number of LimousineDrivers = [%d]\n",34,10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,0);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}

void Test5() //Initialized with 1 car, 1 valet, and 1 ticket taker
{
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	
	Init(1,1,1);
	
	j = 0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = 1;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [20];
	sprintf(name,"ParkingValet[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Valet,i);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char [20];
	sprintf(name,"TicketTaker[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)TicketTaker,i);
	}
		Printx("Number of LimousineDrivers = [%d]\n",34,0);
	Printx("Number of CarDrivers = [%d]\n",28,10000000);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}
void Test6() //Initialized with 1 car, 5 valets, 1 ticket taker, and 1 valet manager
{
	
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	
	Init(1,5,1);
	for (i = 0; i < 5; i++)
	{
	valetStatus[i] = -1;
	}
	j = 0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = 0;
	passengerMax[i] = 1;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [20];
	sprintf(name,"ParkingValet[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Valet,i);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char[20];
	sprintf(name,"TicketTaker[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)TicketTaker,i);
	}
	name = new char[30];
	sprintf(name,"ValetManager");
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)ValetManager,i);
	
	Printx("Number of LimousineDrivers = [%d]\n",34,cars*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);
	

}

//Requires special simulation that initialized 1 valet in the back room
void Test7() //Initialized with 20 cars, 2 valets, 1 ticket taker, and 1 valet manager
{
	
	Thread * t;
	char * name;
	int i;
	int j;
	int r;
	int c;
	
	Init(20,2,1);
	valetStatus[1] = 22;
	j = 0;
	int pj;
	for (i = 0; i < cars; i++)
	{
	name = new char [20];
	c = carCount++;
	c*=2;
	int flip = 0;
	passengerMax[i] = 1;
	if (flip == 1)
	{
	c+=1;
	sprintf(name,"CarDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)CarDriver,((c*1000)+i));
	}
	else
	{
	sprintf(name,"LimousineDriver[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)LimoDriver,((c*1000)+i));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = new char [20];
	sprintf(name,"Visitor[%d]",j);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Visitor,((c*1000)+j));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = new char [20];
	sprintf(name,"Valet[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Valet,i);
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = new char[20];
	sprintf(name,"TicketTaker[%d]",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)TicketTaker,i);
	}
	name = new char[30];
	sprintf(name,"ValetManager");
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)ValetManager,i);
	
	Printx("Number of LimousineDrivers = [%d]\n",34,cars*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}


void
Problem2()
{
	int option = -1;
	Printx("The Museum of Natural History Parking Simulation\n",49,0);
	Printx("\t1. System Test\n",16,0);
	Printx("\t2. Limos Park First, Then Cars Test\n",37,0);
	Printx("\t3. No Valet, No Parking Test\n",30,0);
	Printx("\t4. Ticket Taker Test\n",22,0);
	Printx("\t5. Drivers Leave Properly Test\n",32,0);
	Printx("\t6. Valet Manager Test - Sending Valet to Back Room\n",52,0);
	Printx("\t7. Valet Manager Test - Waking Up Valet From Back Room\n",56,0);
	Printx("Please enter one of the options above: ",39,0);
	scanf("%d",&option);
	switch (option)
	{
	case 1:
	SystemTest();
	break;
	case 2:
	Test2();
	break;
	case 3:
	Test3();
	break;
	case 4:
	Test4();
	break;
	case 5:
	Test5();
	break;
	case 6:
	Test6();
	break;
	case 7:
	Test7();
	break;
	default:
	Printx("Not a valid choice. Simulation terminating.\n",44,0);
	exit(1);
	break;
	}
	
}
