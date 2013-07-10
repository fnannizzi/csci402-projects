#include "syscall.h"

int
main()
{
	int i = 0;
	for (i = 0; i < 5; i++){
        	Exec("../test/fork",12);
	}
	
    Exit(0);
    /* not reached */
}
