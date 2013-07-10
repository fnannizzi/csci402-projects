#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    int i;
    
    Printx("\nTest is creating 500 locks to faill up table\n",45,1); /*Will have to change based off MAXLOCKS*/
    for (i = 0; i < 500; i++)
    {
    	lock = CreateLock(name,4);
   	if (lock == -1)
   		Printx("Lock not created\n",18,1);
    	else
    		Printx("Lock created successfully at index %d%d\n",40,lock*10000);
    }
    Printx("\nTest is attempting to create one more lock\n",44,1);
    lock = CreateLock(name,4);
    if (lock == -1)
   		Printx("Lock not created\n",18,1);
    	else
    		Printx("Lock created successfully at index %d%d\n",40,lock*10000);
    
    Exit(0);
    /* not reached */
}
