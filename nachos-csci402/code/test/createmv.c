#include "syscall.h"
int a[3];
int b, c;



int
main()
{
    char* name = "mv";
    int mv;
    int i;
    
   	Printx("Test is creating an MV named mv with size 0\n",45,1);
   	mv = CreateMV(name, 5,0); /*First index is array length, should it be the second?*/
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
   	Printx("Test is creating an MV named mv with size 1\n",45,1);
   	mv = CreateMV(name, 5,1); /*First index is array length, should it be the second?*/
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
   	Printx("Test is creating another MV named mv with size 1\n",50,1);
   	mv = CreateMV(name, 5,1); /*First index is array length, should it be the second?*/
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
    
    Exit(0);
    /* not reached */
}
