#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include "rsi_wave_util.h"
#include "BSM_create.h"
#include <time.h>


#define OPERATING_CLASS   17
#define CONTROL_CHANNEL   178
#define CCH_INTERVEL      50
#define SCH_INTERVEL      50
#define SYNC_TOLERANCE    02
#define MAX_SWITCH_TIME   02
#define ADD               01
#define DELETE            02
#define CCH_INTERVAL 1
#define SCH_INTERVAL 2
#define CCH_AND_SCH_INTERVAL 3
#define RSI_CCH_SERVICE_REQUEST 0x19
#define RSI_USER_SERVICE_REQUEST 0x20
#define RSI_WSMP_SEND 	       0x21
#define RSI_AVAIL_SRVC          0x27
#define SUCCESSS           0
#define FAILURE            -1
#define AVAILABLE          0
#define	REJECTED           1

void process_wsmp(char *buf,int len);
asn_dec_rval_t J2735_decode(void* ,int );
void sigint(int sigint);
void * message_tx();
pthread_t thread_id_tx;
pthread_t thread_id_rx;
bsm_t	*bsm_message = NULL;

char *buff_rx = NULL;
char *pay_load = NULL;
int lsi = 0;
char psid[4]={0x20};
waveShortMessage *wsm = NULL;


int main(void)
{
    int status = 0;
    void* res = 0;
    int pay_load_len = 0;
    int i = 0;
    struct timeval tv;
    time_t current_time = 0;
    char peer_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
    blob_t *blob = NULL;

    signal(SIGINT, sigint);

    printf("Initialization Queue... ");
    status = rsi_wavecombo_msgqueue_init();
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

    printf("Setting UTC... ");
    status = rsi_wavecombo_set_utc();
    if(status == FAILURE)
        return 1;
    printf("Done! \n");

    lsi = rsi_wavecombo_local_service_index_request();
    if(lsi <= 0)
        return 1;

    status =  rsi_wavecombo_wsmp_queue_init(lsi);

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


    //pthread_create(&thread_id_rx,NULL,message_rx,NULL);

    /****************************************		****************/

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

    // export the gpio and set the direction only once
    system("echo 6 > /sys/class/gpio/export");
    system("echo \"out\" > /sys/class/gpio/gpio6/direction");

    while(1)
    {
        int len, length;
        buff_rx = malloc(1300);
        if(!buff_rx)
            perror("malloc");

        while(1)
        {
            length = rsi_wavecombo_receive_wsmp_packet(buff_rx, 1300);
            if(length > 0)
                process_wsmp(buff_rx,length);
        }
    }

    free(buff_rx);
    status = rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    return 0;
}


void process_wsmp(char *buf,int len)
{
    static int j=0 ;
    int psid_len = 0;
    int payload_len = 0;
    int fd;
    int gpio6 = 6;
    char status_buf[40];

    asn_dec_rval_t Rval;
    blob_t *blob_brk = NULL;
    BasicSafetyMessage_t *bsm_brk =NULL;

    while(buf[14] & (0x80>>psid_len++));
    printf("\nEEBL message received :%d\n",j++);
    payload_len = len - WSMP_PAY_LOAD_OFFSET - psid_len;
    Rval = J2735_decode(buf + WSMP_PAY_LOAD_OFFSET + psid_len,payload_len);
    if(Rval.Message != NULL)
    {
        bsm_brk =(BasicSafetyMessage_t *) Rval.Message;
        blob_brk= (blob_t *)bsm_brk->blob1.buf;
        if(blob_brk == NULL) {

        }

        else if(blob_brk->Brakes == 1)
        {
            system("echo 1 > /sys/class/gpio/gpio6/value");
            //printf(" *************LED ON***************\n");
            usleep(500000);

            //printf("*************LED OFF****************\n");
            system("echo 0 > /sys/class/gpio/gpio6/value");
        }

        J2735_print(Rval.Message,Rval.Type);
        J2735_free(Rval.Message,Rval.Type);
    }
}


void sigint(int signum)
{
    /*Clean up.......*/
    int status = 0;
    void* res = 0;

    system("echo 6 > /sys/class/gpio/unexport"); // unexport  the gpio
    status = rsi_wavecombo_wsmp_service_req(DELETE, lsi, psid);
    rsi_wavecombo_msgqueue_deinit();

    if(buff_rx != NULL)
        free(buff_rx);

    exit(0);
}
