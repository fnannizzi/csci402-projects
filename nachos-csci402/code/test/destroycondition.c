#include "syscall.h"

int
main()
{
	int i = 0;	
	
	Printx("----------------------------------\n",36,1);
    Printx("Beginning destroycondition test\n",34,1);
    Printx("----------------------------------\n",36,1);

	Exec("../test/destroycondition_right_process",39);
	
	/* Yield to allow other thread to create the CV first. */
	for(i = 0; i < 300; i++){ Yield(); }
	
    Exec("../test/destroycondition_wrong_process",39);
    
    Exit(0);
    /* not reached */
}
