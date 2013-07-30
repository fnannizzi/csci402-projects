#include "syscall.h"
int a[3];
int b, c;



int
main()
{
    char* name = "mv";
    int mv;
    int mvNum;
    int i;
    
  	
   	Printx("Test is trying to destroy an MV at index -1\n",45,1);
   	mv = DestroyMV(-1);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mv*10000);	
   	}
   	
   	Printx("\nTest is trying to destroy an MV at index 5001\n",49,1);
   	mv = DestroyMV(5001);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mv*10000);	
   	}
   	
   	Printx("\nTest is creating an MV named mv\n",35,1);
   	mv = CreateMV(name, 5,1);
   	mvNum = mv;
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
   	Printx("\nTest is trying to destroy an MV at index %d%d\n",49,mvNum*10000);
   	mv = DestroyMV(mvNum);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mvNum*10000);	
   	}
   	
   	Printx("\nTest is setting mv[1] to be 5\n",33,1);
   	mv = SetMV(mvNum,1,5);
   	if(mv == -1){
   		Printx("mv[1] not set since it has been destroyed\n",43,1);
   	}
   	else {
   		Printx("mv[1] was set to be 5\n",23,0);	
   	}
   	
   	
   	
    
    Exit(0);
    /* not reached */
}
