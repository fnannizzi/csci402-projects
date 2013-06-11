// p2functions.h 
// Francesca Nannizzi

#include "system.h"
#include "synch.h"
#include "globaldata.h"
#include <cstdlib>

#ifndef MUSEUM_FUNCTIONS

// --------------------------------------------------
// Natural History Museum Parking Simulation
// --------------------------------------------------
// Visitor function
// --------------------------------------------------
void Visitor(int index) {
	// Data
	int lineIndex = 0; // used to index the Ticket Taker data
	int visitDuration = 0; // set randomly, determines length of museum visit
	int carIndex = (index/1000) - 1; // decode the index to determine which car we belong to
	int passengerIndex = index % 1000; // decode the index to determine which passenger position in the car we are
	int totalNumPassengers = totalPassengers[carIndex];

	// Waiting in car	
	driverPassengerLock[carIndex]->Acquire(); // acquire the lock on the driver/passenger interaction

	//printf("%s waiting to exit Car[%d] \n", 
	//		currentThread->getName(), carIndex);
	driverPassengerCV[carIndex]->Wait(driverPassengerLock[carIndex]); // TODO
	printf("%s has been told to exit Car[%d] \n", 
			currentThread->getName(), carIndex);
	
	// Exit the car after signal from driver
	passengerCount[carIndex]--; // decrement to show that we are exiting the car
	printf("%s has exited Car[%d] \n", 
			currentThread->getName(), carIndex);
	if(passengerCount[carIndex] == 0){ // passengers have exited the car
		driverPassengerCV[carIndex]->Signal(driverPassengerLock[carIndex]); // signal the driver to alert him that all passengers are ready
	}
	driverPassengerLock[carIndex]->Release(); // release the lock on the driver/passenger interaction

	if(!onlyPark){ // not running a parking-only simulation		
		// Begin Ticket Taker interaction
		// Choose a Ticket Taker to line up for
		if(museumOpen){ // museum is open (not running a parking test case)
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
			
			ticketVisitorNumberLock[lineIndex]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
			ticketVisitorNumber[lineIndex] = index; // update ticketVisitorNumber[] with my unique index, which serves as my ticket
			ticketVisitorNumberLock[lineIndex]->Release(); // release the lock around ticketVisitorNumber[index]
			
			ticketTakerAlertCV[lineIndex]->Signal(ticketTakerAlertLock[lineIndex]); // alert the Ticket Taker that there is a visitor to serve
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
		} // end if(museumOpen)
		
		// Tell driver we have exited the museum	
		driverPassengerLock[carIndex]->Acquire(); // acquire the lock on the driver/passenger interaction
		passengerCount[carIndex]++; // increment to show that we have exited the museum
		if(passengerCount[carIndex] == totalNumPassengers){ // passengers have exited the museum
			printf("%s signalling Car[%d] \n", 
				currentThread->getName(), carIndex);
			driverPassengerCV[carIndex]->Signal(driverPassengerLock[carIndex]); // signal the driver to alert him that all passengers are ready
		}
		
		// Wait until driver signals us to enter the car
		driverPassengerCV[carIndex]->Wait(driverPassengerLock[carIndex]); // wait until driver sees that all passengers are ready to get into the car
		printf("%s has been told to get into Car[%d] \n", 
				currentThread->getName(), carIndex);
				
		// Enter the car		
		passengerCount[carIndex]--; // decrement to show that we have entered the car
		printf("%s has gotten into Car[%d] and is waiting to leave \n",
				currentThread->getName(), carIndex);
		if(passengerCount[carIndex] == 0){ // passengers have exited the museum
		 	driverPassengerCV[carIndex]->Signal(driverPassengerLock[carIndex]); // signal the driver to alert him that all passengers are ready
		}
		driverPassengerLock[carIndex]->Release(); // release the lock around passengerCount[]
	} // end if(!onlyPark)
}

// --------------------------------------------------
// Limo Driver function
// --------------------------------------------------
void LimoDriver(int index) {
	// Data
	driverState status = WAITING_TO_PARK; // enum that holds our current state
	int valetIndex = 0; // used to index the Valet data
	int numPassengers = 0; 
	
	int totalNumPassengers = totalPassengers[index];
	
	for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield to allow passengers to set up
	
	while(true){		
		if(status == WAITING_TO_PARK){ // waiting in line for the valet	
			if(onlyPark){
				printf("%s has arrived at the museum and is waiting to park. \n", 
						currentThread->getName());
			}
						
			// Wait in the line of limos to be parked
			valetLimoLineLock->Acquire(); // when acquired you are at the front of the line
			status = SEARCHING_FOR_VALET;
		}
		else if(status == SEARCHING_FOR_VALET){ // searching for an available valet
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
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often TODO busy wait
			}
			else { // found an available valet
				valetLimoParkingLock[valetIndex]->Acquire(); // acquire the lock on the valet, so we are ready to interact with him
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				status = WAITING_TO_TELL_PASSENGERS_TO_EXIT;
			}
		}
		else if(status == WAITING_TO_TELL_PASSENGERS_TO_EXIT){ // waiting for passengers to be ready for our signal
			if(totalNumPassengers > 0){
				driverPassengerLock[index]->Acquire(); // TODO
				printf("%s has told his visitors to leave Car[%d] \n", 
						currentThread->getName(), index);
				driverPassengerCV[index]->Broadcast(driverPassengerLock[index]); // TODO
	
				status = WAITING_FOR_PASSENGERS_TO_EXIT;
			}
			else {
				status = PARKING_CAR;
			}
		}
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT){ // waiting for passengers to leave car
			driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to all be out of the car
			status = PARKING_CAR;
		}
		else if(status == PARKING_CAR){ // interacting with valet to park car and exchange keys and token
			printf("%s has parked Car[%d] at the Museum \n", 
					currentThread->getName(), index);

			valetCarNumber[valetIndex] = index; // make the valet unavailable, and let him know which car is being parked
			
			for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
			
			valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // wake up valet if he is sleeping
			//printf("%s signalling valet \n", currentThread->getName());
			valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
			printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), valetIndex, index);
			
			valetTokenExchangeLock[valetIndex]->Acquire(); // acquire the lock on the token exchange so the valet can't drive away until we have received a token										
			//printf("%s waiting on valet \n", currentThread->getName());
			valetLimoParkingCV[valetIndex]->Wait(valetLimoParkingLock[valetIndex]); // wait for the valet to park the car
			printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), index, valetIndex, index);

			valetTokenExchangeLock[valetIndex]->Release(); // release the token exchange lock to allow the valet to park the car
			valetLimoParkingLock[valetIndex]->Release(); // release the lock on the valet, now that the car is parked
			valetLimoLineLock->Release(); // allow the next driver in line to interact with the valets
			
			if(!onlyPark){
				status = WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM; // visitors are inside the museum now
			
			}
			else {
				status = QUIT;
			}
		}	
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM){ // waiting for all passengers to exit museum
			if(totalNumPassengers > 0){
				printf("%s waiting Car[%d] \n", 
					currentThread->getName(), index);
				driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to exit museum
				driverPassengerLock[index]->Release(); // TODO
				
				printf("%s has been notified that all their Visitors have left the Museum for Car[%d] \n",
						currentThread->getName(), index);
			}
			status = WAITING_IN_TOKEN_LINE;
		}
		else if(status == WAITING_IN_TOKEN_LINE){ // waiting in line for the valet			
			// Wait in the line of drivers who want to return their tokens
			valetTokenReturnLineLock->Acquire(); // when acquired you are at the front of the line
			
			status = RETURNING_TOKEN;
		}
		else if(status == RETURNING_TOKEN){ // at the front of the token return line, but don't have a valet helping yet
			for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Acquire(); } // acquire the locks around valetCarNumber[]
			
			// Search for an available valet
			for(int i = 0; i < numValets; i++){
				if(valetCarNumber[i] == WAITING_ON_BENCH){ // if the valet is available
					valetIndex = i; // record his index
					break;
				}
			}
			if(valetCarNumber[valetIndex] != WAITING_ON_BENCH){ // no valets are available
				for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
				
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
			else { // valet available to return car				
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				valetCarNumber[valetIndex] = ((index + 1)*TOKEN_MULTIPLIER); // give the valet our token
				for(int i = 0; i < numValets; i++){ valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
				
				valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // alert the valet that there is a driver trying to return a token
				//printf("%s signalling valet \n", currentThread->getName());
				valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
				printf("%s has given Parking Token[%d] to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
				printf("%s has given a tip to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
						
				valetTokenReturnLock[valetIndex]->Acquire(); // acquire the lock on the token exchange so the valet can return the keys/car
				valetTokenReturnCV[valetIndex]->Wait(valetTokenReturnLock[valetIndex]); // wait for the valet to come back with the keys/car
				valetTokenReturnLock[valetIndex]->Release(); // release the lock on the token exchange after receiving the keys/car
				printf("%s has received their keys from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
			
				valetTokenReturnLineLock->Release(); // allow the next driver to return their token

				status = TELL_PASSENGERS_TO_ENTER_CAR;
			}			
		}
		else if(status == TELL_PASSENGERS_TO_ENTER_CAR){ // alert all passengers to get in the car
			if(totalNumPassengers > 0){
				driverPassengerLock[index]->Acquire(); // TODO
				driverPassengerCV[index]->Broadcast(driverPassengerLock[index]); // TODO
			}
			status = LEAVING_MUSEUM;
		}
		else if(status == LEAVING_MUSEUM){ // waiting for all passengers to get in the car before leaving
			if(totalNumPassengers > 0){
				driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to enter car
				//printf("%s tell passengers to get in Car[%d] \n", 
				//		currentThread->getName(), index);
				driverPassengerLock[index]->Release(); // release the lock around passengerCount[]
			}	
			printf("%s has left the Museum in Car[%d] \n", 
					currentThread->getName(), index);
					
			status = QUIT;
		}
		else if(status == QUIT){
			break;
		}
		//printf("%s looping \n", currentThread->getName());
	}
}


// --------------------------------------------------
// Car Driver function
// --------------------------------------------------
void CarDriver(int index) {

	// Data
	driverState status = WAITING_TO_PARK;
	int valetIndex = 0, numLimosWaiting = 0;
	int lineIndex = 0, visitDuration = 0;
	int numPassengers = 0; 
	
	int totalNumPassengers = totalPassengers[index];
	
	for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield to allow passengers to set up
	
	while(true){
		if(status == WAITING_TO_PARK){ // waiting in line for the valet			
			// Wait in the line of cars to be parked
			if(onlyPark){
				printf("%s has arrived at the museum and is waiting to park. \n", 
						currentThread->getName());
			}
			valetCarLineLock->Acquire(); // when acquired you are at the front of the line
			status = WAITING_FOR_LIMOS;
		}
		else if(status == WAITING_FOR_LIMOS){ // at the front of the car line, but there are limos to be parked
			numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
			numLimosWaiting = numLimosWaitingToPark; // update our local copy of the variable
			numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			
			// Check to see if any limos are waiting to park
			if(numLimosWaiting == 0){ // no limos waiting
				//printf("%s no limos left \n", currentThread->getName());
				status = SEARCHING_FOR_VALET;
			}
			else { // must wait for limos to be parked
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
			//printf("%s yielded \n", currentThread->getName());
		}
		else if(status == SEARCHING_FOR_VALET){ // looking for an available valet
			for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Acquire(); } // acquire the locks around valetCarNumber[]

			// Search for an available valet
			for(int i = 0; i < numValets; i++){
				if(valetCarNumber[i] == WAITING_ON_BENCH){ // if the valet is available
					valetIndex = i; // record his index
					break;
				}
			}
			if(valetCarNumber[valetIndex] != WAITING_ON_BENCH){ // no valets are available
				for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
				
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
			else { // found an available valet
				valetCarParkingLock[valetIndex]->Acquire(); // acquire the lock on the valet, so we are ready to interact with him					
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				status = WAITING_TO_TELL_PASSENGERS_TO_EXIT;
			}
		}
		else if(status == WAITING_TO_TELL_PASSENGERS_TO_EXIT){ // waiting for passengers to be ready for our signal
			if(totalNumPassengers > 0){
				driverPassengerLock[index]->Acquire(); // TODO
				printf("%s has told his visitors to leave Car[%d] \n", 
						currentThread->getName(), index);
				driverPassengerCV[index]->Broadcast(driverPassengerLock[index]); // TODO
	
				status = WAITING_FOR_PASSENGERS_TO_EXIT;
			}
			else {
				status = PARKING_CAR;
			}
		}
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT){ // waiting for passengers to leave car
			driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to all be out of the car
			status = PARKING_CAR;
		}
		else if(status == PARKING_CAR){ // interacting with valet to park car and exchange keys and token
			printf("%s has parked Car[%d] at the Museum \n", 
					currentThread->getName(), index);
			valetCarNumber[valetIndex] = index; // make the valet unavailable, and give him the keys to the car being parked
			for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
			
			valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // alert the valet that there is a car waiting to be parked
			//printf("%s signalling valet \n", currentThread->getName());
			valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
			printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), valetIndex, index);
					
			valetTokenExchangeLock[valetIndex]->Acquire(); // acquire the lock on the token exchange so the valet can't drive away until we have received a token																
			valetCarParkingCV[valetIndex]->Wait(valetCarParkingLock[valetIndex]); // wait for the valet to park the car
			printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), index, valetIndex, index);
					
			valetTokenExchangeLock[valetIndex]->Release(); // release the token exchange lock to allow the valet to park the car		
			valetCarParkingLock[valetIndex]->Release(); // release the lock on the valet, now that the car is parked
			valetCarLineLock->Release(); // allow the next driver in line to interact with the valets
			
			if(!onlyPark){
				status = IN_MUSEUM; // visitors and driver are inside the museum now
			}
			else {
				status = QUIT;
			}
		}	
		else if(status == IN_MUSEUM){ // Begin Ticket Taker interaction
			if(museumOpen){ // if museum is open (not running a parking test case)
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
				
				ticketVisitorNumberLock[lineIndex]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
				ticketVisitorNumber[lineIndex] = (index + 1)*DRIVER_MULTIPLIER; // update ticketVisitorNumber[] with my unique index, which serves as my ticket
				ticketVisitorNumberLock[lineIndex]->Release(); // release the lock around ticketVisitorNumber[index]
				
				ticketTakerAlertCV[lineIndex]->Signal(ticketTakerAlertLock[lineIndex]); // alert the Ticket Taker that there is a visitor to serve
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
			status = WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM;
		}
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM){ // waiting for all passengers to exit museum		
			if(totalNumPassengers > 0){
				printf("%s waiting Car[%d] \n", 
					currentThread->getName(), index);
				driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to exit museum
				driverPassengerLock[index]->Release(); // TODO
			
				printf("%s has been notified that all their Visitors have left the Museum for Car[%d] \n",
						currentThread->getName(), index);
			}
			status = WAITING_IN_TOKEN_LINE;
		}
		else if(status == WAITING_IN_TOKEN_LINE){ // waiting in line for the valet			
		
			// Wait in the line of drivers who want to return their tokens
			valetTokenReturnLineLock->Acquire(); // when acquired you are at the front of the line
			
			status = RETURNING_TOKEN;
		}
		else if(status == RETURNING_TOKEN){ // at the front of the token return line, but don't have a valet helping yet
			for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Acquire(); } // acquire the locks around valetCarNumber[]
			
			// Search for an available valet
			for(int i = 0; i < numValets; i++){
				if(valetCarNumber[i] == WAITING_ON_BENCH){ // if the valet is available
					valetIndex = i; // record his index
					break;
				}
			}
			if(valetCarNumber[valetIndex] != WAITING_ON_BENCH){ // no valets are available
				for(int i = 0; i < numValets; i++){	valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
				
				for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
			}
			else { // valet available to return car				
				valetAlertLock[valetIndex]->Acquire(); // acquire the lock that allows us to signal the valet
				valetCarNumber[valetIndex] = ((index + 1)*TOKEN_MULTIPLIER); // give the valet our token
				for(int i = 0; i < numValets; i++){ valetCarNumberLock[i]->Release(); } // release the locks around valetCarNumber[]
				
				valetAlertCV[valetIndex]->Signal(valetAlertLock[valetIndex]); // alert the valet that there is a driver trying to return a token
				//printf("%s signalling valet \n", currentThread->getName());
				valetAlertLock[valetIndex]->Release(); // release the alert lock now that we have made our presence known
				printf("%s has given Parking Token[%d] to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
				printf("%s has given a tip to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
						
				valetTokenReturnLock[valetIndex]->Acquire(); // acquire the lock on the token exchange so the valet can return the keys/car
				valetTokenReturnCV[valetIndex]->Wait(valetTokenReturnLock[valetIndex]); // wait for the valet to come back with the keys/car
				valetTokenReturnLock[valetIndex]->Release(); // release the lock on the token exchange after receiving the keys/car
				printf("%s has received their keys from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
			
				valetTokenReturnLineLock->Release(); // allow the next driver to return their token
				
				status = TELL_PASSENGERS_TO_ENTER_CAR;
			}			
		}
		else if(status == TELL_PASSENGERS_TO_ENTER_CAR){ // alert all passengers to get in the car
			if(totalNumPassengers > 0){
				driverPassengerLock[index]->Acquire(); // TODO
				driverPassengerCV[index]->Broadcast(driverPassengerLock[index]); // TODO
			}
			status = LEAVING_MUSEUM;
		}
		else if(status == LEAVING_MUSEUM){ // waiting for all passengers to get in the car before leaving
			if(totalNumPassengers > 0){
				driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to enter car
				//printf("%s tell passengers to get in Car[%d] \n", 
				//		currentThread->getName(), index);
				driverPassengerLock[index]->Release(); // release the lock around passengerCount[]
			}		
			printf("%s has left the Museum in Car[%d] \n", 
					currentThread->getName(), index);
			status = QUIT;
		}
		else if(status == QUIT){
			break;
		}
		printf("%s looping \n", currentThread->getName());
	}
}

// --------------------------------------------------
// Valet function
// --------------------------------------------------
void Valet(int index) {
	// Data
	int valetStatus = ON_BENCH_NOT_WAITING; // just a reminder that all valets need to be waiting before drivers can signal them
	int numCarsWaiting = 0, numLimosWaiting = 0;
	int parkingDuration = 0, returningCarDuration = 0;
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
			printf("valet waiting\n");
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
			
			valetAlertCV[index]->Wait(valetAlertLock[index]); // wait for a driver to signal saying there are still cars to be parked
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
			valetCarNumber[index] = ON_BENCH_NOT_WAITING; // update to make sure we're ready for the next driver
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
			
			valetStatusLock[index]->Acquire(); // acquire the lock on valetStatusCV[]
		}
		else if((valetStatus % TOKEN_MULTIPLIER == 0) && (valetStatus / TOKEN_MULTIPLIER != 0)){ // valet is interacting with a driver who is trying to return their token
			if(sleepingOnBench){ 
				printf("%s has been woken up from the bench \n",
						currentThread->getName());
				sleepingOnBench = false;
			}
			
			returningCarDuration = (rand() % MAX_RETURNING_CAR_DURATION) + MIN_RETURNING_CAR_DURATION;
			for(int i = 0; i < returningCarDuration; i++){ currentThread->Yield(); }
			
			valetTokenReturnLock[index]->Acquire(); // acquire the lock on the token exchange so the we can return the keys/car
			valetTokenReturnCV[index]->Signal(valetTokenReturnLock[index]); // signal the driver to give them the keys/car
			valetTokenReturnLock[index]->Release(); // release the lock on the token exchange after receiving a tip
				
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]
			valetCarNumber[index] = ON_BENCH_NOT_WAITING; // update to make sure we're ready for the next driver
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
		
		}
		else { // valet is interacting with a driver who is trying to park
			if(sleepingOnBench){ 
				printf("%s has been woken up from the bench \n",
						currentThread->getName());
				sleepingOnBench = false;
			}
			if(vehicleType[valetStatus] == LIMO){ // odd index means the vehicle is a limo
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
				//printf("%s signalling \n", currentThread->getName());
				printf("%s has given Car Driver[%d] Parking Token[%d] for Car[%d] \n",
						currentThread->getName(), valetStatus, valetStatus, valetStatus);
				valetCarParkingCV[index]->Signal(valetCarParkingLock[index]); // signal the driver to let them know we have received their keys
				valetCarParkingLock[index]->Release(); // release the lock used to signal the driver
														
				numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
				numCarsWaitingToPark--; // update to remove the parked car
				numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark
			}
	
			valetCarNumberLock[index]->Acquire(); // acquire the lock around valetCarNumber[]	
			valetCarNumber[index] = IS_PARKING_CAR;
			valetCarNumberLock[index]->Release(); // release the lock around valetCarNumber[]
			
			valetTokenExchangeLock[index]->Acquire(); // won't be able to acquire until driver has received a token
			printf("%s is parking Car[%d] \n",
					currentThread->getName(), valetStatus);
			valetTokenExchangeLock[index]->Release(); // release the token exchange lock to allow the valet to park the car		
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
			printf("%s has detected four (or more) cars waiting to be parked \n",
					currentThread->getName());
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
		if(valetsOnBench >= MAX_NUM_VALETS_ON_BENCH){
			printf("%s has detected two Parking Valets on the bench \n",
					currentThread->getName());
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
		if(museumOpen){
			// Ticket Taker must acquire the lock in ticketTakerCV[index] before checking the ticket
			ticketTakerLock[index]->Acquire(); // acquire the lock in tickerTakerCV[index]
			ticketVisitorNumberLock[index]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
		
			if(ticketVisitorNumber[index] >= 0){ // the Visitor at the front of my line is offering a ticket
				if((ticketVisitorNumber[index] % DRIVER_MULTIPLIER == 0) && 
				   (ticketVisitorNumber[index] / DRIVER_MULTIPLIER != 0)){ // interacting with a driver
					printf("%s has received a ticket from Car Driver[%d] \n", 
							currentThread->getName(), ((ticketVisitorNumber[index]/DRIVER_MULTIPLIER) - 1));
					ticketTakerCV[index]->Signal(ticketTakerLock[index]); // we are taking their ticket, so we should tell them not to wait 
			
					printf("%s has accepted a ticket from Car Driver[%d] \n", 
							currentThread->getName(), ((ticketVisitorNumber[index]/DRIVER_MULTIPLIER) - 1));
				}
				else { // interacting with a visitor/passenger
					printf("%s has received a ticket from Visitor[%d] \n", 
							currentThread->getName(), ticketVisitorNumber[index]);
					ticketTakerCV[index]->Signal(ticketTakerLock[index]); // we are taking their ticket, so we should tell them not to wait 
			
					printf("%s has accepted a ticket from Visitor[%d] \n", 
							currentThread->getName(), ticketVisitorNumber[index]);
				}
	
				ticketLineLengthLock->Acquire(); // acquire the lock around ticketLineLength[]
				ticketLineLength[index]--; // decrement the length of line, because the Visitor is leaving
				ticketLineLengthLock->Release(); // release the lock around ticketLineLength[]
			}	
			ticketTakerAlertLock[index]->Acquire(); // acquire the lock in ticketTakerAlertCV
			ticketTakerLock[index]->Release(); // release the lock in tickerTakerCV[index]
			
			//printf("%s is waiting to be signalled by a visitor \n", 
			//		currentThread->getName());
			ticketVisitorNumber[index] = NO_TICKET_AVAILABLE; // update to show that the ticket transaction is over
			ticketVisitorNumberLock[index]->Release(); // release the lock around ticketVisitorNumber[index]
			ticketTakerAlertCV[index]->Wait(ticketTakerAlertLock[index]); // wait for a Visitor to arrive
			ticketTakerAlertLock[index]->Release(); // release the lock in ticketTakerAlertCV
		}
	}
}

#endif // MUSEUM_FUNCTIONS
