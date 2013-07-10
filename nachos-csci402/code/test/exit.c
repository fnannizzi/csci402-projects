#include "syscall.h"

int
main()
{
	int i = 0; 
    
    Printx("----------------------------------\n",36,1);
    Printx("Beginning exit test 1\n",24,1);
    Printx("----------------------------------\n",36,1);
    
    Exec("../test/exit_test",18);
    
    /* Yield to allow first process test to complete */
   	for(i = 0; i < 300; i++){ Yield(); }
   	
   	Printx("----------------------------------\n",36,1);
    Printx("Beginning exit test 2\n",24,1);
    Printx("----------------------------------\n",36,1);
    
    Exec("../test/exit_test",18);
    
    /* Yield to allow first process test to complete */
   	for(i = 0; i < 300; i++){ Yield(); }
   	
   	Printx("----------------------------------\n",36,1);
    Printx("Beginning exit test 3\n",24,1);
    Printx("----------------------------------\n",36,1);
    
    Exec("../test/exit_test",18);
    
    /* Yield to allow first process test to complete */
   	for(i = 0; i < 300; i++){ Yield(); }
   	
   	Printx("----------------------------------\n",36,1);
    Printx("Beginning exit test 4\n",24,1);
    Printx("----------------------------------\n",36,1);
    
    Exec("../test/exit_test",18);
    
    /* Yield to allow first process test to complete */
   	for(i = 0; i < 300; i++){ Yield(); }
   	
   	Printx("----------------------------------\n",36,1);
    Printx("Beginning exit test 5\n",24,1);
    Printx("----------------------------------\n",36,1);
    
    Exec("../test/exit_test",18);

    Exit(0);
    /* not reached */
}
