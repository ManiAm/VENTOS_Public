/****************************************************************************/
/// @file    test_init.c
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    June 2016
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#include <stdio.h>   // printf
#include <stdlib.h>  // exit
#include <signal.h>  // signal
#include <string.h>  // memset
#include <stdint.h>
#include <unistd.h>  // getpid
#include <errno.h>   // errno

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

// A simple netlink example is shown here:
// https://gist.github.com/arunk-s/c897bb9d75a6c98733d6

// message format of netlink
typedef struct {
    // header
    struct nlmsghdr n; // 16 bytes
    struct genlmsghdr g; // 4 bytes

    // payload
    char buf[2000];
} rsi_nlPkt_t;

#define MAX_RCV_SIZE  1600

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

// forward declaration
void sigint(int sigint);

int main(void)
{
    signal(SIGINT, sigint);

    // setting up a Netlink socket in user space
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0)
    {
        printf("socket failed! \n");
        return -1;
    }

    // making src_addr
    struct sockaddr_nl src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    // If the application sets nl_pid to 0, then the kernel takes care of assigning it.
    // The kernel assigns the process ID to the first netlink socket the process opens and
    // assigns a unique nl_pid to every netlink socket that the process subsequently creates.
    src_addr.nl_pid = 0;
    src_addr.nl_groups = 0;  // unicast

    int rc = bind(fd, (struct sockaddr *) &src_addr, sizeof(src_addr));
    if (rc < 0)
    {
        printf("bind failed! \n");
        close(fd);
        return -1;
    }

    // making dest_addr
    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;  // linux kernel
    dest_addr.nl_groups = 0;  // unicast

    uint8_t family_name[25] = "OCB_PKT_TXRX";
    uint8_t *req_buff = malloc( NLMSG_LENGTH(GENL_HDRLEN) + NLMSG_ALIGN(strlen((char*)family_name) + 1 + NLA_HDRLEN) );
    if (!req_buff)
    {
        printf("malloc failed! \n");
        close(fd);
        return -1;
    }

    // resolve the family ID
    rsi_nlPkt_t *request = NULL;
    request = (rsi_nlPkt_t *)req_buff;
    request->n.nlmsg_type  = NLMSG_MIN_TYPE;
    request->n.nlmsg_flags = NLM_F_REQUEST;
    request->n.nlmsg_seq   = 0;
    request->n.nlmsg_pid   = getpid();
    request->n.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);
    request->g.cmd         = CTRL_CMD_GETFAMILY;
    request->g.version     = 0x1;

    struct nlattr *na = NULL;
    na = (struct nlattr *) GENLMSG_DATA(req_buff);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len  = strlen((char*)family_name) + 1 + NLA_HDRLEN;
    strcpy((char*)NLA_DATA(na),(char*)family_name);

    request->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    int r = 0;
    int bufLen = request->n.nlmsg_len;

    // sending req_buff to kernel
    while ((r = sendto(fd, req_buff, bufLen, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr))) < bufLen)
    {
        if (r > 0)
        {
            req_buff += r;
            bufLen -= r;
        }
        else if (errno != EAGAIN)
        {
            printf("sendto failed! \n");
            close(fd);
            free(req_buff);
            return -1;
        }
    }

    uint8_t *rsp_buff = malloc(MAX_RCV_SIZE);
    if (!rsp_buff)
    {
        printf("malloc failed! \n");
        close(fd);
        free(req_buff);
        return -1;
    }

    rsi_nlPkt_t *response = (rsi_nlPkt_t *)rsp_buff;

    int l = recv(fd, response, MAX_RCV_SIZE, 0);
    if (l < 0)
    {
        printf("recv failed! \n");
        close(fd);
        free(req_buff);
        free(rsp_buff);
        return -1;
    }

    // invalid reply message
    if (!NLMSG_OK((&response->n), l))
    {
        printf("invalid reply message! \n");
        close(fd);
        free(req_buff);
        free(rsp_buff);
        return -1;
    }

    // error message
    if (response->n.nlmsg_type == NLMSG_ERROR)
    {
        printf("Driver is not loaded! \n");
        close(fd);
        free(req_buff);
        free(rsp_buff);
        return -1;
    }

    na = (struct nlattr *)GENLMSG_DATA(response);
    na = (struct nlattr *)((char *) na + NLA_ALIGN(na->nla_len));
    int fid = 0;
    if (na->nla_type == CTRL_ATTR_FAMILY_ID)
        fid = *(__u16 *) NLA_DATA(na);

    printf("Driver is loaded: fd= %d, family_id= %d \n", fd, fid);

    close(fd);
    free(req_buff);
    free(rsp_buff);
    return 0;
}


void sigint(int signum)
{
    exit(0);
}
