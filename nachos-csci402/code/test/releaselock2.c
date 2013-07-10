#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    Printx("\nTest2 is attempting to release lock at index 1\n",48,1);		
    lock = Acquire(1);
    if (lock == -1)
   	 Printx("Lock not released\n",18,1);
    else
    	Printx("Lock at index %d released successfully\n",39,lock*10000000);
    
    
    /* not reached */
}
