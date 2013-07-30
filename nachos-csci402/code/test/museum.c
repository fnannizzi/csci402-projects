#include "syscall.h"

/* General constants*/
#define RETURN_ERROR				-1
#define MAX_NUM_TICKET_TAKERS 		5 
#define MIN_NUM_TICKET_TAKERS 		1 
#define MAX_NUM_VALETS 				5
#define MIN_NUM_VALETS 				1
#define MAX_NUM_CARS 				20
#define MIN_NUM_CARS 				5
#define MAX_NUM_DRIVERS				20
#define MIN_NUM_DRIVERS				5

/* Valet Mananger constants*/
#define MAX_NUM_VALETS_ON_BENCH		2
#define	MIN_NUM_VEHICLES_WAITING	4
#define YIELD_DURATION				20
/* Valet constants */
#define MIN_PARKING_DURATION		5
#define MAX_PARKING_DURATION		15
#define MIN_RETURNING_CAR_DURATION	5
#define MAX_RETURNING_CAR_DURATION	15
/* Ticket Taker constants*/
#define NO_TICKET_AVAILABLE			-1
#define NOT_WAITING					-2
#define DRIVER_MULTIPLIER			1000
/* Visitor constants*/
#define MIN_VISIT_DURATION			50
/* Driver constants*/
#define TOKEN_MULTIPLIER			1000
#define CAR							0
#define LIMO						1
#define EMPTY						-1
#define TIP							-5000

int limoReturnLineLock;
int carLineLock;
int carReturnLineLock;
int carLock[MAX_NUM_DRIVERS];
int passengerCountLock[MAX_NUM_DRIVERS];
int carStatusLock[MAX_NUM_DRIVERS];
int valetLock[MAX_NUM_VALETS];
int valetStatusLock[MAX_NUM_VALETS];
int limoReturnLineLengthLock;
int carLineLengthLock;
int carReturnLineLengthLock;
int limoReturnLineCondition;
int carLineCondition;
int carReturnLineCondition;
int carCondition[MAX_NUM_DRIVERS];
int valetCondition[MAX_NUM_VALETS];

int valetManagerLock;
int valetManagerCondition;

int ticketTakerLock[MAX_NUM_TICKET_TAKERS];
int ticketLineLock[MAX_NUM_TICKET_TAKERS];
int ticketLineLengthLock[MAX_NUM_TICKET_TAKERS];
int lineCheckLock; 
int ticketLineCondition[MAX_NUM_TICKET_TAKERS];
int ticketTakerCondition[MAX_NUM_TICKET_TAKERS];

int valetLineLock[MAX_NUM_VALETS];
int valetLineCondition[MAX_NUM_VALETS]; 
int valetLineCheckLock;
int valetLineLengthLock[MAX_NUM_VALETS];
int valetLineLengthCondition[MAX_NUM_VALETS];
int limoLineCheckLock;
int limoLineLock[MAX_NUM_VALETS];
int limoLineCondition[MAX_NUM_VALETS];
int limoLineLengthLock[MAX_NUM_VALETS];
int visitorLock[MAX_NUM_DRIVERS];
int visitorCondition[MAX_NUM_DRIVERS];

int backRoomLock[MAX_NUM_VALETS];
int backRoomCondition[MAX_NUM_VALETS];


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
int valetStatus[MAX_NUM_VALETS]; /*-1 if available, -2 if in back, 0 is busy
*/
int valetPriority[MAX_NUM_VALETS];
int valetReturn[MAX_NUM_VALETS];

/*New variables*/
int carList[MAX_NUM_DRIVERS];
int carCountTotal;
int ticketTakerTotal;
int valetTotal;
int visitorTotal;
int visitorCarTotal;
int carCountY;
int carCountTotalLock;
int ticketTakerTotalLock;
int valetTotalLock;
int visitorTotalLock;
int random;

int rand() {
	return random++;
}


void
CarDriver()
{
int b,c,i;

	
	int car;
	int id;

	int valetNumber = -1;
	int returnValue = 0;
	int shortestLength = 999;
	int shortestIndex = 0;
	int a, r, j;
		returnValue = Acquire(carCountTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22,0); }
b = carCountTotal;
car = carCountTotal;
c = carCountTotal++;
c*=2;
c+=1;
i = c*1000 + b;

	/*car = i / 2000; /*car numbers are found by dividing parameter by 2000
*/
	id = i % 1000; /*id numbers are found by modding parameter by 1000
*/

returnValue = Release(carCountTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }


	/*carLastOneOut[i] = car;*/
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	driverIsPresent[car] = 1; /*for visitors to hold off on signaling driver if not initialized yet
*/
returnValue = Acquire(	visitorLock[car]); 

if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Broadcast(	visitorCondition[car] ,visitorLock[car]); /*Signaling visitors car is ready
*/

if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	while (1)
	{
	/*Arriving at Museum
*/
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 0)) 
	{
	Printx("CarDriver[%d] has parked Car[%d] at the Museum\n",47,((car*10000000)+(car*10000)));/*too high*/
returnValue = Acquire(	carStatusLock[car]); /*used to determine stages
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Broadcast(	carCondition[car] ,carLock[car]); /*signals visitors to leave car
*/
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
	Printx("CarDriver[%d] has told his visitors to leave Car[%d]\n",53,((car*10000000)+(car*10000)));	
	}
	if ((passengerCount[car] == 0) && (carStatus[car] == 1))
	{
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	for (a = 0; a < parkingValets; a++) /*checks for available valet line
*/
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
	/*waits here if valet is busy
*/
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
	/*goes ahead if valet available
*/
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 0; /*will tell valet what to do with car
*/
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has given their keys to ParkingValet[%d] for Car[%d]\n",67,((car*10000000) + (shortestIndex*10000) + (car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has received ParkingToken[%d] from ParkingValet[%d] for Car[%d]\n",78,((car*10000000)+(car*10000)+(shortestIndex*100) + car));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
	/*Valet Parking Car
*/
	
	
	
	/*Visit Museum - Should reside in the if statement and not in another if statement
*/
	/*TicketTakerInteraction
*/
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	shortestLength = 999;
	shortestIndex = 0;
	for (a = 0; a < ticketTakers; a++)
	{
	if (ticketLineLength[a] < shortestLength) /*checks for shortest ticket line
*/
	{
	shortestIndex = a;
	shortestLength = ticketLineLength[a];
	}
	}
returnValue = Release(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if (ticketLineLength[shortestIndex] > 0)
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]); /*waits if ticket taker busy
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	ticketLineCondition[shortestIndex] ,ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]); /*goes ahead if not busy
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineVisitor[shortestIndex] = i; /*lets ticket taker know who he is
*/
returnValue = Release(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("CarDriver[%d] is waiting for TicketTaker[%d]\n",45,((car*10000000) + (shortestIndex*10000)));
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has given their ticket to TicketTaker[%d]\n",58,((car*10000000) + (shortestIndex*10000)));
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has entered the Museum\n",37,car*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
	/*Visiting Museum
*/
	r = (rand() % MIN_VISIT_DURATION) + MIN_VISIT_DURATION;
	for (j = 0; j < r; j++){		
		Yield();
	}
	Printx("CarDriver[%d] has left the Museum\n", 34,car*10000000);
	
	
	}
	
	/*Valet Returning Car
*/
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 2))
	{
	Printx("CarDriver[%d] has been notified that all their Visitors have left the Museum for Car[%d]\n",89,((car*10000000) + (car*10000)));
returnValue = Acquire(	carStatusLock[car]); /*Similar to car parking
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	shortestLength = 999;
	shortestIndex = 0;
	for (a = 0; a < parkingValets; a++)
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	valetLock[shortestIndex]); /*Again all of this is similar to car parking
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 1;
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has given ParkingToken[%d] to ParkingValet[%d] for Car[%d]\n",73,((car*10000000)+(car*10000)+(shortestIndex*100)+car));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has given a tip to ParkingValet[%d] for Car[%d]\n",62,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("CarDriver[%d] has received their keys from ParkingValet[%d] for Car[%d]\n",72,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
	Printx("CarDriver[%d] has told his visitors to get into Car[%d]\n",56,((car*10000000)+(car*10000)));
	}
	if ((passengerCount[car] == 0) && (carStatus[car] == 3))
	{
	Printx("CarDriver[%d] has left the Museum in Car[%d]\n",45,((car*10000000) + (car*10000)));
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV1\n", 22, 0); return;  }
	}
	Exit(0);
}

void
LimoDriver() /*This is simply a repeat of Car Driver without any museum visiting or ticket taker interaction
*/
{
int b, c, i;

	int car; 
	int id;
	int valetNumber = -1;
	int returnValue = 0;
	int shortestLength = 999;
	int shortestIndex = 0;
	int a, y, r;

	returnValue = Acquire(carCountTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
b = carCountTotal;
car = carCountTotal;
c = carCountTotal++;
c*=2;
i = c*1000 + b;

/*car = i / 2000;*/
id = i % 1000;

returnValue = Release(carCountTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	

	
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	driverIsPresent[car] = 1;
returnValue = Acquire(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Broadcast(	visitorCondition[car] ,visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	while (1)
	{	
	/*Arriving at Museum
*/
	if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 0)) 
	{
	
	Printx("LimoDriver[%d] has parked Car[%d] at the Museum\n",48,((car*10000000) + (car*10000)));
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
	Printx("LimoDriver[%d] has told his visitors to leave Car[%d]\n",54,((car*10000000) + (car*10000)));	
	}
	
	/*Valet Parking Car
*/
	else if ((passengerCount[car] == 0) && (carStatus[car] == 1))
	{
returnValue = Acquire(	carStatusLock[car]); /*Only difference here is there is a limo line for limo priority
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	
	for (a = 0; a < parkingValets; a++)
	{
	if (limoLineLength[a] < shortestLength)
	{
	shortestIndex = a;
	shortestLength = limoLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if ((limoLineLength[shortestIndex]+valetLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[shortestIndex]++;
returnValue = Release(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	limoLineCondition[shortestIndex] ,limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
returnValue = Acquire(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[shortestIndex]++;
returnValue = Release(	limoLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 2; /*tells valet he is parking a limo
*/
returnValue = Release(	limoLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("LimoDriver[%d] has given their keys to ParkingValet[%d] for Car[%d]\n",68,((car*10000000)+(shortestIndex*10000)+(car*100)));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("LimoDriver[%d] has received ParkingToken[%d] from ParkingValet[%d] for Car[%d]\n",79,((car*10000000)+(car*10000)+(shortestIndex*100)+car));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	
	/*Valet Returning Car
*/
	/*Exactly the same as car driver interaction with valet
*/
	else if ((passengerCount[car] == passengerMax[car]) && (carStatus[car] == 2))
	{
	Printx("LimoDriver[%d] has been notified that all their Visitors have left the Museum for Car[%d]\n",90,((car*10000000)+(car*10000)));
returnValue = Acquire(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	carStatus[car]+=1;
returnValue = Release(	carStatusLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	shortestLength = 999;
	shortestIndex = 0;
	for (a = 0; a < parkingValets; a++)
	{
	if ((valetLineLength[a]+limoLineLength[a]) < shortestLength)
	{
	shortestIndex = a;
	shortestLength = valetLineLength[a];
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if ((valetLineLength[shortestIndex] + limoLineLength[shortestIndex]) > 0)
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	valetLineCondition[shortestIndex] ,valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
returnValue = Acquire(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[shortestIndex]++;
returnValue = Release(	valetLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineCar[shortestIndex] = i;
	valetReturn[shortestIndex] = 1;
returnValue = Release(	valetLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("LimoDriver[%d] has given ParkingToken[%d] to ParkingValet[%d] for Car[%d]\n",74,((car*10000000)+(car*10000)+(shortestIndex*100)+car));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("LimoDriver[%d] has given a tip to ParkingValet[%d] for Car[%d]\n",63,(car*10000000)+(shortestIndex*10000)+(car*100));
returnValue = Signal(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[shortestIndex] ,valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("LimoDriver[%d] has received their keys from ParkingValet[%d] for Car[%d]\n",73,((car*10000000)+(car*10000)+(car*100)));
returnValue = Release(	valetLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
returnValue = Broadcast(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error broadcasting on CV\n", 26, 0); return;  }
	Printx("LimoDriver[%d] has told his visitors to get into Car[%d]\n",57,((car*10000000) + (car*10000)));
	}
	else if ((passengerCount[car] == 0) && (carStatus[car] == 3))
	{
	Printx("LimoDriver[%d] has left the Museum in Car[%d]\n",46,((car*10000000) + (car*10000)));
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV2 %d\n", 25, passengerMax[car]*10000000); return;  }
	
	}	
Exit(0);	
}

void Valet()
{
int i;
int returnValue = 0;

int r,y;
returnValue = Acquire(valetTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
i = valetTotal++;
returnValue = Release(valetTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }


returnValue = Acquire(	valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	while (1)
	{
	if (valetStatus[i] == 22) /*Used for Test 7 to simulate a back room. Otherwise never used
*/
	{
	valetStatus[i] = -2;
	limoLineLength[i] = 999;
returnValue = Acquire(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Wait(	backRoomCondition[i] ,backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	valetStatus[i] = -1;
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[i] = 0;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("Valet[%d] is coming out of the back room\n",41,i*10000000);
returnValue = Release(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	valetStatus[i] = 0; /*Means he is busy unless does not deal with a car
*/
	if ((limoLineLength[i] > 0) && (valetReturn[i] == 2) && (valetStatus[i] > -2))
	{
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0) /*Can determine if car is a limo or car
*/
	limo = 1;
	car/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	if (limo)
	Printx("Valet[%d] has received the keys from LimousineDriver[%d] for Car[%d]\n",69,((i*10000000)+(driver*10000)+(valetLineCar[i]*100)));
	else
	Printx("Valet[%d] has received the keys from CarDriver[%d] for Car[%d]\n",63,((i*10000000)+(driver*10000)+(car*100)));
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	if (limo)
	Printx("Valet[%d] has given LimousineDriver[%d] ParkingToken[%d] for Car[%d]\n",69,((i*10000000)+(driver*10000)+(car*100)+car));
	else
	Printx("Valet[%d] has given CarDriver[%d] ParkingToken[%d] for Car[%d]\n",63,((i*10000000)+(driver*10000)+(car*100)+car));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("Valet[%d] is parking Car[%d]\n",29,((car*10000000)+(car*10000)));
	r = (rand() % 16) + 5;
	for (y = 0; y < r; y++)
	Yield();
returnValue = Acquire(	valetLineCheckLock); /*Decremeting line length
*/
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[i]--;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	
	else if ((valetLineLength[i] > 0) && (valetReturn[i] == 0) && (valetStatus[i] > -2))
	{
	/*Same thing but for car parking
*/
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0)
	limo = 1;
	car/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	if (limo)
	Printx("Valet[%d] has received the keys from LimousineDriver[%d] for Car[%d]\n",69,((i*10000000)+(driver*10000)+(car*100)));
	else
	Printx("Valet[%d] has received the keys from CarDriver[%d] for Car[%d]\n",63,((i*10000000)+(driver*10000)+(car*100)));
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	if (limo)
	Printx("Valet[%d] has given LimousineDriver[%d] ParkingToken[%d] for Car[%d]\n",69,((car*10000000)+(driver*10000)+(car*100)+car));
	else
	Printx("Valet[%d] has given CarDriver[%d] ParkingToken[%d] for Car[%d]\n",63,((i*10000000)+(driver*10000)+(car*100)+car));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("Valet[%d] is parking Car[%d]\n",29,((i*10000000) + (car*10000)));
	r = (rand() % 16) + 5;
	for (y = 0; y < 20; y++)
	Yield();
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[i]--;
returnValue = Release(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else if ((valetLineLength[i] > 0) && (valetReturn[i] == 1) && (valetStatus[i] > -2))
	{
	/*Similar to parking car with different print statements
*/
	int car = valetLineCar[i] / 1000;
	int driver = valetLineCar[i] % 1000;
	int limo = 0;
	if (car % 2 == 0)
	limo = 1;
	car/=2;
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	if (limo)
	Printx("Valet[%d] has received ParkingToken[%d] from LimousineDriver[%d] for Car[%d]\n",77,((i*10000000)+(car*10000)+(driver*100)+car));
	else
	Printx("Valet[%d] has received ParkingToken[%d] from CarDriver[%d] for Car[%d]\n",71,((i*10000000)+(car*10000)+(driver*100)+car));
	r = (rand() % 16) + 5;
	for (y = 0; y < r; y++)
	Yield();
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	if (limo)
	Printx("Valet[%d] has given LimousineDriver[%d] Car[%d]\n",48,((i*10000000)+(driver*10000)+(car*100)));
	else
	Printx("Valet[%d] has given CarDriver[%d] Car[%d]\n",42,((i*10000000)+(driver*10000)+(car*100)));
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	if (limo)
	Printx("Valet[%d] has received tip from LimousineDriver[%d] for Car[%d]\n",64,((i*10000000)+(driver*10000)+(car*100)));
	else
	Printx("Valet[%d] has received tip from CarDriver[%d] for Car[%d]\n",58,((i*10000000)+(driver*10000)+(car*100)));	
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	if (limo)
	Printx("Valet[%d] has returned keys to LimousineDriver[%d] for Car[%d]\n",63,((i*10000000)+(driver*10000)+(car*100)));
	else
	Printx("Valet[%d] has returned keys to CarDriver[%d] for Car[%d]\n",57,((i*10000000)+(driver*10000)+(car*100)));
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLock[i]);	
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineLength[i]--;
returnValue = Release(	valetLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (limoLineLength[i] > 0)
	{
returnValue = Acquire(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	limoLineCondition[i] ,limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	limoLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else if (valetLineLength[i] > 0)
	{
returnValue = Acquire(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	valetLineCondition[i] ,valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else if (valetReturn[i] == -2)
	{
returnValue = Signal(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	valetStatus[i] = -2;
	Printx("Valet[%d] is going to the back room\n",36,i*10000000);
returnValue = Acquire(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Wait(	backRoomCondition[i] ,backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	valetStatus[i] = -1;
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[i] = 0;
returnValue = Release(	limoLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("Valet[%d] is coming out of the back room\n",41,i*10000000);
returnValue = Release(	backRoomLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	if (valetStatus[i] != -2)
	{
	valetStatus[i] = -1;
	Printx("Valet[%d] is going to sleep on the bench\n",41,i*10000000);
	valetManagerCount++;
	if ((valetManagerPresent == 1) && ((valetManagerCount % 7) == 6))
	{
returnValue = Acquire(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}	
returnValue = Wait(	valetCondition[i] ,valetLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("Valet[%d] has been woken up from the bench\n",43,i*10000000);
	
	}
	} 
	Exit(0);
}


void
TicketTaker()
{
int i;
int returnValue = 0;
returnValue = Acquire(ticketTakerTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
i = ticketTakerTotal++;
returnValue = Release(ticketTakerTotalLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }

returnValue = Acquire(	ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	while (1)
	{
	/*Once signaled by next person in line, they will interact with visitor or car driver
*/
	if (ticketLineLength[i] > 0)
	{
	int visitor = ticketLineVisitor[i] % 1000;
returnValue = Signal(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	if (ticketDriver[i] == 1)
	Printx("TicketTaker[%d] has received a ticket from CarDriver[%d]\n",57,((i*10000000) + (visitor*10000)));
	else
	Printx("TicketTaker[%d] has received a ticket from Visitor[%d]\n",55,((i*10000000)+(visitor*10000)));
returnValue = Signal(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	if (ticketDriver[i] == 1)
	Printx("TicketTaker[%d] has accepted a ticket from CarDriver[%d]\n",59,((i*10000000)+(visitor*10000)));
	else
	Printx("TicketTaker[%d] has accepted a ticket from Visitor[%d]\n", 57,((i*10000000)+(visitor*10000000)));
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	ticketLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineLength[i]--;
	ticketDriver[i] = 0;
returnValue = Release(	ticketLineLengthLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (ticketLineLength[i] > 0) /*signals next person in line
*/
	{
returnValue = Acquire(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	ticketLineCondition[i] ,ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	ticketLineLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Release(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Wait(	ticketTakerCondition[i] ,ticketTakerLock[i]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	Exit(0);
}

void
Visitor()
{
	int i, b, c;
	int car;
	int id;
	int returnValue = 0;
	int shortestLength = 999;
	int shortestIndex = 0;
	int a,r,j;
	returnValue = Acquire(visitorTotalLock);
	if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	b = visitorTotal++;
	c = visitorCarTotal;
	carList[c/2]++;
	i = c*1000 + b;
	if (carList[c/2] == passengerMax[c/2])
		visitorCarTotal+=2;
	car = i / 2000;
	id = i % 1000;		
		
	returnValue = Release(visitorTotalLock);
	if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
	
	
	
	if (driverIsPresent[car] == 0) /*This solved the race condition when drivers were not initialized yet and visitors were trying to signal them
*/

	{
returnValue = Acquire(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Wait(	visitorCondition[car] ,visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
returnValue = Release(	visitorLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	
	/*Arriving at Museum
*/
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	passengerCount[car]+=1;
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (passengerCount[car] == passengerMax[car])
returnValue = Signal(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	carCondition[car] ,carLock[car]); /*Will wait for broadcast from driver
*/
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV3\n", 22, 0); return;  }
	Printx("Visitor[%d] has been told to exit Car[%d]\n",42,((id*10000000)+(car*10000)));
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	passengerCount[car]-=1; /*Decrementing passengers in car, MV for car drivers
*/
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("Visitor[%d] has exited Car[%d]\n", 31,((id*10000000)+(car*10000)));
	if (passengerCount[car] == 0)
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); /*If last one out, tells driver
*/
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	}
returnValue = Release(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }

	/*TicketTakerInteraction
*/
returnValue = Acquire(	lineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }

	for (a = 0; a < ticketTakers; a++) /*Will find shortest line
*/
	{
	if (ticketLineLength[a] < shortestLength)
	{
	shortestIndex = a;
	shortestLength = ticketLineLength[a];
	}
	}
returnValue = Release(	lineCheckLock); /*All the same as the car driver
*/
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Acquire(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	if (ticketLineLength[shortestIndex] > 0)
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]); 
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Wait(	ticketLineCondition[shortestIndex] ,ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	else
	{
returnValue = Acquire(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineLength[shortestIndex]++;
returnValue = Release(	ticketLineLengthLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
returnValue = Acquire(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	ticketLineVisitor[shortestIndex] = i;
returnValue = Release(	ticketLineLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("Visitor[%d] is waiting for TicketTaker[%d]\n",43,((id*10000000)+(shortestIndex*10000)));
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("Visitor[%d] has given their ticket to TicketTaker[%d]\n",56,((id*10000000)+(shortestIndex*10000)));
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("Visitor[%d] has entered the Museum\n",35,id*10000000);
returnValue = Signal(	ticketTakerCondition[shortestIndex] ,ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	ticketTakerLock[shortestIndex]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	
	/*Visiting Museum
*/
	r = (r % 51) + 50;
	for (j = 0; j < r; j++)
	Yield();
	Printx("Visitor[%d] has left the Museum\n",32,id*10000000);
returnValue = Acquire(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	passengerCount[car]+=1;
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (passengerCount[car] == passengerMax[car])
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); /*Last one back tells driver all are out of the museum
*/
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	}
returnValue = Wait(	carCondition[car] ,carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV4\n", 22, 0); return;  }
	Printx("Visitor[%d] has been told to get into Car[%d]\n",46,((id*10000000)+(car*10000)));
returnValue = Acquire(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	passengerCount[car]-=1;
	Printx("Visitor[%d] has gotten into Car[%d] and is waiting to leave\n",60,((id*10000000)+(car*10000)));
returnValue = Release(	passengerCountLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if (passengerCount[car] == 0)
	{
returnValue = Signal(	carCondition[car] ,carLock[car]); /*Last one in signals driver all passengers in car
*/
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	}
returnValue = Release(	carLock[car]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
Exit(0);
}


void ValetManager()
{
	int returnValue = 0;
	int valetBench,check,a,index,v;
	valetManagerPresent = 1; /*Avoids race condition in which valets message valetmanager when not initialized
*/
returnValue = Acquire(	valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Wait(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	while (1)
	{
	valetManagerPresent = 0;
	
	valetBench = 0;
	check = 0;
	for (a = 0; a < parkingValets; a++)
	{
	if (valetStatus[a] == -1)
	valetBench++;
	if (valetBench >= 2)
	{
	check = 1;
	Printx("ValetManager has detected two Parking Valets on the bench\n",58,0);
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	for (v = 0; v < parkingValets; v++)
	{
	if ((limoLineLength[v] + valetLineLength[v]) == 0)
	{
	index = v;
returnValue = Acquire(	limoLineLengthLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[v] = 999;
returnValue = Release(	limoLineLengthLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	break;
	}
	}
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	if ((limoLineLength[index] + valetLineLength[index]) == 999)
	{
returnValue = Acquire(	valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	valetLineCar[index] = -1;
	valetReturn[index] = -2;
returnValue = Signal(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Wait(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	Printx("%s has sent Parking Valet[%d] to the back room\n",47,index*10000000);
returnValue = Signal(	valetCondition[index] ,valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
returnValue = Release(	valetLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	}
	else
	{
returnValue = Acquire(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Acquire(	limoLineLengthLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
	limoLineLength[index] = 0;
returnValue = Release(	limoLineLengthLock[index]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
returnValue = Release(	valetLineCheckLock);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	Printx("ValetManager sees more cars out so ParkingValet[%d] is needed\n",62,index*10000000);
	}
	
	}
	if (check == 1)
	break;
	
	}
	carCount = 0;
	for (a = 0; a < parkingValets; a++)
	{
	if ((valetStatus[a] > -2) && (valetReturn[a] != -2))
	{
	carCount+=(valetLineLength[a] + limoLineLength[a]);	
	}
	}
	if (carCount >= 4)
	{
	Printx("ValetManager has detected four (or more cars) waiting to be parked\n",67,0);
	for (v = 0; v < parkingValets; v++)
	{
	if (valetStatus[v] == -2)
	{
returnValue = Acquire(	backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error acquiring lock\n", 22, 0); return;  }
returnValue = Signal(	backRoomCondition[v] ,backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error signalling on CV\n", 24, 0); return;  }
	Printx("ValetManager has told ParkingValet[%d] to come out of the back room\n",68,v*10000000);
returnValue = Release(	backRoomLock[v]);
if(returnValue == RETURN_ERROR){ Printx("Error releasing lock\n", 22, 0); return;  }
	break;
	}
	}
	}
	valetManagerPresent = 1;
returnValue = Wait(	valetManagerCondition ,valetManagerLock);
if(returnValue == RETURN_ERROR){ Printx("Error waiting on CV\n", 21, 0); return;  }
	}
	Exit(0);
}

void 
Init(int c, int v, int t) /*This function initializes all lock arrays and condition arrays
*/
{
	int i;
	char* name;
	
	limoReturnLineLock = CreateLock("limoReturnLineLock",sizeof("limoReturnLineLock"));
carLineLock = CreateLock("carLineLock",sizeof("carLineLock"));
carReturnLineLock = CreateLock("carReturnLineLock",sizeof("carReturnLineLock"));
limoReturnLineLengthLock = CreateLock("limoReturnLineLengthLock",sizeof("limoReturnLineLengthLock"));
carLineLengthLock = CreateLock("carLineLengthLock",sizeof("carLineLengthLock"));
carReturnLineLengthLock = CreateLock("carReturnLineLengthLock",sizeof("carReturnLineLengthLock"));
limoReturnLineCondition = CreateCondition("limoReturnLineCondition",sizeof("limoReturnLineCondition"));
carLineCondition = CreateCondition("carLineCondition",sizeof("carLineCondition"));
carReturnLineCondition = CreateCondition("carReturnLineCondition",sizeof("carReturnLineCondition"));


valetManagerLock = CreateLock("valetManagerLock",sizeof("valetManagerLock"));
valetManagerCondition = CreateCondition("valetManagerCondition",sizeof("valetManagerCondition"));


lineCheckLock = CreateLock("lineCheckLock",sizeof("lineCheckLock")); 



valetLineCheckLock = CreateLock("valetLineCheckLock",sizeof("valetLineCheckLock"));

limoLineCheckLock = CreateLock("limoLineCheckLock",sizeof("limoLineCheckLock"));


/*New variables*/

carCountTotal = 0;
ticketTakerTotal = 0;
valetTotal = 0;
visitorTotal = 0;
visitorCarTotal = 0;
carCountY = 0;
carCountTotalLock = CreateLock("carCountTotalLock",sizeof("carCountTotalLock"));
ticketTakerTotalLock = CreateLock("ticketTakerTotalLock",sizeof("ticketTakerTotalLock"));
valetTotalLock = CreateLock("valetTotalLock",sizeof("valetTotalLock"));
visitorTotalLock = CreateLock("visitorTotalLock",sizeof("visitorTotalLock"));
random = 0;


for( i = 0; i <5; i++) {
	ticketLineLength[i] = 0;
	ticketLineVisitor[i] = -1;
	ticketDriver[i] = 0;
	valetLineLength[i] = 0;
	limoLineLength[i] = 0;
	valetLineLimo[i] =0;
	valetLineCar[i] = 0;
	valetStatus[i] = 0; /*-1 if available, -2 if in back, 0 is busy
*/
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
	carList[i] = 0; /*Added this*/
	}
	
	for (i = 0; i < cars; i++)
	{
	name = "calo";
	/*sprintf(name,"Car[%d]Lock",i);*/
	carLock[i] = CreateLock(name,sizeof(name));
	}
	
	for (i = 0; i < cars; i++)
	{
	name = "cslo";
	/*sprintf(name,"CarStatus[%d]Lock",i);*/
	carStatusLock[i] = CreateLock(name,sizeof(name));
	}
	
	for (i = 0; i < cars; i++)
	{
	name = "caco";
	/*sprintf(name,"Car[%d]Condition",i);*/
	carCondition[i] = CreateCondition(name,sizeof(name));
	}
	
	for (i = 0; i < cars; i++)
	{
	name = "pclo";
	/*sprintf(name,"PassengerCount[%d]Lock",i);*/
	passengerCountLock[i] = CreateLock(name,sizeof(name));
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "valo";
	/*sprintf(name,"Valet[%d]Lock",i);*/
	valetLock[i] = CreateLock(name,sizeof(name));
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "vslo";
	/*sprintf(name,"ValetStatus[%d]Lock",i);*/
	valetStatusLock[i] = CreateLock(name,sizeof(name));
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "vaco";
	/*sprintf(name,"Valet[%d]Condition",i);*/
	valetCondition[i] = CreateCondition(name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "ttlo";
	/*sprintf(name,"TicketTaker[%d]Lock",i);*/
	ticketTakerLock[i] = CreateLock(name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "tllo";
	/*sprintf(name,"TicketLine[%d]Lock",i);*/
	ticketLineLock[i] = CreateLock(name,sizeof(name));
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = "tlll";
	/*sprintf(name,"TicketLineLength[%d]Lock",i);*/
	ticketLineLengthLock[i] = CreateLock(name,sizeof(name));
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = "tlco";
	/*sprintf(name,"TicketLine[%d]Condition",i);*/
	ticketLineCondition[i] = CreateCondition(name,sizeof(name));
	}
	
	for (i = 0; i < ticketTakers; i++)
	{
	name = "ttco";
	/*sprintf(name,"TicketTaker[%d]Condition",i);*/
	ticketTakerCondition[i] = CreateCondition(name,sizeof(name));
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "vlll";
	/*sprintf(name,"valetLineLength[%d]Lock",i);*/
	valetLineLengthLock[i] = CreateLock(name,sizeof(name));
	}	
	for (i = 0; i < parkingValets; i++)
	{
	name = "vllo";
	/*sprintf(name,"valetLine[%d]Lock",i);*/
	valetLineLock[i] = CreateLock(name,sizeof(name));
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "vllc";
	/*sprintf(name,"valetLineLength[%d]Condition",i);*/
	valetLineLengthCondition[i] = CreateCondition(name,sizeof(name));
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "vlco";
	/*sprintf(name,"valetLine[%d]Condition",i);*/
	valetLineCondition[i] = CreateCondition(name,sizeof(name));
	}	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "llll";
	/*sprintf(name,"limoLineLength[%d]Lock",i);*/
	limoLineLengthLock[i] = CreateLock(name,sizeof(name));
	}	
	for (i = 0; i < parkingValets; i++)
	{
	name = "lllo";
	/*sprintf(name,"limoLine[%d]Lock",i);*/
	limoLineLock[i] = CreateLock(name,sizeof(name));
	}	
	
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "llco";
	/*sprintf(name,"limoLine[%d]Condition",i);*/
	limoLineCondition[i] = CreateCondition(name,sizeof(name));
	}	
	
	for (i = 0; i < cars; i++)
	{
	name = "vilo";
	/*sprintf(name,"vistor[%d]Lock",i);*/
	visitorLock[i] = CreateLock(name,sizeof(name));
	}	
	
	
	for (i = 0; i < cars; i++)
	{
	name = "vico";
	/*sprintf(name,"visitor[%d]Condition",i);*/
	visitorCondition[i] = CreateCondition(name,sizeof(name));
	}
	
	for (i = 0; i < parkingValets; i++)
	{
	name = "brco";
	/*sprintf(name,"backRoom[%d]Condition",i);*/
	backRoomCondition[i] = CreateCondition(name,sizeof(name));
	}	

	for (i = 0; i < parkingValets; i++)
	{
	name = "brlo";
	/*sprintf(name,"backRoomLock[%d]",i);*/
	backRoomLock[i] = CreateLock(name,sizeof(name));
	}	
}

void
SystemTest() /*Main test function
*/
{
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj;
	int limoCounter;
	int carCounter;
	int flip;
	ticketTakers = -1;
	parkingValets = -1;
	cars = -1;
	while (ticketTakers == -1)
	{
	Printx("Enter number of Ticket Takers (1-5): ",37,0);
	/*scanf("%d", &ticketTakers);*/
	ticketTakers = 5;
	if ((ticketTakers > 5) || (ticketTakers < 1)) 
	{
	Printx("Number of Ticket Takers must be between 1 and 5\n",48,0);
	ticketTakers = -1;
	}
	}
	while (parkingValets == -1)
	{
	Printx("Enter number of Parking Valets (1-5): ",38,0);
	/*scanf("%d", &parkingValets);*/
	parkingValets = 5;
	if ((parkingValets > 5) || (parkingValets < 1)) 
	{
	Printx("Number of Parking Valets must be between 1 and 5\n",49,0);
	parkingValets = -1;
	}
	}
	while (cars == -1)
	{
	Printx("Enter number of Cars (5-20): ",29,0);
	/*scanf("%d", &cars);*/
	cars = 20;
	if ((cars > 20) || (cars < 5))
	{
	Printx("Number of Cars must be between 5 and 20\n",40,0);
	cars = -1;
	}
	}
	
	Init(cars,parkingValets,ticketTakers);
	for (i = 0; i < parkingValets; i++)
	{
	name = "valet";
	/*sprintf(name,"ParkingValet[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(Valet,name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "tickettaker";
	/*sprintf(name,"TicketTaker[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(TicketTaker,name,sizeof(name));
	}
	
	j = 0;
	limoCounter = 0;
	carCounter = 0;
	for (i = 0; i < cars; i++)
	{
	name = "cardriver";
	/*c = carCountTotal++;*/
	c*=2; /*multiplied by 2 so eventually threads can determine if limo or car and id
*/
	flip = rand() % 2; /*randomly generates car or limo
*/
	/*flip = 1;*/
	passengerMax[i] = (rand() % 5) + 1; /*randomly generates passengers*/
	/*passengerMax[i] = 2;*/

	if (flip == 1)
	{
	carCounter++;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Printx("Forked\n",7,1);
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	limoCounter++;
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "visitor";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	name = "valetmanager";
	/*sprintf(name,"ValetManager");*/
	/*t = new Thread(name)*/;
	Fork(ValetManager,name,sizeof(name));
	
	Printx("Number of LimousineDrivers = [%d]\n",34,limoCounter*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,carCounter*10000000);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);
	
}

void Test2() /*Initialized with 4 limos, 1 car, and 1 valet
*/
{
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;	
	int c;
		int pj;
		int flip;
	Init(5,1,1);
	
	j=0;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = 0;
	if (i == 2)
	flip = 1;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "name";
	/*sprintf(name,"ParkingValet[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(Valet,name,sizeof(name));
	}
	Printx("Number of LimousineDrivers = [%d]\n",34,40000000);
	Printx("Number of CarDrivers = [%d]\n",28,10000000);
	Printx("Number of ParkingValets = [%d]\n",31,10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,0);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);


}

void Test3() /*Initialized with 3 limos with 2 passengers each but no valets
*/
{
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj, flip;
	
	Init(3,1,1);
	
	j = 0;
	pj;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = rand() % 2;
	flip = 0;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
		Printx("Number of LimousineDrivers = [%d]\n",34,30000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,0);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,0);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}

void Test4() /*Initialized with 1 limo of 4 passengers and 1 ticket taker
*/
{
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj, flip;
	
	Init(1,1,1);
	
	j = 0;
	pj;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = 0;
	passengerMax[i] = 4;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "name";
	/*sprintf(name,"TicketTaker[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(TicketTaker,name,sizeof(name));
	}
	
	Printx("Number of LimousineDrivers = [%d]\n",34,10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,0);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}

void Test5() /*Initialized with 1 car, 1 valet, and 1 ticket taker
*/
{
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj, flip;
	
	Init(1,1,1);
	
	j = 0;
	pj;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = 1;
	passengerMax[i] = 2;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "name";
	/*sprintf(name,"ParkingValet[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(Valet,name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "name";
	/*sprintf(name,"TicketTaker[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(TicketTaker,name,sizeof(name));
	}
		Printx("Number of LimousineDrivers = [%d]\n",34,0);
	Printx("Number of CarDrivers = [%d]\n",28,10000000);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}
void Test6() /*Initialized with 1 car, 5 valets, 1 ticket taker, and 1 valet manager
*/
{
	
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj, flip;
	
	Init(1,5,1);
	for (i = 0; i < 5; i++)
	{
	valetStatus[i] = -1;
	}
	j = 0;
	pj;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = 0;
	passengerMax[i] = 1;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "name";
	/*sprintf(name,"ParkingValet[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(Valet,name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "name";
	/*sprintf(name,"TicketTaker[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(TicketTaker,name,sizeof(name));
	}
	name = "name";
	/*sprintf(name,"ValetManager");*/
	/*t = new Thread(name)*/;
	Fork(ValetManager,name,sizeof(name));
	
	Printx("Number of LimousineDrivers = [%d]\n",34,cars*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);
	

}

/*Requires special simulation that initialized 1 valet in the back room
*/
void Test7() /*Initialized with 20 cars, 2 valets, 1 ticket taker, and 1 valet manager
*/
{
	
	/*Thread * t;*/
	char * name;
	int i;
	int j;
	int r;
	int c;
	int pj, flip;
	
	Init(20,2,1);
	valetStatus[1] = 22;
	j = 0;
	pj;
	for (i = 0; i < cars; i++)
	{
	name = "name";
	c = carCount++;
	c*=2;
	flip = 0;
	passengerMax[i] = 1;
	if (flip == 1)
	{
	c+=1;
	/*sprintf(name,"CarDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(CarDriver,name,sizeof(name));
	}
	else
	{
	/*sprintf(name,"LimousineDriver[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(LimoDriver,name,sizeof(name));
	}
	pj = j+passengerMax[i];
	for (j; j < pj; j++)
	{	
	name = "name";
	/*sprintf(name,"Visitor[%d]",j);*/
	/*t = new Thread(name)*/;
	Fork(Visitor,name,sizeof(name));
	}
	}
	for (i = 0; i < parkingValets; i++)
	{
	name = "name";
	/*sprintf(name,"Valet[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(Valet,name,sizeof(name));
	}
	for (i = 0; i < ticketTakers; i++)
	{
	name = "name";
	/*sprintf(name,"TicketTaker[%d]",i);*/
	/*t = new Thread(name)*/;
	Fork(TicketTaker,name,sizeof(name));
	}
	name = "name";
	/*sprintf(name,"ValetManager");*/
	/*t = new Thread(name)*/;
	Fork(ValetManager,name,sizeof(name));
	
	Printx("Number of LimousineDrivers = [%d]\n",34,cars*10000000);
	Printx("Number of CarDrivers = [%d]\n",28,0);
	Printx("Number of ParkingValets = [%d]\n",31,parkingValets*10000000);
	Printx("Number of Visitors = [%d]\n",26,pj*10000000);
	Printx("Number of TicketTakers = [%d]\n",30,ticketTakers*10000000);
	Printx("Number of Cars = [%d]\n",22,cars*10000000);

}
/*int main()
{
	Problem2();
}*/


/*void
Problem2()*/
int main()
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
	/*scanf("%d",&option);*/
	option = 1;
	switch (option)
	{
	case 1:
	Printx("Here\n",5,0);
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
	/*exit(1);*/
	Exit(0);
	break;
	}
	
}
