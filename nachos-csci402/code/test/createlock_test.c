#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "lock1";
    int lock1, lock2, lock3;
    int i;
    
   	Printx("Test is creating a lock named lock1\n",38,1);
   	lock1 = CreateLock(name, 5);
   	if(lock1 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d%d\n",40,lock1*10000);	
   	}
   	
   	Printx("Test is creating another lock named lock1\n",44,1);
   	lock2 = CreateLock(name, 5);
   	if(lock2 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else if(lock2 == lock1){
   		Printx("Lock already exists at index %d%d\n",36,lock2*10000);
   	}
   	else {
   		Printx("Lock created successfully at index %d%d\n",40,lock2*10000);	
   	}
   	
   	name = "lock2";
   	Printx("Test is creating another lock named lock2\n",44,1);
   	lock3 = CreateLock(name, 5);
   	if(lock3 == -1){
   		Printx("Lock not created\n",18,1);
   	}
   	else {
   		Printx("Lock created successfully at index %d%d\n",40,lock3*10000);	
   	}
    
    Exit(0);
    /* not reached */
}
