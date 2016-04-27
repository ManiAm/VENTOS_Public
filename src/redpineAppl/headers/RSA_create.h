#ifndef __RSA_H__
#define __RSA_H__

#include "RoadSideAlert.h"
#include "utils.h"

typedef struct rsa_com
{
    int 		id;
    long 		cnt;
    long 		event;
    int		count;
    long		description[5];
    int 		priority_size; //op
    char 		priority[2]; //op
    int 		heading_size; //op
    char 		heading[2]; //op
    int 		extent; //op
    initialPosition_t *position;
    int 		fid_size;//op
    char 		fid[2];//op
    int 		crc_size;
    char 		crc[2];

}rsa_t;

#endif
