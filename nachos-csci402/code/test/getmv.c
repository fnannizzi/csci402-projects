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
    

   	Printx("Test is getting uninitialized MV\n",34,1);
   	mv = GetMV(0,1);
   	if(mv == -1){
   		Printx("MV not gotten\n",15,1);
   	}
   	else {
   		Printx("MV is %d%d\n",12,mv*10000);	
   	}  
   	
   	Printx("\nTest is getting invalid MV (-1)\n",35,1);
   	mv = GetMV(-1,1);
   	if(mv == -1){
   		Printx("MV not gotten\n",15,1);
   	}
   	else {
   		Printx("MV is %d%d\n",12,mv*10000);	
   	}    
   	
   	Printx("\nTest is getting invalid MV (550)\n",36,1);
   	mv = GetMV(550,1);
   	if(mv == -1){
   		Printx("MV not gotten\n",15,1);
   	}
   	else {
   		Printx("MV is %d%d\n",12,mv*10000);	
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
   	
   	Printx("\nTest is setting mv[1] to be 5\n",33,1);
   	mv = SetMV(mvNum,1,5);
   	if(mv == -1){
   		Printx("mv[1] not set\n",15,1);
   	}
   	else {
   		Printx("mv[1] was set to be 5\n",23,0);	
   	}
   	
   	
   	Printx("\nTest is getting mv[-1]\n",26,1);
   	mv = GetMV(mvNum,-1);
   	if(mv == -1){
   		Printx("mv[-1] not gotten\n",19,1);
   	}
   	else {
   		Printx("mv[-1] is %d%d\n",16,mv*10000);	
   	}
   	
   	Printx("\nTest is getting mv[590]\n",27,1);
   	mv = GetMV(mvNum,590);
   	if(mv == -1){
   		Printx("mv[590] not gotten\n",20,1);
   	}
   	else {
   		Printx("mv[590] is %d%d\n",17,mv*10000);	
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
