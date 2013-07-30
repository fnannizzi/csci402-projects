#include "syscall.h"

int lock, CV;

int
main()
{
	char* nameLock = "lock";
	char* nameCV = "CV";
	int i = 0;
	
	lock = CreateLock(nameLock, 4);
	CV = CreateCondition(nameCV, 2);
	
	Printx("----------------------------------\n",36,1);
    Printx("Beginning wrong process test\n",31,1);
    Printx("----------------------------------\n",36,1);
    
    Printx("Created lock and CV in owner process\n",39,1);
        
    /* not reached */
}
