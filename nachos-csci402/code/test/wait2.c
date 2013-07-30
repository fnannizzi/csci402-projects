int main()
{
	char* name1 = "lock1";
	char* name2 = "cv1";
	int lock1;
	int cv1;
	int lockNum;
	int cvNum;
	int returnValue;
	int i;

	for (i = 0; i < 3000; i++)
		Yield();
	/*Printx("Test is creating a lock named lock1\n",38,1);*/
   	lock1 = CreateLock(name1, 5);
   	lockNum = lock1;
   	/*if(lock1 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d\n",38,lock1*10000000);	
   	}*/
   	
   	/*Printx("\nTest is creating a cv named cv1\n",36,1);*/
   	cv1 = CreateCondition(name2, 5);
   	cvNum = cv1;
   	/*if(cv1 == -1){
   		Printx("CV not created\n",16,1);
   	}
   	else {
   		Printx("CV created successfully at index %d\n",36,cv1*10000000);	
   	}*/
   	
   	Printx("\nTest is acquiring lock at index %d%d\n",38,lockNum*10000);		
    	lock1 = Acquire(lockNum);
    	if (lock1 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    		
    	Printx("\nTest is waiting on CV1\n",26,1);
	returnValue = Wait(cvNum, lockNum);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}
	
	Printx("\nTest has been signalled and has reacquired the lock\n",55,1);
	
	Printx("\nTest is waiting on CV at index -1\n",37,1);
	returnValue = Wait(-1, lockNum);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}
	
	Printx("\nTest is waiting on CV that is NULL\n",38,1);
	returnValue = Wait(200, lockNum);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}
	
	Printx("\nTest is waiting on lock at index -1\n",39,1);
	returnValue = Wait(cvNum, -1);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}
	
	Printx("\nTest is waiting on lock at index 20000\n",42,1);
	returnValue = Wait(cvNum, 20000);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}

	Exit(0);
}
