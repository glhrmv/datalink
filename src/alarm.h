/**
 * @file alarm.h
 * @brief The datalink program alarm header file
 *
 * This is the timer/alarm of the project.
 *
 * Ensures that files are retransmitted if any
 * connection problems occur.
 *
 */
#pragma once

extern int alarm_flag;
extern int timeout;

void alarm_handler(int signal);

void init_alarm();

void stop_alarm();