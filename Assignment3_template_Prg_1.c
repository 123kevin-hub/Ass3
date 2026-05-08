/******************************************************************************* 
The assignment 3 for subject 48450 (RTOS) in University of Technology Sydney(UTS) 
This is a template of Program_1.c template. Please complete the code based on 
the assignment 3 requirement. Assignment 3 

------------------------------Program_1.c template------------------------------
*******************************************************************************/

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>	/* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

typedef struct RR_Params {
  //add your variables here
  
} ThreadParams;



/* this function calculates Round Robin (RR) with a time quantum of 4, writes waiting time and turn-around time to the FIFO */
void *worker1(void *params)
{
   // add your code here
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2()
{
   // add your code here
}

/* this main function creates named pipe and threads */
int main(int argc, char* argv[])
{
	/* creating a named pipe(RR) with read/write permission */
	// add your code 

	/* initialize the parameters */
	 // add your code 
	
	/* create threads */
	 // add your code
	
	/* wait for the thread to exit */
	//add your code
	
	return 0;
}
