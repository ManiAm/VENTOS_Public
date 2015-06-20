/*
 * RouterGlobals.h
 *
 *  Created on: Jun 7, 2015
 *      Author: dylan
 */

/*
 * A collection of magic numbers used by the router code.
 */

#ifndef ROUTERGLOBALS_H_
#define ROUTERGLOBALS_H_


enum LaneCostsMode{MODE_EWMA = 2, MODE_AVERAGE = 1, MODE_NOTHING = 0};
enum RouterMessage{DIJKSTRA = 0, HYPERTREE = 1, DONE = 2, STARTED = 3};
enum TrafficLightLogicMode{FIXED = 0, HIGHDENSITY = 1, LOWDENSITY = 2, COOPERATIVE = 3};
extern int EWMAMultiplier;
extern LaneCostsMode laneCostsMode;

#endif /* ROUTERGLOBALS_H_ */
