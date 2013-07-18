#include "syscall.h"
int a[3];
int b, c;



int
main()
{
    char* name = "mv";
    int mv;
    int i;
    
  	
   	Printx("Test is trying to destroy an MV at index -1\n",45,1);
   	mv = DestroyMV(-1);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mv*10000);	
   	}
   	
   	Printx("Test is trying to destroy an MV at index 501\n",46,1);
   	mv = DestroyMV(501);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mv*10000);	
   	}
   	
   	Printx("Test is creating an MV named mv\n",33,1);
   	mv = CreateMV(name, 5,1);
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
   	Printx("Test is trying to destroy an MV at index 0\n",44,1);
   	mv = DestroyMV(0);
   	if(mv == -1){
   		Printx("MV not destroyed\n",18,1);
   	}
   	else {
   		Printx("MV destroyed successfully at index %d%d\n",41,mv*10000);	
   	}
   	
   	
   	
    
    Exit(0);
    /* not reached */
}
