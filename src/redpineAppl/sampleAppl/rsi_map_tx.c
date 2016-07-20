/****************************************************************************/
/// @file    rsi_map_tx.c
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    Jul 2016
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>   // struct timeval
#include <unistd.h>     // sleep

#include "rsi_wave_util.h"
#include "dsrc_util.h"  // Data_PR_zones
#include "MAP_create.h"

// forward declarations
void sigint(int sigint);
void freeResources();
int map_create(map_t *, char *, int *);

// global variables
int lsi = 0;
uint8 psid[4] = {0xbf, 0xf0};
int no_of_tx = 0;

// global variables - map_message
position3D_t *position = NULL;
approaches_t *approaches[6];
nodeList_t *laneset = NULL;
zone_t *zone = NULL;
signalControlZone_t *signalControlZone[6];
intersections_t *intersections[6];	
dataParameters_t  *parameter = NULL;
map_t *map_message = NULL;
char *pay_load = NULL;


int main(int argc,char *argv[])
{
    signal(SIGINT, sigint);

    if(argc < 3)
    {
        printf("Provide 'channel number' and 'data rate' command line arg.\n");
        return 1;
    }

    // --[ WAVE initialization - start ]--

    // initialize message queues to send requests to 1609 stack
    printf("Initializing Queue... ");
    int status = rsi_wavecombo_msgqueue_init();
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // initialize the management information base (MIB) in 1609 stack
    printf("Initializing MIB... ");
    status = rsi_wavecombo_1609mib_init();
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // send channel synchronization parameters to Wave Combo Module
    printf("Calling update sync params... ");
    status = rsi_wavecombo_update_channel_sync_params(OPERATING_CLASS, CONTROL_CHANNEL, CCH_INTERVEL, SCH_INTERVEL, SYNC_TOLERANCE, MAX_SWITCH_TIME);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    //printf("Setting UTC... ");
    //status = rsi_wavecombo_set_utc();
    //if(status == FAIL)
    //  return 1;
    //  printf("Done! \n");

    // get a local service index for this user from 1609 stack
    lsi = rsi_wavecombo_local_service_index_request();
    if(lsi <= 0)
        return 1;

    // initialize message queue to receive wsm packets from 1609 stack
    status = rsi_wavecombo_wsmp_queue_init(lsi);
    if(status == FAIL)
        return 1;

    // indicating that a higher layer entity requests a short message service
    printf("Sending WSMP service request... ");
    status = rsi_wavecombo_wsmp_service_req(ADD, lsi, psid);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // request stack to allocate radio resources to the indicated service channel
    printf("Sending SCH service request... ");
    status = rsi_wavecombo_sch_start_req(atoi(argv[1]), atoi(argv[2]), 1, 255);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // --[ WAVE initialization - end ]--

    // --[ making map_message - start ]--

    position = malloc(sizeof(position3D_t));
    if(!position)
    {
        perror("malloc");
        return 1;
    }
    memset(position,0,sizeof(position3D_t));

    position->lat = 0;   // 2345523;
    position->Long = 0;  // 7656452;
    position->elevation_size = 2;
    position->elevation[0] = 0;
    position->elevation[0] = 10;

    approaches[0] = malloc(sizeof(approaches_t));
    if(!approaches[0])
    {
        perror("malloc");
        return 1;
    }
    memset(approaches[0],0,sizeof(approaches_t));

    approaches[0]->refPoint = position;
    approaches[0]->laneWidth =  0x45;

    laneset= malloc(sizeof(nodeList_t));
    if(!laneset)
    {
        perror("malloc");
        return 1;
    }
    memset(laneset,0,sizeof(nodeList_t));

    laneset->size = 4;
    laneset->buf[0] = 0x22;
    laneset->buf[1] = 0x24;
    laneset->buf[2] = 0x25;
    laneset->buf[3] = 0x26;

    zone = malloc(sizeof(zone_t));
    if(!zone)
    {
        perror("malloc");
        return 1;
    }
    memset(zone,0,sizeof(zone_t));

    zone->laneSet_count = 1;
    zone->laneSet[0] = laneset;
    zone->laneWidth = 24;
    zone->nodeList_count = 1;
    zone->nodeList[0] = laneset;

    signalControlZone[0] = malloc(sizeof(signalControlZone_t));
    if(!signalControlZone[0])
    {
        perror("malloc");
        return 1;
    }
    memset(signalControlZone[0],0,sizeof(signalControlZone_t));

    signalControlZone[0]->name_size = 3;
    signalControlZone[0]->name[0] = 'R';
    signalControlZone[0]->name[1] = 'S';
    signalControlZone[0]->name[2] = 'I';
    signalControlZone[0]->pValue_size = 2;
    signalControlZone[0]->pValue[0] = 0x41;
    signalControlZone[0]->pValue[1] = 0x49;
    signalControlZone[0]->data.present = Data_PR_zones;
    signalControlZone[0]->data.choice.lanset.laneSet_count = 1;
    signalControlZone[0]->data.choice.lanset.laneSet[0] = laneset;
    signalControlZone[0]->data.choice.zone.tzone[0]->laneWidth = 0x15;
    signalControlZone[0]->data.choice.zone.zone_count = 1;
    signalControlZone[0]->data.choice.zone.tzone[0] = zone;

    intersections[0] = malloc(sizeof(intersections_t));
    if(!intersections[0])
    {
        perror("malloc");
        return 1;
    }
    memset(intersections[0],0,sizeof(intersections_t));

    intersections[0]->name_size = 3;
    intersections[0]->name[0] = 'R';
    intersections[0]->name[1] = 'S';
    intersections[0]->name[2] = 'I';
    intersections[0]->id_size = 8;
    intersections[0]->id[0] = 0x01;
    intersections[0]->id[0] = 0x31;
    intersections[0]->name_size = 1;
    intersections[0]->refPoint = position;
    intersections[0]->refInterNum[0] = 0x52;
    intersections[0]->refInterNum[1] = 0x52;
    intersections[0]->orientation = 7;
    intersections[0]->laneWidth = 5;
    intersections[0]->type_size = 0x01;
    intersections[0]->type[0] = 0x4;
    intersections[0]->approaches_count = 1;
    intersections[0]->approaches[0] = approaches[0];
    intersections[0]->preemptZones_count = 1;
    intersections[0]->preemptZones[0] = signalControlZone[0];
    intersections[0]->priorityZones_count = 1;
    intersections[0]->priorityZones[0] = signalControlZone[0];

    parameter = malloc(sizeof(dataParameters_t));
    if(!parameter)
    {
        perror("malloc");
        return 1;
    }
    memset(parameter,0,sizeof(dataParameters_t));

    parameter->processMethod_size = 2;
    parameter->processMethod[0] = 0x52;
    parameter->processMethod[1] = 0x52;
    parameter->processAgency_size = 0;
    parameter->processAgency[0] = 0x52;
    parameter->processAgency[1] = 0x42;
    parameter->lastCheckedDate_size = 0;
    parameter->lastCheckedDate[0] = 0x42;
    parameter->lastCheckedDate[1] = 0x42;
    parameter->geiodUsed_size = 0;
    parameter->geiodUsed[0] = 0x56;
    parameter->geiodUsed[1] = 0x46;

    map_message = malloc(sizeof(map_t));
    if(!map_message)
    {
        perror("malloc");
        return 1;
    }
    memset(map_message,0,sizeof(map_t));

    map_message->name_size = 3;
    map_message->name[0] = 'R';
    map_message->name[1] = 'S';
    map_message->name[2] = 'I';
    map_message->layerType = 1;
    map_message->layerID = 1;
    map_message->intersection_count = 1;
    map_message->intersection[0] = intersections[0];
    map_message->dataParameters = parameter;
    map_message->crc_size = 2;
    map_message->crc[0] = 01;
    map_message->crc[1] = 02;

    // --[ making map_message - end ]--

    pay_load = malloc(512);
    if(!pay_load)
    {
        perror("malloc");
        return 1;
    }
    memset(pay_load,0,512);

    // adding new line to improve readability
    printf("\n");

    for(int i = 1; i <= 10; i++)
    {
        map_message->msgCnt = ++no_of_tx;
        int pay_load_len = 0;

        if(map_create(map_message, pay_load, &pay_load_len) != SUCCESS)
        {
            printf("MAP message encoding failed. \n");
            return 1;
        }

        waveShortMessage *wsm = malloc(sizeof(waveShortMessage));
        if(!wsm)
        {
            perror("malloc");
            return 1;
        }
        memset(wsm,0,sizeof(waveShortMessage));

        wsm->dataRate 	  = atoi(argv[2]);
        wsm->txPwr   	  = 15;
        wsm->psid[0] 	  = psid[0];
        wsm->psid[1] 	  = psid[1];
        wsm->psid[2] 	  = psid[2];
        wsm->psid[3] 	  = psid[3];
        wsm->priority 	  = 3;
        wsm->wsm_expiry_time = 50;
        wsm->wsm_length = pay_load_len;
        memcpy(wsm->WSM_Data, pay_load, pay_load_len);
        char peer_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
        memcpy(wsm->peer_mac_address, peer_mac_address, 6);
        wsm->channelNumber = atoi(argv[1]);

        printf("Sending MAP msg #%3d of size %d... ", no_of_tx, pay_load_len);

        status = rsi_wavecombo_wsmp_msg_send(wsm);
        if(status < 0)
            printf("Failed! \n");
        else
            printf("Done! \n");

        free(wsm);
        sleep(1);
    }

    freeResources();
    return 0;
}


void freeResources()
{
    printf("\nTotal Tx: %d \n", no_of_tx);

    if(pay_load)
        free(pay_load);

    if(position)
        free(position);

    if(laneset)
        free(laneset);

    if(zone)
        free(zone);

    if(approaches[0])
        free(approaches[0]);

    if(signalControlZone[0])
        free(signalControlZone[0]);

    if(intersections[0])
        free(intersections[0]);

    if(parameter)
        free(parameter);

    if(map_message)
        free(map_message);

    rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();
}


void sigint(int signum)
{
    freeResources();
    exit(0);
}
