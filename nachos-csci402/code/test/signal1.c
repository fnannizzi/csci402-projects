int main()
{
	char* name1 = "lock1";
	char* name2 = "cv1";
	int lock1;
	int cv1;
	int returnValue;

   	/*Need to get the proper lock to signal and wait, probably with a createlock and createcv*/
   	/*SOlve mutual exclusion by checking in message handler for item*/
	Printx("Test is creating a lock named lock1\n",38,1);
   	lock1 = CreateLock(name1, 5);
   	if(lock1 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d%d\n",40,lock1*10000);	
   	}
   	
   	Printx("\nTest is creating a cv named cv1\n",36,1);
   	cv1 = CreateCondition(name2, 5);
   	if(cv1 == -1){
   		Printx("CV not created\n",16,1);
   	}
   	else {
   		Printx("CV created successfully at index %d%d\n",38,cv1*10000);	
   	}

	Exit(0);
}
