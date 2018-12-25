/*
 * json_messages.h
 *
 *  Created on: Dec 20, 2012
 */

#ifndef JSON_MESSAGES_H_
#define JSON_MESSAGES_H_

//FUNCTION PROTOTYPES
void convert_aqualink_data();
int build_RS8_JSON(char* buffer);
char* build_leds_JSON(char* buffer);
int get_led_status(int led_id);
int build_aux_labels_JSON(char* json_buffer);

#endif /* JSON_MESSAGES_H_ */
