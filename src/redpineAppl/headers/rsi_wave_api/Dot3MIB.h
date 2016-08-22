/****************************************************************************/
/// @file    Dot3MIB.h
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

#ifndef __ONEBOX_DOT3MIB_H__
#define __ONEBOX_DOT3MIB_H__

#include "onebox_util.h"
#include "wsa_include.h"

typedef uint_8 uint8;
typedef uint_16 uint16;
typedef uint_32 uint32;

#define MAX_AVAILABLE_SERVICES 32

#define CCH_INTERVAL 1
#define SCH_INTERVAL 2
#define CCH_AND_SCH_INTERVAL 3

#define SERVICE_ADD         1
#define SERVICE_DELETE      2
#define SERVICE_CHANGE      3

typedef struct rsi_16093_StationConfigEntry 
{
    uint16 		StationConfigIndex ; //interfaces present -1
    boolean 	CchAccessImplemented ; 	 //true
    boolean 	SchAccessImplemented ;  //true
    boolean 	IpV6Implemented ;   //false
    boolean 	UdpImplemented ;  //false
    boolean 	TcpImplemented ; //false
    boolean	 	WsmpImplemented ; //true
    boolean	 	WsmpSImplemented ; //false
    boolean 	UserRoleImplemented ; //true
    boolean 	ProviderRoleImplemented ;  //false
    boolean 	TimingAdvertisementServiceImplemented ; // false
    boolean 	ManagementDataServiceImplemented; //true

}__attribute__((__packed__)) _rsi_16093_StationConfigEntry_t, *_rsi_16093_StationConfigEntry_tp;

typedef struct rsi_16093_LocalDeviceInfo
{
    uint8 		NumberOfChannelsSupported; //channels simulataneously
    uint8 		AdvertiserIdentifier[32]; //
    uint16 		RegistrationPort; //udp port for registration of external applications
    uint16 		WsmMaxLength ; //1400

}__attribute__((__packed__)) _rsi_16093_LocalDeviceInfo_t,*_rsi_16093_LocalDeviceInfo_tp;

/*parameters populated by cch service request*/
typedef struct rsi_16093_CchServiceRequestTableEntry
{
    long 		type;
    uint32          pid;
    uint8 		CchServiceRequestTableIndex ;//0-127
    uint8 		CchServiceRequestChannelInterval ; //cch intervel/sch intervel/both
    uint8 		CchServiceRequestPriority ;//0-63
    uint8 		CchServiceRequestStatus; //0 -pending 1-satisfied 2-partially satisfied
    struct rsi_16093_CchServiceRequestTableEntry *next;

}__attribute__((__packed__)) _rsi_16093_CchServiceRequestTableEntry_t,*_rsi_16093_CchServiceRequestTableEntry_tp;

typedef struct rsi_16093_WSMServiceRequestTable
{
    long type;
    uint32 WsmServiceRequestPid;
    uint16 WsmServiceRequestTableIndex;
    uint8 WsmServiceRequestPsid[4];
    struct rsi_16093_WSMServiceRequestTable *next;

}__attribute__((__packed__)) _rsi_16093_WSMServiceRequestTableEntry_t,*_rsi_16093_WSMServiceRequestTableEntry_tp;


#define	USR_TYPE_AUTO_ACCESS_MATCH	0x0
#define	USR_TYPE_AUTO_ACCESS_UN_COND	0x1
#define	USR_TYPE_NO_SCH_ACCESS		0x2

#define	USR_SECURED_WSA			1
#define	USR_UNSECURED_WSA		2
#define	USR_SECURED_OR_UNSEC_WSA	3
#define	USR_SEC_ANY			4

#define	USR_STATUS_PENDING		0x0
#define	USR_STATUS_SATISFIED		0x1
#define	USR_STATUS_PARTIAL_SATISFIED	0x2
#define USR_STATUS_AVAIL_PENDING        0x3

/* parameters populated by user service request */
typedef struct rsi_16093_UserServiceRequestTableEntry 
{
    long 		type;
    uint32          pid;
    uint16 		UserServiceRequestTableIndex ;
    uint16 		UserServiceRequestType ;//0,1,2 --- usr_type defines
    uint8 		UserServiceRequestProviderServiceIdentifier[4]; 	// OCTETSTRING;
    uint8 		UserServiceRequestProviderServiceContext[32]; 		// OCTET STRING;
    uint8 		UserServiceRequestPriority ;
    uint8		UserServiceRequestWsaTypes;//1-4 above defines
    uint8 		UserServiceRequestSourceMacAddress[6]; 			// MacAddress;
    uint8 		UserServiceRequestAdvertiserIdentifier[32]; 		// OCTET STRING;
    uint16 		UserServiceRequestOperatingClass ;
    uint8		UserServiceRequestChannelNumber ;
    uint16 		UserServiceRequestLinkQuality ;
    boolean		UserServiceRequestImmediateAccess ;
    boolean		UserServiceRequestExtendedAccess ;
    uint16 		UserServiceStatus;
    struct rsi_16093_UserServiceRequestTableEntry *next;

}__attribute__((__packed__)) _rsi_16093_UserServiceRequestTableEntry_t, *_rsi_16093_UserServiceRequestTableEntry_tp;


#define	UAS_SECURED	0x1
#define	UAS_UNSECURED	0x2

#define	UAS_SUCCESS				0x1
#define	UAS_INVALID_INPUT			0x2
#define	UAS_CERT_NOT_FOUND			0x3
#define	UAS_UNSUPPORTED_SIGN			0x4
#define	UAS_NOT_MOST_RECENT_WSA			0x5
#define	UAS_CANT_CONSTUCT_CHAIN			0x6
#define	UAS_INCORRECT_CA_CERTIFICATE		0x7
#define	UAS_INCONSISTENT_CERTIFICATE_SUB	0x8
#define	UAS_INCONSISTENT_PERMISSIONS		0x9
#define	UAS_INCONSISTENT_GEOGRAPHIC_SCOPE	0xA
#define	UAS_UNAUTH_GEN_LOCATION			0xB
#define	UAS_PSID_PRIO_IN_WSA			0xC
#define	UAS_REVOKED_CERTIFICATE			0xD
#define	UAS_NO_UPTODATE_CRL			0xE
#define	UAS_UNAVAILABLE_PERMISSIONS		0xF
#define	UAS_UNAVAILABLE_GEOGRAPHIC_SCOPE	0x10
#define	UAS_CERT_VERIFICATION_FAIL		0x11
#define	UAS_MSG_VERIFICATION_FAIL		0x12
#define	UAS_OTHER_FAIL				0x13

#define	IPV6_SIZE				16
#define	UAS_CHANNEL_ACCESS_SCH_ONLY		0x0
#define	UAS_CHANNEL_ACCESS_SCH_CCH		0x1

#define	UAS_SERVICE_AVAILABLE			0x0
#define	UAS_SERVICE_ACTIVE			0x1

typedef struct rsi_16093_UserAvailableServiceTableEntry
{
    uint16 		ServiceTableIndex;
    uint8 		rsu_macid[6];
    uint8 		change_cnt;
    uint8 		WsaType; //secured or unsecured above defines
    uint8 		ResultCode; //above define 0x1-0x13
    uint8 		GenerationTime[8]; // OCTET STRING; time of wsa security info 1609.2
    uint8 		Lifetime[8]; // OCTET STRING; life time of wsa secured info
    uint8 		ExpectedCrlTime[8]; // OCTET STRING;1609.2
    uint8 		SourceMacAddress[6]; // MacAddress;

#if 0 
    uint8 		ProviderServiceIdentifier[8]; // OCTET STRING;
    uint8		ServiceSpecificPermissions[32]; // OCTET STRING;1609.2
    uint8 		ServicePriority;
    uint8 		ProviderServiceContext[32]; // OCTET STRING;
    uint8 		Ipv6Address[IPV6_SIZE] ;// Ipv6Address;
    uint16		ServicePort;
    uint8		ProviderMacAddress[6]; // MacAddress;
    uint8 		RcpiThreshold;//IEEE 802.11k/17.3.10.6.
    uint8		Rcpi;
    uint8		WsaCountThreshold;
#endif

    _rsi_WSAServiceInfo_t serviceEntry[32];

#if 0
    uint16		OperatingClass;
    uint8		ChannelNumber;
    boolean		Adaptable ;
    uint8		DataRate;
    int_16		TransmitPowerLevel;
    uint8		ChannelAccess;//sch only or sch,cch
#endif

    _rsi_ChannelInfo_t channelEntry[32];
    uint8		AdvertiserIdentifier[32]; // OCTET STRING;
    uint32 		TxLatitude; //latitude of advertising device
    uint32 		TxLongitude; // longitude of advertising device
    uint8 		TxElevation[2]; // OCTET STRING;
    uint8		TxPositionConfidence;
    uint8		TxElevationConfidence;   // add defines
    /* Byte 1: semi-major accuracy at one standard dev
range 0-12.7 meter, LSB = .05m
0xFE=254=any value equal or greater than 12.70 meter
0xFF=255=unavailable semi-major value
Byte 2: semi-minor accuracy at one standard dev
range 0-12.7 meter, LSB = .05m
0xFE=254=any value equal or greater than 12.70 meter
0xFF=255=unavailable semi-minor value
Bytes 3-4: orientation of semi-major axis
relative to true north (0~359.9945078786 degrees)
LSB units of 360/65535 deg = 0.0054932479
a value of 0x0000 =0 shall be 0 degrees
a value of 0x0001 =1 shall be 0.0054932479degrees
a value of 0xFFFE =65534 shall be 359.9945078786 deg
a value of 0xFFFF =65535 shall be used for orientation
unavailable (In NMEA GPGST)   
     */

    uint8		TxPositionAccuracy[4]; // OCTET STRING;
    uint16		LinkQuality;
    uint16		ServiceStatus ;//0 -available ,1 - active depending on link quality
    uint8		EdcaBkCWmin ;
    uint16		EdcaBkCWmax ;
    uint8		EdcaBkAifsn ;
    uint16		EdcaBkTxopLimit ;
    boolean		EdcaBkMandatory ;
    uint16		EdcaBeCWmin ;
    uint16 		EdcaBeCWmax ;
    uint16		EdcaBeAifsn ;
    uint16		EdcaBeTxopLimit ;
    boolean		EdcaBeMandatory ;
    uint16		EdcaViCWmin ;
    uint16		EdcaViCWmax ;
    uint16		EdcaViAifsn ;
    uint16		EdcaViTxopLimit ;
    boolean		EdcaViMandatory ;
    uint16		EdcaVoCWmin ;
    uint16		EdcaVoCWmax ;
    uint16		EdcaVoAifsn ;
    uint16		EdcaVoTxopLimit ;
    boolean		EdcaVoMandatory;
    _rsi_WAVE_RoutingAdvertisement_t wra_info;
    uint8 		rcvd_pkt_cnt;
    struct rsi_16093_UserAvailableServiceTableEntry *next;

}__attribute__((__packed__)) _rsi_16093_UserAvailableServiceTableEntry_t, *_rsi_16093_UserAvailableServiceTableEntry_tp;


typedef struct rsi_16093_MIB
{
    _rsi_16093_WSMServiceRequestTableEntry_t _rsi_16093_WSMServiceRequestTableEntry;
    _rsi_16093_StationConfigEntry_t _rsi_16093_StationConfigEntry;
    _rsi_16093_LocalDeviceInfo_t _rsi_16093_LocalDeviceInfo;
    _rsi_16093_CchServiceRequestTableEntry_t _rsi_16093_CchServiceRequestTableEntry;
    _rsi_16093_UserServiceRequestTableEntry_t _rsi_16093_UserServiceRequestTableEntry;
    _rsi_16093_UserAvailableServiceTableEntry_t _rsi_16093_UserAvailableServiceTableEntry;
    _rsi_16093_WSMServiceRequestTableEntry_t    *_rsi_16093_WSMServiceRequestTableEntry_head;
    _rsi_16093_CchServiceRequestTableEntry_t    *_rsi_16093_CchServiceRequestTableEntry_head;
    _rsi_16093_UserServiceRequestTableEntry_t   *_rsi_16093_UserServiceRequestTableEntry_head;
    _rsi_16093_UserAvailableServiceTableEntry_t *_rsi_16093_UserAvailableServiceTableEntry_head;
    volatile uint8                               no_of_cch_srvc;
    volatile uint8                               no_of_user_srvc;
    volatile uint8                               no_of_wsmp_srvc;
    volatile uint8                               no_of_avail_srvc;

}__attribute__((__packed__)) _rsi_16093_MIB_t; 


#endif
