/****************************************************************************/
/// @file    SPAT_create.h
/// @author
/// @author
/// @date
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#ifndef __SPAT_H__
#define __SPAT_H__

#include "utils.h"
#include "SPAT.h"

typedef struct movement
{
    int 			movementName_size;
    char 			movementName[64];//op 63 byte max
    long 			laneCnt;//op
    char 			laneSet_size;
    char 			laneSet[127];
    long 			currState;//op
    long 			pedState;//op
    long 			specialState;//op
    long 			timeToChange;
    long 			stateConfidence;//op
    long 			yellState;//op
    long 			yellPedState;//op
    long 			yellTimeToChange;//op
    long 			yellStateConfidence;//op
    long 			vehicleCount;//op
    long 			pedDetect;//op
    long 			pedCount;//op

}movementState_t;

typedef struct Intersectionstate
{
    int			name_size;
    char			name[32];//op
    int			id_size;
    char			id[4];
    int			status_size;
    char			status[4];
    long			timeStamp;//op
    long 			lanesCnt;//op
    int			count;
    movementState_t		*states[6];
    int			priority_size;
    char			priority[2];//op
    int			preempt_size;
    char			preempt[2];//op

}intersectionState_t;

typedef struct
{
    int			name_size;
    char			name[32];
    int			count;
    intersectionState_t	*intersectionState[6];

}spat_t;

#endif
