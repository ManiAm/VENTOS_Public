/****************************************************************************/
/// @file    eebl_rx.c
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
#define RSI_WSMP_SEND 	           0x21
#define RSI_AVAIL_SRVC             0x27
#define SUCCESSS                   0
#define FAILURE                    -1
#define AVAILABLE                  0
#define	REJECTED                   1


void sigint(int sigint);
void process_wsmp(char *buf,int len);
asn_dec_rval_t J2735_decode(void* ,int );

// defined globally to be accessible in sigint
int gpio6 = 6;
int lsi = 0;
char psid[4] = {0x20};
char *pay_load = NULL;
bsm_t *bsm_message = NULL;
blob_t *blob = NULL;
char *buff_rx = NULL;


int main(void)
{
    signal(SIGINT, sigint);

    // un-exporting at the beginning
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("closed");
    char status_buf[80] = {0};
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    printf("Initialization Queue... ");
    int status = rsi_wavecombo_msgqueue_init();
    if(status == FAILURE)
        return 1;

    printf("Initializing MIB... ");;
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

    bsm_message = malloc(sizeof(bsm_t));
    if(!bsm_message)
        perror("malloc");
    memset(bsm_message,0,sizeof(sizeof(bsm_t)));

    // ########################
    // end of memory allocation
    // ########################

    printf("Exporting the GPIO pin... ");
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd == -1)
        perror("open:export");
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);
    printf("Done! \n");

    printf("Feeding direction 'out' to GPIO... ");
    sprintf(status_buf, "/sys/class/gpio/gpio%d/direction", gpio6);
    fd = open(status_buf, O_WRONLY);
    if(fd == -1)
        perror("open:direction");
    write(fd, "out", 3);
    close(fd);
    printf("Done! \n");

    buff_rx = malloc(1300);
    if(!buff_rx)
        perror("malloc");

    while(1)
    {
        printf("Listening... \n\n");
        int length = rsi_wavecombo_receive_wsmp_packet(buff_rx, 1300);
        if(length > 0)
        {
            printf("EEBL message received of size %d... \n", length);
            process_wsmp(buff_rx, length);
        }
    }

    // ############
    // finishing up
    // ############

    if(buff_rx)
        free(buff_rx);

    // un-exporting at the beginning
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("closed");
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    status = rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    return 0;
}


void process_wsmp(char *buf,int len)
{
    int psid_len = 0;
    while( buf[14] & (0x80>>psid_len++) );

    int payload_len = len - WSMP_PAY_LOAD_OFFSET - psid_len;
    asn_dec_rval_t Rval = J2735_decode(buf + WSMP_PAY_LOAD_OFFSET + psid_len, payload_len);
    if(Rval.Message != NULL)
    {
        BasicSafetyMessage_t *bsm_brk =(BasicSafetyMessage_t *) Rval.Message;
        blob_t *blob_brk = (blob_t *) bsm_brk->blob1.buf;
        if(blob_brk == NULL)
        {

        }
        else if(blob_brk->Brakes == 1)
        {
            // turn on LED
            system("echo 1 > /sys/class/gpio/gpio6/value");

            usleep(500000);

            // turn off LED
            system("echo 0 > /sys/class/gpio/gpio6/value");
        }

        J2735_print(Rval.Message, Rval.Type);
        J2735_free(Rval.Message, Rval.Type);
    }
}


void sigint(int signum)
{
    if(buff_rx != NULL)
        free(buff_rx);

    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("closed");
    char status_buf[80] = {0};
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    int status = rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}
