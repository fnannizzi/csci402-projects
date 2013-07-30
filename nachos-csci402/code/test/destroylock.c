#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    char* name = "test";
    int lock;
    int lockNum;
    
    Printx("\nTest is attempting to destroy a lock at index -1\n",50,1);
    lock = DestroyLock(-1);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
    }
    else {
    	Printx("Lock at index %d%d set to be destroyed\n",40,lock*10000);
	}
	
    Printx("\nTest is attempting to destroy a lock at index 200\n",51,1);
    lock = DestroyLock(200);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
   	}
    else {
    	Printx("Lock at index %d%d destroyed successfully\n",43,lock*10000);
    }
    
    Printx("\nTest is creating a lock at index 0\n",36,1);
    lock = CreateLock(name,4);
    lockNum = lock;
    if (lock == -1){
   		Printx("Lock not created\n",19,1);
   	}
    else {
    	Printx("Lock created successfully at index %d%d\n",40,lockNum*10000);
    }	
    
    Printx("\nTest is acquiring lock at index %d%d\n",38,lockNum*10000);	
    lock = Acquire(lockNum);
    if (lock == -1){
   		Printx("Lock not acquired\n",18,1);
    }
    else {
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    }
    
/*    Printx("\nTest is attempting to destroy previously acquired lock at index 0\n",67,1);
    lock = DestroyLock(0);
    if (lock == -1){
   	 	Printx("Lock not set to be destroyed\n",29,1);
   	}
    else {
    	Printx("Lock at index %d set to be destroyed\n",38,lock*10000000);
    }*/
       
    Printx("\nTest is releasing lock at index %d%d\n",38,lockNum*10000);		
    lock = Release(lockNum);
    if (lock == -1){
   		Printx("Lock not released\n",20,1);
   	}
    else {
    	Printx("Lock released successfully from index %d%d\n",45,lockNum*10000);
    }
  	
    Printx("\nTest is attempting to destroy previously released lock at index %d%d\n",70,lockNum*10000);
    lock = DestroyLock(lockNum);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
    }
    else {
    	Printx("Lock at index %d%d set to be destroyed\n",40,lockNum*10000);
    }
/*    
    Printx("\nTest is attempting to destroy previously destroyed lock at index 0\n",68,1);	
    lock = DestroyLock(0);
    if (lock == -1){
   		Printx("Lock not set to be destroyed\n",29,1);
    }
    else {
    	Printx("Lock at index %d set to be destroyed\n",38,lock*10000000);
    }*/
    
    Printx("\nTest is acquiring lock at index %d%d\n",38,lockNum*10000);	
    lock = Acquire(lockNum);
    if (lock == -1){
   		Printx("Lock not acquired because it has been destroyed\n",49,1);
    }
    else {
    	Printx("Lock acquired successfully from index %d%d\n",43,lockNum*10000);
    }
    
    Exit(0);
    	
    /*Exec("../test/destroylock1",20);
    Exec("../test/destroylock2",20);
    */
   /*Need to show that acquiring or releasing from one thread interacts with another one trying to destroy*/
    
    
    	
   
    
    /* not reached */
}
