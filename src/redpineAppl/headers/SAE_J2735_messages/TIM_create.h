/****************************************************************************/
/// @file    TIM_create.h
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

#include "TravelerInformation.h"
#include "utils.h"

#define CON 	0

typedef struct
{
    int	len;
    char	packetID[9];//op
    char	urlB[45];//op
    char	crc[4];
}data_t;

typedef struct
{
    long 		f_type;
    long		startyear;//op
    long		m_year;
    long		m_duration;
    long		s_priority;
    long		lane_width;//op
    long		direction;//op
    char		url[15];//op
}dataframe;

typedef struct
{
    long		lat;
    long		Long;
    int 		elev_size;
    char 		elev[2];//op
}__attribute__ ((packed))position_t;

typedef struct
{
    int 		type;
    char		furtherInfo[2];
    position_t	*location;
    char		angle[2];//op
    long		code;	//op
    char 		crc[2];	//op
}msgID;

typedef struct
{
    char 		direction[2];
    long		extent;//op
    int 		choice;
    position_t	*anchore;//op
    long		lane_w;//op
    long		d_nality;//op
    int 		count;
    char		nodlist[16][4];//
    int		radiis_choice;//
    long		steps;
    long		miles;
    long		km;//}
    char		regionpoint[64];
}validRegion;

typedef struct
{
    int	present;
    int 	choice;
    int 	count;
    long	itis[16];//union
    char	text[16][4];//64
}content;

typedef struct
{
    data_t		*data;
    dataframe	*frame;
    msgID		*msgid;
    position_t 	*threeD;
    validRegion	*v_region;
    content		*con;
}__attribute__ ((packed))tim_t;

typedef struct
{
    int 	present;
    union
    {
        struct further_info_t
        {
            int      size;
            char	 Info[2];
        }furtherInfo;

        struct  roadSignID_t
        {
            position_t	position;
            int		viewAngle_size;
            char		viewAngle[2];//op
            long		mutcdCode;//op
            int 		crc_size;
            char 		crc[2];	//op
        }roadSignID;

    }choice;

}__attribute__ ((packed))msgId_t;

typedef struct
{
    int		size;
    char		buf[4];
}__attribute__ ((packed))nodeList_t;

typedef struct regionOffsets
{
    long	 xOffset;
    long	 yOffset;
    long	*zOffset	/* OPTIONAL */;
}__attribute__ ((packed)) regionOffsets_t;

typedef struct
{
    int   direction_size;
    char  direction[2];
    long	 extent;//op
    struct Area
    {
        int present;

        union validRegion__area_u
        {
            struct shapePointSet
            {
                position_t	*anchor;//op
                long		laneWidth;//op
                long		directionality;//op
                int		nodeList_cnt;
                nodeList_t	**nodeList;
            }shapePointSet;

            struct circle_t
            {
                position_t	center;
                struct raduis_t{
                    int		present;
                    union circle__raduis_u
                    {
                        long	 radiusSteps;
                        long	 miles;
                        long	 km;
                        long     val;
                    }choice;
                }raduis;
            }circle;

            struct regionPointSet_t
            {
                position_t	*anchor;
                int		nodeList_cnt;
                regionOffsets_t nodeList[10];
            }regionPointSet;

        }choice;

    } area;

}__attribute__ ((packed))validRegion_t;	

typedef struct iTIScodesAndText
{
    int present;
    int count;
    union mem
    {
        long	itis[16];
        struct text_t
        {
            int     size;
            char	txt[16][4];
        }text;

    }choice;

}__attribute__ ((packed))iTIScodesAndText_t;

typedef struct
{
    int		     present;
    union u_content
    {
        iTIScodesAndText_t	 advisory;
        iTIScodesAndText_t	 workZone;
        iTIScodesAndText_t	 genericSign;
        iTIScodesAndText_t	 speedLimit;
        iTIScodesAndText_t	 exitService;
    }choice;

}__attribute__ ((packed))content_t;

typedef struct
{
    long 		frameType;
    msgId_t		msgId;
    long		startYear;//op
    long		startTime;
    long		duratonTime;
    long		priority;
    position_t	*commonAnchor;
    long		commonLaneWidth;//op
    long		commonDirectionality;//op
    int		regions_count;
    validRegion_t   *regions[5];
    content_t	content;
    char		url_size;//op
    char		url[15];//op
}__attribute__ ((packed))dataFrames_t;

typedef struct
{
    int		msgID;
    int		packetID_size;
    char		*packetID;
    int		urlB_size;
    char		*urlB;
    long		dataFrameCount;
    long		frameCount;
    dataFrames_t    *dataFrames[5];
    int		crc_size;
    char		crc[2];

}__attribute__ ((packed))dsrc_tim;
