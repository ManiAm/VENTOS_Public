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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/time.h>    // struct timeval

#include "rsi_wave_util.h"
#include "BSM_create.h"

// forward declarations
void sigint(int sigint);
void process_wsmp(uint8 *, int);
asn_dec_rval_t J2735_decode(void *, int);
void J2735_print(void *, int);
void J2735_free(void *, int);

// global variables
uint8 *buff_rx = NULL;
int lsi = 0;
uint8 psid[4] = {0x20};
int no_of_rx = 0;
int gpio6 = 6;


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

    // --[ making gpio6 ready - start ]--

    char status_buf[80] = {0};

    printf("Exporting the GPIO pin... ");
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd == -1)
    {
        perror("open:export");
        return 1;
    }
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);
    printf("Done! \n");

    printf("Feeding direction 'out' to GPIO... ");
    sprintf(status_buf, "/sys/class/gpio/gpio%d/direction", gpio6);
    fd = open(status_buf, O_WRONLY);
    if(fd == -1)
    {
        perror("open:direction");
        return 1;
    }
    write(fd, "out", 3);
    close(fd);
    printf("Done! \n");

    // --[ making gpio6 ready - end ]--

    buff_rx = malloc(1300);
    if(!buff_rx)
    {
        perror("malloc");
        return 1;
    }

    while(1)
    {
        int length = rsi_wavecombo_receive_wsmp_packet(buff_rx, 1300);
        if(length > 0)
        {
            printf("\nMessage received of size %d... \n", length);
            process_wsmp(buff_rx, length);
            printf("\nNumber of packets received: %d \n", ++no_of_rx);
        }
    }

    return 0;
}


void process_wsmp(uint8 *buf, int len)
{
    int psid_len = 0;
    while(buf[14] & (0x80 >> psid_len++));

    int payload_len = len - WSMP_PAY_LOAD_OFFSET - psid_len;
    asn_dec_rval_t Rval = J2735_decode(buf + WSMP_PAY_LOAD_OFFSET + psid_len, payload_len);

    if(Rval.Message != NULL)
    {
        BasicSafetyMessage_t *bsm_brk = (BasicSafetyMessage_t *) Rval.Message;
        blob_t *blob_brk = (blob_t *) bsm_brk->blob1.buf;
        if(blob_brk != NULL && blob_brk->Brakes == 1)
        {
            // turn on LED
            system("echo 1 > /sys/class/gpio/gpio6/value");

            // wait for 0.5 second
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
    printf("\nTotal Rx: %d \n", no_of_rx);

    if(buff_rx)
        free(buff_rx);

    // unexporting gpio6
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == -1)
        perror("closed");
    char status_buf[80] = {0};
    sprintf(status_buf, "%d", gpio6);
    write(fd, status_buf, strlen(status_buf));
    close(fd);

    rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    exit(0);
}
