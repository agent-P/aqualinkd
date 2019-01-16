/*
 * aqualink.h
 *
 *  Created on: Aug 17, 2012
 */

#ifndef AQUALINK_H_
#define AQUALINK_H_

// packet offsets
#define PKT_DEST        2
#define PKT_CMD         3
#define PKT_DATA        4

// DEVICE CODES
// devices probed by master are 08-0b, 10-13, 18-1b, 20-23,
#define DEV_MASTER      0

#define NUM_DEVICES     16

// COMMAND KEYS
enum {
	KEY_PUMP = 0,
	KEY_SPA,
	KEY_AUX1,
	KEY_AUX2,
	KEY_AUX3,
	KEY_AUX4,
	KEY_AUX5,
	KEY_AUX6,
	KEY_AUX7,
	KEY_HTR_POOL,
	KEY_HTR_SPA,
	KEY_HTR_SOLAR,
	KEY_MENU,
	KEY_CANCEL,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_HOLD,
	KEY_OVERRIDE,
	KEY_ENTER
};

#define NUM_KEYS   19

/* COMMANDS */
#define CMD_PROBE       0x00
#define CMD_ACK         0x01
#define CMD_STATUS      0x02
#define CMD_MSG         0x03
#define CMD_MSG_LONG    0x04

/*
 CMD_COMMAND data is:
 <status> <keypress>
 status is 0 if idle, 1 if display is busy
 keypress is 0, or a keypress code
 CMD_STATUS is sent in response to all probes from DEV_MASTER
 DEV_MASTER continuously sends CMD_COMMAND probes for all devices
 until it discovers a particular device.

 CMD_STATUS data is 5 bytes long bitmask
 defined as STAT_* below

 CMD_MSG data is <line> followed by <msg>
 <msg> is ASCII message up to 16 chars (or null terminated).
 <line> is NUL if single line message, else
 1 meaning it is first line of multi-line message,
 if so, next two lines come as CMD_MSG_LONG with next byte being
 2 or 3 depending on second or third line of message.
 */

enum {
	STAT_PUMP = 0,
	STAT_SPA,
	STAT_AUX1,
	STAT_AUX2,
	STAT_AUX3,
	STAT_AUX4,
	STAT_AUX5,
	STAT_AUX6,
	STAT_AUX7,
	STAT_HTR_POOL_ON,
	STAT_HTR_SPA_ON,
	STAT_HTR_SOLAR_ON,
	STAT_PUMP_BLINK,
	STAT_HTR_POOL_EN,
	STAT_HTR_SPA_EN,
	STAT_HTR_SOLAR_EN
};

enum {
	STAT_BYTE = 0,
	STAT_MASK
};

enum {
	OFF = 0,
	ON,
	ENABLED
};

#define MAX_LED_STAT_LENGTH 8

enum {
	VERSION_NOUN = 0,
	DATE_NOUN,
	TIME_NOUN,
	TEMP_UNITS_NOUN,
	AIR_TEMP_NOUN,
	POOL_TEMP_NOUN,
	SPA_TEMP_NOUN,
	BATTERY_NOUN,
	POOL_HTR_TEMP_NOUN,
	SPA_HTR_TEMP_NOUN,
	FRZ_PROTECT_NOUN,
	FRZ_PROTECT_TEMP_NOUN,
	LEDS_NOUN
};

enum {
	PUMP_NOUN = 0,
	SPA_NOUN,
	AUX1_NOUN,
	AUX2_NOUN,
	AUX3_NOUN,
	AUX4_NOUN,
	AUX5_NOUN,
	AUX6_NOUN,
	AUX7_NOUN,
	POOL_HTR_NOUN,
	SPA_HTR_NOUN,
	SOLAR_HTR_NOUN
};

#define NUM_RS8_NOUNS 13

#define NUM_LED_NOUNS 12

// TIME IDENTIFIERS
enum {
	YEAR = 0,
	MONTH,
	DAY,
	HOUR,
	MINUTE
};

#define MAX_TIME_FIELD_LENGTH  7

// Battery Status Identifiers
enum {
	OK = 0,
	LOW
};

// PARAMETER IDENTIFIERS
enum {
	PARAM_POOL_TEMP = 0,
	PARAM_SPA_TEMP,
	PARAM_FRZ_PROTECT
};

#define NUM_PARAMS 3

// General defines
#define FALSE 0
#define TRUE  1

//#define OFF   FALSE
//#define ON    TRUE

// PACKET DEFINES
#define NUL  0x00
#define DLE  0x10
#define STX  0x02
#define ETX  0x03

#define MINPKTLEN  5
#define MAXPKTLEN 64
#define PSTLEN 5
#define MSGLEN 16
#define MSGLONGLEN 128
#define TADLEN 13
/* how many seconds spa stays warm after shutdown */
#define SPA_COOLDOWN   (20*60)

#define FAHRENHEIT 0
#define CELSIUS    1
#define UNKNOWN    2

enum {
	DEBUG = 0,
	INFO,
	WARNING,
	ERROR
};

// GLOBALS

struct AQUALINK_DATA
{
	char version[MSGLEN];
	char date[MSGLEN];
	char time[MSGLEN];
	char last_message[MSGLONGLEN];
	unsigned char status[PSTLEN];
	int air_temp;
	int pool_temp;
	int spa_temp;
	int temp_units;
	int battery;
	int freeze_protection;
	int frz_protect_set_point;
	int pool_htr_set_point;
	int spa_htr_set_point;
};

#define MAXLEN      256
struct CONFIG_PARAMETERS
{
  char serial_port[MAXLEN];
  int log_level;
  int socket_port;
  char running_directory[MAXLEN];
  unsigned char device_id;
  int time_error;
  int freeze_protect_high;
  int freeze_protect_low;
};


int send_cmd(const char* cmd);
char* trim (char * s);


#endif /* AQUALINK_H_ */
