int main()
{
	char* name1 = "lock1";
	char* name2 = "cv1";
	int lock1;
	int cv1;
	int returnValue;
	int i;

	for (i = 0; i < 2000; i++)
		Yield();
   	
   	Printx("\nTest3 is acquiring lock at index 0\n",36,1);		
    	lock1 = Acquire(0);
    	if (lock1 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d\n",41,lock1*10000000);
    		
    	Printx("\nTest3 is waiting on CV1\n",27,1);
	returnValue = Wait(cv1, lock1);
	if(returnValue == -1){ 
		Printx("Test not able to wait on the CV\n",32,1);
	}
	
	Printx("\nTest3 has been signalled and has reacquired the lock\n",56,1);
	
	Printx("\nTest3 is releasing lock at index 0\n",36,1);
    	lock1 = Release(0);
    	if (lock1 == -1)
   		Printx("Lock not released\n",18,1);
    	else
    		Printx("Lock at index %d released successfully\n",39,lock1*10000000);
   	
	Exit(0);
}
