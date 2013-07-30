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
    

   	
   	Printx("Test is setting a non-existent MV to be 5\n",43,1);
   	mv = SetMV(1,0,5);
   	if(mv == -1){
   		Printx("MV not set\n",12,1);
   	}
   	else {
   		Printx("MV was set to be 5\n",20,0);	
   	}
   	
   	Printx("\nTest is setting an invalid MV (-1) to be 5\n",46,1);
   	mv = SetMV(-11,0,5);
   	if(mv == -1){
   		Printx("MV not set\n",12,1);
   	}
   	else {
   		Printx("MV was set to be 5\n",20,0);	
   	}
   	
   	Printx("\nTest is setting an invalid MV (700) to be 5\n",47,1);
   	mv = SetMV(700,0,5);
   	if(mv == -1){
   		Printx("MV not set\n",12,1);
   	}
   	else {
   		Printx("MV was set to be 5\n",20,0);	
   	}
   	
   	Printx("\nTest is creating an MV named mv\n",35,1);
   	mv = CreateMV(name, 5,5);
   	mvNum = mv;
   	if(mv == -1){
   		Printx("MV not created\n",16,1);
   	}
   	else {
   		Printx("MV created successfully at index %d%d\n",38,mv*10000);	
   	}
   	
   	Printx("\nTest is setting mv[-1] to be 5\n",34,1);
   	mv = SetMV(mvNum,-1,5);
   	if(mv == -1){
   		Printx("mv[-1] not set\n",16,1);
   	}
   	else {
   		Printx("mv[-1] was set to be 5\n",24,0);	
   	}
   	
   	Printx("\nTest is setting mv[600] to be 5\n",35,1);
   	mv = SetMV(mvNum,600,5);
   	if(mv == -1){
   		Printx("mv[600] not set\n",17,1);
   	}
   	else {
   		Printx("mv[600] was set to be 5\n",25,0);	
   	}
   	
   	Printx("\nTest is setting mv[1] to be 5\n",33,1);
   	mv = SetMV(mvNum,1,5);
   	if(mv == -1){
   		Printx("mv[1] not set\n",15,1);
   	}
   	else {
   		Printx("mv[1] was set to be 5\n",23,0);	
   	}
   	
   	Printx("\nTest is getting mv[1]\n",25,1);
   	mv = GetMV(mvNum,1);
   	if(mv == -1){
   		Printx("mv[1] not gotten\n",18,1);
   	}
   	else {
   		Printx("mv[1] is %d%d\n",15,mv*10000);	
   	}
   	
    
    Exit(0);
    /* not reached */
}
