#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    
    Printx("\nTest is attempting to acquire lock at index -1\n",48,1);
    lock = Acquire(-1);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully\n",27,1);
    
    Printx("\nTest is attempting to acquire lock at index 200\n",49,1);    
    lock = Acquire(200);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    
    Printx("\nTest is attempting to acquire null lock at index 0\n",52,1);	
    lock = Acquire(0);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
 
    Printx("\nTest is creating lock at index 0\n",34,1);	    	   	
    lock = CreateLock(name,4);
    if (lock == -1)
   	 Printx("Lock not created\n",17,1);
    else
    	Printx("Lock created successfully at index %d\n",38,lock*10000000);
    
    Printx("\nTest is acquiring lock at index 0\n",35,1);		
    lock = Acquire(0);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    
    Printx("\nTest is attempting to reacquire lock at index 0\n",49,1);		
    lock = Acquire(0);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    	
    Exec("../test/acquirelock1",20);
    Exec("../test/acquirelock2",20);

    Exec("../test/acquirelock3",20);
    
    /* not reached */
}
