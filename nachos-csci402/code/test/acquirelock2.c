#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    Printx("\nTest2 is attempting to acquire lock at index 1\n",48,1);		
    lock = Acquire(1);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
    
    
    /* not reached */
}
