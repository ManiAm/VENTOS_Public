/****************************************************************************/
/// @file    utility.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    August 2017
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#ifndef UTILITYVENTOS_H_
#define UTILITYVENTOS_H_

#include <netinet/in.h>

namespace VENTOS {

typedef struct
{
    std::string ipv4_str;
    in_addr_t ipv4_n;
} ipv4_info_t;

class utility
{
public:
    static ipv4_info_t getIPv4ByHostName(std::string);
};

}

#endif
