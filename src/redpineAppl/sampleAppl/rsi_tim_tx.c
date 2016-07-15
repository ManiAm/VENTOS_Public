/****************************************************************************/
/// @file    rsi_tim_tx.c
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
#include "dsrc_util.h"
#include "TIM_create.h"

// forward declarations
void sigint(int sigint);
int tim_create(dsrc_tim *, char *, int *);

int lsi = 0;
uint8 psid[4]={0x80,0x03};
int no_of_tx = 0;
char *pay_load = NULL;

// defined globally to be accessible in sigint
waveShortMessage *wsm = NULL;
dsrc_tim *tim_message = NULL;
dataFrames_t *dataFrames[5];
position_t *position;
validRegion_t *region[2]; 
char pktid[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00};


int main(int argc, char *argv[])
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
    status = rsi_wavecombo_update_channel_sync_params(OPERATING_CLASS,CONTROL_CHANNEL,CCH_INTERVEL,SCH_INTERVEL,SYNC_TOLERANCE,MAX_SWITCH_TIME);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    //printf("Setting UTC... ");
    //status=rsi_wavecombo_set_utc();
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
    status = rsi_wavecombo_wsmp_service_req(ADD,lsi,psid);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    printf("Sending CCH service request... ");
    status = rsi_wavecombo_cch_service_req(lsi, ADD, both, 0);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // --[ WAVE initialization - start ]--

    // --[ making tim_message - start ]--

    position = malloc(sizeof(position_t));
    if(!position)
    {
        perror("malloc");
        return 1;
    }
    memset(position,0,sizeof(position_t));

    position->lat = 783793635;
    position->Long = 174449712;
    position->elev_size = 2;
    position->elev[0] = 0x02;
    position->elev[1] = 0x03;

    region[0] = malloc(sizeof(validRegion_t));
    if(!region[0])
    {
        perror("malloc");
        return 1;
    }
    memset(region[0],0,sizeof(validRegion_t));

    region[0]->direction_size = 2;
    region[0]->direction[0] = 0x00;
    region[0]->direction[0] = 0x40;
    region[0]->extent = extent_useFor500meters;
    region[0]->area.present = Area_PR_circle;
    region[0]->area.choice.circle.center.lat = position->lat;
    region[0]->area.choice.circle.center.Long = position->Long;
    region[0]->area.choice.circle.center.elev_size = position->elev_size;
    region[0]->area.choice.circle.center.elev[0] = position->elev[0];
    region[0]->area.choice.circle.raduis.present = Raduis_PR_km;
    region[0]->area.choice.circle.raduis.choice.km = 10;

    dataFrames[0] = malloc(sizeof(dataFrames_t));
    if(!dataFrames[0])
    {
        perror("malloc");
        return 1;
    }
    memset(dataFrames[0],0,sizeof(dataFrames_t));

    dataFrames[0]->frameType = travelerInfoType_roadSignage;
    dataFrames[0]->msgId.present = MsgId_PR_roadSignID;
    dataFrames[0]->msgId.choice.roadSignID.position.lat = position->lat;
    dataFrames[0]->msgId.choice.roadSignID.position.Long = position->Long;
    dataFrames[0]->msgId.choice.roadSignID.position.elev_size = position->elev_size;
    dataFrames[0]->msgId.choice.roadSignID.position.elev[0] = position->elev[0];
    dataFrames[0]->msgId.choice.roadSignID.position.elev[1] = position->elev[1];
    dataFrames[0]->msgId.choice.roadSignID.viewAngle_size = 0x02;
    dataFrames[0]->msgId.choice.roadSignID.viewAngle[0] =  0x00;
    dataFrames[0]->msgId.choice.roadSignID.viewAngle[1] =  0x02;
    dataFrames[0]->msgId.choice.roadSignID.mutcdCode =  mUTCDCode_warning;
    dataFrames[0]->msgId.choice.roadSignID.crc_size =  0x02;
    dataFrames[0]->msgId.choice.roadSignID.crc[0] =  0x00;
    dataFrames[0]->msgId.choice.roadSignID.crc[1] =  0x00;
    dataFrames[0]->startYear = 2014;
    dataFrames[0]->startTime = 512000;
    dataFrames[0]->duratonTime = 3600;
    dataFrames[0]->priority = 3;
    dataFrames[0]->commonAnchor = position;;
    dataFrames[0]->commonLaneWidth = 400;
    dataFrames[0]->commonDirectionality = directionOfUse_forward;
    dataFrames[0]->priority = 3;
    dataFrames[0]->regions_count = 0;
    dataFrames[0]->regions[0] = region[0];
    dataFrames[0]->content.present = Content_PR_advisory;
    dataFrames[0]->content.choice.advisory.present = Item_PR_itis;
    dataFrames[0]->content.choice.advisory.count = 1;
    dataFrames[0]->content.choice.advisory.choice.itis[0] = 300;
    dataFrames[0]->url_size = 0;

    tim_message = malloc(sizeof(dsrc_tim));
    if(!tim_message)
    {
        perror("malloc");
        return 1;
    }
    memset(tim_message,0,sizeof(dsrc_tim));

    tim_message->msgID = _DSRCmsgID_travelerInformation;
    tim_message->packetID_size = 9;
    tim_message->packetID = pktid;
    tim_message->urlB_size = 0;
    tim_message->urlB = NULL;
    tim_message->dataFrameCount =  0x01;
    tim_message->frameCount =  0x01;
    tim_message->dataFrames[0] = dataFrames[0];
    tim_message->crc_size =  0x02;
    tim_message->crc[0] =  0x00;
    tim_message->crc[1] =  0x00;

    // --[ making tim_message - end ]--

    pay_load = malloc(1400);
    if(!pay_load)
    {
        perror("malloc");
        return 1;
    }
    memset(pay_load,0,1400);

    // adding new line to improve readability
    printf("\n");

    while(1)
    {
        ++no_of_tx;
        int pay_load_len = 0;

        if(tim_create(tim_message, pay_load, &pay_load_len) != SUCCESS)
        {
            printf("TIM message encoding failed. \n");
            return 1;
        }

        wsm = malloc(sizeof(waveShortMessage));
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
        memcpy(wsm->WSM_Data,pay_load,pay_load_len);
        char peer_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
        memcpy(wsm->peer_mac_address,peer_mac_address,6);
        wsm->channelNumber = atoi(argv[1]);

        printf("Sending TIM msg #%3d of size %d... ", no_of_tx, pay_load_len);

        status = rsi_wavecombo_wsmp_msg_send(wsm);
        if(status < 0)
            printf("Failed! \n");
        else
            printf("Done! \n");

        free(wsm);
        sleep(1);
    }

    return 0;
}


void sigint(int signum)
{
    printf("\nTotal Tx: %d \n", no_of_tx);

    if(pay_load)
        free(pay_load);

    if(tim_message)
        free(tim_message);

    int ii = 0;
    while(region[ii])
        free(region[ii++]);

    ii = 0;
    while(dataFrames[ii])
        free(dataFrames[ii++]);

    if(position)
        free(position);

    rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_cch_service_req(lsi, DELETE, both, 0);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}
