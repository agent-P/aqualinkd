/*
 * globals.c
 *
 *  Created on: Sep 22, 2012
 */

#include "aqualink.h"

struct AQUALINK_DATA aqualink_data;
struct CONFIG_PARAMETERS config_parameters;

int PROGRAMMING = FALSE;
const char* DEVICE_STRINGS[] = {
		"08",
		"09",
		"0a",
		"0b",
		"10",
		"11",
		"12",
		"13",
		"18",
		"19",
		"1a",
		"1b",
		"20",
		"21",
		"22",
		"23"
};

const unsigned char DEVICE_CODES[] = {
		0x08,
		0x09,
		0x0a,
		0x0b,
		0x10,
		0x11,
		0x12,
		0x13,
		0x18,
		0x19,
		0x1a,
		0x1b,
		0x20,
		0x21,
		0x22,
		0x23
};

const char* KEY_CMD_TEXT[] = {
	"KEY_PUMP",
	"KEY_SPA",
	"KEY_AUX1",
	"KEY_AUX2",
	"KEY_AUX3",
	"KEY_AUX4",
	"KEY_AUX5",
	"KEY_AUX6",
	"KEY_AUX7",
	"KEY_HTR_POOL",
	"KEY_HTR_SPA",
	"KEY_HTR_SOLAR",
	"KEY_MENU",
	"KEY_CANCEL",
	"KEY_LEFT",
	"KEY_RIGHT",
	"KEY_HOLD",
	"KEY_OVERRIDE",
	"KEY_ENTER"
};

const unsigned char KEY_CODES[] = {
		0x02,
		0x01,
		0x05,
		0x0a,
		0x0f,
		0x06,
		0x0b,
		0x10,
		0x15,
		0x12,
		0x17,
		0x1c,
		0x09,
		0x0e,
		0x13,
		0x18,
		0x19,
		0x1e,
		0x1d
};

const unsigned char LED_STATES[2][16] = {
		{ 1,    1,    1,    0,    0,    2,    1,    2,    0,    3,    4,    4,    1,    3,    4,    4    },
		{ 0x10, 0x04, 0x01, 0x40, 0x10, 0x01, 0x40, 0x40, 0x01, 0x10, 0x01, 0x10, 0x20, 0x40, 0x04, 0x40 }
};

const char* LED_STATES_TEXT[] = {
		"off",
		"on",
		"enabled"
};

const char ROOT_NOUN[] = { "AqualinkRS8" };

const int LEDS_INDEX = 12;
const char* RS8_NOUNS[] = {
		"version",
		"date",
		"time",
		"temp_units",
		"air_temp",
		"pool_temp",
		"spa_temp",
		"battery",
		"pool_htr_set_pnt",
		"spa_htr_set_pnt",
		"freeze_protection",
		"frz_protect_set_pnt",
		"leds"
};

const char* LED_NOUNS[] = {
		"pump",
		"spa",
		"aux1",
		"aux2",
		"aux3",
		"aux4",
		"aux5",
		"aux6",
		"aux7",
		"pool_heater",
		"spa_heater",
		"solar_heater"
};

const char* TIME_FIELD_TEXT[] = {
		"YEAR",
		"MONTH",
		"DAY",
		"HOUR",
		"MINUTE"
};

const char* BATTERY_STATUS_TEXT[] = {
		"ok",
		"low"
};

const char* temp_units_strings[] = { "F", "C", "u" };

const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR" };

const char PROGRAM_NAME[] = "aqualinkd";

unsigned char aqualink_cmd = 0x00;

#define LABEL_LENGTH 32
char aux_function_labels[7][LABEL_LENGTH];

const int NUM_FUNCTION_LABELS = 7;
const char* LABEL_NOUNS[] = {
		"aux1_label",
		"aux2_label",
		"aux3_label",
		"aux4_label",
		"aux5_label",
		"aux6_label",
		"aux7_label"
};

const char* PARAM_TEXT[] = {
		"POOL_HTR",
		"SPA_HTR",
		"FRZ_PROTECT"
};
