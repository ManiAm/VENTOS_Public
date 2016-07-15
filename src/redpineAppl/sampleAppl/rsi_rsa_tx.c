/****************************************************************************/
/// @file    rsi_rsa_tx.c
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
#include "RSA_create.h"

// forward declarations
void sigint(int sigint);
int rsa_create(rsa_t *, char *, int *);

int lsi = 0;
uint8 psid[4]={0xe0,0x52,0x53,0x41};
int no_of_tx = 0;
char *pay_load = NULL;

// defined globally to be accessible in sigint
waveShortMessage *wsm = NULL;
rsa_t *rsa_message = NULL;
initialPosition_t *initialPosition = NULL;
ddate_t *date = NULL;	
char *time_buf = NULL;


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

    // request stack to allocate radio resources to the indicated service channel
    printf("Sending SCH service request... ");
    status = rsi_wavecombo_sch_start_req(atoi(argv[1]) ,atoi(argv[2]) ,1 ,255);
    if(status == FAIL)
        return 1;
    printf("Done! \n");

    // --[ WAVE initialization - start ]--

    // --[ making rsa_message - start ]--

    rsa_message = malloc(sizeof(rsa_t));
    if(!rsa_message)
    {
        perror("malloc");
        return 1;
    }
    memset(rsa_message,0,sizeof(sizeof(rsa_t)));

    time_buf = malloc(50);
    if(!time_buf)
    {
        perror("malloc");
        return 1;
    }
    memset(time_buf,0,50);

    date = malloc(sizeof(ddate_t));
    if(!date)
    {
        perror("malloc");
        return 1;
    }
    memset(date,0,sizeof(ddate_t));

    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t current_time = tv.tv_sec;
    strftime(time_buf, 50, "%Y %d %m %H %M %S", localtime(&current_time));
    sscanf(time_buf, "%ld %ld %ld %ld %ld %ld", &date->year, &date->day, &date->month, &date->hour, &date->minute, &date->second);

    initialPosition = malloc(sizeof(initialPosition_t));
    if(!initialPosition)
    {
        perror("malloc");
        return 1;
    }
    memset(initialPosition,0,sizeof(initialPosition_t));

    initialPosition->utcTime	= date;
    initialPosition->Long 		= 783793635;
    initialPosition->lat 		= 174449712;
    initialPosition->heading 	= 0x00;
    initialPosition->timeConfidence	= _TimeConfidence_unavailable;
    initialPosition->posConfidence	= 0;
    initialPosition->speedConfidence= 0;
    initialPosition->elevation_size = 0x01;
    initialPosition->elevation[0] = 0x02;
    initialPosition->speed_size = 0x00;
    initialPosition->speed[0] = 0x00;
    initialPosition->posAccuracy_size = 0x00;
    initialPosition->posAccuracy[0] = 0x00;

    rsa_message->id 	= _DSRCmsgID_roadSideAlert;
    rsa_message->event 	= 6952;
    rsa_message->extent 	= _Extent_useFor100meters;
    rsa_message->position	= initialPosition;
    rsa_message->count	= 0x3;
    int ii = 0;
    for(ii=0;ii<rsa_message->count ;ii++)
    {
        rsa_message->description[ii] = 0;
    }
    rsa_message->priority_size = 0x01;
    rsa_message->priority[0] = 0x01;
    rsa_message->heading_size  = 0x2;
    rsa_message->heading[0]  = 0xff;
    rsa_message->heading[1]  = 0xff;
    rsa_message->fid_size = 0x01;
    rsa_message->fid[0] = 0x70;
    rsa_message->crc_size = 0x02;
    rsa_message->crc[0] = 0x00;
    rsa_message->crc[1] = 0x00;

    // --[ making rsa_message - end ]--

    pay_load = malloc(1200);
    if(!pay_load)
    {
        perror("malloc");
        return 1;
    }
    memset(pay_load,0,1200);

    // adding new line to improve readability
    printf("\n");

    while(1)
    {
        rsa_message->cnt = ++no_of_tx;
        int pay_load_len = 0;

        if(rsa_create(rsa_message,pay_load,&pay_load_len) != SUCCESS)
        {
            printf("RSA message encoding failed. \n");
            return 1;
        }

        wsm = malloc(sizeof(waveShortMessage));
        if(!wsm)
        {
            perror("malloc");
            return 1;
        }
        memset(wsm,0,sizeof(waveShortMessage));

        wsm->dataRate 	  = atoi(argv[2]);//RATE_12;
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

        printf("Sending RSA msg #%3d of size %d... ", no_of_tx, pay_load_len);

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

    if(rsa_message)
        free(rsa_message);

    if(date)
        free(date);

    if(time_buf)
        free(time_buf);

    if(initialPosition)
        free(initialPosition);

    rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}

