/****************************************************************************/
/// @file    eebl_tx.c
/// @author
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#define _BSD_SOURCE

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/time.h>

#include "rsi_wave_util.h"
#include "BSM_create.h"


#define OPERATING_CLASS            17
#define CONTROL_CHANNEL            178
#define CCH_INTERVEL               50
#define SCH_INTERVEL               50
#define SYNC_TOLERANCE             02
#define MAX_SWITCH_TIME            02
#define ADD                        01
#define DELETE                     02
#define CCH_INTERVAL               1
#define SCH_INTERVAL               2
#define CCH_AND_SCH_INTERVAL       3
#define RSI_CCH_SERVICE_REQUEST    0x19
#define RSI_USER_SERVICE_REQUEST   0x20
#define RSI_WSMP_SEND              0x21
#define RSI_AVAIL_SRVC             0x27
#define SUCCESSS                   0
#define FAILURE                    -1
#define AVAILABLE                  0
#define REJECTED                   1


void sigint(int sigint);
asn_dec_rval_t J2735_decode(void*, int);

// defined globally to be accessible in sigint
int gpio9 = 9;
int lsi = 0;
char psid[4] = {0x20};
char *pay_load = NULL;
blob_t *blob = NULL;
char *time_buf = NULL;
ddate_t *date = NULL;
initialPosition_t *initialPosition = NULL;
rTCMPackage_t *rTCMPackage = NULL;
path_t *path = NULL;
pathPrediction_t *pathPrediction = NULL;
vehiclesafetyExtension_t *vehiclesafetyextension = NULL;
bsm_t *bsm_message = NULL;


int main(void)
{
    signal(SIGINT, sigint);

    // un-exporting at the beginning
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("closed");
    char status_buf[80] = {0};
    sprintf(status_buf, "%d", gpio9);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    printf("Initialization Queue... ");
    int status = rsi_wavecombo_msgqueue_init();
    if(status == FAILURE)
        return 1;

    printf("Initializing MIB... ");
    status = rsi_wavecombo_1609mib_init();
    if(status == FAILURE)
        return 1;
    printf("Done! \n");

    printf("Calling update sync params... ");
    status = rsi_wavecombo_update_channel_sync_params(OPERATING_CLASS, CONTROL_CHANNEL, CCH_INTERVEL, SCH_INTERVEL, SYNC_TOLERANCE, MAX_SWITCH_TIME);
    if(status == FAILURE)
        return 1;
    printf("Done! \n");

    //    printf("Setting UTC... ");
    //    status = rsi_wavecombo_set_utc();
    //    if(status == FAILURE)
    //        return 1;
    //    printf("Done! \n");

    lsi = rsi_wavecombo_local_service_index_request();
    if(lsi <= 0)
        return 1;

    status = rsi_wavecombo_wsmp_queue_init(lsi);

    printf("Sending WSMP service request... ");
    status = rsi_wavecombo_wsmp_service_req(ADD, lsi, psid);
    if(status == FAILURE)
        return 1;
    printf("Done! \n");

    printf("Sending SCH service request... ");
    status = rsi_wavecombo_sch_start_req(172, RATE_6, 1, 255);
    if(status == FAILURE)
        return 1;
    printf("Done! \n");

    // ##########################
    // start of memory allocation
    // ##########################

    pay_load = malloc(512);
    if(!pay_load)
        perror("malloc");
    memset(pay_load,0,512);

    blob = malloc(sizeof(blob_t));
    if(!blob)
        perror("malloc");
    memset(blob,0,sizeof(sizeof(blob_t)));

    blob->MsgCnt = 0;
    blob->Id = 0x32;
    blob->SecMark = 0x01;
    blob->Lat = 0xbc4d4f21;
    blob->Long = 0xcb39edc1;
    blob->Elev = 0xcb;
    blob->Accuracy = 0;
    blob->Heading = 0;
    blob->AccelLong = 0;
    blob->AccelLatSet = 0;
    blob->AccelVert = 0;
    blob->AccelYaw = 0;
    blob->Brakes = 1;
    blob->VehicleWidth_MSB = 0x24;
    blob->VehicleLength_WidthLSB = 0x18;

    time_buf = malloc(50);
    if(!time_buf)
        perror("malloc");
    memset(time_buf,0,50);

    date = malloc(sizeof(ddate_t));
    if(!date)
        perror("malloc");
    memset(date,0,sizeof(ddate_t));

    initialPosition = malloc(sizeof(initialPosition_t));
    if(!initialPosition)
        perror("malloc");
    memset(initialPosition,0,sizeof(initialPosition_t));

    initialPosition->utcTime    = date;
    initialPosition->Long       = 0xbc4d4f21;
    initialPosition->lat        = 0xcb39edc1;
    initialPosition->elevation_size = 0x02;
    initialPosition->elevation[0]   = 0x5b;
    initialPosition->elevation[1]   = 0x5b;
    initialPosition->heading    = 0xffff;
    initialPosition->timeConfidence = 3;
    initialPosition->posConfidence  = 3;
    initialPosition->speedConfidence= 3;
    initialPosition->elevation_size = 1;
    initialPosition->elevation[0] = 0x02;
    initialPosition->speed_size = 0x01;
    initialPosition->speed[0] = 0x03;
    initialPosition->posAccuracy_size = 0x01;
    initialPosition->posAccuracy[0] = 0x03;

    rTCMPackage = malloc(sizeof(rTCMPackage_t));
    if(!rTCMPackage)
        perror("malloc");
    memset(rTCMPackage,0,sizeof(rTCMPackage_t));

    rTCMPackage->anchorPoint = initialPosition;
    rTCMPackage->rtcmHeader_size = 0x1;
    rTCMPackage->rtcmHeader[0] = 0x42;
    rTCMPackage->msg1001_size = 0x01;
    rTCMPackage->msg1001[0] = 0x01;

    path = malloc(sizeof(path_t));
    if(!path)
        perror("malloc");
    memset(path,0,sizeof(path_t));

    path->crumbData.present = 1;
    path->crumbData.pathHistoryPointSets_01_count = 1;
    path->crumbData.pathHistoryPointSets_01[0].latOffset = 0x05;
    path->crumbData.pathHistoryPointSets_01[0].longOffset = 0x04;
    path->crumbData.pathHistoryPointSets_01[0].elevationOffset = 0x00;
    path->crumbData.pathHistoryPointSets_01[0].timeOffset = 0x0;
    path->crumbData.pathHistoryPointSets_01[0].posAccuracy_size = 0x02;
    path->crumbData.pathHistoryPointSets_01[0].posAccuracy[0] = 0x00;
    path->crumbData.pathHistoryPointSets_01[0].posAccuracy[1] = 0x00;
    path->crumbData.pathHistoryPointSets_01[0].heading = 0x01;
    path->crumbData.pathHistoryPointSets_01[0].speed_size = 0x02;
    path->crumbData.pathHistoryPointSets_01[0].speed[0] = 0x01;
    path->crumbData.pathHistoryPointSets_01[0].speed[1] = 0x02;
    path->crumbData.data_size = 0x01;
    path->crumbData.data[0] = 0x01;
    path->crumbData.data[1] = 0x02;
    path->crumbData.data[2] = 0x03;

    pathPrediction = malloc(sizeof(pathPrediction_t));
    if(!pathPrediction)
        perror("malloc");
    memset(pathPrediction,0,sizeof(pathPrediction_t));

    pathPrediction->radiusOfCurve = 0x3;
    pathPrediction->confidence = 0x3;

    vehiclesafetyextension = malloc(sizeof(vehiclesafetyExtension_t));
    if(!vehiclesafetyextension)
        perror("malloc");
    memset(vehiclesafetyextension, 0, sizeof(vehiclesafetyExtension_t));

    vehiclesafetyextension->events = 128;
    vehiclesafetyextension->path = path;
    vehiclesafetyextension->pathPrediction = pathPrediction;
    vehiclesafetyextension->theRTCM = NULL; //rTCMPackage;

    bsm_message = malloc(sizeof(bsm_t));
    if(!bsm_message)
        perror("malloc");
    memset(bsm_message, 0, sizeof(sizeof(bsm_t)));

    bsm_message->blob = (char *) blob;
    bsm_message->extention = vehiclesafetyextension;

    // ########################
    // end of memory allocation
    // ########################

    printf("Exporting the GPIO pin... ");
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd == -1)
        perror("open:export");
    sprintf(status_buf, "%d", gpio9);
    write(fd, status_buf, strlen(status_buf));
    close(fd);
    printf("Done! \n");

    printf("Feeding direction 'in' to GPIO... ");
    sprintf(status_buf, "/sys/class/gpio/gpio%d/direction", gpio9);
    fd = open(status_buf, O_WRONLY);
    if(fd == -1)
        perror("open:direction");
    write(fd, "in", 2);
    close(fd);
    printf("Done! \n");

    sprintf(status_buf,"/sys/class/gpio/gpio%d/value", gpio9);

    while(1)
    {
        char switch_status = '1';

        // keep polling switch until pressed -- not pressed: '1' or 0x31, pressed: '0' or 0x30
        while(switch_status != '0')
        {
            // we need to open this every time!
            fd = open(status_buf, O_RDONLY);
            if(fd == -1)
                perror("open:value");

            read(fd, &switch_status, 1);
            close(fd);
        }

        printf("Emergency break applied. Sending 5 messages... \n");

        for(int i = 1; i <= 5; i++)
        {
            int pay_load_len = 0;

            struct timeval tv;
            gettimeofday(&tv,NULL);
            time_t current_time = tv.tv_sec;
            strftime(time_buf,50,"%Y %d %m %H %M %S",localtime(&current_time));
            sscanf(time_buf,"%ld %ld %ld %ld %ld %ld", &date->year, &date->day, &date->month, &date->hour, &date->minute, &date->second);
            blob->MsgCnt = i;

            if(bsm_create(bsm_message, pay_load, &pay_load_len) == SUCCESS)
            {
                printf("    %d. BSM message created of size %d... ", i, pay_load_len);
            }
            else
            {
                printf("    BSM message encoding failed. \n");
                break;
            }

            waveShortMessage *wsm = malloc(sizeof(waveShortMessage));
            if(!wsm)
                perror("malloc");

            memset(wsm,0,sizeof(waveShortMessage));

            wsm->dataRate 	  = RATE_6;
            wsm->txPwr   	  = 15;
            wsm->psid[0] 	  = psid[0];
            wsm->psid[1] 	  = psid[1];
            wsm->psid[2] 	  = psid[2];
            wsm->psid[3] 	  = psid[3];
            wsm->priority 	  = 3;
            wsm->wsm_expiry_time = 50;
            wsm->wsm_length = pay_load_len;
            wsm->channelNumber = 172;

            memcpy(wsm->WSM_Data, pay_load, pay_load_len);

            char peer_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
            memcpy(wsm->peer_mac_address, peer_mac_address, 6);

            status = rsi_wavecombo_wsmp_msg_send(wsm);
            if(status < 0)
                printf("Failed! \n");
            else
                printf("Sent! \n");

            sleep(1);
            free(wsm);
        }
    }

    // ############
    // finishing up
    // ############

    free(pay_load);
    free(blob);
    free(time_buf);
    free(date);
    free(initialPosition);
    free(rTCMPackage);
    free(path);
    free(pathPrediction);
    free(vehiclesafetyextension);
    free(bsm_message);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("open");
    sprintf(status_buf, "%d", gpio9);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    status = rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    return 0;
}


void sigint(int signum)
{
    if(pay_load)
        free(pay_load);

    if(blob)
        free(blob);

    if(time_buf)
        free(time_buf);

    if(date)
        free(date);

    if(initialPosition)
        free(initialPosition);

    if(rTCMPackage)
        free(rTCMPackage);

    if(path)
        free(path);

    if(pathPrediction)
        free(pathPrediction);

    if(vehiclesafetyextension)
        free(vehiclesafetyextension);

    if(bsm_message)
        free(bsm_message);

    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("open");
    char status_buf[80] = {0};
    sprintf(status_buf, "%d", gpio9);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    int status = rsi_wavecombo_wsmp_service_req(DELETE , lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}

