#include "syscall.h"

int
main()
{
	int i = 0;	
	int cv1, cv2;
	char* name = "condition1";
	
	Printx("----------------------------------\n",36,1);
    Printx("Beginning DestroyCondition test\n",34,1);
    Printx("----------------------------------\n",36,1);

	Printx("Test is attempting to destroy a condition that hasn't been created\n",69,1);
   	cv1 = DestroyCondition(0);
   	if(cv1 == -1){
   		Printx("Condition not destroyed\n",26,1);
   	}
   	else {
   		Printx("Condition destroyed successfully at index %d%d\n",49,0*10000);	
   	}
   	
   	Printx("----------------------------------\n",36,1);
   	
   	Printx("Test is creating a condition named condition1\n",48,1);
   	cv1 = CreateCondition(name, 11);
   	if(cv1 == -1){
   		Printx("Condition not created\n",24,1);
   	}
   	else {
   		Printx("Condition created successfully at index %d%d\n",47,cv1*10000);	
   	}
   	
   	Printx("----------------------------------\n",36,1);
   	
   	Printx("Test is destroying condition1\n",32,1);
   	cv1 = DestroyCondition(0);
   	if(cv1 == -1){
   		Printx("Condition not destroyed\n",26,1);
   	}
   	else {
   		Printx("Condition destroyed successfully at index %d%d\n",49,cv1*10000);	
   	}
   	
   	Printx("----------------------------------\n",36,1);

   	Printx("Test is attempting to destroy condition1 again\n",49,1);
   	cv2 = DestroyCondition(0);
   	if(cv2 == -1){
   		Printx("Condition not destroyed\n",26,1);
   	}
   	else {
   		Printx("Condition destroyed successfully at index %d%d\n",49,cv2*10000);	
   	}

	/*Exec("../test/destroycondition_right_process",39);
	
	/* Yield to allow other thread to create the CV first. */
	/*for(i = 0; i < 300; i++){ Yield(); }
	
    /*Exec("../test/destroycondition_wrong_process",39);*/
    
    Exit(0);
    /* not reached */
}
