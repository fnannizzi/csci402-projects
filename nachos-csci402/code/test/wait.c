#include "syscall.h"

int
main()
{
	int i = 0; 
    
    /*Exec("../test/wait_right_process",27);
    Exec("../test/wait_wrong_process",27);*/
    
    /* Yield to allow wrong process test to complete */
   	/*for(i = 0; i < 700; i++){ Yield(); }*/
    
    Exec("../test/wait_non_normative",27);
    
    /* Yield to allow non-normative test to complete */
   	/*for(i = 0; i < 700; i++){ Yield(); }
    
    Exec("../test/wait_destroy",21);*/
    
    /* Yield to allow destroy test to complete */
   	for(i = 0; i < 700; i++){ Yield(); }
    
    Exec("../test/wait_normative",23);
   
    Exit(0);
    /* not reached */
}
