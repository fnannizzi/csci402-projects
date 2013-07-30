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

	for (i = 0; i < 7000; i++)
		Yield();
		
	/*Printx("Test2 is creating a lock named lock1\n",39,1);*/
   	lock1 = CreateLock(name1, 5);
   	lockNum = lock1;
   	/*if(lock1 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d\n",38,lock1*10000000);	
   	}*/
   	
   	/*Printx("\nTest2 is creating a cv named cv1\n",37,1);*/
   	cv1 = CreateCondition(name2, 5);
   	cvNum = cv1;
   	/*if(cv1 == -1){
   		Printx("CV not created\n",16,1);
   	}
   	else {
   		Printx("CV created successfully at index %d\n",36,cv1*10000000);	
   	}*/
	
	
   	
   	Printx("\nTest2 is acquiring lock at index %d%d\n",39,lockNum*10000);		
    	lock1 = Acquire(lockNum);
    	if (lock1 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    		
    	Printx("\nTest2 is signalling the waiter using CV at index -1\n",55,1);
	returnValue = Signal(-1, lockNum);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}
	
	Printx("\nTest2 is signalling the waiter using CV at index 10000\n",58,1);
	returnValue = Signal(10000, lockNum);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}
	
	Printx("\nTest2 is signalling the waiter using CV at index 143\n",56,1);
	returnValue = Signal(143, lockNum);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}
	
	Printx("\nTest2 is signalling the waiter using lock at index -1\n",57,1);
	returnValue = Signal(cvNum, -1);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}	
	
	Printx("\nTest2 is signalling the waiter using lock at index 5698\n",59,1);
	returnValue = Signal(cvNum, 5698);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}	
	
	Printx("\nTest2 is signalling the waiter\n",33,1);
	returnValue = Signal(cvNum, lockNum);
	if(returnValue == -1){ 
		Printx("Test2 not able to signal on the CV\n",35,1);
	}
	
	    
    Printx("\nTest2 is releasing lock at index %d%d\n",39,lockNum*10000);
    lock1 = Release(lockNum);
    if (lock1 == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);


	Exit(0);
}
