#include "syscall.h"

void exit_norm_1(){ 
	
	Printx("Thread 1 is running\n",22,1);
	Printx("Thread 1 is exiting\n",22,1);
	
	Exit(0);
}

void exit_norm_2(){ 
	
	Printx("Thread 2 is running\n",22,1);
	Printx("Thread 2 is exiting\n",22,1);
	
	Exit(0);
}

void exit_norm_3(){ 
	
	Printx("Thread 3 is running\n",22,1);
	Printx("Thread 3 is exiting\n",22,1);
	
	Exit(0);
}

void exit_norm_4(){ 
	
	Printx("Thread 4 is running\n",22,1);
	Printx("Thread 4 is exiting\n",22,1);
	
	Exit(0);
}

void exit_norm_5(){ 
	
	Printx("Thread 5 is running\n",22,1);
	Printx("Thread 5 is exiting\n",22,1);
	
	Exit(0);
}

int
main()
{
	int i = 0;
    
    Fork(exit_norm_1, "", 0);
    
    /* Yield */
   	for(i = 0; i < 50; i++){ Yield(); }
   	
   	Fork(exit_norm_2, "", 0);
    
    /* Yield */
   	for(i = 0; i < 50; i++){ Yield(); }
   	
   	Fork(exit_norm_3, "", 0);
    
    /* Yield */
   	for(i = 0; i < 50; i++){ Yield(); }
   	
   	Fork(exit_norm_4, "", 0);
    
    /* Yield */
   	for(i = 0; i < 50; i++){ Yield(); }
   	
   	Fork(exit_norm_5, "", 0);
        
    /* not reached */
}
