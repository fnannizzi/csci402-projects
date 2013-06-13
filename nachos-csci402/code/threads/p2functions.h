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

	if(!onlyTickets){
	// Waiting in car	
	driverPassengerLock[carIndex]->Acquire(); // acquire the lock on the driver/passenger interaction
	//printf("%s waiting to exit Car[%d] \n", 
	//		currentThread->getName(), carIndex); TODO
	driverPassengerCV[carIndex]->Wait(driverPassengerLock[carIndex]); // wait for driver to signal us to get out
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
	}
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
			
			// Make ticket available to the Ticket Taker
			ticketVisitorNumberLock[lineIndex]->Acquire(); // acquire the lock around ticketVisitorNumber[index]
			ticketVisitorNumber[lineIndex] = index; // update ticketVisitorNumber[] with my unique index, which serves as my ticket
			ticketVisitorNumberLock[lineIndex]->Release(); // release the lock around ticketVisitorNumber[index]
			
			// Alert the Ticket Taker that there is a visitor to serve
			ticketTakerAlertCV[lineIndex]->Signal(ticketTakerAlertLock[lineIndex]); // signal the ticket taker
			printf("%s has given their ticket to TicketTaker[%d] \n", 
					currentThread->getName(), lineIndex);
			ticketTakerAlertLock[lineIndex]->Release(); // release the lock now that we have made our ticket available
			
				
			// Wait for the Ticket Taker to approve the ticket
			ticketTakerCV[lineIndex]->Wait(ticketTakerLock[lineIndex]); // wait until the Ticket Taker is available
			ticketTakerLock[lineIndex]->Release(); // the Visitor's ticket was accepted
			ticketLineLock[lineIndex]->Release(); // release the lock, allowing the next thread to interact with the Ticket Taker
		
			// Ticket Taker has approved the ticket and we can enter the museum
			printf("%s has entered the Museum \n", 
					currentThread->getName());
		
			// Begin Museum visit
			visitDuration = (rand() % MIN_VISIT_DURATION) + MIN_VISIT_DURATION;
			for(int i = 0; i < visitDuration; i++){ currentThread->Yield(); }
		
			// Exit the Museum
			printf("%s has left the Museum \n", 
					currentThread->getName());
					
		} // end if(museumOpen)
		if(!onlyTickets){
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
		}
	} // end if(!onlyPark)
}

// --------------------------------------------------
// Limo Driver function
// --------------------------------------------------
void LimoDriver(int index) {
	// Data
	driverState status = WAITING_TO_PARK; // enum that holds our current state
	int valetIndex = 0; // used to index the Valet data
	int numPassengers = 0; // number of passengers currently in the car
	int key = index, token = 0;
	
	int totalNumPassengers = totalPassengers[index]; // total number of passengers car is transporting
	
	for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield to allow passengers to set up TODO
	
	while(true){		
		if(status == WAITING_TO_PARK){ // waiting in line for the valet	
			
			// Wait in the line of limos to be parked
			if(onlyPark){
				printf("%s has arrived at the museum and is waiting to park. \n", 
						currentThread->getName());
			}
			numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
			numLimosWaitingToPark++; // update to add ourselves to line
			numLimosWaitingToParkCV->Broadcast(numLimosWaitingToParkLock);
			numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			
									
			
			valetLimoLineLock->Acquire(); // when acquired you are at the front of the line
			status = SEARCHING_FOR_VALET;
		}
		else if(status == SEARCHING_FOR_VALET){ // searching for an available valet
				
			for(int i = 0; i < numValets; i++){	
				valetStatusLock[i]->Acquire(); // acquire the locks around the valetStatusCV
			}
				
			waitOnValetLock->Acquire(); // acquire lock that allows us to wait for a valet
			limoWaitOnValetStatus[index] = WAITING_ON_VALET_TO_PARK; // update to let valets know a limo is waiting
						
			
			// Wake up all the sleeping valets			
			for(int i = 0; i < numValets; i++){	
				valetStatusCV[i]->Signal(valetStatusLock[i]); // wake up all the sleeping valets
				//printf("%s Signalled valets %d \n", currentThread->getName(), i);	
				valetStatusLock[i]->Release(); // release the alert locks now that we have made our presence known
			}
			//printf("%s Signalled valets\n", currentThread->getName());	
			// Wait for a signal from the next available valet
			limoWaitOnValetCV[index]->Wait(waitOnValetLock); 
			//printf("%s Waiting on valet\n", currentThread->getName());

			waitOnValetLock->Release(); // TODO
			
			status = WAITING_TO_TELL_PASSENGERS_TO_EXIT; // valet is ready and waiting for us now
		
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
					
			waitOnValetLock->Acquire(); // acquire lock that allows us to communicate with valets
			valetIndex = limoWaitOnValetStatus[index]; // valet should update the status register with his index
			
			// Hand over the keys
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = key; // update to give valet the "keys" 
			printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
		
			// Signal the valet waiting on us now that we are ready
			limoWaitOnValetCV[index]->Signal(waitOnValetLock); 
			//printf("%s Waiting \n", currentThread->getName());
			// Wait for the valet to return a parking token
			limoWaitOnValetCV[index]->Wait(waitOnValetLock);
			//printf("%s Received signal \n", currentThread->getName());
			// Consume the parking token 
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			token = valetExchange[valetIndex]; // consume the parking token
			valetExchange[valetIndex] = EMPTY; // leave the register empty
			printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), index, valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnValetLock->Release(); // release the lock and allow other drivers to communicate with the valets
			valetLimoLineLock->Release(); // allow the next driver in line to interact with the valets TODO
			
			if(!onlyPark){
				status = WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM; // visitors and driver are inside the museum now
			}
			else {
				status = WAITING_IN_TOKEN_LINE; // we are only testing parking, so quit
			}
		}	
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM){ // waiting for all passengers to exit museum
			if(totalNumPassengers > 0){
				//printf("%s waiting Car[%d] \n", 
				//	currentThread->getName(), index);
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
			
			for(int i = 0; i < numValets; i++){	valetStatusLock[i]->Acquire(); } // acquire the locks around the valetStatusCV
				
			waitOnValetLock->Acquire(); // acquire lock that allows us to wait for a valet
			tokenReturnStatus[index] = WAITING_ON_VALET_TO_RETURN_TOKEN; // update to let valets know a driver is waiting
						
			// Wake up all the sleeping valets			
			for(int i = 0; i < numValets; i++){	
				valetStatusCV[i]->Signal(valetStatusLock[i]); // wake up all the sleeping valets
				//printf("%s Signalled valets %d \n", currentThread->getName(), i);	
				valetStatusLock[i]->Release(); // release the alert locks now that we have made our presence known
			}
			//printf("%s Signalled valets\n", currentThread->getName());
				
			// Wait for a signal from the next available valet
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock); 
			//printf("%s Waiting on valet\n", currentThread->getName());

			valetIndex = tokenReturnStatus[index]; // valet should update the status register with his index
			
			// Hand over the token
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = index; // update to give valet the "token" 
			printf("%s has given Parking Token[%d] to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
		
			// Signal the valet waiting on us now that we are ready
			waitOnTokenReturnCV[index]->Signal(waitOnValetLock); 
			//printf("%s Waiting \n", currentThread->getName());
			// Wait for the valet to return a parking token
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock);
			//printf("%s Received signal \n", currentThread->getName());
			// Consume the keys
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = TIP; // tip the valet
			printf("%s has given a tip to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);			
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnTokenReturnCV[index]->Signal(waitOnValetLock); 
			printf("%s Waiting \n", currentThread->getName());
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock); // wait to receive keys
			printf("%s Signalled by valets \n", currentThread->getName());
			
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = EMPTY; // leave the register empty
			printf("%s has received their keys from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnValetLock->Release(); // release the lock and allow other drivers to communicate with the valets
			valetTokenReturnLineLock->Release(); // allow the next driver to return their token

			status = TELL_PASSENGERS_TO_ENTER_CAR;		
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
	if(onlyTickets) { status = IN_MUSEUM; }
	int valetIndex = 0, numLimosWaiting = 0;
	int lineIndex = 0, visitDuration = 0;
	int numPassengers = 0; 
	int key = index, token = 0;
	
	int totalNumPassengers = totalPassengers[index];
	
	for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield to allow passengers to set up
	
	while(true){
		if(status == WAITING_TO_PARK){ // waiting in line for the valet			
			// Wait in the line of cars to be parked
			
			numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
			numCarsWaitingToPark++; // update to add ourselves to line
			numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark
			
			if(onlyPark){
				printf("%s has arrived at the museum and is waiting to park. \n", 
						currentThread->getName());
			}
			
			valetCarLineLock->Acquire(); // when acquired you are at the front of the line
			status = WAITING_FOR_LIMOS;
		}
		else if(status == WAITING_FOR_LIMOS){
			numLimosWaitingToParkLock->Acquire();
			numLimosWaitingToParkCV->Wait(numLimosWaitingToParkLock);
			
			if(numLimosWaitingToPark == 0){
				status = SEARCHING_FOR_VALET;
			}
			else {
				numLimosWaitingToParkLock->Release();
			}
		}
		else if(status == SEARCHING_FOR_VALET){ // looking for an available valet			
			
			for(int i = 0; i < numValets; i++){	
				valetStatusLock[i]->Acquire(); // acquire the locks around the valetStatusCV	
			}		
			waitOnValetLock->Acquire(); // acquire lock that allows us to wait for a valet
			numLimosWaitingToParkLock->Release();
			
			carWaitOnValetStatus[index] = WAITING_ON_VALET_TO_PARK; // update to let valets know a car is waiting
						
			// Wake up all the sleeping valets			
			for(int i = 0; i < numValets; i++){	
				valetStatusCV[i]->Signal(valetStatusLock[i]); // wake up all the sleeping valets
				valetStatusLock[i]->Release(); // release the alert locks now that we have made our presence known
			}
			//printf("%s Signalled valets \n", currentThread->getName());	
			// Wait for a signal from the next available valet
			carWaitOnValetCV[index]->Wait(waitOnValetLock); 
			//printf("%s Waiting on valet\n", currentThread->getName());
	
			waitOnValetLock->Release(); // TODO
			//printf("%s Received signal \n", currentThread->getName());
			
			status = WAITING_TO_TELL_PASSENGERS_TO_EXIT; // valet is ready and waiting for us now
		}
		else if(status == WAITING_TO_TELL_PASSENGERS_TO_EXIT){ // waiting for passengers to be ready for our signal
			if(totalNumPassengers > 0){ // our car was initialized with passengers
				driverPassengerLock[index]->Acquire(); // acquire the lock that allows us to communicate with the passengers
				printf("%s has told his visitors to leave Car[%d] \n", 
						currentThread->getName(), index);
				driverPassengerCV[index]->Broadcast(driverPassengerLock[index]); // Signal all the passengers to have them exit the car
	
				status = WAITING_FOR_PASSENGERS_TO_EXIT;
			}
			else { // we are not carrying any passengers
				status = PARKING_CAR; // so we should not wait for them to exit
			}
		}
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT){ // waiting for passengers to leave car
			driverPassengerCV[index]->Wait(driverPassengerLock[index]); // wait for passengers to all be out of the car
			status = PARKING_CAR;
		}
		else if(status == PARKING_CAR){ // interacting with valet to park car and exchange keys and token
			printf("%s has parked Car[%d] at the Museum \n", 
					currentThread->getName(), index);
					
			waitOnValetLock->Acquire(); // acquire lock that allows us to communicate with valets
			valetIndex = carWaitOnValetStatus[index]; // valet should update the status register with his index
			
			// Hand over the keys
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = key; // update to give valet the "keys" 
			printf("%s has given their keys to Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
		
			// Signal the valet waiting on us now that we are ready
			carWaitOnValetCV[index]->Signal(waitOnValetLock); 
			
			// Wait for the valet to return a parking token
			carWaitOnValetCV[index]->Wait(waitOnValetLock);
			//printf("%s Received signal \n", currentThread->getName());
			// Consume the parking token 
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			token = valetExchange[valetIndex]; // consume the parking token
			valetExchange[valetIndex] = EMPTY; // leave the register empty
			printf("%s has received Parking Token[%d] from Parking Valet[%d] for Car[%d] \n", 
					currentThread->getName(), index, valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnValetLock->Release(); // release the lock and allow other drivers to communicate with the valets
			valetCarLineLock->Release(); // allow the next driver in line to interact with the valets TODO
			
			if(!onlyPark){
				status = IN_MUSEUM; // visitors and driver are inside the museum now
			}
			else {
				status = WAITING_IN_TOKEN_LINE; // we are only testing parking, so quit
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
			if(!onlyTickets){ status = WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM; }
			else { status = QUIT; }
		}
		else if(status == WAITING_FOR_PASSENGERS_TO_EXIT_MUSEUM){ // waiting for all passengers to exit museum		
			if(totalNumPassengers > 0){
				//printf("%s waiting Car[%d] \n", 
				//	currentThread->getName(), index);
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
			for(int i = 0; i < numValets; i++){	valetStatusLock[i]->Acquire(); } // acquire the locks around the valetStatusCV
				
			waitOnValetLock->Acquire(); // acquire lock that allows us to wait for a valet
			tokenReturnStatus[index] = WAITING_ON_VALET_TO_RETURN_TOKEN; // update to let valets know a driver is waiting
						
			// Wake up all the sleeping valets			
			for(int i = 0; i < numValets; i++){	
				valetStatusCV[i]->Signal(valetStatusLock[i]); // wake up all the sleeping valets
				//printf("%s Signalled valets %d \n", currentThread->getName(), i);	
				valetStatusLock[i]->Release(); // release the alert locks now that we have made our presence known
			}
			//printf("%s Signalled valets\n", currentThread->getName());
				
			// Wait for a signal from the next available valet
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock); 
			//printf("%s Waiting on valet\n", currentThread->getName());

			valetIndex = tokenReturnStatus[index]; // valet should update the status register with his index
			
			// Hand over the token
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = index; // update to give valet the "token" 
			printf("%s has given Parking Token[%d] to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), index, valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
		
			// Signal the valet waiting on us now that we are ready
			waitOnTokenReturnCV[index]->Signal(waitOnValetLock); 
			//printf("%s Waiting \n", currentThread->getName());
			// Wait for the valet to return a parking token
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock);
			//printf("%s Received signal \n", currentThread->getName());
			// Consume the keys
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = TIP; // tip the valet
			printf("%s has given a tip to Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);			
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnTokenReturnCV[index]->Signal(waitOnValetLock); 
			//printf("%s Waiting \n", currentThread->getName());
			waitOnTokenReturnCV[index]->Wait(waitOnValetLock); // wait to receive keys
			//printf("%s Signalled by valets \n", currentThread->getName());
			
			valetExchangeLock[valetIndex]->Acquire(); // acquire the lock on the key/token/tip exchange
			valetExchange[valetIndex] = EMPTY; // leave the register empty
			printf("%s has received their keys from Parking Valet[%d] for Car[%d] \n", 
						currentThread->getName(), valetIndex, index);
			valetExchangeLock[valetIndex]->Release(); // release the lock on the key/token/tip exchange
			
			waitOnValetLock->Release(); // release the lock and allow other drivers to communicate with the valets
			valetTokenReturnLineLock->Release(); // allow the next driver to return their token

			status = TELL_PASSENGERS_TO_ENTER_CAR;		
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
// Valet function
// --------------------------------------------------
void Valet(int index) {
	// Data
	int numCarsWaiting = 0, numLimosWaiting = 0;
	int parkingDuration = 0, returningCarDuration = 0;
	bool sleepingOnBench = false;
	int carIndex = 0;                              
	
	// Read our updated status   
	valetStatusLock[index]->Acquire(); // acquire the lock around valetStatus[]
	
	while(true){	
		if(valetStatusRegister[index] == IN_BACK_ROOM){ // if valet is in the back room
			valetStatusCV[index]->Wait(valetStatusLock[index]); // wait on a signal to be brought back to the bench
			printf("%s is coming out of the back room \n",
					currentThread->getName());
						
			valetStatusRegister[index] = ON_BENCH; // update status
		}
		else if(valetStatusRegister[index] == ON_BENCH){	
			numLimosWaitingToParkLock->Acquire();
			numLimosWaitingToParkCV->Broadcast(numLimosWaitingToParkLock);
			numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
				
			// Wait to be signaled by either a driver or the manager
			//printf("%s Waiting on driver\n", currentThread->getName());

			valetStatusCV[index]->Wait(valetStatusLock[index]); 

			//printf("%s Woken up \n", currentThread->getName());
			if(sleepingOnBench){ 
				printf("%s has been woken up from the bench \n",
						currentThread->getName());
				sleepingOnBench = false;
			}
			
			// Check to see valet manager wants us to go to the back room
			if(valetStatusRegister[index] == IN_BACK_ROOM){
				printf("%s is going to the back room \n",
					currentThread->getName());
				valetStatusLock[index]->Release(); // release the lock on valetStatusCV[]
				continue; // skip the rest of the loop and go straight to the back room
			}
			else { // check to see if any cars need to be parked
				//printf("%s attempting to acquire lock \n", currentThread->getName());
				// Determine number of limos waiting to be parked
				numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
				numLimosWaiting = numLimosWaitingToPark; // update our copy of the variable
				numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
				//printf("%s limos %d \n", currentThread->getName(), numLimosWaiting);
				// Determine number of cars waiting to be parked
				numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
				numCarsWaiting = numCarsWaitingToPark; // update our copy of the variable
				numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark
				
				//printf("%s cars %d \n", currentThread->getName(), numCarsWaiting);
				
				if(numLimosWaiting > 0){ // there are limos waiting to be parked
					waitOnValetLock->Acquire(); // TODO
					for(int i = 0; i < numCars; i++){
						if(limoWaitOnValetStatus[i] ==  WAITING_ON_VALET_TO_PARK){ // there's a limo waiting to park
							carIndex = i;
							break;
						}
					}
					if(limoWaitOnValetStatus[carIndex] ==  WAITING_ON_VALET_TO_PARK){ // at least one car is waiting
						
						numLimosWaitingToParkLock->Acquire(); // acquire the lock around global int numLimosWaitingToPark
						numLimosWaitingToPark--; // update to remove the parked limo
						numLimosWaitingToParkLock->Release(); // release the lock around global int numLimosWaitingToPark
			
						
						limoWaitOnValetStatus[carIndex] = index; // give the limo driver our index 
						limoWaitOnValetCV[carIndex]->Signal(waitOnValetLock); // signal the limo to tell them we're ready to help
						//printf("%s Waiting on driver[%d] for keys\n", currentThread->getName(), carIndex);
						limoWaitOnValetCV[carIndex]->Wait(waitOnValetLock); // wait for limo to be ready to hand over keys
						
						// Consume the keys and return a parking token
						valetExchangeLock[index]->Acquire(); 
						if(valetExchange[index] == carIndex){ // keys match the driver we're helping
							printf("%s has received the keys from Limousine Driver[%d] for Car[%d] \n",
									currentThread->getName(), carIndex, carIndex);
							valetExchange[index] = index; // update to give driver the "token" 
							printf("%s has given Limousine Driver[%d] Parking Token[%d] for Car[%d] \n",
									currentThread->getName(), carIndex, carIndex, carIndex);		
							limoWaitOnValetCV[carIndex]->Signal(waitOnValetLock); // signal the limo to give them the token
							waitOnValetLock->Release();
						}
						else { // keys don't match the driver we're helping, a big problem
							fprintf(stderr, "%s has received the incorrect keys[%d] from Limousine Driver[%d] for Car[%d] \n",
									currentThread->getName(), valetExchange[index], carIndex, carIndex);
						}
						valetExchangeLock[index]->Release(); 
						
						valetStatusRegister[index] = IS_PARKING_CAR;
					}
				}	
				else if(numCarsWaiting > 0){ // there are no limos, but some cars waiting to be parked
					waitOnValetLock->Acquire(); // TODO
					//printf("%s acquired car waiting lock \n",
					//		currentThread->getName());		
					for(int i = 0; i < numCars; i++){
						if(carWaitOnValetStatus[i] ==  WAITING_ON_VALET_TO_PARK){ // there's a car waiting to park
							carIndex = i;
							break;
						}
					}
					if(carWaitOnValetStatus[carIndex] ==  WAITING_ON_VALET_TO_PARK){ // at least one car is waiting
						carWaitOnValetStatus[carIndex] = index; // give the car driver our index
						
						//printf("%s acquiring car[%d] waiting lock \n",
						//	currentThread->getName(), carIndex);
						numCarsWaitingToParkLock->Acquire(); // acquire the lock around global int numCarsWaitingToPark
						numCarsWaitingToPark--; // update to remove the parked car
						numCarsWaitingToParkLock->Release(); // release the lock around global int numCarsWaitingToPark	
						
						carWaitOnValetCV[carIndex]->Signal(waitOnValetLock); // signal the car to tell them we're ready to help
						//printf("%s signalled car[%d] \n",
						//	currentThread->getName(), carIndex);
						carWaitOnValetCV[carIndex]->Wait(waitOnValetLock); // wait for car to be ready to hand over keys
						
						// Consume the keys and return a parking token
						valetExchangeLock[index]->Acquire(); 
						if(valetExchange[index] == carIndex){ // keys match the driver we're helping
							printf("%s has received the keys from Car Driver[%d] for Car[%d] \n",
									currentThread->getName(), carIndex, carIndex);
							valetExchange[index] = index; // update to give driver the "token" 
							printf("%s has given Car Driver[%d] Parking Token[%d] for Car[%d] \n",
									currentThread->getName(), carIndex, carIndex, carIndex);		
							carWaitOnValetCV[carIndex]->Signal(waitOnValetLock); // signal the car to give them the token
							waitOnValetLock->Release();
						}
						else { // keys don't match the driver we're helping, a big problem
							fprintf(stderr, "%s has received the incorrect keys[%d] from Car Driver[%d] for Car[%d] \n",
									currentThread->getName(), valetExchangeLock[index], carIndex, carIndex);
						}
						valetExchangeLock[index]->Release(); 
						
						valetStatusRegister[index] = IS_PARKING_CAR;
					}
				}
				else { // if there are no vehicles waiting, check to see if any tokens are being offered
					waitOnValetLock->Acquire(); // TODO
					//printf("%s acquired car waiting lock \n",
					//		currentThread->getName());		
					for(int i = 0; i < numCars; i++){
						if(tokenReturnStatus[i] ==   WAITING_ON_VALET_TO_RETURN_TOKEN){ // there's a car waiting to return a token
							carIndex = i;
							break;
						}
					}
					if(tokenReturnStatus[carIndex] ==   WAITING_ON_VALET_TO_RETURN_TOKEN){ // at least one car is waiting
						tokenReturnStatus[carIndex] = index; // give the car driver our index
				
						waitOnTokenReturnCV[carIndex]->Signal(waitOnValetLock); // signal the car to tell them we're ready to help
						//printf("%s Waiting for token \n", currentThread->getName());
						waitOnTokenReturnCV[carIndex]->Wait(waitOnValetLock); // wait for the token
		
						// Signal the valet waiting on us now that we are ready
						waitOnTokenReturnCV[carIndex]->Signal(waitOnValetLock); 
						//printf("%s Waiting for tip\n", currentThread->getName());
				
						waitOnTokenReturnCV[carIndex]->Wait(waitOnValetLock); // wait for the tip
						//printf("%s got tip\n", currentThread->getName());
						// Consume the tip
						valetExchangeLock[index]->Acquire(); // acquire the lock on the key/token/tip exchange
						if(valetExchange[index] != TIP){ // tip the valet	
							printf("%s was not tipped by Driver[%d] \n", currentThread->getName(), carIndex);
						}
						else {
							waitOnTokenReturnCV[carIndex]->Signal(waitOnValetLock); 
							valetExchange[index] = carIndex; // give them the keys
						}	
						valetExchangeLock[index]->Release(); // release the lock on the key/token/tip exchange
					
						waitOnValetLock->Release(); // TODO
					}
					else if(!sleepingOnBench){
						printf("%s is going to sleep on the bench \n",
								currentThread->getName());
						sleepingOnBench = true; 
					}
				}
			}
		}
		else if(valetStatusRegister[index] == IS_PARKING_CAR){
			// Alert the valet manager that there are cars being parked
			valetManagerAlertLock->Acquire(); // acquire the lock in valetManagerAlertCV
			valetManagerAlertCV->Signal(valetManagerAlertLock); // signal to wake up the valet manager
			valetManagerAlertLock->Release(); // release the lock in valetManagerAlertCV
			
			// Parking the car should appear to take a random amount of time
			printf("%s is parking Car[%d] \n",
					currentThread->getName(), carIndex);
			parkingDuration = (rand() % MAX_PARKING_DURATION) + MIN_PARKING_DURATION;
			for(int i = 0; i < parkingDuration; i++){ currentThread->Yield(); }

			valetStatusRegister[index] = ON_BENCH;
								
		}
		//printf("%s looping \n", currentThread->getName());
	}
	valetStatusLock[index]->Release(); // release the lock on valetStatusCV[]
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
		numTotalVehiclesWaiting = 0;
		numCarsWaitingToParkLock->Acquire(); // acquire lock around global int numCarsWaitingToPark 
		numTotalVehiclesWaiting += numCarsWaitingToPark; // read globals to determine number of vehicles waiting
		numCarsWaitingToParkLock->Release(); // release lock around global int numCarsWaitingToPark
		 
		numLimosWaitingToParkLock->Acquire(); // acquire lock around global int numLimosWaitingToPark	
		numTotalVehiclesWaiting += numLimosWaitingToPark; // read globals to determine number of vehicles waiting	
		numLimosWaitingToParkLock->Release(); // release lock around global int numLimosWaitingToPark
									  
		if(numTotalVehiclesWaiting >= MIN_NUM_VEHICLES_WAITING){ // if 4 or more cars waiting
			printf("%s has detected four (or more) cars waiting to be parked \n",
					currentThread->getName());
					
			for(int i = 0; i < numValets; i++){	valetStatusLock[i]->Acquire(); } // acquire the lock in valetStatusCV[]
			
			for(int i = 0; i < numValets; i++){ 
				if(valetStatusRegister[i] == IN_BACK_ROOM){ // if valet is in the back room
					backRoomValet = i; // record position of first valet available in back room
					break;
				}
			}
			if(valetStatusRegister[backRoomValet] == IN_BACK_ROOM){ // if at least one valet is in the back room
				valetStatusRegister[backRoomValet] = ON_BENCH; // change state of valet
				valetStatusCV[index]->Signal(valetStatusLock[index]);
				printf("%s has told Parking Valet[%d] to come out of the back room \n", 
						currentThread->getName(), backRoomValet);
			}
			for(int i = 0; i < numValets; i++){ valetStatusLock[i]->Release(); } // release the lock in valetStatusCV[]
		}	
		
		for(int i = 0; i < 50; i++){ currentThread->Yield(); } // yield so we don't check too often
	
		// Check to see if more than 2 valets are sitting on bench	
		for(int i = 0; i < numValets; i++){	valetStatusLock[i]->Acquire(); } // acquire the lock in valetStatusCV[]
		
		for(int i = 0; i < numValets; i++){ 
			if(valetStatusRegister[i] == ON_BENCH){ // if valet is sitting on bench
				valetsOnBench++; // increment number of benched valets
				benchValet = i; // record position of last benched valet
			}
		}
		if(valetsOnBench >= MAX_NUM_VALETS_ON_BENCH){
			printf("%s has detected two (or more) Parking Valets on the bench \n",
					currentThread->getName());
			if(valetStatusRegister[benchValet] == ON_BENCH){ // at least three valets are sitting on bench
				valetStatusRegister[benchValet] = IN_BACK_ROOM; // change state of valet
				printf("%s has sent Parking Valet[%d] to the back room \n", 
						currentThread->getName(), benchValet);
				
				// Let valet know we are sending him to the back room		
				valetStatusCV[benchValet]->Signal(valetStatusLock[benchValet]);	
			}
		}		
		for(int i = 0; i < numValets; i++){ valetStatusLock[i]->Release(); } // release the lock in valetStatusCV[]
		
		for(int i = 0; i < YIELD_DURATION; i++){ currentThread->Yield(); } // yield so we don't check too often
		
		valetManagerAlertLock->Acquire(); // acquire the lock in valetManagerAlertCV
		valetManagerAlertCV->Wait(valetManagerAlertLock); // wait for a Valet to signal saying there are still cars to be parked
		valetManagerAlertLock->Release(); // release the lock in valetManagerAlertCV

		//printf("%s looping \n", currentThread->getName());
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
