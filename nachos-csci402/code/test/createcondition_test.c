#include "syscall.h"

int
main()
{
    char* name = "condition1";
    int cv1, cv2, cv3;
    
    Printx("----------------------------------\n",36,1);
    Printx("Beginning CreateCondition test.\n",34,1); 
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
   	
   	Printx("Test is attempting to create another condition named condition1\n",66,1);
   	cv2 = CreateCondition(name, 11);
   	if(cv2 == -1){
   		Printx("Condition not created\n",24,1);
   	}
   	else if(cv2 == cv1){
   		Printx("Condition already exists at index %d%d\n",41,cv2*10000);
   	}
   	else {
   		Printx("Condition created successfully at index %d%d\n",47,cv2*10000);	
   	}
   	
   	Printx("----------------------------------\n",36,1);
   	
   	name = "condition2";
   	Printx("Test is creating another condition named condition2\n",54,1);
   	cv3 = CreateCondition(name, 11);
   	if(cv3 == -1){
   		Printx("Condition not created\n",24,1);
   	}
   	else {
   		Printx("Condition created successfully at index %d%d\n",47,cv3*10000);	
   	}
    
    Exit(0);
    /* not reached */
}
