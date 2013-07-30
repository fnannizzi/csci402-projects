#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    int lockNum;
      
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
    	Printx("Lock acquired successfully from index %d%d\n",43,lock*10000);
/*    
    Printx("\nTest is attempting to acquire null lock at index 0\n",52,1);	
    lock = Acquire(0);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
*/ 
    Printx("\nTest is creating lock at index 0\n",34,1);	    	   	
    lock = CreateLock(name,4);
    lockNum = lock;
    if (lock == -1)
   	 Printx("Lock not created\n",17,1);
    else
    	Printx("Lock created successfully at index %d%d\n",40,lock*10000);
    
    Printx("\nTest is acquiring lock at index %d%d\n",38,lockNum*10000);		
    lock = Acquire(lockNum);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    
    Printx("\nTest is attempting to reacquire lock at index %d%d\n",52,lockNum*10000);		
    lock = Acquire(lockNum);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    	
    Printx("\nTest is releasing lock at index %d%d\n",38,lockNum*10000);		
    lock = Release(lockNum);
    if (lock == -1){
   		Printx("Lock not released\n",20,1);
   	}
    else {
    	Printx("Lock released successfully from index %d%d\n",45,lockNum*10000);
    }	
    	
    Printx("\nTest is attempting to destroy the lock at index %d%d\n",57,lockNum*10000);
    lock = DestroyLock(lockNum);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
   	}
    else {
    	Printx("Lock at index %d%d destroyed successfully\n",43,lockNum*10000);
    }
    
    Printx("\nTest is attempting to acquire lock at index %d%d\n",53,lockNum*10000);    
    lock = Acquire(lockNum);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    
    		
/*    	
    Exec("../test/acquirelock1",20);
    Exec("../test/acquirelock2",20);

    Exec("../test/acquirelock3",20);
*/    
    /* not reached */
}
