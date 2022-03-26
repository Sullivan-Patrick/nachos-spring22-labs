/* halt.c
 *	Simple program to test whether running a user program works.
 *
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

// int
// main()
// {
//     Halt();
//     /* not reached */
// }
#include "syscall.h"

int global_cnt=0;

void sum(){

	int i;

	for (i=0;i<100;i++)
	{
	    global_cnt++;
	}
	Exit(i);
}

int main()
{
	int i=0;

	global_cnt++;

	Fork(sum);
	Yield();


	global_cnt++;
	Fork(sum);

	Yield();

	global_cnt++;
	Fork(sum);

	Yield();

	global_cnt++;
	Exit(global_cnt);
}
