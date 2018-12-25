/*
 * aqualinkd.h
 *
 *  Created on: Sep 22, 2012
 */

#ifndef AQUALINKD_H_
#define AQUALINKD_H_


void send_ack(int file_descriptor, unsigned char command);
void log_packet(unsigned char* packet, int length);
int genchecksum(unsigned char* buf, int len);
int get_packet(int file_descriptor, unsigned char* packet);
void process_packet(unsigned char* packet, int length);
int init_serial_port(char* tty, struct termios* oldtio);
void close_port(int file_descriptor, struct termios* oldtio);
int addmsg(char* newmsg);
void maketad(char* s);


#endif /* AQUALINKD_H_ */
