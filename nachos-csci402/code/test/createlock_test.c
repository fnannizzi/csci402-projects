#include "syscall.h"

int
main()
{
	Exec("../test/createlock",19);
	/*Exec("../test/createlock2",20);*/
    
    Exit(0);
    /* not reached */
}

