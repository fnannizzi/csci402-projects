int main()
{
	char* name1 = "lock1";
	char* name2 = "cv1";
	int lock1;
	int cv1;
	int lockNum;
	int cvNum;
	int returnValue;

   	
	Printx("Test is creating a lock named lock1\n",38,1);
   	lock1 = CreateLock(name1, 5);
   	lockNum = lock1;
   	if(lock1 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d%d\n",40,lock1*10000);	
   	}
   	
   	Printx("\nTest is creating a cv named cv1\n",36,1);
   	cv1 = CreateCondition(name2, 5);
   	cvNum = cv1;
   	if(cv1 == -1){
   		Printx("CV not created\n",16,1);
   	}
   	else {
   		Printx("CV created successfully at index %d%d\n",38,cv1*10000);	
   	}
   	
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
	
	Printx("\nTest is releasing lock at index %d%d\n",38,lockNum*10000);
    	lock1 = Release(lockNum);
    	if (lock1 == -1)
   		Printx("Lock not released\n",18,1);
    	else
    		Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);
   	
   	

	Exit(0);
}
