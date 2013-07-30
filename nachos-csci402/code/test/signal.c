#include "syscall.h"

int
main()
{
	int i = 0; 
    
    /*Exec("../test/signal_right_process",29);
    Exec("../test/signal_wrong_process",29);*/
    
    /* Yield to allow wrong process test to complete */
   	/*for(i = 0; i < 500; i++){ Yield(); }*/
    
    Exec("../test/signal_non_normative",29);
    
    /* Yield to allow non-normative test to complete */
   	for(i = 0; i < 500; i++){ Yield(); }
    
    /*Exec("../test/signal_destroy",23);*/
    
    /* Yield to allow destroy test to complete */
   	/*for(i = 0; i < 500; i++){ Yield(); }*/
    
    Exec("../test/signal_normative",25);
   
    Exit(0);
    /* not reached */
}
