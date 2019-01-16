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


int is_last_message_time_message()
{
    int ret_value = FALSE;
    static char message[MSGLEN+1];
    
    strcpy(message, aqualink_data.last_message);
    trim(message);

    if((message[1] == ':' && message[4] == ' ') || (message[2] == ':' && message[5] == ' ')) {
        ret_value = TRUE;
    }
    
    return ret_value;
}


void cancel_menu()
{
    // Sending a KEY_CANCEL command causes a quick toggle in and out 
    // of the menu state, but to an abbreviated menu. An aqualink 
    // message containing the time string indicates that it has left the
    // menu state.
	send_cmd("KEY_CANCEL");
    usleep(250000);
    
    int cancelled = FALSE;
    int attempts = 0;
    while(!cancelled) {
        if(is_last_message_time_message()) {
            cancelled = TRUE;
        }
        else {
            send_cmd("KEY_CANCEL");
            usleep(250000);
        }
        
        attempts++;
        
        if(attempts > 10) {
            log_message(WARNING, "s", "Cancel menu has failed after 10 attempts.");
            break;
        }
    }
}


int select_menu_item(char* item_string)
{
    // Select the MENU using KEY_MENU, and wait 0.25 seconds to give the
    // RS8 time to respond. Note that KEY_CANCEL causes a quick toggle
    // in and out of the menu state, but to an abbreviated menu. An 
    // aqualink message containing the string "PROGRAM" indicates that
    //  it has entered the menu and is at the top of the menu hierarchy.
    send_cmd("KEY_MENU");
    
    // Loop for approximately two (2) seconds to wait for the indicator
    // string. Fails after five (5) attempts.
    int found = FALSE;
    int attempts = 0;
    while(!found) {
        usleep(250000);
        if(strstr(aqualink_data.last_message, "PROGRAM") != NULL) {
            found = TRUE;
            break;
        }
        
        if(attempts < 5) {
            // Didn't find it on this attempt. Increment the attempt
            // counter, and toggle the menu state.
            attempts++;
            send_cmd("KEY_MENU");
        }
        else {
            // Exceeded allowed number of attempts. Break out of the 
            // loop.
            log_message(WARNING, "s", "Failed to enter menu mode.");
            break;
        }
    }

    int item = FALSE;
    if(found) {
        // In the menu, Try to select the menu item.
        item = select_sub_menu_item(item_string);
    }
    
    // Returns TRUE if the mode specified by the argument is selected, 
    // FALSE, if not.
    return item;
}


int select_sub_menu_item(char* item_string)
{
    // Select the mode specified by the argument. Note only 10 attempts to
    // enter the mode are made.
	int item = FALSE;
    int iterations = 0;
    while(!item && iterations < 10) {
    	log_message(WARNING, "ssss", "select_sub_menu_item() comparing - ", aqualink_data.last_message, " : ", item_string);
    	if(strstr(aqualink_data.last_message, item_string) != NULL) {
    		// We found the specified mode. Set the flag to break out of
    		// the loop.
    		item = TRUE;

    		// Enter the mode specified by the argument.
    		send_cmd("KEY_ENTER");
        	usleep(250000);
    	}
    	else {
    		// Next menu item and wait 1 second to give the RS8 time
    		// to respond.
    		send_cmd("KEY_RIGHT");
    		usleep(250000);
    	}
    	iterations++;
    }

    // Return TRUE if the mode specified by the argument is selected, FALSE, if not.
    return item;
}


void wait_for_time_message() {
    while(!is_last_message_time_message()) {
        log_message(INFO, "ss", "Not time message: ", aqualink_data.last_message);
        usleep(250000);
    }
    
    log_message(INFO, "ss", "Found time message: ", aqualink_data.last_message);
}
