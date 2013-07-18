// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <sstream>


//----------------------------------------------------------------------
// ThreadID
// Struct that holds data on the machine and thread ID 
//----------------------------------------------------------------------
typedef struct ThreadID {
	int machineID, threadID;
	bool equals(int n) const { return ((machineID == n) && (threadID == n)); }
	bool equals(int m, int t) const { return ((machineID == m) && (threadID == t)); }
	void setIDs(int n) { machineID = n; threadID = n; }
	void setIDs(int m, int t) { machineID = m; threadID = t; }
};

enum ServerLockState { AVAILABLE, BUSY };

//----------------------------------------------------------------------
// ServerLock
// Struct that acts as a lock
//----------------------------------------------------------------------
typedef struct ServerLock {
	char* name;
	ServerLockState state;
	ThreadID* owner;
	List* queue;
	bool isToBeDeleted, isDeleted;
};

void constructServerLock(int index, char* buf);
int findLockByName(char* name);

//----------------------------------------------------------------------
// ServerCondition
// Struct that acts as a condition variable
//----------------------------------------------------------------------
typedef struct ServerCondition {
	char* name;
	int lockIndex;
	List* queue;
	bool isToBeDeleted, isDeleted;
};

void constructServerCondition(int index, char* buf);
int findConditionByName(char* name);

//----------------------------------------------------------------------
// MonitorVariable
// Struct that acts as a monitor variable
//----------------------------------------------------------------------
typedef struct MonitorVariable {
	char* name;
	int* array, arraySize;
	bool isToBeDeleted, isDeleted;
};

void constructMonitorVariable(int index, int arraySize, char* buf);
int findMonitorVariableByName(char* name);

//----------------------------------------------------------------------
// Server data
//----------------------------------------------------------------------
#define MAX_SERVERLOCKS 		500
#define MAX_SERVERCONDITIONS 	500
#define MAX_MONITORVARIABLES 	100
#define MAILBOX					0
#define NO_OWNER				-1
#define ERROR_CODE				-1
#define SUCCESS_CODE			0

ServerLock* serverLockTable;
ServerCondition* serverConditionTable;
MonitorVariable* monitorVariableTable;

int numLocks = 0, numConditions = 0, numMonitorVariables = 0;

Message* decodeMessage(char* buf);
bool sendMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);
bool sendErrorMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);
bool sendSuccessMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);

//----------------------------------------------------------------------
// Server function
//----------------------------------------------------------------------
void Server(){
	// Initialize tables of variables
	serverLockTable = new ServerLock[MAX_SERVERLOCKS];
	serverConditionTable = new ServerCondition[MAX_SERVERCONDITIONS];
	monitorVariableTable = new MonitorVariable[MAX_MONITORVARIABLES];
	
	bool success = FALSE;

	while(true){
		PacketHeader outPktHdr, inPktHdr;
    	MailHeader outMailHdr, inMailHdr;
    	char* buffer = new char[MaxMailSize];
    	Message* messageOut = new Message;
    	
    	// Wait to receive a message
		postOffice->Receive(MAILBOX, &inPktHdr, &inMailHdr, buffer);
		printf("Server has received a message: %s\n", buffer);
		
		// Decode the message
		Message* messageIn = decodeMessage(buffer);	
		outPktHdr.to = inPktHdr.from;
    	outMailHdr.to = inMailHdr.from; 
		
		// Decode syscall
		if(strstr(messageIn->request, "HT")){ /* Halt */ }	
		else if(strstr(messageIn->request, "EX")){ /* Exit */ }
		else if(strstr(messageIn->request, "EC")){ /* Exec */ }	
		else if(strstr(messageIn->request, "JO")){ /* Join */ }
		else if(strstr(messageIn->request, "CR")){ /* Create */ }
		else if(strstr(messageIn->request, "OP")){ /* Open */ }
		else if(strstr(messageIn->request, "RD")){ /* Read */ }
		else if(strstr(messageIn->request, "WR")){ /* Write */ }
		else if(strstr(messageIn->request, "CS")){ /* Close */ }
		else if(strstr(messageIn->request, "FK")){ /* Fork */ }
		else if(strstr(messageIn->request, "YD")){ /* Yield */ }
		// ACQUIRE
		else if(strstr(messageIn->request, "AQ")){
			printf("Server has decoded Acquire request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// Invalid index passed
			if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0) || (messageIn->index >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			// Lock already deleted
			else if((serverLockTable[messageIn->index].isDeleted) || (serverLockTable[messageIn->index].isToBeDeleted)){	
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// No one is waiting on the lock, so the thread can acquire it
				if((serverLockTable[messageIn->index].queue->IsEmpty()) && (serverLockTable[messageIn->index].owner->equals(NO_OWNER))){
					serverLockTable[messageIn->index].owner->setIDs(inPktHdr.from, inMailHdr.from);
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    			}
    			// Lock already owned by that thread
    			else if(serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from)){
    				printf("Server has received request to reacquire lock already owned by machine%d mailbox%d \n", 
							inPktHdr.from, inMailHdr.from);
    				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
    			}
    			// someone else is waiting on or owns the lock, so thread must wait
    			else {
    				ThreadID* id = new ThreadID;
    				id->setIDs(inPktHdr.from, inMailHdr.from);
    				serverLockTable[messageIn->index].queue->Append((void*)id);
    			}
    		}
		}
		// RELEASE 
		else if(strstr(messageIn->request, "RL")){
			printf("Server has decoded Release request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Invalid index passed
			if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0) || (messageIn->index >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			// Lock already deleted
			else if(serverLockTable[messageIn->index].isDeleted){
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// Lock is owned by thread releasing
				if(serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from)){
					serverLockTable[messageIn->index].owner->setIDs(NO_OWNER);
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				
    				// Other threads waiting on the lock, so give it to the thread waiting longest
    				if(!(serverLockTable[messageIn->index].queue->IsEmpty())){
    					ThreadID* id = (ThreadID*)serverLockTable[messageIn->index].queue->Remove();
    					serverLockTable[messageIn->index].owner = id;
    					outPktHdr.to = id->machineID;
    					outMailHdr.to = id->threadID;
    					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
    				// No one is waiting on the lock, and it should be destroyed
    				else if(serverLockTable[messageIn->index].isToBeDeleted){
    					serverLockTable[messageIn->index].isDeleted = TRUE;
    				}
    			}
    			// Lock is not owned by thread releasing
    			else {
    				printf("Server has received request to release lock at index %d not owned by machine%d mailbox%d \n", 
						messageIn->index, inPktHdr.from, inMailHdr.from);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);	
    			}
    		}
		}
		// WAIT
		else if(strstr(messageIn->request, "WT")){
			printf("Server has decoded Wait request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0) || (messageIn->index >= numConditions)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Invalid lock index passed
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0) || (messageIn->index2 >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Lock already deleted
			else if(serverLockTable[messageIn->index2].isDeleted){
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// Lock does not belong to thread, so shouldn't allow thread to wait
				if(!(serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from))){
					printf("Server has received request to wait on condition at index %d, but lock not owned by machine%d mailbox%d \n", 
							messageIn->index, inPktHdr.from, inMailHdr.from);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
    			}
    			// Lock belongs to thread asking to wait, so let them wait on it
    			else {
    				ThreadID* id = new ThreadID;
    				id->setIDs(serverLockTable[messageIn->index2].owner->machineID, serverLockTable[messageIn->index2].owner->threadID);
    				serverLockTable[messageIn->index2].owner->setIDs(NO_OWNER);
    				serverConditionTable[messageIn->index].queue->Append((void*)id);
    				
    				// Other threads waiting on the lock, so give it to the thread waiting longest
    				if(!(serverLockTable[messageIn->index2].queue->IsEmpty())){
    					id = (ThreadID*)serverLockTable[messageIn->index2].queue->Remove();
    					serverLockTable[messageIn->index2].owner = id;
    					outPktHdr.to = id->machineID;
    					outMailHdr.to = id->threadID;
    					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
    			}
    		}
		}
		// SIGNAL
		else if(strstr(messageIn->request, "SG")){
			printf("Server has decoded Signal request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0) || (messageIn->index >= numConditions)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Invalid lock index passed
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0) || (messageIn->index2 >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Lock already deleted
			else if(serverLockTable[messageIn->index2].isDeleted){
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// Lock does not belong to thread, so shouldn't allow thread to signal
				if(!(serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from))){
					printf("Server has received request to signal on condition at index %d, but lock not owned by machine%d mailbox%d \n", 
							messageIn->index, inPktHdr.from, inMailHdr.from);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
    			}
    			// Lock belongs to thread asking to signal, so let them signal on it
    			else {
    				ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove(); // remove thread from top of CV wait queue
    				serverLockTable[messageIn->index2].queue->Append((void*)id); // append thread to back of lock wait queue
    				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    			}
    		}
		}
		// BROADCAST
		else if(strstr(messageIn->request, "BC")){
			printf("Server has decoded Broadcast request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0) || (messageIn->index >= numConditions)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Invalid lock index passed
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0) || (messageIn->index2 >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Lock already deleted
			else if(serverLockTable[messageIn->index2].isDeleted){
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// Lock does not belong to thread, so shouldn't allow thread to broadcast
				if(!(serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from))){
					printf("Server has received request to broadcast on condition at index %d, but lock not owned by machine%d mailbox%d \n", 
							messageIn->index, inPktHdr.from, inMailHdr.from);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
    			}
    			// Lock belongs to thread asking to broadcast, so let them broadcast on it
    			else {
    				while(!(serverConditionTable[messageIn->index].queue->IsEmpty())){
	    				ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove(); // remove thread from top of CV wait queue
    					serverLockTable[messageIn->index2].queue->Append((void*)id); // append thread to back of lock wait queue
    				}	
    				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    			}
    		}
		}
		// CREATE LOCK
		else if(strstr(messageIn->request, "CL")){
			printf("Server has decoded CreateLock request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// No more room for locks		
			if(numLocks >= MAX_SERVERLOCKS){
				printf("Server cannot create a lock for machine%d mailbox%d because the max number of locks has been reached \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				int index = findLockByName(messageIn->name);
				// Lock with that name not yet created
				if(index == -1){
					constructServerLock(numLocks, messageIn->name);
					messageOut->index = numLocks;
					numLocks++;
				}
				// Lock already created
				else {
					messageOut->index = index;
				}
				success = sendMessage(outPktHdr, outMailHdr, messageOut);
			} 
		}
		// DESTROY LOCK
		else if(strstr(messageIn->request, "DL")){
			printf("Server has decoded DestroyLock request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
	
			// Invalid index passed
			if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0) || (messageIn->index >= numLocks)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				printf("Server has received valid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);				
				// No one is waiting on the lock, so destroy it
				if(serverLockTable[messageIn->index].queue->IsEmpty() && serverLockTable[messageIn->index].owner->equals(NO_OWNER)){
					serverLockTable[messageIn->index].isToBeDeleted = TRUE;
					serverLockTable[messageIn->index].isDeleted = TRUE;
				}
				// At least one thread is waiting on the lock, so can't destroy it
				else {
					serverLockTable[messageIn->index].isToBeDeleted = TRUE;
				}
				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    		} 
		}
		// CREATE CONDITION
		else if(strstr(messageIn->request, "CC")){
			printf("Server has decoded CreateCondition request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Max conditions reached
			if(numConditions >= MAX_SERVERCONDITIONS){
				printf("Server cannot create a conditon for machine%d mailbox%d because the max number of conditions has been reached \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				int index = findConditionByName(messageIn->name);
				// Condition with that name not yet created
				if(index == -1){ 
					constructServerCondition(numConditions, messageIn->name);
					messageOut->index = numConditions;
					numConditions++;
				}
				// Condition already created
				else {
					messageOut->index = index;
				}
				success = sendMessage(outPktHdr, outMailHdr, messageOut);
			}
		}
		// DESTROY CONDITION
		else if(strstr(messageIn->request, "DC")){
			printf("Server has decoded DestroyCondition request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// Invalid index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0) || (messageIn->index >= numConditions)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				// No thread is waiting on the Condition, so destroy it
				if(serverConditionTable[messageIn->index].queue->IsEmpty()){
					serverConditionTable[messageIn->index].isToBeDeleted = TRUE;
					serverConditionTable[messageIn->index].isDeleted = TRUE;
				}
				// At least one thread is waiting on the Condition, so we can't destroy it
				else {
					serverConditionTable[messageIn->index].isToBeDeleted = TRUE;
				}
				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    		} 
		}
		// CREATE MV
		else if(strstr(messageIn->request, "CM")){
			printf("Server has decoded CreateMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Max MVs reached
			if(numMonitorVariables >= MAX_MONITORVARIABLES){
				printf("Server cannot create an MV for machine%d mailbox%d because the max number of MVs has been reached \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			// Invalid array size (must be greater than 0)
			else if(messageIn->index2 < 1){
				printf("Server has received invalid array size from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				int index = findMonitorVariableByName(messageIn->name);
				// MV with that name not yet created
				if(index == -1){
					constructMonitorVariable(numMonitorVariables, messageIn->index2, messageIn->name);
					messageOut->index = numMonitorVariables;
					numMonitorVariables++;
					success = sendMessage(outPktHdr, outMailHdr, messageOut);
				}
				// MV already created
				else { 
					messageOut->index = index;
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
				}
				
			}
		}
		// DESTROY MV
		else if(strstr(messageIn->request, "DM")){
			printf("Server has decoded DestroyMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// Invalid index passed
			if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0) || (messageIn->index >= numMonitorVariables)){
				printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				monitorVariableTable[messageIn->index].isToBeDeleted = TRUE;
				monitorVariableTable[messageIn->index].isDeleted = TRUE;
				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    		} 
		}
		// GET MV
		else if(strstr(messageIn->request, "GM")){
			printf("Server has decoded GetMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// Invalid index passed
			if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0) || (messageIn->index >= numMonitorVariables)){
				printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			// Invalid array index passed
			else if((messageIn->index2 < 0) || (messageIn->index2 >= monitorVariableTable[messageIn->index].arraySize)){
				printf("Server has received invalid MV array index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				// Allow thread to access MV
				messageOut->index = monitorVariableTable[messageIn->index].array[messageIn->index2];
				success = sendMessage(outPktHdr, outMailHdr, messageOut);
    		} 
		}
		// SET MV
		else if(strstr(messageIn->request, "SM")){
			printf("Server has decoded SetMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			// Invalid index passed
			if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0) || (messageIn->index >= numMonitorVariables)){
				printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			// Invalid array index passed
			else if((messageIn->index2 < 0) || (messageIn->index2 >= monitorVariableTable[messageIn->index].arraySize)){
				printf("Server has received invalid MV array index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				// Allow thread to access MV
				monitorVariableTable[messageIn->index].array[messageIn->index2] = messageIn->index3;
				success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    		} 
		}
		// ILLEGAL REQUEST
		else {
			printf("Server has received illegal request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
		}
		
		if(messageIn->name){ delete messageIn->name; }
		if(messageIn->request){ delete messageIn->request; }
		delete messageIn;
		/*if(messageOut->name){ delete messageOut->name; }
		if(messageOut->request){ delete messageOut->request; }	*/	
		delete messageOut;

		memset(&buffer[0], 0, sizeof(buffer));
		//delete[] buffer;
		printf("-----------------------------------------------\n");
	}
}

bool sendMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response){
	response->request = new char[2];
	response->request[0] = 'H';
	response->request[1] = 'I';
	response->name = "index";
	response->index2 = SUCCESS_CODE;
	response->index3 = SUCCESS_CODE;
	
	char* messageOut = msgPrepare(response);
	outMailHdr.length = strlen(messageOut) + 1;
    bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut); 
    return success;
}

bool sendErrorMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response){
	response->request = new char[2];
	response->request[0] = 'H';
	response->request[1] = 'I';
	response->name = "error";
	response->index = ERROR_CODE;
	response->index2 = ERROR_CODE;
	response->index3 = ERROR_CODE;
	
	char* messageOut = msgPrepare(response);
	outMailHdr.length = strlen(messageOut) + 1;
	bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut); 
	return success;
}	

bool sendSuccessMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response){
	response->request = new char[2];
	response->request[0] = 'H';
	response->request[1] = 'I';
	response->name = "success";
	response->index = SUCCESS_CODE;
	response->index2 = SUCCESS_CODE;
	response->index3 = SUCCESS_CODE;
	
	char* messageOut = msgPrepare(response);				
	outMailHdr.length = strlen(messageOut) + 1;
    bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
    return success;
}

void constructServerLock(int index, char* buf){
	serverLockTable[index].name = new char[20];
	strcpy(serverLockTable[index].name, buf);
	serverLockTable[index].state = AVAILABLE;
	serverLockTable[index].owner = new ThreadID;
	serverLockTable[index].owner->setIDs(NO_OWNER);
	serverLockTable[index].queue = new List;
	serverLockTable[index].isToBeDeleted = FALSE;
	serverLockTable[index].isDeleted = FALSE;
}

int findLockByName(char* name){
	for(int i = 0; i < numLocks; i++){
		if(!strcmp(serverLockTable[i].name, name)){
			return i;
		}
	}
	return -1;
}

void constructServerCondition(int index, char* buf){
	serverConditionTable[index].name = new char[20];
	strcpy(serverConditionTable[index].name, buf);
	serverConditionTable[index].queue = new List;
	serverConditionTable[index].lockIndex = -1;
	serverConditionTable[index].isToBeDeleted = FALSE;
	serverConditionTable[index].isDeleted = FALSE;
}

int findConditionByName(char* name){
	for(int i = 0; i < numConditions; i++){
		if(!strcmp(serverConditionTable[i].name, name)){
			return i;
		}
	}
	return -1;
}

void constructMonitorVariable(int index, int arraySize, char* buf){
	monitorVariableTable[index].name = new char[20];
	strcpy(monitorVariableTable[index].name, buf);
	monitorVariableTable[index].array = new int[arraySize];
	monitorVariableTable[index].arraySize = arraySize;
	monitorVariableTable[index].isToBeDeleted = FALSE;
	monitorVariableTable[index].isDeleted = FALSE;
}

int findMonitorVariableByName(char* name){
	for(int i = 0; i < numMonitorVariables; i++){
		if(!strcmp(monitorVariableTable[i].name, name)){
			return i;
		}
	}
	return -1;
}
	
// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}
