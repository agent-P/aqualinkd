/*
 * aqualink_time.c
 *
 *  Created on: Sep 22, 2012
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "aqualink.h"
#include "globals.h"

int check_aqualink_time(char* _date, char* _time)
{
    time_t now;
    struct tm *tmptr;
    int time_date_correct = FALSE;

    int a_hour;
    int a_min;
    int a_day;
    int a_month;
    int a_year;
    char a_pm[3];

    // Get the current time.
    time(&now);
    tmptr = localtime(&now);

    // Make sure time is at least 2 minutes apart to prevent HI/LO toggling.
    // Get the minutes for each time for a comparison.
    struct tm str_time;
    time_t aqualink_time;

    // Parse the time and date components to create a time value for
    // comparison to the current time.
    sscanf(_time, "%d:%d %s", &a_hour, &a_min, a_pm);
    sscanf(_date, "%d/%d/%d", &a_month, &a_day, &a_year);

    // Build a time structure from the Aqualink time.
    str_time.tm_year = a_year + 2000 - 1900;  // adjust for correct century
    str_time.tm_mon = a_month - 1;     // adjust or correct date 0-based index
    str_time.tm_mday = a_day;
    if(a_hour < 12 && (strstr(a_pm, "PM") != NULL)) {
    	str_time.tm_hour =  a_hour + 12;
    }
    else if(a_hour == 12 && (strstr(a_pm, "AM") != NULL)) {
    	str_time.tm_hour =  0;
    }
    else {
    	str_time.tm_hour = a_hour;
    }
    str_time.tm_min = a_min;
    str_time.tm_sec = 0;       // Aqualink time doesn't have seconds.
    str_time.tm_isdst = 0;     // always standard time in AZ

    // Make the Aqualink time value.
    aqualink_time = mktime(&str_time);
    //log_message(DEBUG, ctime(&now));
    //log_message(DEBUG, ctime(&aqualink_time));

    // Calculate the time difference between Aqualink time and local time.
    int time_difference = (int)difftime(now, aqualink_time);

    log_message(DEBUG, "sis", "Aqualink time is off by ",  time_difference, " seconds...");

    if(abs(time_difference) <= config_parameters.time_error) {
    	// Time difference is less than or equal to 90 seconds (1 1/2 minutes).
    	// Set the return value to TRUE.
    	time_date_correct = TRUE;
    }


	return time_date_correct;
}


/* Set the Aqualink RS8's time's year to the local time's year.
 *
 */
void set_aqualink_time_field(int field, char* field_string)
{
	log_message(DEBUG, "s", "Attempting to set aqualink time field...");

	int local_field;
	char local_mod[MAX_TIME_FIELD_LENGTH];
	int aqualink_field;
	char field_id[MAX_TIME_FIELD_LENGTH];
	char field_mod[MAX_TIME_FIELD_LENGTH];

	// Parse the local time field string.
	sscanf(field_string, "%d %s", &local_field, local_mod);
    log_message(DEBUG, "ss", "field string: ", field_string);

	if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[field]) != NULL) {
		 do {
			 // Parse the numeric field from the Aqualink data string.
			sscanf(aqualink_data.last_message, "%s %d %s", field_id, &aqualink_field, field_mod);

			if(strcmp(local_mod, "AM") == 0 || strcmp(local_mod, "PM") == 0) {
				if(strcmp(local_mod, field_mod) == 0 && local_field == aqualink_field) {
					// Field is set correctly. Send the ENTER command
					// to move to the next time field, and break out
					// of the loop.
					send_cmd("KEY_ENTER");
					break;
				}
				else {
					// Increment the field.
					send_cmd("KEY_RIGHT");
				}
			}
			else {
                log_message(DEBUG, "sisi", "current value: ", aqualink_field, "  -  set value: ", local_field);
				if(aqualink_field < local_field) {
					// Increment the field.
					send_cmd("KEY_RIGHT");
				}
				else if(aqualink_field > local_field) {
					// Decrement the field.
					send_cmd("KEY_LEFT");
				}
				else {
					// Field is set correctly. Send the ENTER command
					// to move to the next time field, and break out
					// of the loop.
					send_cmd("KEY_ENTER");
					break;
				}
			}

			// Wait one second.
			sleep(1);
		} while(aqualink_field != local_field);
	}
}


/* Set the Aqualink RS8's time to the local time.
 *
 */
void* set_aqualink_time(void* arg)
{
	log_message(INFO, "s", "Attempting to set aqualink time...");

	if(!PROGRAMMING) {
		// Set the flag to let the daemon know a program of multiple
		// commands is being sent to the Aqualink RS8.
		PROGRAMMING = TRUE;

		const int YEAR_LENGTH = 5;
		const int MONTH_LENGTH = 3;
		const int DAY_LENGTH = 3;
		const int HOUR_LENGTH = 6;
		const int MINUTE_LENGTH = 3;

		int set_time_mode = FALSE;

		time_t now;
		struct tm *tmptr;
		char year[YEAR_LENGTH];
		char month[MONTH_LENGTH];
		char day[DAY_LENGTH];
		char hour[HOUR_LENGTH];
		char minute[MINUTE_LENGTH];

		// Detach the thread so that it cleans up as it exits. Memory leak without it.
		pthread_detach(pthread_self());

		// Get the current time.
		time(&now);
		tmptr = localtime(&now);

		// Convert to time and date strings compatible with the Aqualink
		// time and date strings.
		strftime(year, YEAR_LENGTH, "%Y", tmptr);
		strftime(month, MONTH_LENGTH, "%m", tmptr);
		strftime(day, DAY_LENGTH, "%d", tmptr);
		strftime(hour, HOUR_LENGTH, "%I %p", tmptr);
		strftime(minute, MINUTE_LENGTH, "%M", tmptr);
        
        log_message(DEBUG, "ss", "year: ", year);

		// Select the MENU and wait 1 second to give the RS8 time to respond.
		send_cmd("KEY_MENU");
		sleep(1);

		// Select the SET TIME mode. Note only 10 attempts to enter the mode
		// are made.
		int iterations = 0;
		while(!set_time_mode && iterations < 10) {
			//log_message(DEBUG, aqualink_data.last_message);
			if(strstr(aqualink_data.last_message, "SET TIME") != NULL) {
				// We found SET TIME mode. Set the flag to break out of the
				// loop.
				set_time_mode = TRUE;

				// Enter SET TIME mode.
				send_cmd("KEY_ENTER");
				sleep(1);
			}
			else {
				// Next menu item and wait 1 second to give the RS8 time
				// to respond.
				send_cmd("KEY_RIGHT");
				sleep(1);
			}
			iterations++;
            
            // Check to see if the PROGRAMMING activity should be cancelled.
            // If so, clean up and break out of the loop.
            if(CANCEL_PROGRAMMING) {
                log_message(WARNING, "s", "Cancelling set_aqualink_time thread.");
                CANCEL_PROGRAMMING = FALSE;
                set_time_mode = FALSE;
                send_cmd("KEY_CANCEL");
                break;
            }
		}

		// If we are in set time mode, attempt to set the time, else the
		// function just falls through and returns. A warning is logged.
		if(set_time_mode) {

			while(set_time_mode) {
				if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[YEAR]) != NULL) {
					set_aqualink_time_field(YEAR, year);
				}
				else if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[MONTH]) != NULL) {
					set_aqualink_time_field(MONTH, month);
				}
				else if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[DAY]) != NULL) {
					set_aqualink_time_field(DAY, day);
				}
				else if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[HOUR]) != NULL) {
					set_aqualink_time_field(HOUR, hour);
				}
				else if(strstr(aqualink_data.last_message, TIME_FIELD_TEXT[MINUTE]) != NULL) {
					set_aqualink_time_field(MINUTE, minute);
				}
				else {
					set_time_mode = FALSE;
				}
				sleep(1);
			}
			log_message(INFO, "ssssssssss", "Aqualink time set to: ",
					year, "-",
					month, "-",
					day, "  ",
					hour, ":",
					minute
			);
		}
		else {
			log_message(WARNING, "s", "Could not enter SET TIME mode.");
		}

		// Reset the programming state flag.
		PROGRAMMING = FALSE;
	}
	else {
		// A multiple command program is currently being sent to the aqualink
		// controller. Just log it, and fall through to the function return.
		// Time set will be retried by the main program loop that checks that
		// aqualink time is correct.
		log_message(INFO, "s", "Aqualink Programming in progress. Could not set time.");
	}

	return 0;
}
