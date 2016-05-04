/****************************************************************************/
/// @file    wsa_include.h
/// @author
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

#ifndef _ONEBOX_WSA_H_
#define _ONEBOX_WSA_H_

#include "onebox_util.h"

#define SCH_AND_CCH_INTERVAL 0 //CONTINUOS ACCESS
#define SCH_INTERVAL_ONLY 1 //ALTERNATE ACCESS

//typedef char uint8;
typedef unsigned short int uint16;

/* 11p ioctls response codes*/
#define WME_BE_Q        0   //best_effort
#define WME_BK_Q        1   //background
#define WME_VI_Q        2   //video	
#define WME_VO_Q        3   //voice

/*structure definition*/
struct ieee80211_wme_acparams
{
    uint8		acp_aci_aifsn;
    uint8		acp_logcwminmax;
    uint8	        acp_txop[2];

}__attribute__((__packed__));

typedef struct
{
    uint8 latitude[4];
    uint8 longitude[4];
    uint8 elevation[2];
    uint8 positionElevationConfidence;
    uint8 positionAccuracy[4];

}__attribute__((__packed__)) _rsi_location3D_t;

typedef struct
{
    uint8 rsu_macid[6];
    uint8 WSAversion_Changecnt;
    uint8 repeatCount;
    char txPower;
    uint8 latitude[4];
    uint8 longitude[4];
    _rsi_location3D_t location3D;
    uint8 advertiseIdentifier[32];
    uint8 country[3];

}__attribute__((__packed__)) _rsi_WSAHeader_t;

typedef struct 
{
    uint8 WAVE_element_id;
    uint8 PSID[4];
    uint8 servicePriority;
    uint8 channelIndex;
    uint8 providerServiceContext[31];
    uint8 IPv6Address[16];
    uint8 servicePort[2];
    uint8 providerMACId[6];
    char RCPI_threshold;
    uint8 WSAcountThreshold;
    uint8 WSAcountThresholdInterval;

}__attribute__((__packed__)) _rsi_WSAServiceInfo_t;

typedef struct
{
    uint8 WAVEelementId;
    uint8 operatingClass;
    uint8 channelNumber;
    uint8 adaptable;
    uint8 dataRate;
    uint8 txPower;
    uint8 edca_id;
    uint8 edca_len;
    uint8 qos_info;
    uint8 reserved;
    struct ieee80211_wme_acparams  wme_param[4];
    //uint8 edca params should add.....to discuss later;
    uint8 channelAccess;

}__attribute__((__packed__)) _rsi_ChannelInfo_t;

typedef struct
{
    uint8 WAVEelemntId;
    uint8 routerLifetime[2];
    uint8 ipPrefix[16];
    uint8 prefixLength;
    uint8 defaultGateway[16];
    uint8 primaryDNS[16];
    uint8 secondaryDNS[16];
    uint8 gatewayMACAddress[6];

}__attribute__((__packed__)) _rsi_WAVE_RoutingAdvertisement_t;

typedef struct 
{
    _rsi_WSAHeader_t WSAheader;
    _rsi_WSAServiceInfo_t serviceInfo[32];
    _rsi_ChannelInfo_t channelInfo[32];
    _rsi_WAVE_RoutingAdvertisement_t WAVEroutingAdvertisement;

}__attribute__((__packed__)) WSA_Frame_t;

struct wsa_info
{
    unsigned char  provider_macaddr[6];
    unsigned char  channel;
    unsigned char  adaptable;
    unsigned char  rate;
    unsigned char  tx_pwr_level;
    unsigned char  edca_id;
    unsigned char  edca_len;
    unsigned char  qos_info;
    unsigned char  reserved;
    struct ieee80211_wme_acparams  wme_params[4];

}__attribute__((__packed__)) ;

void print_wsa(WSA_Frame_t wsa_p);

#endif
