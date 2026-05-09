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
  int quantum_ms;
  char output_file[256];
  char fifo_name[256];
} ThreadParams;

static ThreadParams g_params;

static const int PROCESS_COUNT = 7;

static int p_arrival[7] = {8, 10, 14, 9, 16, 21, 26};
static int p_burst[7] = {10, 3, 7, 5, 4, 6, 2};

static int p_remaining[7];
static int p_completion[7];
static bool p_added[7];

static const int QUEUE_SIZE = 20;
static int ready_queue[20];
static int q_head = 0;
static int q_tail = 0;
static int q_count = 0;

static void init_data()
{
	for (int i = 0; i < PROCESS_COUNT; i++) {
		p_remaining[i] = p_burst[i];
		p_completion[i] = -1;
		p_added[i] = false;
	}
	q_head = 0;
	q_tail = 0;
	q_count = 0;
}

static void queue_push(int process_index)
{
	if (q_count >= QUEUE_SIZE) {
		return;
	}
	ready_queue[q_tail] = process_index;
	q_tail++;
	if (q_tail >= QUEUE_SIZE) {
		q_tail = 0;
	}
	q_count++;
}

static bool queue_pop(int *out_index)
{
	if (q_count <= 0) {
		return false;
	}
	*out_index = ready_queue[q_head];
	q_head++;
	if (q_head >= QUEUE_SIZE) {
		q_head = 0;
	}
	q_count--;
	return true;
}

static void add_arrivals(int from_t, int to_t)
{
	for (int t = from_t; t <= to_t; t++) {
		for (int i = 0; i < PROCESS_COUNT; i++) {
			if (!p_added[i] && p_arrival[i] == t) {
				queue_push(i);
				p_added[i] = true;
			}
		}
	}
}

static void rr_run(int quantum_ms, double *avg_waiting, double *avg_turnaround)
{
	init_data();

	int time = 0;
	int completed = 0;

	while (completed < PROCESS_COUNT) {
		add_arrivals(time, time);

		if (q_count == 0) {
			time++;
			continue;
		}

		int current = -1;
		if (!queue_pop(&current)) {
			continue;
		}

		int start_time = time;
		int run_time = p_remaining[current] < quantum_ms ? p_remaining[current] : quantum_ms;
		int end_time = start_time + run_time;

		if (start_time + 1 <= end_time - 1) {
			add_arrivals(start_time + 1, end_time - 1);
		}

		time = end_time;
		p_remaining[current] -= run_time;

		if (p_remaining[current] == 0) {
			p_completion[current] = time;
			completed++;
		} else {
			queue_push(current);
		}

		add_arrivals(time, time);
	}

	double total_waiting = 0.0;
	double total_turnaround = 0.0;

	for (int i = 0; i < PROCESS_COUNT; i++) {
		int turnaround = p_completion[i] - p_arrival[i];
		int waiting = turnaround - p_burst[i];
		total_turnaround += (double)turnaround;
		total_waiting += (double)waiting;
	}

	*avg_waiting = total_waiting / (double)PROCESS_COUNT;
	*avg_turnaround = total_turnaround / (double)PROCESS_COUNT;
}

static bool fifo_write(const char *fifo_name, double avg_waiting, double avg_turnaround)
{
	int fd = open(fifo_name, O_WRONLY);
	if (fd < 0) {
		return false;
	}

	char buf[128];
	int len = snprintf(buf, sizeof(buf), "%.6f %.6f\n", avg_waiting, avg_turnaround);
	if (len <= 0) {
		close(fd);
		return false;
	}

	write(fd, buf, (size_t)len);
	close(fd);
	return true;
}

static bool fifo_read(const char *fifo_name, double *avg_waiting, double *avg_turnaround)
{
	int fd = open(fifo_name, O_RDONLY);
	if (fd < 0) {
		return false;
	}

	char buf[128];
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0) {
		return false;
	}
	buf[n] = '\0';

	if (sscanf(buf, "%lf %lf", avg_waiting, avg_turnaround) != 2) {
		return false;
	}
	return true;
}

/* this function calculates Round Robin (RR) with a time quantum of 4, writes waiting time and turn-around time to the FIFO */
void *worker1(void *params)
{
   // add your code here
	ThreadParams *p = (ThreadParams *)params;

	double avg_waiting = 0.0;
	double avg_turnaround = 0.0;

	rr_run(p->quantum_ms, &avg_waiting, &avg_turnaround);
	fifo_write(p->fifo_name, avg_waiting, avg_turnaround);
	return NULL;
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2()
{
   // add your code here
	double avg_waiting = 0.0;
	double avg_turnaround = 0.0;

	if (!fifo_read(g_params.fifo_name, &avg_waiting, &avg_turnaround)) {
		return NULL;
	}

	FILE *fp = fopen(g_params.output_file, "w");
	if (fp == NULL) {
		return NULL;
}

fprintf(fp, "Average waiting time: %.6f\n", avg_waiting);
	fprintf(fp, "Average turn-around time: %.6f\n", avg_turnaround);
	fclose(fp);
	return NULL;
}

/* this main function creates named pipe and threads */
int main(int argc, char* argv[])
{
	/* creating a named pipe(RR) with read/write permission */
	// add your code 
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <time_quantum_ms> <output_file>\n", argv[0]);
		return 1;
	}

	memset(&g_params, 0, sizeof(g_params));
	g_params.quantum_ms = atoi(argv[1]);
	strncpy(g_params.output_file, argv[2], sizeof(g_params.output_file) - 1);
	strncpy(g_params.fifo_name, "RR", sizeof(g_params.fifo_name) - 1);

	if (g_params.quantum_ms <= 0) {
		fprintf(stderr, "Invalid time quantum\n");
		return 1;
	}

	unlink(g_params.fifo_name);
	if (mkfifo(g_params.fifo_name, 0666) != 0) {
		return 1;
	}
	/* initialize the parameters */
	 // add your code 
	
	/* create threads */
	 // add your code
	 pthread_t thread1;
	pthread_t thread2;

	if (pthread_create(&thread2, NULL, worker2, NULL) != 0) {
		unlink(g_params.fifo_name);
		return 1;
	}

	if (pthread_create(&thread1, NULL, worker1, &g_params) != 0) {
		pthread_join(thread2, NULL);
		unlink(g_params.fifo_name);
		return 1;
	}
	
 	/* wait for the thread to exit */
	//add your code
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	unlink(g_params.fifo_name);
	
	return 0;
}
