#include "syscall.h"

int
main()
{
	int i = 0; 
    
    /*Exec("../test/broadcast_right_process",32);
    Exec("../test/broadcast_wrong_process",32);*/
    
    /* Yield to allow wrong process test to complete */
   	/*for(i = 0; i < 700; i++){ Yield(); }*/
    
    Exec("../test/broadcast_non_normative",32);
    
    /* Yield to allow non-normative test to complete */
   	/*for(i = 0; i < 700; i++){ Yield(); }
    
    /*Exec("../test/broadcast_destroy",26);
    
    /* Yield to allow destroy test to complete */
   	for(i = 0; i < 700; i++){ Yield(); }
    
    Exec("../test/broadcast_normative",28);
   
    Exit(0);
    /* not reached */
}
