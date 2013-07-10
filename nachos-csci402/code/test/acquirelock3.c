#include "syscall.h"
int a[3];
int b, c;

void AL0() {
	    char* name = "test";
    int lock;
	
	Printx("\nTest3.1 is creating lock at index 2\n",37,1);	    	   	
    	lock = CreateLock(name,4);
   	if (lock == -1)
   		Printx("Lock not created\n",17,1);
   	else
    		Printx("Lock created successfully at index %d\n",38,lock*10000000);
	Exit(0);
}

void AL1() {
	    char* name = "test";
    int lock;
   	Printx("\nTest3.2 is destroying lock at index 2\n",39,1);		
    	lock = DestroyLock(2);
    if (lock == -1)
   	 Printx("Lock not destroyed\n",19,1);
    else
    	Printx("Lock at index %d successfully set to be destroyed\n",50,lock*10000000);
    
    Printx("\nTest3.2 is attempting to acquire lock at index 2\n",50,1);	
    	lock = Acquire(2);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
 
   Printx("\nTest3.2 is attempting to acquire lock at index 2\n",50,1);	   	
    	 lock = Acquire(2);
    if (lock == -1)
   	 Printx("Lock not acquired\n",18,1);
    else
    	Printx("Lock acquired successfully from index %d\n",41,lock*10000000);
Exit(0);
} 

int
main()
{
	    char* name = "test";
    int lock;
	Fork(AL0,"AL0",3);
	Fork(AL1,"AL1",3);
    
    /* not reached */
}
