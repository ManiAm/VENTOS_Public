#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "wsm_include.h"

// global variables
int sockfd = -1;
struct ifreq if_idx;
struct ifreq if_mac;
char myIPaddr[20];

#define MY_DEST_MAC0    0x00
#define MY_DEST_MAC1    0x00
#define MY_DEST_MAC2    0x00
#define MY_DEST_MAC3    0x00
#define MY_DEST_MAC4    0x00
#define MY_DEST_MAC5    0x00

#define BUF_SIZ     1024

// calculating the checksum field
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


int sendToVENTOS(waveShortMessage* wsm)
{
    // construct the Ethernet header
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

    // convert wsm message into a byte-array
    char b[sizeof(*wsm)];
    memcpy(b, wsm, sizeof(*wsm));
    unsigned int i;
    for(i = 0; i < sizeof(*wsm); i++)
        sendbuf[tx_len++] = (int)b[i];

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
