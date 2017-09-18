
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "utility.h"

namespace VENTOS {

// complete example can be found here:
// http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#getaddrinfo

ipv4_info_t utility::getIPv4ByHostName(std::string host)
{
    struct addrinfo hints = {};
    // AF_UNSPEC: both ipv4 and ipv6
    // AF_INET  : only ipv4
    // AF_INET6 : only ipv6
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    int status = getaddrinfo(host.c_str(), NULL, &hints, &res);
    if (status != 0)
    {
        std::string error = std::string("ERROR in getaddrinfo: ") + gai_strerror(status);
        throw std::runtime_error(error.c_str());
    }

    void *addr = NULL;
    in_addr_t addr_n = {};
    uint32_t numFound = 0;

    for(auto p = res; p != NULL; p = p->ai_next)
    {
        // IPv4
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            addr_n = ipv4->sin_addr.s_addr;

            numFound++;
        }
    }

    if(numFound == 0)
        throw std::runtime_error("ERROR: cannot find IPv4 address of host name");

    if(numFound > 1)
        std::cout << "WARNING: more than one IPv4 address is found for host name '" << host << "' \n" << std::flush;

    // convert the IP to a string
    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, addr, ipstr, sizeof ipstr);

    // free the linked list
    freeaddrinfo(res);

    ipv4_info_t entry;

    entry.ipv4_str = ipstr;
    entry.ipv4_n = addr_n;

    return entry;
}

}
