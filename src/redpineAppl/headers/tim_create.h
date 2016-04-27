#ifndef __TIM_CREATE__H__
#define __TIM_CREATE__H__

#include "utils.h"

typedef struct
{
    int			present;
    char			furtherInfo[2];
    position3D_t		position;
    char			viewAngle[2];
    long			mutcdCode;//op
    char			crc[4];//op

}msgId_t;

typedef struct
{
    position3D_t		center;
    int			present;
    long			radiusSteps;

}circle_t; 

typedef struct
{
    position3D_t		*center;//op
    long			xOffset;
    long			yOffset;
    long			zOffset;//op

}regionPointSet_t;

typedef struct
{
    position3D_t		*anchor;//op
    long			laneWidth;//op
    long			directionality;//op
    char			nodeList[16];

}shapePointSet_t;

typedef struct
{
    char			direction[2];
    long			extent;//op
    int			present;
    circle_t		*circle;
    regionPointSet_t	*regionPointSet;
    shapePointSet_t		*shapePointSet;

}validRegion_t;

typedef struct
{
    int			present;
    int			choice;
    long			itis;
    char			text[16];

}content_t;

typedef struct
{
    long			frameType;
    int			present;
    msgId_t			msgId;
    long			startYear;//op
    long			startTime;
    long			duratonTime;
    long			priority;
    position3D_t		*commonAnchor;//op
    long			commonLaneWidth;//op
    long			commonDirectionality;//op
    validRegion_t		*regions;
    content_t		*content;
    char			url[15];//op

}dataFrames_t;

typedef struct
{
    char			packetID[32];//op
    char			urlB[15];//op
    long			dataFrameCount;//op
    dataFrames_t		*dataFrames;
    char			crc[4];

}tim_t;


#endif
