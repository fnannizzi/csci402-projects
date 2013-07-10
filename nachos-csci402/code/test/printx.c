#include "syscall.h"

int
main()
{
	Exec("../test/printx_test", 20);
	
    Exit(0);
    /* not reached */
}
