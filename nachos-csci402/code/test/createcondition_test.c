#include "syscall.h"

int
main()
{
    char* nameCV = "CV";
    int CV = 0, i = 0;
    
    Printx("Test is creating 500 CVs to fill up the table\n",48,1);
    for(i = 0; i < 500; i++){
    	CV = CreateCondition(nameCV,2);
    	if (CV == -1){
   			Printx("CV not created\n",16,1);
    	}
    	else {
    		Printx("CV created successfully at index %d%d\n",40,CV*10000);
    	}
    }
    
    Printx("\nTest is attempting to create one more CV\n",42,1);
    CV = CreateCondition(nameCV,2);
    if(CV == -1){
   		Printx("CV not created\n",16,1);
    }
    else {
    	Printx("CV created successfully at index %d%d\n",40,CV*10000);
    }
    
    Exit(0);
    /* not reached */
}
