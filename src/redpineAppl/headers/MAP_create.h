/****************************************************************************/
/// @file    MAP_create.h
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

#ifndef __MAP_H__
#define __MAP_H__

#include "MapData.h"
#include "utils.h"

typedef struct
{
    int			processMethod_size;
    char			processMethod[32];//op
    char			processAgency_size;
    char			processAgency[32];//op
    char			lastCheckedDate_size;
    char			lastCheckedDate[32];//op
    char			geiodUsed_size;
    char			geiodUsed[32];//op

}dataParameters_t;

typedef struct
{
    int			size;
    char			buf[4];

}nodeList_t; 


typedef struct
{
    int			laneNumber_size;
    char			laneNumber[16];
    long			laneWidth;//op
    long			laneAttributes;
    int			nodeList_count;
    nodeList_t		*nodeList[16];
    int			keepOutList_count;
    nodeList_t		*keepOutList[16];//op
    int			connectsTo_size;
    char			connectsTo[4];//op

}vehicleReferenceLane_t;

typedef struct
{
    char			laneNumber[16];
    long			laneWidth;//op
    long			laneAttributes;
    char			refLaneNum[32];
    long			lineOffset;
    char			connectsTo[4];//op

}vehicleComputedLane_t; 

typedef struct
{
    int			laneNumber_size;
    char			laneNumber[16];
    long			laneWidth;//op
    long			barrierAttributes;
    int			nodeList_count;
    nodeList_t		*nodeList[16];

}barrierLane_t;

typedef struct
{
    int			name_size;
    char			name[64];//op
    long			id;//op
    int			drivingLanes_count;
    vehicleReferenceLane_t	*drivingLanes;
    int			computedLanes_count;
    vehicleComputedLane_t	*computedLanes;
    int			trainsAndBuses_count;
    vehicleReferenceLane_t	*trainsAndBuses;
    int			barriers_count;
    barrierLane_t		*barriers;
    int			crosswalks_count;
    vehicleReferenceLane_t	*crosswalks;

}approach_t;

typedef struct
{
    position3D_t		*refPoint;//op
    long			laneWidth;//op
    approach_t		*approach;//op
    approach_t		*egress;//op

}approaches_t; 

typedef struct
{
    int			laneSet_count;
    nodeList_t		*laneSet[16];
    long			laneWidth;
    int			nodeList_count;
    nodeList_t		*nodeList[16];

} zone_t;

typedef struct
{
    int			name_size;
    char			name[64];//op
    int			pValue_size;
    char			pValue[32];//op
    struct data_t
    {
        int			present;
        union choice_t
        {
            struct laneset_t
            {
                int			laneSet_count;
                nodeList_t		*laneSet[16];
            } lanset;

            struct zone_t
            {
                int			zone_count;
                zone_t			*tzone[16];
            } zone;

        } choice;

    } data;

} signalControlZone_t;

typedef struct
{
    int			name_size;
    char			name[32];//op
    int			id_size;
    char			id[4];
    position3D_t		*refPoint;//op
    int			refInterNum_size;
    char			refInterNum[16];//op
    long			orientation;//op
    long			laneWidth;//op
    int			type_size;
    char			type[4];//op
    int			approaches_count;
    approaches_t		*approaches[6];
    int			preemptZones_count;
    signalControlZone_t	*preemptZones[6];
    int			priorityZones_count;
    signalControlZone_t	*priorityZones[6];

}intersections_t;

typedef struct
{
    long			msgCnt;
    int			name_size;
    char			name[64];//op
    long			layerType;//op
    long			layerID;//op
    int			intersection_count;
    intersections_t		*intersection[6];
    dataParameters_t	*dataParameters;//op
    int			crc_size;
    char			crc[4];//op

}map_t;


#endif

