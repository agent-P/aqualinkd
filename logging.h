/*
 * logging.h
 *
 *  Created on: Sep 29, 2012
 */

#ifndef LOGGING_H_
#define LOGGING_H_

// FUNCTION PROTOTYPES
void timestamp(char* time_string);
void log_to_syslog(int level, const char* format, ...);
void log_message(int level, const char* format, ...);
void set_logging_parameters(char* running_directory, int level);

#endif /* LOGGING_H_ */
