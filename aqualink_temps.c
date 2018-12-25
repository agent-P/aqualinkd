/*
 * aqualink_temps.c
 *
 *  Created on: Sep 23, 2012
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "aqualink.h"
#include "globals.h"
#include "aqualink_menu.h"

static void set_aqualink_temp_field(char* temperature) {
	int value;            // the value to set the set point to
	char description[5];  // description of the field (POOL, SPA, FRZ)
	int local_val;        // integer value of the current set point
	char degrees[3];      // the degrees and scale

	sscanf(temperature, "%d", &value);

	do {
		sscanf(aqualink_data.last_message, "%s %d%s", description, &local_val, degrees);

		if(value > local_val) {
			// Increment the field.
			send_cmd("KEY_RIGHT");
		}
		else if(value < local_val) {
			// Decrement the field.
			send_cmd("KEY_LEFT");
		}
		else {
			// Just send ENTER. We are at the right value.
			send_cmd("KEY_ENTER");
		}

		// sleep 1 second before next iteration
		sleep(1);

	} while(value != local_val);
}

void* get_pool_spa_htr_temps(void* arg)
{
	static int thread_running;

	// Detach the thread so that it cleans up as it exits. Memory leak without it.
	pthread_detach(pthread_self());

	if(!thread_running) {
		thread_running = TRUE;

		int done = FALSE;
		while(!done) {
			if(!PROGRAMMING) {
				// Set the flag to let the daemon know a program of multiple
				// commands is being sent to the Aqualink RS8.
				PROGRAMMING = TRUE;

				log_message(INFO, "s", "Retrieving pool and spa heater set points...");

				// Select REVIEW mode.
				int review_mode = select_menu_item("REVIEW");

				if(review_mode) {
					// Retrieve the pool and spa heater temperature set points.
					select_sub_menu_item("TEMP SET");

					// Wait 5 seconds for the messages to be sent from the master device.
					sleep(5);
				}
				else {
					// Log the failure and take it back to a known state.
					log_message(WARNING, "s", "Could not select REVIEW to retrieve heater set points.");
					cancel_menu();
				}

				// Reset the programming state flag.
				PROGRAMMING = FALSE;
				done = TRUE;
			}
			else {
				log_message(DEBUG, "s", "Get Pool and Spa Heater Set Points waiting to execute...");
				sleep(5);
			}
		}

		thread_running = FALSE;
	}
	else {
		log_message(WARNING, "s", "Get Pool and Spa Heater Set Points thread instance already running...");
	}

    return 0;
}


void* set_pool_htr_temp(void* arg)
{
	static int thread_running;

	// Detach the thread so that it cleans up as it exits. Memory leak without it.
	pthread_detach(pthread_self());

	if(!thread_running) {
		thread_running = TRUE;

		int done = FALSE;
		while(!done) {
			if(!PROGRAMMING) {
				// Set the flag to let the daemon know a program of multiple
				// commands is being sent to the Aqualink RS8.
				PROGRAMMING = TRUE;

				log_message(INFO, "s", "Setting the pool heater set point...");

				// Select SET TEMP mode.
				int set_temp_mode = select_menu_item("SET TEMP");

				if(set_temp_mode) {
					// Retrieve the pool and spa heater temperature set points.
					int set_pool_temp_mode = select_sub_menu_item("SET POOL TEMP");

					if(set_pool_temp_mode) {
						set_aqualink_temp_field(arg);
					}
				}
				else {
					// Log the failure and take it back to a known state.
					log_message(WARNING, "s", "Could not select SET TEMP to set heater set points.");
					cancel_menu();
				}

				// Reset the programming state flag.
				PROGRAMMING = FALSE;
				done = TRUE;
			}
			else {
				log_message(INFO, "s", "Set Pool Heater Set Point waiting to execute...");
				sleep(5);
			}
		}

		thread_running = FALSE;
	}
	else {
		log_message(WARNING, "s", "Set Pool Heater Set Point thread instance already running...");
	}

    return 0;
}


void* set_spa_htr_temp(void* arg)
{
	static int thread_running;

	// Detach the thread so that it cleans up as it exits. Memory leak without it.
	pthread_detach(pthread_self());

	if(!thread_running) {
		thread_running = TRUE;

		int done = FALSE;
		while(!done) {
			if(!PROGRAMMING) {
				// Set the flag to let the daemon know a program of multiple
				// commands is being sent to the Aqualink RS8.
				PROGRAMMING = TRUE;

				log_message(INFO, "s", "Setting the spa heater set point...");

				// Select SET TEMP mode.
				int set_temp_mode = select_menu_item("SET TEMP");

				if(set_temp_mode) {
					// Retrieve the pool and spa heater temperature set points.
					int set_spa_temp_mode = select_sub_menu_item("SET SPA TEMP");

					if(set_spa_temp_mode) {
						set_aqualink_temp_field(arg);
					}
				}
				else {
					// Log the failure and take it back to a known state.
					log_message(WARNING, "s", "Could not select SET TEMP to set heater set points.");
					cancel_menu();
				}

				// Reset the programming state flag.
				PROGRAMMING = FALSE;
				done = TRUE;
			}
			else {
				log_message(INFO, "s", "Set Spa Heater Set Point waiting to execute...");
				sleep(5);
			}
		}

		thread_running = FALSE;
	}
	else {
		log_message(WARNING, "s", "Set Spa Heater Set Point thread instance already running...");
	}

    return 0;
}



void* get_frz_protect_temp(void* arg)
{
	static int thread_running;

	// Detach the thread so that it cleans up as it exits. Memory leak without it.
	pthread_detach(pthread_self());

	if(!thread_running) {
		thread_running = TRUE;

		int done = FALSE;
		while(!done) {
			if(!PROGRAMMING) {
				// Set the flag to let the daemon know a program of multiple
				// commands is being sent to the Aqualink RS8.
				PROGRAMMING = TRUE;

				log_message(INFO, "s", "Retrieving freeze protection set point...");

				// Select REVIEW mode.
				int review_mode = select_menu_item("REVIEW");

				if(review_mode) {
					// Retrieve the pool and spa heater temperature set points.
					select_sub_menu_item("FRZ PROTECT");

					// Wait 5 seconds for the messages to be sent from the master device.
					sleep(5);
				}
				else {
					// Log the failure and take it back to a known state.
					log_message(WARNING, "s", "Could not select REVIEW to retrieve freeze protection set point.");
					cancel_menu();
				}

				// Reset the programming state flag.
				PROGRAMMING = FALSE;
				done = TRUE;
			}
			else {
				log_message(DEBUG, "s", "Get Freeze Protect Set Point waiting to execute...");
				sleep(5);
			}
		}

		thread_running = FALSE;
	}
	else {
		log_message(WARNING, "s", "Get Pool and Spa Heater Set Points thread instance already running...");
	}

	return 0;
}


void* set_frz_protect_temp(void* arg)
{
	static int thread_running;

	// Detach the thread so that it cleans up as it exits. Memory leak without it.
	pthread_detach(pthread_self());

	if(!thread_running) {
		thread_running = TRUE;

		int done = FALSE;
		while(!done) {
			if(!PROGRAMMING) {
				// Set the flag to let the daemon know a program of multiple
				// commands is being sent to the Aqualink RS8.
				PROGRAMMING = TRUE;

				log_message(INFO, "s", "Setting the freeze protection set point...");

				// Select SET TEMP mode.
				int set_temp_mode = select_menu_item("FRZ PROTECT");

				if(set_temp_mode) {
					// Retrieve the pool and spa heater temperature set points.
					int set_frz_protect_temp_mode = select_sub_menu_item("TEMP SETTING");

					if(set_frz_protect_temp_mode) {
						set_aqualink_temp_field(arg);
					}
				}
				else {
					// Log the failure and take it back to a known state.
					log_message(WARNING, "s", "Could not select SET TEMP to set heater set points.");
					cancel_menu();
				}

				// Reset the programming state flag.
				PROGRAMMING = FALSE;
				done = TRUE;
			}
			else {
				log_message(INFO, "s", "Set Freeze Protection Set Point waiting to execute...");
				sleep(5);
			}
		}

		thread_running = FALSE;
	}
	else {
		log_message(WARNING, "s", "Set Freeze Protection Set Point thread instance already running...");
	}

    return 0;
}


