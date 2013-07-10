#include "syscall.h"
int a[3];
int b, c;
int i;
void print0() {Printx("Test0\n",6,1);Exit(0);}
void print1() {Printx("Test1\n",6,1);Exit(0);}
void print2() {
Printx("Test2\n",6,1);
	for (i = 0; i < 100; i ++)
		Yield();
	Exit(0);
		}
int
main()
{

    Fork(print0,"Print0",6);
    Fork(print1,"Print1",6);
    Fork(print2,"Print2",6);
    

    /* not reached */
}
