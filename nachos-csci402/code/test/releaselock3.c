#include "syscall.h"
int a[3];
int b, c;

void RL1() {
	char* name = "test";
    int lock;
    int lock2;
    int cv;
    int wait;
	
	Printx("\nTest4.1 is creating lock at index 2\n",37,1);	    	   	
    	lock = CreateLock(name,4);
   	if (lock == -1)
   		Printx("Lock not created\n",17,1);
   	else
    		Printx("Lock created successfully at index %d\n",38,lock*10000000);
    		
    	Printx("\nTest4.1 is creating lock at index 3\n",37,1);	    	   	
    	lock2 = CreateLock(name,4);
   	if (lock2 == -1)
   		Printx("Lock not created\n",17,1);
   	else
    		Printx("Lock created successfully at index %d\n",38,lock2*10000000);
	
	Printx("\nTest4.1 is acquiring lock at index 2\n",38,1);
    	lock = Acquire(lock);
    	if (lock == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    		
    	Printx("\nTest4.1 is acquiring lock at index 3\n",38,1);
    	lock2 = Acquire(lock2);
    	if (lock2 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d\n",41,lock2*10000000);
    		
    	   Printx("\nTest4.1 is creating CV at index 0\n",35,1);
    	cv = CreateCondition(name,4);
      	if (cv == -1)
   		Printx("CV not created\n",15,1);
   	else
    		Printx("CV created successfully at index %d\n",36,cv*10000000);		   
	
	Printx("\nTest4.1 is waiting with CV 0 and Lock 3\n",41,1);
	wait = Wait(cv,lock2);
	if (wait == -1)
		Printx("CV did not successfully wait\n",29,1);
	else
		Printx("CV is successfully waiting\n",27,1);
		
	   Printx("\nTest4.1 is releasing lock at index 3\n",38,1);
   	 lock = Release(3);
    	if (lock == -1)
   		 Printx("Lock not released\n",18,1);
   	 else
   	 	Printx("Lock at index %d released successfully\n",39,lock*10000000);
   	 
   	 	   Printx("\nTest4.1 is releasing lock at index 2\n",38,1);
   	 lock = Release(2);
    	if (lock == -1)
   		 Printx("Lock not released\n",18,1);
   	 else
   	 	Printx("Lock at index %d released successfully\n",39,lock*10000000);
   	 
   	 	Exit(0);
	
}

void RL2() {
	    char* name = "test";
    int lock;
    int lock2;
    int signal;
    
    	Printx("\nTest4.2 is acquiring lock at index 3\n",38,1);
    	lock2 = Acquire(3);
    	if (lock2 == -1)
   		 Printx("Lock not acquired\n",18,1);
    	else
    		Printx("Lock acquired successfully from index %d\n",41,lock2*10000000);
   	
   	Printx("\nTest4.2 is destroying lock at index 2\n",39,1);		
    	lock = DestroyLock(2);
    if (lock == -1)
   	 Printx("Lock not destroyed\n",19,1);
    else
    	Printx("Lock at index %d successfully set to be destroyed\n",50,lock*10000000);
    	
    	
    	Printx("\nTest4.2 is signaling with CV 0 and Lock 3\n",43,1);
    	signal = Signal(0,3);
    	if (signal == -1)
		Printx("CV did not successfully signal\n",31,1);
	else
		Printx("CV is successfully signaling\n",29,1);
		
	 Printx("\nTest4.2 is releasing lock at index 3\n",38,1);
    lock = Release(3);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    	

Exit(0);
	
}

int
main()
{
    char* name = "test";
    int lock;
    Fork(RL1,"RL1",3);
    Fork(RL2,"RL2",3);
    /* not reached */
}
