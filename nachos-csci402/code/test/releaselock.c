#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    int lockNum;
    
    Printx("\nTest is attempting to release a lock at index -1\n",50,1);
    lock = Release(-1);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lock*10000);

    Printx("\nTest is attempting to release a lock at index 200\n",51,1);    	
    lock = Release(200);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lock*10000);
  	
    Printx("\nTest is creating a lock at index 0\n",36,1);
    lock = CreateLock(name,4);
    lockNum = lock;
    if (lock == -1)
   	 	Printx("Lock not created\n",19,1);
    else
    	Printx("Lock created successfully at index %d%d\n",40,lockNum*10000);
    	
    Printx("\nTest is attempting to release lock at index %d%d without acquiring\n",68,lockNum*10000);
    lock = Release(lockNum);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);
    	
    Printx("\nTest is acquiring lock at index %d%d\n",38,lockNum*10000);
    lock = Acquire(lockNum);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    
    Printx("\nTest is releasing lock at index %d%d\n",38,lockNum*10000);
    lock = Release(lockNum);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);
    	
    	
   	Printx("\nTest is destroying previously released lock at index %d%d\n",62,lockNum*10000);
    lock = DestroyLock(lockNum);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
    }
    else {
    	Printx("Lock at index %d%d set to be destroyed\n",40,lockNum*10000);
    }
    
    Printx("\nTest is attempting to release previously destroyed lock at index %d%d\n",74,lockNum*10000);
    lock = Release(lockNum);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d%d released successfully\n",41,lockNum*10000);
/*    	
    Exec("../test/releaselock1",20);
    Exec("../test/releaselock2",20);
    Exec("../test/releaselock3",20);
*/    
    /* not reached */
}
