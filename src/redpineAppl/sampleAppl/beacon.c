/****************************************************************************/
/// @file    beacon.c
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

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

#define MY_DEST_MAC0    0x00
#define MY_DEST_MAC1    0x00
#define MY_DEST_MAC2    0x00
#define MY_DEST_MAC3    0x00
#define MY_DEST_MAC4    0x00
#define MY_DEST_MAC5    0x00

#define BUF_SIZ     1024

void init_socket();
int sendToVENTOS(char *msg);
unsigned short csum(unsigned short *buf, int nwords);
void sigint(int sigint);
asn_dec_rval_t J2735_decode(void*, int);

// global variables -- to be accessible in sigint
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

// global variables -- used by socket
int sockfd = -1;
struct ifreq if_idx;
struct ifreq if_mac;
char myIPaddr[20];

int main(void)
{
    signal(SIGINT, sigint);

    // un-exporting gpio9
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

    date = malloc(sizeof(ddate_t));
    if(!date)
        perror("malloc");
    memset(date,0,sizeof(ddate_t));

    initialPosition = malloc(sizeof(initialPosition_t));
    if(!initialPosition)
        perror("malloc");
    memset(initialPosition, 0, sizeof(initialPosition_t));

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

    // todo: where this variable is being used?
    rTCMPackage = malloc(sizeof(rTCMPackage_t));
    if(!rTCMPackage)
        perror("malloc");
    memset(rTCMPackage, 0, sizeof(rTCMPackage_t));

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

    blob = malloc(sizeof(blob_t));
    if(!blob)
        perror("malloc");
    memset(blob, 0, sizeof(sizeof(blob_t)));

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

    bsm_message = malloc(sizeof(bsm_t));
    if(!bsm_message)
        perror("malloc");
    memset(bsm_message, 0, sizeof(sizeof(bsm_t)));

    bsm_message->blob = (char *) blob;
    bsm_message->extention = vehiclesafetyextension;

    pay_load = malloc(512);
    if(!pay_load)
        perror("malloc");
    memset(pay_load, 0, 512);

    time_buf = malloc(50);
    if(!time_buf)
        perror("malloc");
    memset(time_buf, 0, 50);

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

    sprintf(status_buf, "/sys/class/gpio/gpio%d/value", gpio9);

    // initialization of socket
    init_socket();

    int msgCount = 0;
    printf("\n");   // adding a new line to improve readability

    // main loop
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

        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t current_time = tv.tv_sec;
        strftime(time_buf, 50, "%Y %d %m %H %M %S", localtime(&current_time));
        sscanf(time_buf, "%ld %ld %ld %ld %ld %ld", &date->year, &date->day, &date->month, &date->hour, &date->minute, &date->second);

        msgCount++;
        blob->MsgCnt = msgCount;

        int pay_load_len = 0;
        if(bsm_create(bsm_message, pay_load, &pay_load_len) == SUCCESS)
        {
            printf("%02ld/%02ld/%ld %02ld:%02ld:%02ld BSM message #%d created of size %d... ", date->month, date->day, date->year, date->hour, date->minute, date->second, msgCount, pay_load_len);
        }
        else
        {
            printf("BSM message encoding failed. \n");
            break;
        }

        waveShortMessage *wsm = malloc(sizeof(waveShortMessage));
        if(!wsm)
            perror("malloc");

        memset(wsm, 0, sizeof(waveShortMessage));

        wsm->dataRate     = RATE_6;
        wsm->txPwr        = 15;
        wsm->psid[0]      = psid[0];
        wsm->psid[1]      = psid[1];
        wsm->psid[2]      = psid[2];
        wsm->psid[3]      = psid[3];
        wsm->priority     = 3;
        wsm->wsm_expiry_time = 50;
        wsm->wsm_length = pay_load_len;
        wsm->channelNumber = 172;

        memcpy(wsm->WSM_Data, pay_load, pay_load_len);

        char peer_mac_address[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
        memcpy(wsm->peer_mac_address, peer_mac_address, 6);

        sendToVENTOS("testing");

        status = rsi_wavecombo_wsmp_msg_send(wsm);
        if(status < 0)
            printf("Failed! \n");
        else
            printf("Sent! \n");

        sleep(1);
        free(wsm);
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


unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum = 0; nwords > 0; nwords--)
        sum += *buf++;

    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);

    return (unsigned short)(~sum);
}


void init_socket()
{
    /* Get interface name */
    char ifName[IFNAMSIZ];
    strcpy(ifName, "eth0");  // default interface

    // add new line to improve readability
    printf("\n");

    /* Open RAW socket to send on */
    printf("Opening interface %s ... ", ifName);
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1)
        perror("socket");
    printf("Done! \n");

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");
    printf("    Interface index is %d \n", if_idx.ifr_ifindex);

    /* Get the IPv4 address attached to ifName */
    struct ifreq if_ip;
    memset(&if_ip, 0, sizeof(struct ifreq));
    strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
    if(ioctl(sockfd, SIOCGIFADDR, &if_ip) < 0)
        perror("SIOCGIFADDR");
    char *ipv4 = inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr);
    strcpy(myIPaddr, ipv4);
    printf("    IPv4 address is %s \n", myIPaddr);

    /* Get the subnet mask of ifName */
    struct ifreq if_subnet;
    memset(&if_subnet, 0, sizeof(struct ifreq));
    strncpy(if_subnet.ifr_name, ifName, IFNAMSIZ-1);
    if(ioctl(sockfd, SIOCGIFNETMASK, &if_subnet) < 0)
        perror("SIOCGIFNETMASK");
    char *mySubnet = inet_ntoa(((struct sockaddr_in *)&if_subnet.ifr_addr)->sin_addr);
    printf("    Subnet mask is %s \n", mySubnet);

    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, "eth0", IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        printf("    MAC address is %02x:%02x:%02x:%02x:%02x:%02x \n",
                (unsigned char) if_mac.ifr_hwaddr.sa_data[0],
                (unsigned char) if_mac.ifr_hwaddr.sa_data[1],
                (unsigned char) if_mac.ifr_hwaddr.sa_data[2],
                (unsigned char) if_mac.ifr_hwaddr.sa_data[3],
                (unsigned char) if_mac.ifr_hwaddr.sa_data[4],
                (unsigned char) if_mac.ifr_hwaddr.sa_data[5]);
}


int sendToVENTOS(char *msg)
{
    /* Construct the Ethernet header */
    char sendbuf[BUF_SIZ];
    memset(sendbuf, 0, BUF_SIZ);
    struct ether_header *eh = (struct ether_header *) sendbuf;
    // fill-in the source MAC address
    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
    // fill-in the destination MAC address
    eh->ether_dhost[0] = MY_DEST_MAC0;
    eh->ether_dhost[1] = MY_DEST_MAC1;
    eh->ether_dhost[2] = MY_DEST_MAC2;
    eh->ether_dhost[3] = MY_DEST_MAC3;
    eh->ether_dhost[4] = MY_DEST_MAC4;
    eh->ether_dhost[5] = MY_DEST_MAC5;

    /* Ethertype field */
    eh->ether_type = htons(ETH_P_IP);
    int tx_len = 0;
    tx_len += sizeof(struct ether_header);

    /* Construct the IP Header */
    struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 16; // Low delay
    iph->id = htons(54321);
    iph->ttl = 25; // hops
    iph->protocol = 17; // UDP
    iph->saddr = inet_addr(myIPaddr); // Source IP address
    iph->daddr = inet_addr("192.168.60.111"); // Destination IP address
    tx_len += sizeof(struct iphdr);

    /* Construct the UDP Header */
    struct udphdr *udph = (struct udphdr *) (sendbuf + sizeof(struct iphdr) + sizeof(struct ether_header));
    udph->source = htons(3423);
    udph->dest = htons(5342);
    udph->check = 0; // skip
    tx_len += sizeof(struct udphdr);

    /* Packet data: dead beef */
    sendbuf[tx_len++] = 0xde;
    sendbuf[tx_len++] = 0xad;
    sendbuf[tx_len++] = 0xbe;
    sendbuf[tx_len++] = 0xef;

    /* Length of UDP payload and header */
    udph->len = htons(tx_len - sizeof(struct ether_header) - sizeof(struct iphdr));
    /* Length of IP payload and header */
    iph->tot_len = htons(tx_len - sizeof(struct ether_header));
    /* Calculate IP checksum on completed header */
    iph->check = csum( (unsigned short *)(sendbuf+sizeof(struct ether_header)), sizeof(struct iphdr)/2 );

    /* Index of the network device */
    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length */
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = MY_DEST_MAC0;
    socket_address.sll_addr[1] = MY_DEST_MAC1;
    socket_address.sll_addr[2] = MY_DEST_MAC2;
    socket_address.sll_addr[3] = MY_DEST_MAC3;
    socket_address.sll_addr[4] = MY_DEST_MAC4;
    socket_address.sll_addr[5] = MY_DEST_MAC5;

    /* Send packet */
    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
        printf("Send failed \n");

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
