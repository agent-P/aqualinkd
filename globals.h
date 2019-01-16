/*
 * globals.h
 *
 *  Created on: Sep 22, 2012
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

extern struct AQUALINK_DATA aqualink_data;
extern struct CONFIG_PARAMETERS config_parameters;
extern struct FUNCTION_LABELS function_labels;
extern int PROGRAMMING;
extern int CANCEL_PROGRAMMING;
extern const char* DEVICE_STRINGS[];
extern const unsigned char DEVICE_CODES[];
extern const char* KEY_CMD_TEXT[];
extern const unsigned char KEY_CODES[];
extern const unsigned char LED_STATES[2][16];
extern const char* LED_STATES_TEXT[];
extern const char ROOT_NOUN[];
extern const int LEDS_INDEX;
extern const char* RS8_NOUNS[];
extern const char* LED_NOUNS[];
extern const char* LABEL_NOUNS[];
extern const int NUM_FUNCTION_LABELS;
extern const char* TIME_FIELD_TEXT[];
extern const char* BATTERY_STATUS_TEXT[];
extern const char* temp_units_strings[];
extern const char* level_strings[];
extern const char PROGRAM_NAME[];
extern unsigned char aqualink_cmd;
extern const char* PARAM_TEXT[];

extern int air_temp_error_low;
extern int air_temp_error_high;

#define LABEL_LENGTH 32
extern char aux_function_labels[][LABEL_LENGTH];

enum {
	AUX1_LABEL = 0,
	AUX2_LABEL,
	AUX3_LABEL,
	AUX4_LABEL,
	AUX5_LABEL,
	AUX6_LABEL,
	AUX7_LABEL,
};

void log_message(int level, const char* format, ...);
void daemon_shutdown();


#endif /* GLOBALS_H_ */
