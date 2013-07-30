#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    Printx("\nTest2 is attempting to destroy lock at index 1\n",48,1);		
    lock = DestroyLock(1);
     if (lock == -1)
   	 Printx("Lock not set to be destroyed\n",29,1);
    else
    	Printx("Lock at index %d set to be destroyed\n",38,lock*10000000);
    
    
    /* not reached */
}
