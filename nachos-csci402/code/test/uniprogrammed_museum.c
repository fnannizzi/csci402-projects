#include "syscall.h"

int
main()
{
	Exec("../test/museum", 15);
	
    Exit(0);
    /* not reached */
}
