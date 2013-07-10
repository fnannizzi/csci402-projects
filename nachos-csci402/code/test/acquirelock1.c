#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    
    Printx("\nTest1 is creating lock at index 1\n",35,1);	    	   	
    lock = CreateLock(name,4);
    if (lock == -1)
   	 Printx("Lock not created\n",17,1);
    else
    	Printx("Lock created successfully at index %d\n",38,lock*10000000);
    
    /* not reached */
}
