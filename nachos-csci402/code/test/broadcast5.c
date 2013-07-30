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
	
	for (i = 0; i < 10000; i++)
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
   		Printx("CV created successfully at index %d\n",40,cv1*10000000);	
   	}*/
   	

   	   	
   	Printx("\nTest5 is acquiring lock at index %d%d\n",39,lockNum*10000);		
    	lock1 = Acquire(lockNum);
    	if (lock1 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);


   	Printx("\nTest 5 is broadcasting on CV at index -1\n",44,1);
	returnValue = Broadcast(-1, lockNum);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}
	
	Printx("\nTest 5 is broadcasting on CV at index 6000\n",46,1);
	returnValue = Broadcast(6000, lockNum);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}
	
	Printx("\nTest 5 is broadcasting on NULL CV\n",37,1);
	returnValue = Broadcast(234, lockNum);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}
	
	Printx("\nTest 5 is broadcasting on lock at index -1\n",46,1);
	returnValue = Broadcast(cvNum, -1);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}
	
	Printx("\nTest 5 is broadcasting on lock at index 7000\n",48,1);
	returnValue = Broadcast(cvNum, 7000);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}

    		
    	Printx("\nTest5 is signalling the waiters\n",34,1);
   	   	
   	Printx("\nTest 5 is broadcasting\n",26,1);
	returnValue = Broadcast(cvNum, lockNum);
	if(returnValue == -1){ 
		Printx("Test 5 not able to broadcast on the CV\n",40,1);
	}
		
	Printx("\nTest5 is releasing lock at index %d%d\n",39,lockNum*10000);
    	lock1 = Release(lockNum);
    	if (lock1 == -1)
   		Printx("Lock not released\n",18,1);
    	else
    		Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);

	Exit(0);
}
