/*
 * logging.c
 *
 *  Created on: Sep 29, 2012
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include "globals.h"

const int MAX_MESSAGE_LENGTH = 256;
const char LOG_FILE[] = {"aqualinkd.log"};

char log_filename[256];
int log_level;

void set_logging_parameters(char* running_directory, int level)
{
	strcpy(log_filename, running_directory);
	strcat(log_filename, "/log/");
	strcat(log_filename, LOG_FILE);

	log_level = level;
}


// Get a timestamp with the current local time.
// time_string - the string to put the timestamp in
//
void timestamp(char* time_string)
{
    time_t now;
    struct tm *tmptr;

    time(&now);
    tmptr = localtime(&now);
    strftime(time_string, MAX_MESSAGE_LENGTH, "%b-%d-%y %H:%M:%S %p ", tmptr);
}


// Logs a message to a syslog file building the message string
// from a variable list of arguments.
//
// int level - the log level (LOG_DEBUG, LOG_INFO, LOG_NOTICE, LOG_WARNING, LOG_ERR)
// const char* format - the format string for the remaining arguments:
//                     (s = string, i = integer, f = float or double)
// ... - a variable list of arguments (can be string, int, or float)
void log_to_syslog(int level, const char* format, ...)
{
	char message_buffer[1024];
	char temp_buffer[1024];


	va_list arguments;
	va_start(arguments, format);

	strcpy(message_buffer, "");
	int i;
	for(i=0; format[i]!='\0'; i++) {
        if(format[i] == 'f')
        {
              double FArg = va_arg(arguments, double);
              sprintf(temp_buffer, "%.3lf", FArg);
              strcat(message_buffer, temp_buffer);
        }
        else if(format[i] == 'i')
        {
              int IArg=va_arg(arguments, int);
              sprintf(temp_buffer, "%d", IArg);
              strcat(message_buffer, temp_buffer);
        }
        else if(format[i] == 's') {
        	char *str = va_arg (arguments, char *);
            sprintf(temp_buffer, "%s", str);
            strcat(message_buffer, temp_buffer);
        }
        else {
        	sprintf(message_buffer, "Invalid argument type for log message...");
        }
	}
    va_end(arguments);

	openlog(PROGRAM_NAME,  LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	syslog(level, message_buffer);

	closelog();
}

// Logs a message to a private log file building the message string
// from a variable list of arguments.
//
// int level - the log level (DEBUG, INFO, WARNING, ERROR
// const char* format - the format string for the remaining arguments:
//                     (s = string, i = integer, f = float or double)
// ... - a variable list of arguments (can be string, int, or float)
void log_message(int level, const char* format, ...)
{
	if (level >= log_level) {
		// At or above the configured log level. Process the message.
		char message_buffer[1024];
		char temp_buffer[1024];

		va_list arguments;
		va_start(arguments, format);

		strcpy(message_buffer, "");
		int i;
		for(i=0; format[i]!='\0'; i++) {
			if(format[i] == 'f')
			{
				double FArg = va_arg(arguments, double);
				sprintf(temp_buffer, "%.3lf", FArg);
				strcat(message_buffer, temp_buffer);
			}
			else if(format[i] == 'i')
			{
				int IArg = va_arg(arguments, int);
				sprintf(temp_buffer, "%d", IArg);
				strcat(message_buffer, temp_buffer);
			}
			else if(format[i] == 's') {
				char *str = va_arg(arguments, char *);
				sprintf(temp_buffer, "%s", str);
				strcat(message_buffer, temp_buffer);
			}
	        else {
	        	sprintf(message_buffer, "Invalid argument type for log message...");
	        }
		}
		va_end(arguments);


		char time_string[MAX_MESSAGE_LENGTH];
		FILE *logfile;

		// Attempt to open the private log file.
		logfile = fopen(log_filename, "a");
		if(!logfile) {
			// Couldn't open the log file. Log log filename to system log and return.
			log_to_syslog(LOG_ERR, "ss", "Couldn't open private log file:  ", log_filename);
		}
		else {
			// Else the log file was opened successfully. Get the timestamp, and log the message.
			timestamp(time_string);
			fprintf(logfile,"%s- %s: %s\n", time_string, level_strings[level], message_buffer);
			fclose(logfile);
		}
	}
}
