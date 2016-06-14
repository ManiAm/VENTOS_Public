/****************************************************************************/
/// @file    utils.h
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VehicleSafetyExtension.h"
#include "VehicleStatus.h"

#ifndef __SAFETY_EX_H__
#define __SAFETY_EX_H__

#define MEM_ALLOC(type,size) \
        (type *)calloc(1,(sizeof(type)*size)+1)

#define SUCCESS  0
#define FAIL	-1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;

#if 1
typedef struct Blob1
{
    uint8   	MsgCnt;
    uint32  	Id;
    uint16  	SecMark;
    int   		Lat;
    int   		Long;
    short int   Elev;
    uint32  	Accuracy;
    uint16  	Speed;
    uint16  	Heading;
    char   		Angle;
    short int  	AccelLong;
    short int  	AccelLatSet;
    char   		AccelVert;
    short int  	AccelYaw;
    uint16  	Brakes;
    uint8 		VehicleWidth_MSB;
    uint16  	VehicleLength_WidthLSB;

} __attribute__ ((packed)) blob_t;

#endif

#define BLOB_LEN 38

asn_enc_rval_t J2735_encode(void* ,int ,char* );

typedef struct
{
    long			lat;
    long			Long;
    int			elevation_size;
    char			elevation[2];//op

}position3D_t;

typedef struct ddate
{
    long 			year;//op
    long 			month;//op
    long 			day;//op
    long 			hour;//op
    long 			minute;//op
    long 			second;//op

}ddate_t;

typedef struct initialPosition
{
    ddate_t      		*utcTime;
    long 			Long;
    long 			lat;
    int			elevation_size;
    char 			elevation[2];//op
    long 			heading;//op
    int			speed_size;
    char 			speed[2];//op
    int			posAccuracy_size;
    char 			posAccuracy[4];//op
    int  			timeConfidence;//op
    char 			posConfidence;//op
    char 			speedConfidence;//op

}__attribute__((__packed__))initialPosition_t;

typedef struct
{
    int present;
    int pathHistoryPointSets_01_count;
    struct
    {
        long		latOffset;
        long		longOffset;
        long		elevationOffset;//op
        long		timeOffset;//op
        int		posAccuracy_size;
        char 		posAccuracy[4];//op
        long		heading;//op
        int		speed_size;
        char		speed[2];//op
    }pathHistoryPointSets_01[6];

    int			data_size;
    char 			data[345];

}__attribute__((__packed__))crumbData_t;

typedef struct
{
    initialPosition_t	*initialPosition;
    int			currGPSstatus_size;
    char			currGPSstatus[4];
    long			itemCnt;
    crumbData_t		crumbData;

}__attribute__((__packed__))path_t;

typedef struct
{
    long			radiusOfCurve;
    long			confidence;

}__attribute__((__packed__))pathPrediction_t;	

typedef struct
{
    initialPosition_t	*anchorPoint;//op
    int			rtcmHeader_size;
    char			rtcmHeader[32];
    int			msg1001_size;
    char			msg1001[64];//op

}__attribute__((__packed__))rTCMPackage_t;

typedef struct
{
    int			events;
    path_t			*path;//op
    pathPrediction_t	*pathPrediction;//op
    rTCMPackage_t		*theRTCM;//op

}__attribute__((__packed__))vehiclesafetyExtension_t;

typedef struct
{
    char			name[64];//op
    char			vin[18];//op
    char			ownerCode[4];//op
    char			id[4];//op
    long			vehicleType;//op
    int			vehicle_present;
    long			group;

}__attribute__((__packed__))vehicleIdent_t;

int vehicleIdent(VehicleIdent_t **ident,vehicleIdent_t *ident_data);
int full_PositionVector(FullPositionVector_t *initialPosition,initialPosition_t *initialPosition_data);
int fullPositionVector(FullPositionVector_t **initialPosition,initialPosition_t *initialPosition_data);
int vehiclesafetyExtension(VehicleSafetyExtension_t **safetyExt,vehiclesafetyExtension_t *safetyExt_data);

#endif
