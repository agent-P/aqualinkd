/*
 * aqualink_menu.c
 *
 *  Created on: Sep 23, 2012
 */
#include <string.h>
#include <unistd.h>
#include "aqualink.h"
#include "aqualink_menu.h"
#include "globals.h"


void cancel_menu()
{
	send_cmd("KEY_CANCEL");
}


int select_menu_item(char* item_string)
{
    // Select the MENU and wait 1 second to give the RS8 time to respond.
    send_cmd("KEY_MENU");
    sleep(1);

    int item = select_sub_menu_item(item_string);

    // Return TRUE if the mode specified by the argument is selected, FALSE, if not.
    return item;
}


int select_sub_menu_item(char* item_string)
{
    // Select the mode specified by the argument. Note only 10 attempts to
    // enter the mode are made.
	int item = FALSE;
    int iterations = 0;
    while(!item && iterations < 10) {
    	//log_message(DEBUG, aqualink_data.last_message);
    	if(strstr(aqualink_data.last_message, item_string) != NULL) {
    		// We found the specified mode. Set the flag to break out of
    		// the loop.
    		item = TRUE;

    		// Enter the mode specified by the argument.
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
    }

    // Return TRUE if the mode specified by the argument is selected, FALSE, if not.
    return item;
}
