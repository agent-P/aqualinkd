/*
 * json_messages.c
 *
 *  Created on: Dec 20, 2012
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aqualink.h"
#include "globals.h"
#include "logging.h"

char rs8_data[NUM_RS8_NOUNS][MSGLONGLEN];
char led_data[NUM_LED_NOUNS][MAX_LED_STAT_LENGTH];


int build_aux_labels_JSON(char* json_buffer)
{
	char _buffer[256];

	strcpy(json_buffer, "{\"type\": \"aux_labels\"");

	int i;
	for(i=0; i<NUM_FUNCTION_LABELS; i++) {
		strcat(json_buffer, ",");
		sprintf(_buffer, "\"%s\": \"%s\"", LABEL_NOUNS[i], aux_function_labels[i]);
		strcat(json_buffer, _buffer);
	}

	strcat(json_buffer, "}");

	return strlen(json_buffer);
}


int get_led_status(int led_id)
{
	int status = OFF;

	switch(led_id) {
	case STAT_PUMP:
		if((aqualink_data.status[LED_STATES[STAT_BYTE][led_id]] & LED_STATES[STAT_MASK][led_id]) != 0) {
			status = ON;
		}
		else if(aqualink_data.status[LED_STATES[STAT_BYTE][led_id+12]] & LED_STATES[STAT_MASK][led_id+12]) {
			status = ENABLED;
		}
		break;
	case STAT_HTR_POOL_ON:
	case STAT_HTR_SPA_ON:
	case STAT_HTR_SOLAR_ON:
		if((aqualink_data.status[LED_STATES[STAT_BYTE][led_id]] & LED_STATES[STAT_MASK][led_id]) != 0) {
			status = ON;
		}
		else if(aqualink_data.status[LED_STATES[STAT_BYTE][led_id+4]] & LED_STATES[STAT_MASK][led_id+4]) {
			status = ENABLED;
		}
		break;
	case STAT_SPA:
	case STAT_AUX1:
	case STAT_AUX2:
	case STAT_AUX3:
	case STAT_AUX4:
	case STAT_AUX5:
	case STAT_AUX6:
	case STAT_AUX7:
		if((aqualink_data.status[LED_STATES[STAT_BYTE][led_id]] & LED_STATES[STAT_MASK][led_id]) != 0) {
			status = ON;
		}
		break;
	default:
		break;
	}

	return status;
}


char* build_leds_JSON(char* buffer)
{
	char _buffer[64];

	strcpy(buffer, "{");

	int i;
	for(i=0; i<NUM_LED_NOUNS; i++) {
		if(i != 0) {
			strcat(buffer, ",");
		}
		sprintf(_buffer, "\"%s\": \"%s\"", LED_NOUNS[i], led_data[i]);
		strcat(buffer, _buffer);
	}

	strcat(buffer, "}");

	return buffer;
}


int build_RS8_JSON(char* buffer)
{
	char _buffer[256];

	strcpy(buffer, "{\"type\": \"status\"");

	int i;
	for(i=0; i<NUM_RS8_NOUNS-1; i++) {
		strcat(buffer, ",");
		sprintf(_buffer, "\"%s\": \"%s\"", RS8_NOUNS[i], rs8_data[i]);
		strcat(buffer, _buffer);
	}

	sprintf(_buffer, ",\"%s\": ", RS8_NOUNS[LEDS_INDEX]);
	strcat(buffer, _buffer);

	strcat(buffer, build_leds_JSON(_buffer));

	strcat(buffer, "}\0");

	return strlen(buffer);
}


// Convert the raw Aqualink data into strings that can be built
// into a JSON string.
void convert_aqualink_data()
{
	if(strlen(aqualink_data.version) + 1 > MSGLONGLEN) {
		log_message(WARNING, "ss", "Version string too long: ", aqualink_data.version);
	}
	else {
		strcpy(rs8_data[VERSION_NOUN], aqualink_data.version);
	}

	if(strlen(aqualink_data.date) + 1 > MSGLONGLEN) {
		log_message(WARNING, "ss", "Date string too long: ", aqualink_data.date);
	}
	else {
		strcpy(rs8_data[DATE_NOUN], aqualink_data.date);
	}

	if(strlen(aqualink_data.time) + 1 > MSGLONGLEN) {
		log_message(WARNING, "ss", "Time string too long: ", aqualink_data.time);
	}
	else {
		strcpy(rs8_data[TIME_NOUN], aqualink_data.time);
	}

//	if(strlen(aqualink_data.last_message) + 1 > MSGLONGLEN) {
//		log_message(WARNING, "ss", "Last Message string too long: ", aqualink_data.last_message);
//	}
//	else {
//		strcpy(rs8_data[LAST_MESSAGE_NOUN], aqualink_data.last_message);
//	}

	if(aqualink_data.temp_units >= FAHRENHEIT && aqualink_data.temp_units <= UNKNOWN) {
		strcpy(rs8_data[TEMP_UNITS_NOUN], temp_units_strings[aqualink_data.temp_units]);
	}
	else {
		log_message(WARNING, "sis", "Temperature units value, ", aqualink_data.temp_units, " invalid.");
	}

	sprintf(rs8_data[AIR_TEMP_NOUN], "%d", aqualink_data.air_temp);
	if(aqualink_data.pool_temp == -999) {
		sprintf(rs8_data[POOL_TEMP_NOUN], "%s", " ");
	}
	else {
		sprintf(rs8_data[POOL_TEMP_NOUN], "%d", aqualink_data.pool_temp);
	}
	if(aqualink_data.spa_temp == -999) {
		sprintf(rs8_data[SPA_TEMP_NOUN], "%s", " ");
	}
	else {
		sprintf(rs8_data[SPA_TEMP_NOUN], "%d", aqualink_data.spa_temp);
	}

	if(aqualink_data.battery >= OK && aqualink_data.battery <= LOW) {
		strcpy(rs8_data[BATTERY_NOUN], BATTERY_STATUS_TEXT[aqualink_data.battery]);
	}
	else {
		log_message(WARNING, "sis", "Battery Status value, ", aqualink_data.battery, " invalid.");
	}

	int i;
	for(i=0; i<NUM_LED_NOUNS; i++) {
		int state_value = get_led_status(i);
		if(state_value >= OFF && state_value <= ENABLED) {
			strcpy(led_data[i], LED_STATES_TEXT[state_value]);
		}
		else {
			log_message(WARNING, "sis", "LED State value, ", state_value, " invalid.");
		}
	}

	sprintf(rs8_data[POOL_HTR_TEMP_NOUN], "%d", aqualink_data.pool_htr_set_point);
	sprintf(rs8_data[SPA_HTR_TEMP_NOUN], "%d", aqualink_data.spa_htr_set_point);
	sprintf(rs8_data[FRZ_PROTECT_TEMP_NOUN], "%d", aqualink_data.frz_protect_set_point);

	if(aqualink_data.freeze_protection >= OFF  &&  aqualink_data.freeze_protection <= ENABLED) {
		strcpy(rs8_data[FRZ_PROTECT_NOUN],  LED_STATES_TEXT[aqualink_data.freeze_protection]);
	}
	else {
		log_message(WARNING, "sis", "Freeze Protection State value, ", aqualink_data.freeze_protection, " invalid.");
	}

}
