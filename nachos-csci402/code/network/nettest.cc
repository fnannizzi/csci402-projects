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
	void getIDs() {printf("Machine ID: %i, Thread ID %i\n",machineID,threadID);}
};

//----------------------------------------------------------------------
// InProgressMessage
// Struct that holds a message, plus the number of yes and no replies 
// the MessageHandler has received
//----------------------------------------------------------------------
typedef struct InProgressMessage {
	Message* message;
	int yesReplies, noReplies;
};

void incrementYesReplies(int ptr);
void incrementNoReplies(int ptr);

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
int findLockByName(char* name, int machine);

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
int findConditionByName(char* name, int machine);

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
int findMonitorVariableByName(char* name, int machine);

//----------------------------------------------------------------------
// Server data
//----------------------------------------------------------------------
#define MAX_SERVERLOCKS 		5000
#define MAX_SERVERCONDITIONS 	5000
#define MAX_MONITORVARIABLES 	5000
#define MAILBOX					0
#define MESSAGE_MAILBOX			1
#define NO_OWNER				-1
#define ERROR_CODE				-1
#define SUCCESS_CODE			0


ServerLock* serverLockTable;
ServerCondition* serverConditionTable;
MonitorVariable* monitorVariableTable;

List messageQueue; // holds messages that the messangeHandler has made ready for the server
List inProgressMessages; // holds messages that the messageHandler is currently dealing with

//int IDer = 0;
int messageID = -1;
int numLocks = 0, numConditions = 0, numMonitorVariables = 0;

Message* decodeMessage(char* buf);
bool sendToMessageHandler(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);
bool sendMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);
bool sendErrorMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);
bool sendSuccessMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response);


//----------------------------------------------------------------------
// Message handler function
//----------------------------------------------------------------------
void MessageHandler(){
	while(TRUE){
		PacketHeader outPktHdr, inPktHdr;
    	MailHeader outMailHdr, inMailHdr;
    	char* buffer = new char[MaxMailSize];
    	
    	// Wait to receive a message
		postOffice->Receive(MESSAGE_MAILBOX, &inPktHdr, &inMailHdr, buffer);
		printf("MessageHandler of Server%d has received a message: %s\n", inPktHdr.to, buffer);
		
		// Decode the message
		Message* messageIn = decodeMessage(buffer);
	
		
		if(inPktHdr.from == inPktHdr.to){ // message from my own server
			printf("MessageHandler has received a message from own server\n");
			// Create a new object to store the message while we wait for replies
			InProgressMessage* inProgressMessage = new InProgressMessage;
			inProgressMessage->message = messageIn;
			inProgressMessage->yesReplies = 0;
			inProgressMessage->noReplies = 0;
			inProgressMessages.Append((void*)inProgressMessage);
			
			outPktHdr.from = inPktHdr.from;
			outMailHdr.from = MESSAGE_MAILBOX;
			if (numServers == 1) //Double check this
			{
				InProgressMessage* readyMessage = (InProgressMessage*)inProgressMessages.Remove();
				outMailHdr.from = MESSAGE_MAILBOX;
				outPktHdr.to = inPktHdr.to;
	    			outMailHdr.to = MAILBOX; 
	    			char* messageOut = msgPrepare(readyMessage->message);
				outMailHdr.length = strlen(messageOut) + 1;
				printf("MessageHandler%d is sending a request to its own server: %s \n",
				inPktHdr.to, messageOut);
	    			bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
		
			}
			else 
			{	
			for(int i = 0; i < numServers; i++){
				if(i == inPktHdr.to){ // my own server
					continue; // don't send him another message	
				}
				else { // a different machine
					outPktHdr.to = i;
					outMailHdr.to = MESSAGE_MAILBOX;
					outMailHdr.length = inMailHdr.length;
					printf("MessageHandler%d is sending a request to machine%d mailbox%d\n",
							outPktHdr.from, outPktHdr.to, outMailHdr.to);
    				bool success = postOffice->Send(outPktHdr, outMailHdr, buffer);
    				if(!success){
    					printf("MessageHandler%d failed to send a request to machine%d mailbox%d\n",
								outPktHdr.from, outPktHdr.to, outMailHdr.to);
					}
				}
			}
			}
		}
		else { // message from another message handler		
			// LOCK
			if((strstr(messageIn->request, "CL")) || (strstr(messageIn->request, "DL")) 
			|| (strstr(messageIn->request, "RL")) || (strstr(messageIn->request, "AQ"))){
		 		int index;
		 		if (strstr(messageIn->request, "CL"))
		 		 	index = findLockByName(messageIn->name, inPktHdr.to);
		 		else
		 			index = messageIn->index;
		 		bool check = false;
		 		/*ListElement* current = inProgressMessages.first;
		 		while (current != NULL)
		 		{
		 			InProgressMessage* temp = (InProgressMessage*)current->item;
		 			if (!strcmp(temp->message->name,messageIn->name))
		 			{
		 				printf("IDS: %i vs %i\n",messageIn->ID,temp->message->ID);
		 				if (messageIn->ID < temp->message->ID)
		 				{
		 					printf("OPTION 1\n");
		 					check = true;
		 		 			messageIn->request[0] = 'N';
		 					messageIn->request[1] = 'O';	
		 					break;
		 					
		 				}
		 				else if (messageIn->ID > temp->message->ID)
		 				{
		 					printf("OPTION 2\n");
		 					check = true;
		 					outMailHdr.from = MESSAGE_MAILBOX;
							outPktHdr.to = inPktHdr.to;
	    						outMailHdr.to = MAILBOX; 
	    						char* messageOut = msgPrepare(messageIn);
							outMailHdr.length = strlen(messageOut) + 1;
							printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    						bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
		 		 			messageIn->request[0] = 'Y';
		 					messageIn->request[1] = 'S';	
		 					break;
		 					
		 				}
		 					
		 			}
		 			current = current->next;
		 		}*/
				if (!check)
				{
		 		if((index == -1) && (strstr(messageIn->request, "CL"))){ // lock not on this server
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			printf("Lock %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 		}
		 		else if (serverLockTable[index].name == NULL)
		 		{
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			//printf("Lock %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			printf("Lock is not a CL and not found\n");
		 		}
		 		else {
					printf("Lock %s owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			// Message our server now - we'll message the other thread at the bottom
		 			outMailHdr.from = MESSAGE_MAILBOX;
					outPktHdr.to = inPktHdr.to;
	    			outMailHdr.to = MAILBOX; 
	    			char* messageOut = msgPrepare(messageIn);
					outMailHdr.length = strlen(messageOut) + 1;
					printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    			bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
	    			
	    			// Set up the response for later
	    			messageIn->request[0] = 'Y';
		 			messageIn->request[1] = 'S';		 			
		 		}
		 		}
			}
			// CONDITION
			else if((strstr(messageIn->request, "CC")) || (strstr(messageIn->request, "DC")) || (strstr(messageIn->request, "WT")) 
			|| (strstr(messageIn->request, "SG")) || (strstr(messageIn->request, "BC"))){
				int index;
				if (strstr(messageIn->request, "CC"))
		 		 	index = findConditionByName(messageIn->name, inPktHdr.to);
		 		else
		 			index = messageIn->index;
		 		bool check = false;
		 		/*ListElement* current = inProgressMessages.first;
		 		while (current != NULL)
		 		{
		 			InProgressMessage* temp = (InProgressMessage*)current->item;
		 			if (!strcmp(temp->message->name,messageIn->name))
		 			{
		 				if (messageIn->ID < temp->message->ID)
		 				{
		 					check = true;
		 		 			messageIn->request[0] = 'N';
		 					messageIn->request[1] = 'O';	
		 					break;
		 					
		 				}
		 				else if (messageIn->ID > temp->message->ID)
		 				{
		 					check = true;
		 					outMailHdr.from = MESSAGE_MAILBOX;
							outPktHdr.to = inPktHdr.to;
	    						outMailHdr.to = MAILBOX; 
	    						char* messageOut = msgPrepare(messageIn);
							outMailHdr.length = strlen(messageOut) + 1;
							printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    						bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
		 		 			messageIn->request[0] = 'Y';
		 					messageIn->request[1] = 'S';	
		 					break;
		 					
		 				}
		 					
		 			}
		 			current = current->next;
		 		}*/
		 		if (!check)
		 		{
		 		if((index == -1) && (strstr(messageIn->request, "CC"))){ // CV not on this server
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			printf("CV %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 		}
		 		else if (serverConditionTable[index].name == NULL)
		 		{
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			//printf("Lock %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			printf("CV is not a CC and not found\n");
		 		}
		 		else {
					printf("CV %s owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			// Message our server now - we'll message the other thread at the bottom
		 			outMailHdr.from = MESSAGE_MAILBOX;
					outPktHdr.to = inPktHdr.to;
	    			outMailHdr.to = MAILBOX; 
	    			char* messageOut = msgPrepare(messageIn);
					outMailHdr.length = strlen(messageOut) + 1;
					printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    			bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
	    			
	    			// Set up the response for later
	    			messageIn->request[0] = 'Y';
		 			messageIn->request[1] = 'S';		 			
		 		}
		 		}
			}		
			// MONITOR VARIABLE
			else if((strstr(messageIn->request, "CM")) || (strstr(messageIn->request, "DM"))
			|| (strstr(messageIn->request, "GM")) || (strstr(messageIn->request, "SM"))){
				printf("MV\n");
				int index;
				if (strstr(messageIn->request, "CM"))
		 		 	index = findMonitorVariableByName(messageIn->name, inPktHdr.to);
		 		else
		 			index = messageIn->index;
		 		bool check = false;
		 		/*ListElement* current = inProgressMessages.first;
		 		while (current != NULL)
		 		{
		 			InProgressMessage* temp = (InProgressMessage*)current->item;
		 			if (!strcmp(temp->message->name,messageIn->name))
		 			{
		 				if (messageIn->ID < temp->message->ID)
		 				{
		 					printf("OPTION 1\n");
		 					check = true;
		 		 			messageIn->request[0] = 'N';
		 					messageIn->request[1] = 'O';	
		 					break;
		 					
		 				}
		 				else if (messageIn->ID > temp->message->ID)
		 				{
		 					printf("OPTION 2\n");
		 					check = true;
		 					outMailHdr.from = MESSAGE_MAILBOX;
							outPktHdr.to = inPktHdr.to;
	    						outMailHdr.to = MAILBOX; 
	    						char* messageOut = msgPrepare(messageIn);
							outMailHdr.length = strlen(messageOut) + 1;
							printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    						bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
		 		 			messageIn->request[0] = 'Y';
		 					messageIn->request[1] = 'S';	
		 					break;
		 					
		 				}
		 					
		 			}
		 			current = current->next;
		 		}*/
		 		if (!check)
		 		{
		 		if((index == -1) && (strstr(messageIn->request, "CM"))){ // CV not on this server
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			printf("MV %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 		}
		 		else if (monitorVariableTable[index].name == NULL)
		 		{
		 			messageIn->request[0] = 'N';
		 			messageIn->request[1] = 'O';
		 			//printf("Lock %s not owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			printf("MV is not a CM and not found\n");
		 		}
		 		else {
					printf("MV %s owned by Server%d\n", messageIn->name, inPktHdr.to);
		 			// Message our server now - we'll message the other thread at the bottom
		 			outMailHdr.from = MESSAGE_MAILBOX;
					outPktHdr.to = inPktHdr.to;
	    			outMailHdr.to = MAILBOX; 
	    			char* messageOut = msgPrepare(messageIn);
					outMailHdr.length = strlen(messageOut) + 1;
					printf("MessageHandler%d is sending a request to its own server: %s \n",
							inPktHdr.to, messageOut);
	    			bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
	    			
	    			// Set up the response for later
	    			messageIn->request[0] = 'Y';
		 			messageIn->request[1] = 'S';		 			
		 		}
		 		}
			}
			else if((strstr(messageIn->request, "NO")) || (strstr(messageIn->request, "YS"))){
				printf("REPLY\n");
				if(strstr(messageIn->request, "NO")){
					printf("MessageHandler%d has received NO from machine%d mailbox%d \n", 
						inPktHdr.to, inPktHdr.from, inMailHdr.from);
					messageID = messageIn->ID;
					inProgressMessages.Mapcar((VoidFunctionPtr)incrementNoReplies);
				}
				else if(strstr(messageIn->request, "YS")){
					printf("MessageHandler%d has received YES from machine%d mailbox%d \n", 
						inPktHdr.to, inPktHdr.from, inMailHdr.from);
					messageID = messageIn->ID;
					inProgressMessages.Mapcar((VoidFunctionPtr)incrementYesReplies);
				}
				InProgressMessage* readyMessage = (InProgressMessage*)inProgressMessages.Remove();
				// There was at least one message in the least
				if(readyMessage){
					// All servers have replied
					if((readyMessage->yesReplies + readyMessage->noReplies) == (numServers - 1)){
						// another server owns the object
						if(readyMessage->yesReplies == 1){
							delete readyMessage; // throw away the message and let the other server deal with it
						}
						// no other server owns the object
						else if((readyMessage->noReplies) == (numServers - 1)){
						if (messageIn->clientMailbox == 9){
							outMailHdr.from = MESSAGE_MAILBOX;
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
							char* messageOut = msgPrepare(messageIn);
							outMailHdr.length = strlen(messageOut) + 1;
							bool success = sendErrorMessage(outPktHdr,outMailHdr,messageIn);
							
						}
						else {
							outMailHdr.from = MESSAGE_MAILBOX;
							outPktHdr.to = inPktHdr.to;
	    					outMailHdr.to = MAILBOX; 
	    					char* messageOut = msgPrepare(readyMessage->message);
							outMailHdr.length = strlen(messageOut) + 1;
							printf("MessageHandler%d is sending a request to its own server: %s \n",
									inPktHdr.to, messageOut);
	    					bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
	    					}
						}
						// something is very wrong because more than 1 server replied yes
						else {
							printf("ERROR: More than one server owns object: %s\n",
									readyMessage->message->name); // error
							delete readyMessage; // throw away message
						}
					}
					// Not all servers have replied
					else {
						// Put the message back on the front of the queue
						inProgressMessages.Prepend((void*)readyMessage);
					}
					continue;
				}
				// No messages in the list
				else {
					continue;
				}	
			}			
			else { // invalid request
				printf("MessageHandler%d has received illegal request from machine%d mailbox%d \n", 
						inPktHdr.to, inPktHdr.from, inMailHdr.from);
				// reply?
				continue;
			}
			
			outMailHdr.from = MESSAGE_MAILBOX;
			outPktHdr.to = inPktHdr.from;
    		outMailHdr.to = inMailHdr.from; 
    		char* messageOut = msgPrepare(messageIn);
			outMailHdr.length = strlen(messageOut) + 1;
			printf("MessageHandler%d is sending a reply to machine%d mailbox%d: %s \n",
					inPktHdr.to, outPktHdr.to, outMailHdr.to, messageOut);
    		bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut);
    	
		}

		//memset(&buffer[0], 0, sizeof(buffer));
		//delete[] buffer;
		printf("-----------------------------------------------\n");
	}
}


//----------------------------------------------------------------------
// Server function
//----------------------------------------------------------------------
void Server(){
	// Initialize tables of variables
	serverLockTable = new ServerLock[MAX_SERVERLOCKS];
	serverConditionTable = new ServerCondition[MAX_SERVERCONDITIONS];
	monitorVariableTable = new MonitorVariable[MAX_MONITORVARIABLES];
	
	bool success = FALSE;

	while(TRUE){
		PacketHeader outPktHdr, inPktHdr;
    	MailHeader outMailHdr, inMailHdr;
    	char* buffer = new char[MaxMailSize];
    	Message* messageOut = new Message;
    	Message* messageIn;
    	
    	// MessageHandler has messages waiting for us
    	if(!messageQueue.IsEmpty()){
    		messageIn = (Message*)messageQueue.Remove();
    	}
    	// No messages waiting, so receive a new one from the post office
    	else {
	    	// Wait to receive a message
			postOffice->Receive(MAILBOX, &inPktHdr, &inMailHdr, buffer);
			printf("Server has received a message: %s\n", buffer);
			
			// Decode the message
			messageIn = decodeMessage(buffer);	
		}
		outPktHdr.to = inPktHdr.from;
    	outMailHdr.to = inMailHdr.from; 
		
		// Decode syscall
		// ACQUIRE
		if(strstr(messageIn->request, "AQ")){
			printf("Server has decoded Acquire request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (serverLockTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("Lock does not exist so cannot be acquired\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {
				// Invalid index passed
				if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0)){
					printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}
				// Lock already deleted
				else if((serverLockTable[messageIn->index].isDeleted) || (serverLockTable[messageIn->index].isToBeDeleted)){	
					printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
						if (messageIn->clientMachine == -1)
						{
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
						}
						else
						{
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
						}
						success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}
				else {
					// No one is waiting on the lock, so the thread can acquire it
					if((serverLockTable[messageIn->index].queue->IsEmpty()) && (serverLockTable[messageIn->index].owner->equals(NO_OWNER))){
						if (messageIn->clientMachine == -1)
						{
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
							serverLockTable[messageIn->index].owner->setIDs(inPktHdr.from, inMailHdr.from);
						}
						else
						{
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
							serverLockTable[messageIn->index].owner->setIDs(messageIn->clientMachine,messageIn->clientMailbox);
						}
						success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    					}
    				// Lock already owned by that thread
    				//Message either directly from thread or from the handler
    					else if ((serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from)) || ((serverLockTable[messageIn->index].owner->equals(messageIn->clientMachine,messageIn->clientMailbox)) && (messageIn->clientMachine != -1))){
    						printf("Server has received request to reacquire lock already owned by machine%d mailbox%d \n", 
							inPktHdr.from, inMailHdr.from);
						if (messageIn->clientMachine == -1)
						{
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
						}
						else
						{
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
						}		
    						success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
    					}
    				// someone else is waiting on or owns the lock, so thread must wait
    					else {
    						
    						ThreadID* id = new ThreadID;
 
    						if (messageIn->clientMachine == -1)
						{
						  	id->setIDs(inPktHdr.from, inMailHdr.from);
						}
						else
						{
							id->setIDs(messageIn->clientMachine,messageIn->clientMailbox);
						}
    						serverLockTable[messageIn->index].queue->Append((void*)id);
    						id->getIDs();
    					}
    				}
    			}
		}
		// RELEASE 
		else if(strstr(messageIn->request, "RL")){
			printf("Server has decoded Release request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			if (serverLockTable[messageIn->index].name == NULL) {//Maybe add index checks but no numLock check 
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("Lock does not exist so cannot be released\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {		
			// Invalid index passed
			if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}		
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			// Lock already deleted
			else if(serverLockTable[messageIn->index].isDeleted){
				printf("Server has received index of deleted lock from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
			else {
				// Lock is owned by thread releasing
				//Either coming directly from thread or from message handler
				if (((serverLockTable[messageIn->index].owner->equals(inPktHdr.from, inMailHdr.from)) || ((serverLockTable[messageIn->index].owner->equals(messageIn->clientMachine,messageIn->clientMailbox)) && (messageIn->clientMachine != -1))) || (inMailHdr.from == 9) || (messageIn->clientMailbox == 9)){
					serverLockTable[messageIn->index].owner->setIDs(NO_OWNER);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
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
    					serverLockTable[messageIn->index].owner->getIDs();
    					printf("Server has received request to release lock at index %d not owned by machine%d mailbox%d \n", 
					messageIn->index, inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					printf("Sending to %i,%i\n",outPktHdr.to,outMailHdr.to);
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);	
    				}
    			}
    		}
		}
		// WAIT
		else if(strstr(messageIn->request, "WT")){
			printf("Server has decoded Wait request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (serverConditionTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//CV must not exist yet
				else {
					printf("CV does not exist\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {		
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
			 	if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	

			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	

    			else {
    				printf("Thread is now waiting\n");
    				if (serverLockTable[messageIn->index2].name == NULL)
    				{	
    				PacketHeader packetHeaderSend, packetHeaderReceive;
				MailHeader mailHeaderSend,mailHeaderReceive;
   				char* receivedData = new char[MaxMailSize];
  				int returnValue;
    
   				 //packetHeaderSend.to = 0;  
   				 //DOes not need to be a pesudo message
   				 //Release needs to check where it is coming from so it does not chec kif lock has been acquired  
      				packetHeaderSend.to = inPktHdr.to;
    				mailHeaderSend.to = 1;
    				mailHeaderSend.from = 9;
    
    				Message* msg = new Message;
    				msg->request = new char[2];
    				msg->request[0] = 'R';
    				msg->request[1] = 'L';
    				msg->name = new char[20];
    				msg->name = "Release";
    				msg->name2 = new char[20];
    				msg->name2 = "Lock";
    				msg->index = messageIn->index2; //Maybe need to rename
    				msg->index2 = -1;
    				msg->index3 = -1;
    				msg->ID = (inPktHdr.to * 100) + IDer;
    				IDer++;
    				if (messageIn->clientMachine == -1)
				{
					msg->clientMachine = inPktHdr.to;
					msg->clientMailbox = 9;//inMailHdr.from;
				}
				else
				{
					msg->clientMachine = inPktHdr.to;
					msg->clientMailbox = 9;//messageIn->clientMailbox;
				}

    
				char* msgstringstream = msgPrepare(msg);
    
  			 	mailHeaderSend.length = MaxMailSize;
  			 	//Releases to mimic a wait
  			 	printf("Sending RELEASE to own message handler\n");
 			  	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
 			  	postOffice->Receive(9,&packetHeaderReceive,&mailHeaderReceive,receivedData);
    
   				Message* receivedMsg = decodeMessage(receivedData);
   				returnValue = receivedMsg->index;
   				if (returnValue != -1)
   				{
   					ThreadID* id = new ThreadID;
   					if (messageIn->clientMachine == -1)
					{
						id->setIDs(inPktHdr.from,inMailHdr.from);
					}
					else
					{
						id->setIDs(messageIn->clientMachine,messageIn->clientMailbox);
					}
   					serverConditionTable[messageIn->index].queue->Append((void*)id);
   					id->getIDs();
   					printf("Waiting on Lock%i\n",returnValue);
   				}
   				else
   				{
   					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
   				}
    				}
    				else
    				{
    					printf("Owns lock and waiting\n");
    					ThreadID* id = new ThreadID;
    					id->setIDs(serverLockTable[messageIn->index2].owner->machineID, serverLockTable[messageIn->index2].owner->threadID);
    					serverLockTable[messageIn->index2].owner->setIDs(NO_OWNER);
    					serverConditionTable[messageIn->index].queue->Append((void*)id);
    					//Other threads waiting on the lock, so give it to the thread waiting longest
    					if(!(serverLockTable[messageIn->index2].queue->IsEmpty())){
    						id = (ThreadID*)serverLockTable[messageIn->index2].queue->Remove();
    						serverLockTable[messageIn->index2].owner = id;
    						outPktHdr.to = id->machineID;
    						outMailHdr.to = id->threadID;
    						success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    					}
    				}
    			}
    			
    			//}
    		}
		}
		// SIGNAL
		else if(strstr(messageIn->request, "SG")){
			printf("Server has decoded Signal request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (serverConditionTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//CV must not exist yet
				else {
					printf("CV does not exist\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {
					
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
			 	if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Invalid lock index passed
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
			 	if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
			 	if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// Lock already deleted

			else {


    			// Lock belongs to thread asking to signal, so let them signal on it
    				if (serverLockTable[messageIn->index2].name == NULL) {
   				   				PacketHeader packetHeaderSend, packetHeaderReceive;
					MailHeader mailHeaderSend,mailHeaderReceive;
	   				char* receivedData = new char[MaxMailSize];
	  				int returnValue;
    	
   					 //DOes not need to be a pesudo message
   					 //Release needs to check where it is coming from so it does not chec kif lock has been acquired  
      					packetHeaderSend.to = inPktHdr.to;
    					mailHeaderSend.to = 1;
    					mailHeaderSend.from = 0;
    
    					Message* msg = new Message;
    					msg->request = new char[2];
    					msg->request[0] = 'A';
    					msg->request[1] = 'Q';
    					msg->name = new char[20];
    					msg->name = "Acquire";
    					msg->name2 = new char[20];
    					msg->name2 = "Lock";
    					msg->index = messageIn->index2; //Maybe need to rename
    					msg->index2 = -1;
    					msg->index3 = -1;
    					msg->ID = (inPktHdr.to * 100) + IDer;
    					IDer++;
    					ThreadID* temp = (ThreadID*)serverConditionTable[messageIn->index].queue->getFirst();
    					msg->clientMachine = temp->machineID;
    					msg->clientMailbox = temp->threadID;

	

   	 
					char* msgstringstream = msgPrepare(msg);
   	 
  				 	mailHeaderSend.length = MaxMailSize;
  				 	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
 				  	ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove();
 				  	 if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
 				  	

    				}
    				else {
    					ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove(); // remove thread from top of CV wait queue
    					serverLockTable[messageIn->index2].queue->Append((void*)id); // append thread to back of lock wait queue
    				 	if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut); //For the signaller
					
    				}
    			
    			
    			}
    		}
		}
		// BROADCAST
		else if(strstr(messageIn->request, "BC")){
			if (serverConditionTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//CV must not exist yet
				else {
					printf("CV does not exist\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {
			printf("Server has decoded Broadcast request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					
			// Invalid condition index passed
			if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0)){
				printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}
			// Invalid lock index passed
			else if((messageIn->index2 >= MAX_SERVERLOCKS) || (messageIn->index2 < 0)){
				printf("Server has received invalid lock index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
			 	if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			// CV already deleted
			else if((serverConditionTable[messageIn->index].isDeleted) || (serverConditionTable[messageIn->index].isToBeDeleted)){
				printf("Server has received index of deleted condition from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut); 
			}	
			else {
    				// Lock belongs to thread asking to broadcast, so let them broadcast on it
    				if (serverLockTable[messageIn->index2].name == NULL) {
    					while(!(serverConditionTable[messageIn->index].queue->IsEmpty())){

    					PacketHeader packetHeaderSend, packetHeaderReceive;
					MailHeader mailHeaderSend,mailHeaderReceive;
	   				char* receivedData = new char[MaxMailSize];
	  				int returnValue;
    	
   					 //DOes not need to be a pesudo message
   					 //Release needs to check where it is coming from so it does not chec kif lock has been acquired  
      					packetHeaderSend.to = inPktHdr.to;
    					mailHeaderSend.to = 1;
    					mailHeaderSend.from = 0;
    
    					Message* msg = new Message;
    					msg->request = new char[2];
    					msg->request[0] = 'A';
    					msg->request[1] = 'Q';
    					msg->name = new char[20];
    					msg->name = "Acquire";
    					msg->name2 = new char[20];
    					msg->name2 = "Lock";
    					msg->index = messageIn->index2; //Maybe need to rename
    					msg->index2 = -1;
    					msg->index3 = -1;
    					msg->ID = (inPktHdr.to * 100) + IDer;
    					IDer++;
    					ThreadID* temp = (ThreadID*)serverConditionTable[messageIn->index].queue->getFirst();
    					msg->clientMachine = temp->machineID;
    					msg->clientMailbox = temp->threadID;

	

   	 
					char* msgstringstream = msgPrepare(msg);
   	 
  				 	mailHeaderSend.length = MaxMailSize;
  				 	   ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove(); //NEED TO REMOVE IN SIGNAL TOO
  				 	postOffice->Send(packetHeaderSend,mailHeaderSend,msgstringstream);
  				 	}
  				 	if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
    				else {
    					while(!(serverConditionTable[messageIn->index].queue->IsEmpty())){
	    					ThreadID* id = (ThreadID*)serverConditionTable[messageIn->index].queue->Remove(); // remove thread from top of CV wait queue
    						serverLockTable[messageIn->index2].queue->Append((void*)id); // append thread to back of lock wait queue
    					}	
    					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
    					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
    			}
    			}
		}
		// CREATE LOCK
		else if(strstr(messageIn->request, "CL")){
			printf("Server has decoded CreateLock request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			
			int index = findLockByName(messageIn->name,inPktHdr.to);
			// Lock with that name not on this server
			if(index == -1){		
				// Message didn't pass through MessageHandler already, so need to check with other servers

				if(messageIn->ID == -1){
				printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    				outMailHdr.to = MESSAGE_MAILBOX;
    				success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				// Message came through MessageHandler, so create a new lock
				else {	
					// No more room for locks		
					if(numLocks >= MAX_SERVERLOCKS){
						printf("Server cannot create a lock for machine%d mailbox%d because the max number of locks has been reached \n", 
								inPktHdr.from, inMailHdr.from);
						if (messageIn->clientMachine == -1)
						{	
							outPktHdr.to = inPktHdr.from;
							outMailHdr.to = inMailHdr.from;
						}
						else
						{
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
						}
						success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
					}	
					else {
						constructServerLock((100*inPktHdr.to + numLocks), messageIn->name);

						messageOut->index = 100*inPktHdr.to + numLocks;
						numLocks++;
						outPktHdr.to = messageIn->clientMachine;
    					outMailHdr.to = messageIn->clientMailbox;
						success = sendMessage(outPktHdr, outMailHdr, messageOut);
					}
				}
			}
			// Lock already created on this server
			else {
				printf("Lock is already on this server\n");
				messageOut->index = index;
				
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
				success = sendMessage(outPktHdr, outMailHdr, messageOut);
			}
		}
		// DESTROY LOCK
		else if(strstr(messageIn->request, "DL")){
			printf("Server has decoded DestroyLock request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			//Lock not on server
			if (serverLockTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("Lock does not exist so cannot be destroyed\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else
			{
				// Invalid index passed
				if((messageIn->index >= MAX_SERVERLOCKS) || (messageIn->index < 0)){
					printf("Server has received invalid lock index from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
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
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
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
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}	
			else {
				int index = findConditionByName(messageIn->name,inPktHdr.to);
				// Condition with that name not yet created
				if(index == -1){ 
					if(messageIn->ID == -1){
						printf("Didn't go through message handler so sending out\n");
						messageIn->ID = (inPktHdr.to * 100) + IDer;
						IDer++;
						messageIn->clientMachine = inPktHdr.from;
						messageIn->clientMailbox = inMailHdr.from;
						outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
					}
					else {
						constructServerCondition((100*inPktHdr.to + numConditions), messageIn->name);
						messageOut->index = 100*inPktHdr.to + numConditions;
						numConditions++;
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
						success = sendMessage(outPktHdr, outMailHdr, messageOut);
					}
				}
				// Condition already created
				else {
					printf("Conditon already on this server at index%i\n",index);
					messageOut->index = index;
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendMessage(outPktHdr, outMailHdr, messageOut);
				}
			}
		}
		// DESTROY CONDITION
		else if(strstr(messageIn->request, "DC")){
			printf("Server has decoded DestroyCondition request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (serverConditionTable[messageIn->index].name == NULL) {
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("CV does not exist so cannot be destroyed\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else
			{
				// Invalid index passed
				if((messageIn->index >= MAX_SERVERCONDITIONS) || (messageIn->index < 0)){
					printf("Server has received invalid condition index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
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
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				}
    			} 
		}
		// CREATE MV
		else if(strstr(messageIn->request, "CM")){
			printf("Server has decoded CreateMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);	
			if(numMonitorVariables >= MAX_MONITORVARIABLES){
				printf("Server cannot create an MV for machine%d mailbox%d because the max number of MVs has been reached \n", 
						inPktHdr.from, inMailHdr.from);
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
					success = sendMessage(outPktHdr, outMailHdr, messageOut);
			}
			// Invalid array size (must be greater than 0)
			else if(messageIn->index2 < 1){
				printf("Server has received invalid array size from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
				if (messageIn->clientMachine == -1)
				{
					outPktHdr.to = inPktHdr.from;
					outMailHdr.to = inMailHdr.from;
				}
				else
				{
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
				}
				success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
			}
						
			else {									
				int index = findMonitorVariableByName(messageIn->name,inPktHdr.to);
				// Lock with that name not on this server
				if(index == -1){		
				// Message didn't pass through MessageHandler already, so need to check with other servers

					if(messageIn->ID == -1){
						printf("Didn't go through message handler so sending out\n");
						messageIn->ID = (inPktHdr.to * 100) + IDer;
						IDer++;
						messageIn->clientMachine = inPktHdr.from;
						messageIn->clientMailbox = inMailHdr.from;
						outPktHdr.to = inPktHdr.to;
    						outMailHdr.to = MESSAGE_MAILBOX;
    						success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
					}
				// Message came through MessageHandler, so create a new lock

					else {
						// MV with that name not yet created
							constructMonitorVariable((100*inPktHdr.to + numMonitorVariables), messageIn->index2, messageIn->name);
							messageOut->index = 100*inPktHdr.to + numMonitorVariables;
							numMonitorVariables++;
							outPktHdr.to = messageIn->clientMachine;
							outMailHdr.to = messageIn->clientMailbox;
							success = sendMessage(outPktHdr, outMailHdr, messageOut);
					}
				}					// MV already created
				else { 
					printf("MV already on this server\n");
					messageOut->index = index;
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendMessage(outPktHdr, outMailHdr, messageOut);
				}
				
			}

		}
		// DESTROY MV

		else if(strstr(messageIn->request, "DM")){
			printf("Server has decoded DestroyMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (monitorVariableTable[messageIn->index].name == NULL) {//Maybe add index checks but no numLock check 
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("MV does not exist so cannot be released\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {	
			// Invalid index passed
				if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0)){
					printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
				else {
					monitorVariableTable[messageIn->index].isToBeDeleted = TRUE;
					monitorVariableTable[messageIn->index].isDeleted = TRUE;
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				} 
    			}
		}
		// GET MV
		else if(strstr(messageIn->request, "GM")){
			printf("Server has decoded GetMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (monitorVariableTable[messageIn->index].name == NULL) {//Maybe add index checks but no numLock check 
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("MV does not exist so cannot be released\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {
			// Invalid index passed
				if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0)){
					printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			// Invalid array index passed
				else if((messageIn->index2 < 0) || (messageIn->index2 >= monitorVariableTable[messageIn->index].arraySize)){
					printf("Server has received invalid MV array index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
				else {
					// Allow thread to access MV
					messageOut->index = monitorVariableTable[messageIn->index].array[messageIn->index2];
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendMessage(outPktHdr, outMailHdr, messageOut);
    				} 
    			}
		}
		// SET MV
		else if(strstr(messageIn->request, "SM")){
			printf("Server has decoded SetMV request from machine%d mailbox%d \n", 
					inPktHdr.from, inMailHdr.from);
			if (monitorVariableTable[messageIn->index].name == NULL) {//Maybe add index checks but no numLock check 
			
				if(messageIn->ID == -1){
					printf("Didn't go through message handler so sending out\n");
					messageIn->ID = (inPktHdr.to * 100) + IDer;
					IDer++;
					messageIn->clientMachine = inPktHdr.from;
					messageIn->clientMailbox = inMailHdr.from;
					outPktHdr.to = inPktHdr.to;
    					outMailHdr.to = MESSAGE_MAILBOX;
    					success = sendToMessageHandler(outPktHdr, outMailHdr, messageIn);
				}
				//Lock must not exist yet
				else {
					printf("MV does not exist so cannot be released\n");
					outPktHdr.to = messageIn->clientMachine;
					outMailHdr.to = messageIn->clientMailbox;
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
			}
			else {
				// Invalid index passed
				if((messageIn->index >= MAX_MONITORVARIABLES) || (messageIn->index < 0)){
					printf("Server has received invalid MV index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
				// Invalid array index passed
				else if((messageIn->index2 < 0) || (messageIn->index2 >= monitorVariableTable[messageIn->index].arraySize)){
					printf("Server has received invalid MV array index from machine%d mailbox%d \n", 
						inPktHdr.from, inMailHdr.from);
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}		
					success = sendErrorMessage(outPktHdr, outMailHdr, messageOut);
				}	
				else {
					// Allow thread to access MV
					monitorVariableTable[messageIn->index].array[messageIn->index2] = messageIn->index3;
					if (messageIn->clientMachine == -1)
					{
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
					}
					else
					{
						outPktHdr.to = messageIn->clientMachine;
						outMailHdr.to = messageIn->clientMailbox;
					}	
					success = sendSuccessMessage(outPktHdr, outMailHdr, messageOut);
    				} 
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

void incrementYesReplies(int ptr){
	InProgressMessage* nextMessage = (InProgressMessage*)ptr;
	if(nextMessage->message->ID == messageID){
		nextMessage->yesReplies++; 
		printf("MessageHandler is incrementing yesReplies to %d on message%d\n",
				nextMessage->yesReplies, messageID);
	}
}

void incrementNoReplies(int ptr){
	InProgressMessage* nextMessage = (InProgressMessage*)ptr;
	if(nextMessage->message->ID == messageID){
		nextMessage->noReplies++; 
		printf("MessageHandler is incrementing noReplies to %d on message%d\n",
				nextMessage->noReplies, messageID);
	}
}

bool sendToMessageHandler(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response){	
	char* messageOut = msgPrepare(response);
	outMailHdr.length = strlen(messageOut) + 1;
    bool success = postOffice->Send(outPktHdr, outMailHdr, messageOut); 
    return success;
}

bool sendMessage(PacketHeader outPktHdr, MailHeader outMailHdr, Message* response){
	response->request = new char[2];
	response->request[0] = 'H';
	response->request[1] = 'I';
	response->name = "index";
	response->name2 = new char[20];
	response->name2 = "reply";
	response->index2 = SUCCESS_CODE;
	response->index3 = SUCCESS_CODE;
	response->ID = -1;
	response->clientMachine = -1;
	response->clientMailbox = -1;
	
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
	response->name2 = new char[20];
	response->name2 = "reply";
	response->index = ERROR_CODE;
	response->index2 = ERROR_CODE;
	response->index3 = ERROR_CODE;
	response->ID = -1;
	response->clientMachine = -1;
	response->clientMailbox = -1;
	
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
	response->name2 = new char[20];
	response->name2 = "reply";
	response->index = SUCCESS_CODE;
	response->index2 = SUCCESS_CODE;
	response->index3 = SUCCESS_CODE;
	response->ID = -1;
	response->clientMachine = -1;
	response->clientMailbox = -1;
	
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

int findLockByName(char* name, int machineID){
	for(int i = 0; i < numLocks; i++){
		if(!strcmp(serverLockTable[machineID*100 + i].name, name)){
			return (machineID*100 + i);
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

int findConditionByName(char* name, int machineID){
	for(int i = 0; i < numConditions; i++){
		if(!strcmp(serverConditionTable[machineID*100 + i].name, name)){
			return (machineID*100 + i);
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

int findMonitorVariableByName(char* name, int machineID){
	for(int i = 0; i < numMonitorVariables; i++){
		if(!strcmp(monitorVariableTable[machineID*100 + i].name, name)){
			return (machineID*100 + i);
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
