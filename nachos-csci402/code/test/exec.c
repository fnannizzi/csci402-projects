#include "syscall.h"

int
main()
{
	int a;
	int i = 0;
	for (i = 0; i < 2; i++){
        	a = Exec("../test/matmult",15);
	}
	
    Exit(0);
    /* not reached */
}
