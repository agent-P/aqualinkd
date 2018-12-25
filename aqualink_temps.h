/*
 * aqualink_temps.h
 *
 *  Created on: Sep 23, 2012
 */

#ifndef AQUALINK_TEMPS_H_
#define AQUALINK_TEMPS_H_

void* get_pool_spa_htr_temps(void* arg);
void* set_pool_htr_temp(void* arg);
void* set_spa_htr_temp(void* arg);

void* get_frz_protect_temp(void* arg);
void* set_frz_protect_temp(void* arg);

#endif /* AQUALINK_TEMPS_H_ */
