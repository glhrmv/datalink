/**
 * @file alarm.c
 * @brief The datalink program alarm header file
 *
 * This is the timer/alarm of the project.
 *
 *
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "alarm.h"
#include "util.h"

int alarm_flag = FALSE;
int timeout = 0;
int num_timeouts = 0;

void alarm_handler(int signal) {
	if (signal != SIGALRM)
		return;

	alarm_flag = TRUE;
	printf("Connection time out!\n\nRetrying:\n");

	num_timeouts++;
	alarm(timeout);
}

void init_alarm() {
	struct sigaction action;
	action.sa_handler = alarm_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarm_flag = FALSE;

	alarm(timeout);
}

void stop_alarm() {
	struct sigaction action;
	action.sa_handler = NULL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarm(0);
}