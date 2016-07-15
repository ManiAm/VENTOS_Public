/****************************************************************************/
/// @file    rsi_spat_tx.c
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
#include "SPAT_create.h"

void sigint(int sigint);
int spat_create(spat_t *, char *, int *);

int lsi = 0;
uint8 psid[4]={0xbf,0xe0};
int no_of_tx = 0;
char *pay_load = NULL;

// defined globally to be accessible in sigint
waveShortMessage *wsm = NULL;
spat_t	*spat_message = NULL;
movementState_t *movementState[6];
intersectionState_t *intersectionState[6];	


int main(int argc, char *argv[])
{
    signal(SIGINT, sigint);

    if(argc < 3)
    {
        printf("Provide 'channel number' and 'data rate' command line arg. \n");
        return 1;
    }

    // --[ WAVE initialization - start ]--

    // initialize message queues to send requests to 1609 stack
    printf("Initializing Queue... ");
    int status = rsi_wavecombo_msgqueue_init();
    if(status == FAIL)
        return 1;

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

    // --[ making spat_message - start ]--

    movementState[0] = malloc(sizeof(movementState_t));
    if(!movementState[0])
    {
        perror("malloc");
        return 1;
    }
    memset(movementState[0],0,sizeof(movementState_t));

    movementState[0]->movementName_size = 0x02;
    movementState[0]->movementName[0] = 'A';
    movementState[0]->movementName[1] = 'A';
    movementState[0]->laneSet_size = 0x02;
    movementState[0]->laneSet[0] = 0x01;
    movementState[0]->laneSet[1] = 0x02;
    movementState[0]->currState = 0x02;
    movementState[0]->pedState = pedestrianSignalState_stop;
    movementState[0]->specialState = specialSignalState_notInUse;
    movementState[0]->timeToChange = 0x27;
    movementState[0]->stateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[0]->yellState = 0x02;
    movementState[0]->yellPedState = 0x1;
    movementState[0]->yellStateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[0]->vehicleCount = 0x0;
    movementState[0]->pedDetect = pedestrianDetect_none;
    movementState[0]->pedCount = 0x0;

    movementState[1] = malloc(sizeof(movementState_t));
    if(!movementState[1])
    {
        perror("malloc");
        return 1;
    }
    memset(movementState[1],0,sizeof(movementState_t));

    movementState[1]->movementName_size = 0x02;
    movementState[1]->movementName[0] = 'A';
    movementState[1]->movementName[1] = 'B';
    movementState[1]->laneSet_size = 0x02;
    movementState[1]->laneSet[0] = 0x02;
    movementState[1]->laneSet[1] = 0x03;
    movementState[1]->currState = 0x02;
    movementState[1]->pedState = pedestrianSignalState_stop;
    movementState[1]->specialState = specialSignalState_notInUse;
    movementState[1]->timeToChange = 0x27;
    movementState[1]->stateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[1]->yellState = 0x02;
    movementState[1]->yellPedState = 0x1;
    movementState[1]->yellStateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[1]->vehicleCount = 0x0;
    movementState[1]->pedDetect = pedestrianDetect_none;
    movementState[1]->pedCount = 0x0;

    movementState[2] = malloc(sizeof(movementState_t));
    if(!movementState[2])
    {
        perror("malloc");
        return 1;
    }
    memset(movementState[2],0,sizeof(movementState_t));

    movementState[2]->movementName_size = 0x02;
    movementState[2]->movementName[0] = 'A';
    movementState[2]->movementName[1] = 'C';
    movementState[2]->laneSet_size = 0x02;
    movementState[2]->laneSet[0] = 0x05;
    movementState[2]->laneSet[1] = 0x06;
    movementState[2]->currState = 0x02;
    movementState[2]->pedState = pedestrianSignalState_stop;
    movementState[2]->specialState = specialSignalState_notInUse;
    movementState[2]->timeToChange = 0x27;
    movementState[2]->stateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[2]->yellState = 0x02;
    movementState[2]->yellPedState = 0x1;
    movementState[2]->yellStateConfidence = stateConfidence_timeLikeklyToChange;
    movementState[2]->vehicleCount = 0x0;
    movementState[2]->pedDetect = pedestrianDetect_none;
    movementState[2]->pedCount = 0x0;

    intersectionState[0] = malloc(sizeof(intersectionState_t));
    if(!intersectionState[0])
    {
        perror("malloc");
        return 1;
    }
    memset(intersectionState[0],0,sizeof(intersectionState_t));

    intersectionState[0]->name_size = 0x01;
    intersectionState[0]->name[0] = 0x41;
    intersectionState[0]->name[1] = 0x42;
    intersectionState[0]->id_size = 0x02;
    intersectionState[0]->id[0] = 0x00;
    intersectionState[0]->id[1] = 0x00;
    intersectionState[0]->status_size = 0x01;
    intersectionState[0]->status[0] = 0x00;
    intersectionState[0]->timeStamp = 00;
    intersectionState[0]->lanesCnt = 46;
    intersectionState[0]->count = 3;
    intersectionState[0]->states[0] = movementState[0];
    intersectionState[0]->states[1] = movementState[1];
    intersectionState[0]->states[2] = movementState[2];
    intersectionState[0]->priority_size= 0x2;
    intersectionState[0]->priority[0] = 0x4;
    intersectionState[0]->preempt[1] = 0x3;

    spat_message = malloc(sizeof(spat_t));
    if(!spat_message)
    {
        perror("malloc");
        return 1;
    }
    memset(spat_message,0,sizeof(sizeof(spat_t)));

    spat_message->name_size = 0x02;
    spat_message->name[0] = 0x42;
    spat_message->name[1] = 0x43;
    spat_message->count = 0x01;
    spat_message->intersectionState[0] = intersectionState[0];

    // --[ making spat_message - start ]--

    pay_load = malloc(512);
    if(!pay_load)
    {
        perror("malloc");
        return 1;
    }
    memset(pay_load,0,512);

    // adding new line to improve readability
    printf("\n");

    while(1)
    {
        ++no_of_tx;
        int pay_load_len = 0;
        if(spat_create(spat_message,pay_load,&pay_load_len) != SUCCESS)
        {
            printf("MAP message encoding failed. \n");
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

        printf("Sending SPAT msg #%3d of size %d... ", no_of_tx, pay_load_len);

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

    if(spat_message)
        free(spat_message);

    int ii = 0;
    while(intersectionState[ii])
        free(intersectionState[ii++]);

    ii = 0;
    while(movementState[ii])
        free(movementState[ii++]);

    rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}
