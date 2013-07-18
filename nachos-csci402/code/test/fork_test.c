#include "syscall.h"
int a[3];
int b, c;
int i;

int main()
{
	int a;
	a = Exec("../test/fork",12);
	Exit(0);
}
