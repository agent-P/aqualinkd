/*
 * aqualinkd.c
 *
 *  Created on: Aug 13, 2012
 *
 * To test daemon:	ps -ef|grep aqualinkd (or ps -aux on BSD systems)
 * To test log:	tail -f /[running_dir]/log/aqualinkd.log
 * To test signal:	kill -HUP `cat /[running_dir]/aqualinkd.lock`
 * To terminate:	kill `cat /[running_dir]/aqualinkd.lock`
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <netinet/in.h>     //
#include <sys/socket.h>     // for socket system calls
#include <arpa/inet.h>      // for socket system calls (bind)

#include "aqualink.h"
#include "aqualinkd.h"
#include "globals.h"
#include "aqualink_time.h"
#include "aqualink_temps.h"
#include "logging.h"



#define RUNNING_DIR	"/tmp"
#define DEVICE_ID	"0a"
#define LOCK_FILE	"aqualinkd.lock"
#define CONFIG_FILE "/etc/aqualinkd.conf"

char log_filename[MAXLEN+1];

int RUNNING = TRUE;
int CONNECTED = TRUE;

int PUMP_STATUS = OFF;
int SPA_STATUS = OFF;


int opts = 0;
const char *interface = NULL;

void set_socket_port(char* port)
{
	config_parameters.socket_port = atoi(port);
}

void set_log_level(char* level)
{
	if (strcmp(level, "DEBUG") == 0) {
		config_parameters.log_level = DEBUG;
	}
	else if (strcmp(level, "INFO") == 0) {
		config_parameters.log_level = INFO;
	}
	else if (strcmp(level, "WARNING") == 0) {
		config_parameters.log_level = WARNING;
	}
	else {
		config_parameters.log_level = ERROR;
	}
}

// The id of the Aqualink terminal device. Devices probed by RS8 master are:
// 08-0b, 10-13, 18-1b, 20-23
// If a match is not found with a valid device id, the default value of
// "device_id" is unchanged.
void set_device_id(char* id)
{
	int i;
	int found = FALSE;

	// Iterate through the valid device ids, and match against the specified
	// id.
	for(i=0; i<NUM_DEVICES; i++) {
		if (strcmp(id, DEVICE_STRINGS[i]) == 0) {
			// Found the device id, which means the specified device
			// id is valid. Set the device id parameter and break out
			// of the loop.
			found = TRUE;
			config_parameters.device_id = DEVICE_CODES[i];
			break;
		}
	}

	// Check if we found a valid id.
	if(!found) {
		// Not found, log that we are using the default.
		char dev_id[2];
		sprintf(dev_id, "%02x.", config_parameters.device_id);
		log_to_syslog(LOG_WARNING, "ssss", "Specified device id:", id, ", is not valid. Using default id: ", dev_id);
	}
}


void daemon_shutdown()
{
	log_to_syslog(LOG_NOTICE, "s", "Aqualink daemon shutting down...");
	//exit(0);
	RUNNING = FALSE;
	CONNECTED = FALSE;
}


void signal_handler(int sig)
{
	switch(sig) {
	case SIGHUP:
		log_message(INFO, "s", "hangup signal caught");
		break;
	case SIGINT:
		log_message(INFO, "s", "interrupt signal caught");
		daemon_shutdown();
		break;
	case SIGTERM:
		log_message(INFO, "s", "terminate signal caught");
		daemon_shutdown();
		break;
	}
}


void daemonize()
{
	int i, lfp;
	char str[10];

	// If it is already a daemon, simply return without
	// doing anything.
	if(getppid()==1) return;

	// For the process.
	i = fork();
	if (i < 0) {
		// fork error, exit.
		log_to_syslog(LOG_ERR, "s", "fork() error, exiting.");
		exit(1);
	}
	if (i > 0) {
		exit(0); // parent exits
	}

	// child (daemon) continues
	// get a new process group
	setsid();

	// close all descriptors
	for (i = getdtablesize(); i >= 0; --i) {
		close(i);
	}

	// handle standard I/O
	i = open("/dev/null",O_RDWR); dup(i); dup(i);

	// set newly created file permissions, and change running directory.
	umask(027);
	chdir(config_parameters.running_directory);

	lfp = open(LOCK_FILE, O_RDWR|O_CREAT, 0640);
	if (lfp < 0) {
		log_to_syslog(LOG_ERR, "sssss", "Can not open the lock file: ", config_parameters.running_directory, "/", LOCK_FILE, ", exiting.");
		// Can not open the lock file. Exit
		exit(EXIT_FAILURE);
	}
	if (lockf(lfp, F_TLOCK, 0) < 0) {
		log_to_syslog(LOG_ERR, "sssss", "Could not lock PID lock file: ", config_parameters.running_directory, "/", LOCK_FILE, ", exiting.");
		// Can not lock it.
		exit(EXIT_FAILURE);
	}

	// The first instance continues...
	// Get the pid, and record it to the lockfile.
	sprintf(str, "%d\n", getpid());
	write(lfp, str, strlen(str));

	// Setup signal handling...
	signal(SIGCHLD, SIG_IGN); // ignore child
	signal(SIGTSTP, SIG_IGN); // ignore tty signals
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, signal_handler); // catch hangup signal
	signal(SIGTERM, signal_handler); // catch kill signal
	signal(SIGINT, signal_handler); // catch kill signal
}

/*
 * trim: get rid of trailing and leading whitespace...
 *       ...including the annoying "\n" from fgets()
 */
char* trim (char * s)
{
	/* Initialize start, end pointers */
	char *s1 = s, *s2 = &s[strlen (s) - 1];

	/* Trim and delimit right side */
	while ( (isspace (*s2)) && (s2 >= s1) )
		s2--;
	*(s2+1) = '\0';

	/* Trim left side */
	while ( (isspace (*s1)) && (s1 < s2) )
		s1++;

	/* Copy finished string */
	strcpy (s, s1);
	return s;
}

/*
 * initialize data to default values
 */
void init_parameters (struct CONFIG_PARAMETERS * parms)
{
	strncpy (parms->serial_port, "/dev/ttyUSB0", MAXLEN);
	set_log_level ("WARNING");
	set_socket_port("6500");
	strncpy (parms->running_directory, RUNNING_DIR, MAXLEN);
	set_device_id(DEVICE_ID);
}

/*
 * parse external parameters file
 *
 */
void parse_config (struct CONFIG_PARAMETERS * parms)
{
	char *s, buff[256];
	FILE *fp = fopen (CONFIG_FILE, "r");
	if (fp == NULL)
	{
		log_to_syslog(LOG_WARNING, "ss", "Couldn't open configuration file: ", CONFIG_FILE);
		return;
	}

	/* Read next line */
	while ((s = fgets (buff, sizeof buff, fp)) != NULL)
	{
		/* Skip blank lines and comments */
		if (buff[0] == '\n' || buff[0] == '#')
			continue;

		/* Parse name/value pair from line */
		char name[MAXLEN], value[MAXLEN];
		s = strtok (buff, "=");
		if (s==NULL)
			continue;
		else
			strncpy (name, s, MAXLEN);
		s = strtok (NULL, "=");
		if (s==NULL)
			continue;
		else
			strncpy (value, s, MAXLEN);
		trim (value);

		/* Copy into correct entry in parameters struct */
		if (strcmp(name, "log_level") == 0) {
			set_log_level(value);
		}
		else if (strcmp(name, "running_directory") == 0) {
			strncpy (parms->running_directory, value, MAXLEN);
		}
		else if (strcmp(name, "serial_port") == 0)
			strncpy (parms->serial_port, value, MAXLEN);
		else if (strcmp(name, "socket_port") == 0) {
			set_socket_port(value);
		}
		else if (strcmp(name, "device_id") == 0) {
			set_device_id(value);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX1_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX1_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX2_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX2_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name,LABEL_NOUNS[AUX3_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX3_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX4_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX4_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX5_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX5_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX6_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX6_LABEL], value, LABEL_LENGTH);
		}
		else if (strcmp(name, LABEL_NOUNS[AUX7_LABEL]) == 0) {
			strncpy(aux_function_labels[AUX7_LABEL], value, LABEL_LENGTH);
		}
		else {
			log_to_syslog(LOG_WARNING, "ssss", name, "/", value, "Unknown configuration name/value pair!");
		}
	}

	// Set the logging parameters: running_directory, and level
	set_logging_parameters(parms->running_directory, parms->log_level);

	/* Close file */
	fclose (fp);
}


// **********************************************************************************************
// AQUALINK FUNCTIONS
// **********************************************************************************************


// Initialize the Aqualink data. Some data is state dependent, and will not be accurate
// until the Aqualink master device is in the proper state. For example, pool temperature
// and spa temperature are not available unless the filter pump is on.
void init_aqualink_data()
{
	aqualink_data.air_temp = -999;
	aqualink_data.pool_temp = -999;
	aqualink_data.spa_temp = -999;
	aqualink_data.version[0] = '\0';
	aqualink_data.date[0] = '\0';
	aqualink_data.time[0] = '\0';
	aqualink_data.temp_units = UNKNOWN;
	aqualink_data.freeze_protection = OFF;
}

/*
 Open and Initialize the serial communications port to the Aqualink RS8 device.
 Arg is tty or port designation string
 returns the file descriptor
 */
int init_serial_port(char* tty, struct termios* oldtio)
{
	long BAUD = B9600;
	long DATABITS = CS8;
	long STOPBITS = 0;
	long PARITYON = 0;
	long PARITY = 0;

	struct termios newtio;       //place for old and new port settings for serial port

    int file_descriptor = open(tty, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (file_descriptor < 0)  {
        log_to_syslog(LOG_ERR, "ss", "init_serial_port(): Unable to open port: ", tty);
    }
    else {
    	// Set the port to block on read() if there is no data.
    	fcntl(file_descriptor, F_SETFL, 0);

    	tcgetattr(file_descriptor, oldtio); // save current port settings
    	// set new port settings for canonical input processing
    	newtio.c_cflag = BAUD | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;
    	newtio.c_iflag = IGNPAR;
    	newtio.c_oflag = 0;
    	newtio.c_lflag = 0;       // ICANON;
    	newtio.c_cc[VMIN]= 1;
    	newtio.c_cc[VTIME]= 0;
    	tcflush(file_descriptor, TCIFLUSH);
    	tcsetattr(file_descriptor, TCSANOW, &newtio);
    }

    return file_descriptor;
}

/* close tty port */
void close_port(int file_descriptor, struct termios* oldtio)
{
	tcsetattr(file_descriptor, TCSANOW, oldtio);
    close(file_descriptor);
}


// Generate and return checksum of packet.
int generate_checksum(unsigned char* packet, int length)
{
    int i, sum, n;

    n = length - 3;
    sum = 0;
    for (i = 0; i < n; i++)
        sum += (int) packet[i];
    return(sum & 0x0ff);
}


/**
 * 
 */
int send_cmd(const char* cmd)
{
	int i;
	int not_found = TRUE;

	// Iterate through the valid command keys, and look for a match.
	for(i=0; i<NUM_KEYS; i++) {
		if(strstr(cmd, KEY_CMD_TEXT[i]) != NULL) {
			// Found a match to a valid command. Set the command
			// variable.
			not_found = FALSE;
			aqualink_cmd = KEY_CODES[i];
			log_message(DEBUG, "ss", KEY_CMD_TEXT[i], " command sent");
			break;
		}
	}

	return not_found;
}


// Send an ack packet to the Aqualink RS8 master device.
// file_descriptor: the file descriptor of the serial port connected to the device
// command: the command byte to send to the master device, NUL if no command
void send_ack(int file_descriptor, unsigned char command)
{
    const int length = 11;
    unsigned char ackPacket[] = { NUL, DLE, STX, DEV_MASTER, CMD_ACK, NUL, NUL, 0x13, DLE, ETX, NUL };

    // Update the packet and checksum if command argument is not NUL.
    if(command != NUL) {
    	ackPacket[6] = command;
    	ackPacket[7] = generate_checksum(ackPacket, length-1);

    	// NULL out the command byte if it is the same. Difference implies that
    	// a new command has come in, and is awaiting processing.
        if(aqualink_cmd == command) {
        	aqualink_cmd = NUL;
        }

        if(config_parameters.log_level == DEBUG) {
        	// In debug mode, log the packet to the private log file.
        	log_packet(ackPacket, length);
        }
    }

    // Send the packet to the master device.
    write(file_descriptor, ackPacket, length);
}


// Reads the bytes of the next incoming packet, and
// returns when a good packet is available in packet
// file_descriptor: the file descriptor to read the bytes from
// packet: the unsigned char buffer to store the bytes in
// returns the length of the packet
int get_packet(int file_descriptor, unsigned char* packet)
{
	unsigned char byte;
	int bytesRead;
	int index = 0;
	int endOfPacket = FALSE;
	int packetStarted = FALSE;
	int foundDLE = FALSE;

	while (!endOfPacket) {

		bytesRead = read(file_descriptor, &byte, 1);

		if (bytesRead == 1) {

			if (byte == DLE) {
				// Found a DLE byte. Set the flag, and record the byte.
				foundDLE = TRUE;
				packet[index] = byte;
			}
	        else if (byte == STX && foundDLE == TRUE) {
	            // Found the DLE STX byte sequence. Start of packet detected.
	            // Reset the DLE flag, and record the byte.
	            foundDLE = FALSE;
	            packetStarted = TRUE;
	            packet[index] = byte;
	        }
	        else if (byte == NUL && foundDLE == TRUE) {
	            // Found the DLE NUL byte sequence. Detected a delimited data byte.
	            // Reset the DLE flag, and decrement the packet index to offset the
	            // index increment at the end of the loop. The delimiter, [NUL], byte
	        	// is not recorded.
	            foundDLE = FALSE;
	            //trimmed = true;
	            index--;
	        }
	        else if (byte == ETX && foundDLE == TRUE) {
	            // Found the DLE ETX byte sequence. End of packet detected.
	            // Reset the DLE flag, set the end of packet flag, and record
	            // the byte.
	            foundDLE = FALSE;
	            packetStarted = FALSE;
	            endOfPacket = TRUE;
	            packet[index] = byte;
	        }
	        else if (packetStarted == TRUE) {
	            // Found a data byte. Reset the DLE flag just in case it is set
	            // to prevent anomalous detections, and record the byte.
	            foundDLE = FALSE;
	            packet[index] = byte;
	        }
	        else {
	        	// Found an extraneous byte. Probably a NUL between packets.
	        	// Ignore it, and decrement the packet index to offset the
	            // index increment at the end of the loop.
	            index--;
	        }

	        // Finished processing the byte. Increment the packet index for the
	        // next byte.
	        index++;

	        // Break out of the loop if we exceed maximum packet
	        // length.
	        if (index >= MAXPKTLEN) {
	            break;
	        }
		}
		else if(bytesRead <= 0) {
			// Got a read error. Wait one millisecond for the next byte to
			// arrive.
			log_message(WARNING, "siss", "Read error: ", errno, " - ", strerror(errno));
			if(errno == 9) {
				// Bad file descriptor. Port has been disconnected for some reason.
				// Return a -1.
				return -1;
			}
			usleep(1000);
		}
	}

	// Return the packet length.
	return index;
}


void log_packet(unsigned char* packet, int length)
{
    int i;
    char temp_string[64];
    char message_buffer[MAXLEN];

	sprintf(temp_string, "%02x ", packet[0]);
	strcpy(message_buffer, temp_string);

    for (i = 1; i < length; i++) {
    	sprintf(temp_string, "%02x ", packet[i]);
    	strcat(message_buffer, temp_string);
    }

    log_message(DEBUG, "s", message_buffer);
}


void process_long_message(char* message)
{
	log_message(DEBUG, "ss", "Processing long message: ", message);

	const char pool_setting_string[] = { "POOL TEMP IS SET TO" };
	const char spa_setting_string[] = { "SPA TEMP IS SET TO" };
	const char frz_protect_setting_string[] = { "FREEZE PROTECTION IS SET TO" };

	// Remove any leading white space.
	trim(message);

	// Extract data and warnings from long messages.
	if(strstr(message, "BATTERY LOW") != NULL) {
		aqualink_data.battery = LOW;
	}
	else if(strstr(message, pool_setting_string) != NULL) {
		//log_message(DEBUG, "ss", "pool htr long message: ", message+20);
		aqualink_data.pool_htr_set_point = atoi(message+20);
	}
	else if(strstr(message, spa_setting_string) != NULL) {
		//log_message(DEBUG, "ss", "spa htr long message: ", message+19);
		aqualink_data.spa_htr_set_point = atoi(message+19);
	}
	else if(strstr(message, frz_protect_setting_string) != NULL) {
		//log_message(DEBUG, "ss", "frz protect long message: ", message+28);
		aqualink_data.frz_protect_set_point = atoi(message+28);
	}

}


void process_packet(unsigned char* packet, int length)
{
	static int got_long_msg;
	static char message[MSGLEN+1];
	static unsigned char last_packet[MAXPKTLEN];

	pthread_t set_time_thread;
	pthread_t get_htr_set_pnts_thread;
	pthread_t get_frz_protect_set_pnt_thread;

	if(memcmp(packet, last_packet, MAXPKTLEN) == 0) {
		// Don't process redundant packets. They can occur for two reasons.
		// First, status doesn't change much so the vast majority of packets
		// are identical under normal circumstances. It is more efficient to
		// process only changes. Second, the master will send redundant packets
		// if it misses an ACK response up to 3 times before it sends a
		// command probe. Redundant message packets can corrupt long message
		// processing.

		// Log the redundant packets other than STATUS packets at DEBUG level.
		if(packet[PKT_CMD] != CMD_STATUS && config_parameters.log_level == DEBUG) {
			log_message(DEBUG, "s", "Trapped redundant packet...");
			log_packet(packet, length);
		}

		// Return without processing the packet.
		return;
	}
	else {
		// Normal packet. Copy it for testing against the next packet.
		memcpy(last_packet, packet, MAXPKTLEN);
	}

	// Process by packet type.
	if(packet[PKT_CMD] == CMD_STATUS) {
		// Update status. Copy it to last status buffer.
		memcpy(aqualink_data.status, packet+4, PSTLEN);

		// Set mode status. Accurate temperature processing is
		// dependent on this.
		if((aqualink_data.status[LED_STATES[STAT_BYTE][STAT_PUMP]] & LED_STATES[STAT_MASK][STAT_PUMP]) != 0) {
			PUMP_STATUS = ON;
		}
		else if(aqualink_data.status[LED_STATES[STAT_BYTE][STAT_PUMP_BLINK]] & LED_STATES[STAT_MASK][STAT_PUMP_BLINK]) {
			PUMP_STATUS = ENABLED;
		}
		else {
			PUMP_STATUS = OFF;
		}
		if((aqualink_data.status[LED_STATES[STAT_BYTE][STAT_SPA]] & LED_STATES[STAT_MASK][STAT_SPA]) != 0) {
			SPA_STATUS = ON;
		}
		else {
			SPA_STATUS = OFF;
		}
	}
	else if(packet[PKT_CMD] == CMD_MSG && (packet[PKT_DATA] == 0)) {
		// Packet is a single line message.
		if(got_long_msg) {
			got_long_msg = FALSE;
			process_long_message(aqualink_data.last_message);
			log_message(DEBUG, "s", aqualink_data.last_message);
		}
		// First, extract the message from the packet.
		memset(message, 0, MSGLEN+1);
		strncpy(message, (char*)packet+PKT_DATA+1, MSGLEN);
		log_message(DEBUG, "s", message);

		// Copy the message to the Aqualink data structure as the latest message.
		memset(aqualink_data.last_message, 0, MSGLONGLEN);
		strncpy(aqualink_data.last_message, message, MSGLEN+1);

		// Remove the leading white space so the pointer offsets are accurate
		// and consistent.
		trim(message);

		if(strstr(message, "AIR TEMP") != NULL) {
            aqualink_data.air_temp = atoi(message+8);
            // Check temperature units. Note AIR TEMP is always present.
            // So, get the temperature units here.
            if(strstr(message, "F") != NULL) {
            	aqualink_data.temp_units = FAHRENHEIT;
            }
            else if(strstr(message, "C") != NULL) {
            	aqualink_data.temp_units = CELSIUS;
            }
            else {
            	aqualink_data.temp_units = UNKNOWN;
            	log_message(WARNING, "ss", "Can't determine temperature units from message: ", message);
            }

            // Reset pool and spa temperatures to unknown values if the
            // corresponding modes are off.
            if(PUMP_STATUS == OFF) {
            	aqualink_data.pool_temp = -999;
            }
            if(SPA_STATUS == OFF) {
            	aqualink_data.spa_temp = -999;
            }
		}
		else if(strstr(message, "POOL TEMP") != NULL) {
            aqualink_data.pool_temp = atoi(message+9);
		}
		else if(strstr(message, "SPA TEMP") != NULL) {
            aqualink_data.spa_temp = atoi(message+8);
		}
		else if((message[1] == ':' && message[4] == ' ') || (message[2] == ':' && message[5] == ' ')) {
			// A time message.
			strcpy(aqualink_data.time, message);

			// If date is not an empty string, check the Aqualink time and date
			// against system time.
			if(aqualink_data.date[0] != '\0') {
				if(!check_aqualink_time(aqualink_data.date, aqualink_data.time)) {
					log_message(DEBUG, "sssss", "Aqualink time ", aqualink_data.date, " ", aqualink_data.time, " is incorrect");

					if(pthread_create(&set_time_thread, NULL, &set_aqualink_time, (void*)NULL)) {
						log_to_syslog(LOG_WARNING, "s", "Error creating SET TIME thread.");
					}
				}
			}
		}
		else if (message[2] == '/' && message[5] == '/') {
			// A date message.
			strcpy(aqualink_data.date, message);
		}
		else if(strstr(message, " REV ") != NULL) {
			// A master firmware revision message.
			strcpy(aqualink_data.version, message);

			// This is a good indicator that the daemon has just started
			// or has just been forced to resynch with the master. It is a
			// good time to initialize the daemon with some Aqualink set
			// point data.
			if(pthread_create(&get_htr_set_pnts_thread, NULL, &get_pool_spa_htr_temps, (void*)NULL)) {
				log_to_syslog(LOG_WARNING, "s", "Error creating get heater set points thread.");
			}
			if(pthread_create(&get_frz_protect_set_pnt_thread, NULL, &get_frz_protect_temp, (void*)NULL)) {
				log_to_syslog(LOG_WARNING, "s", "Error creating get freeze protection set point thread.");
			}

		}
	}
	else if(packet[PKT_CMD] == CMD_MSG && (packet[PKT_DATA] == 1)) {
		// Packet is the start of a multi-line message.
		if(got_long_msg) {
			got_long_msg = FALSE;
			process_long_message(aqualink_data.last_message);
			log_message(DEBUG, "s", aqualink_data.last_message);
		}
		got_long_msg = TRUE;
		memset(message, 0, MSGLEN+1);
		strncpy(message, (char*)packet+PKT_DATA+1, MSGLEN);
		strcpy(aqualink_data.last_message, message);
	}
	else if(packet[PKT_CMD] == CMD_MSG_LONG) {
		// Packet is continuation of a long message.
		strncpy(message, (char*)packet+PKT_DATA+1, MSGLEN);
		strcat(aqualink_data.last_message, message);
	}
	else if(packet[PKT_CMD] == CMD_PROBE) {
		// Packet is a command probe. The master is trying to find
		// this device.
		log_message(INFO, "s", "Synch'ing with Aqualink master device...");
	}
}




// **********************************************************************************************
// MAIN FUNCTION
// **********************************************************************************************

int main()
{
	// Log only NOTICE messages and above. Debug and info messages
	// will not be logged to syslog.
	setlogmask(LOG_UPTO (LOG_NOTICE));

	// Initialize the daemon's parameters.
	init_parameters(&config_parameters);

	// Initialize Aqualink data.
	init_aqualink_data();

	// Parse the external configuration file, and override initialization
	// of the default values of the specified parameters.
	parse_config(&config_parameters);

	// Log the start of the daemon to syslog.
	log_to_syslog(LOG_NOTICE, "s", "Aqualink daemon started...");

	// Turn this application into a daemon.
	daemonize();


	// Start the main loop of the primary thread. It will make 10 connect attempts
	// before exiting on failure. It will sleep two (2) minutes between attempts.
	int init_attempts = 0;
	while(CONNECTED) {
		int packet_length;

		// Initialize the serial port connected to the Aqualink master device.
		// Save the old termios in case we want to reset when we close the port.
		struct termios oldtio;
		int file_descriptor = init_serial_port(config_parameters.serial_port, &oldtio);
		if(file_descriptor < 0) {
			if(init_attempts > 10) {
				log_to_syslog(LOG_ERR, "s", "Aqualink daemon exit on failure to connect to master device.");
				return EXIT_FAILURE;
			}
			else {
				log_to_syslog(LOG_NOTICE, "s", "Aqualink daemon attempting to connect to master device...");
				init_attempts++;
			}
		}
		else {
			// Reset attempts counter, and log the successful connection.
			init_attempts = 0;
			log_to_syslog(LOG_NOTICE, "ss", "Listening to Aqualink RS8 on serial port: ", config_parameters.serial_port);

			// The packet buffer.
			unsigned char packet_buffer[MAXPKTLEN];

			// The primary loop that reads and processes the Aqualink RS8 serial port data.
			while(RUNNING) {
				packet_length = get_packet(file_descriptor, packet_buffer);

				if(packet_length == -1) {
					// Unrecoverable read error. Break out of the inner loop,
					// and attempt to reconnect.
					break;
				}

				// Ignore packets not for this Aqualink terminal device.
				if (packet_buffer[PKT_DEST] == config_parameters.device_id) {

					// The packet was meant for this device. Acknowledge it, including any command
					// that might be waiting in the "aqualink_cmd" variable. Note "aqualink_cmd"
					// is reset to NUL after a command is sent.
					send_ack(file_descriptor, aqualink_cmd);

					//log_packet(packet_buffer, packet_length);

					// Process the packet. This includes deriving general status, and identifying
					// warnings and errors.
					process_packet(packet_buffer, packet_length);

				}
			}

			// Reset and close the port.
			close_port(file_descriptor, &oldtio);
		}

		if(packet_length == -1 || file_descriptor < 0) {
			// Wait 2 minutes before trying to reconnect.
			log_to_syslog(LOG_NOTICE, "ss", "Connection lost to Aqualink RS8 on serial port: ", config_parameters.serial_port);
			log_to_syslog(LOG_NOTICE, "s", "Pausing 2 minutes before attempting reconnect...");
			sleep(120);
		}
	}

	log_message(INFO, "s", "Aqualink daemon exiting...");
	return EXIT_SUCCESS;
}

/* EOF */


