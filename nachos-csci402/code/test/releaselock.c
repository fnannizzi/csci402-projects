#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    Printx("\nTest is attempting to release a lock at index -1\n",50,1);
    lock = Release(-1);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);

    Printx("\nTest is attempting to release a lock at index 200\n",51,1);    	
    lock = Release(200);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    
    Printx("\nTest is attempting to release a null lock at index 0\n",54,1);
    lock = Release(0);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    	
    Printx("\nTest is creating a lock at index 0\n",36,1);
    lock = CreateLock(name,4);
    if (lock == -1)
   	 Printx("Lock ran into a problem\n",24,1);
    else
    	Printx("Lock created successfully at index %d\n",38,lock*10000000);
    	
    Printx("\nTest is attempting to release lock at index 0 without acquiring\n",65,1);
    lock = Release(0);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    	
    Printx("\nTest is acquiring lock at index 0\n",35,1);
    lock = Acquire(0);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    
    Printx("\nTest is releasing lock at index 0\n",35,1);
    lock = Release(0);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    
    Printx("\nTest is attempting to release previously released lock at index 0\n",67,1);
    lock = Release(0);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    	
    Exec("../test/releaselock1",20);
    Exec("../test/releaselock2",20);
    Exec("../test/releaselock3",20);
    
    /* not reached */
}
