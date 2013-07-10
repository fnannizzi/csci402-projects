#include "syscall.h"

int
main()
{
    int i = 0;
    
    Printx("----------------------------------\n",36,1); 
    Printx("Beginning yield test\n",23,1);
    Printx("----------------------------------\n",36,1); 
    
    /* Yield 100 times*/
   	for(i = 0; i < 100; i++){ 
   		Yield();
   		Printx("Yield: %d\n",12,i*10000000);
   	}
    
    Exit(0);
    /* not reached */
}
