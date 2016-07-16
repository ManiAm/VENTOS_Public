/****************************************************************************/
/// @file    rsi_wave_util.h
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

#ifndef __RSI_WAVE_UTIL_H__
#define __RSI_WAVE_UTIL_H__

////////11p data rate values for redpine driver
#define  RATE_1_5            1
#define  RATE_2_2_5          2
#define  RATE_3              3
#define  RATE_4_5            4
#define  RATE_6              6
#define  RATE_9              9
#define  RATE_12            12
#define  RATE_13_5          13
#define  RATE_18            18
#define  RATE_24            24
#define  RATE_27            27
#define  RATE_36            36
#define  RATE_48            48
#define  RATE_54            54

#define OPERATING_CLASS   17
#define CONTROL_CHANNEL   178
#define CCH_INTERVEL      50
#define SCH_INTERVEL      50
#define SYNC_TOLERANCE    02
#define MAX_SWITCH_TIME   02

#define SCH_STRT_RESP 		        0x10
#define RSI_CANCEL_TX_RESP         	0x11
#define SCH_END_RESP	                0x12
#define ADDR_CHANGE_RESP	        0x13
#define REGISTER_TX_RESP                0X14
#define MIB_INIT_RESP		        0x15
#define RSI_SET_CHANNEL_RESP		0x16
#define RSI_WSMP_SERVICE_RESP 	        0X17
#define INIT_16093_MIB_RESP		0x18
#define RSI_CCH_SERVICE_RESP            0x19
#define RSI_USER_SERVICE_RESP           0x20
#define RSI_WSMP_SEND_RESP 		0x21
#define DELETE_USER_RESP 	        0x25
#define RSI_UTC_RESP 		        0x22
#define SET_UTC_TIME_RESP 		72
#define RSI_UPDATE_SYNC_PARAMS_RESP     0x23
#define RSI_CLOSE_APP_RESP		0x24
#define RSI_MIB_GET_RESP                0x26
#define RSI_AVAIL_SRVC_RESP             0x27
#define RSI_RESET_INFO_RESP             0x28
#define RSI_SEC_CERT_RESP               0x29
#define RSI_SEC_CMH_RESP                0x30
#define RSI_WSMP_SEC_SEND_RESP 	        0x31
#define RSI_WSMP_RCV_RESP 	        0x32
#define RSI_LSI_RESP 	                0x33

////////////////////////////////////////////command numbers for wave_util.c .
#define SCH_STRT_REQ 		0x10
#define RSI_CANCEL_TX         	0x11
#define SCH_END_REQ		0x12
#define ADDR_CHANGE_REQ		0x13
#define REGISTER_TX 		0x14
#define MIB_INIT		0x15
#define RSI_SET_CHANNEL		0x16
#define RSI_WSMP_SERVICE 	0X17
#define INIT_16093_MIB		0x18
#define RSI_CCH_SERVICE_REQUEST 0x19
#define RSI_USER_SERVICE_REQUEST 0x20
#define RSI_WSMP_SEND 		0x21
#define DELETE_USER_REQ 	0x25
#define RSI_GET_UTC 		0x22
#define SET_UTC_TIME 		72
#define RSI_UPDATE_SYNC_PARAMS  0x23
#define RSI_CLOSE_APP		0x24
#define RSI_MIB_GET             0x26
#define RSI_AVAIL_SRVC          0x27
#define RSI_RESET_INFO          0x28
#define RSI_SEC_CERT_REQ        0x29
#define RSI_SEC_CMH_REQ         0x30
#define RSI_WSMP_SEC_SEND 	0x31
#define RSI_WSMP_RCV 	        0x32
#define RSI_LSI_REQ 	        0x33
////////////////////////////////////////////command numbers for wave_util.c END.
#define MAC_RESPONSE		3 
#define WSA_RCV 		0xd0	
#define CHANNEL_RESPONSE	82	
#define NODE_AVAIL		12
#define NODE_UPDATE_LIST	9

#define OPT_CHANNEL_NUMBER_TYPE			15
#define TRANSMIT_POWER_USED_TYPE		4
#define DATARATE_TYPE				16


#define ADD_CERT_ONLY                    1
#define ADD_PEER_CERT                    2
#define ADD_CERT_AND_PRIV_KEY            3
#define ADD_CERT_PRIV_KEY_ENCRYPTED      4

#define CERT                             0
#define IDENTIFIER                       1


#define    SIGN                          1
#define    ENCRYPT                       2
#define    SIGN_AND_ENCRYPT              3
#define    WSMP_PAY_LOAD_OFFSET		 26	

#if 0
/*element types */

#define SERVICE_INFO_TYPE			1	
#define CHANNEL_INFO_TYPE			2
#define WRA_TYPE				3
#define TRANSMIT_POWER_USED_TYPE		4
#define 2D_LOCATION_TYPE			5
#define 3DLOCATIONANDCONFIDENCE_TYPE	6
#define ADVERTISER_IDENTIFIER_TYPE		7
#define PROVIDER_SERVICE_CONTEXT_TYPE	8
#define IPV6_ADDRESS_TYPE			9
#define SERVICE_PORT_TYPE			10
#define PROVIDER_MAC_ADDRESS_TYPE		11
#define EDCA_PARAMETER_SET_TYPE			12
#define SECONDARY_DNS_TYPE			13
#define GATEWAY_MAC_ADDRESS_TYPE		14
#define CHANNEL_NUMBER_TYPE			15
#define DATARATE_TYPE				16
#define REPEAT_RATE_TYPE			17
#define COUNTRY_STRING_TYPE			18
#define RCPI_THRESHOLD_TYPE			19
#define WSA_COUNT_THRESHOLD_TYPE		20
#define CHANNEL_ACCESS_TYPE			21
#define WSA_COUNT_THRESHOLD_INTERVAL_TYPE	22

/*end element types*/
#endif

#define MAX_LENGTH 		1300
#define WSM_LENGTH      1250

#if 0
typedef char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
#endif

//typedef unsigned short int uint16;
#include "rsi_wave_includes.h"

typedef struct //Wsmp_extended_header
{
    uint8 channelNumber;//0-200
    uint8 DataRate;//2-127
    char tx_pwr;
}wsmp_ext_header;

typedef struct //WaveShortMessage
{
    long type;
    int pid;
    uint8 peer_mac_address[6];
    uint8 channelNumber;//0-200
    uint8 dataRate;//2-127
    char  txPwr;
    uint8 psid[4];
    uint8 priority;
    uint16 wsm_expiry_time;
    uint16 wsm_length;// 0:11 indicates length in octets of wsm data ,12:15 reserved
    uint8 WSM_Data[WSM_LENGTH];
    wsmp_ext_header Wsm_optional_header;
    /*indicates type of wsm data contained,to assist recipient (128 - WAve Short Message)*/
    uint8 waveElement;

}__attribute__((__packed__)) waveShortMessage;

enum Action
{
    ADD = 1,
    DELETE,
    CHANGE
};

enum ChannelInterval
{
    CCH_Interval,
    SCH_Interval,
    both
};

enum ServiceStatus
{
    Accepted,
    Rejected_invalid_parameter,
    Rejected_unspecified,
    Rejected
};

enum UserRequestType
{
    Auto_access_on_service_match,
    Auto_access_unconditionally,
    No_SCH_access
};

enum WsaTypes
{
    Unsecured_WSA,
    Secured_WSA,
    Secured_or_Unsecured,
    WSA,
    Any
};

enum Status_Confirm
{
    //SUCCESS,
    FEATURE_NOT_SUPPORTED,
    CHANNEL_UNAVAILABLE
};

enum status_indication
{
    LOSS_OF_SYNC,
    UNSPECIFIED_REASON
};

enum SCHconfirm
{
    PARTIAL_SUCCESS_NoExtendedAccess,
    PARTIAL_SUCCESS_NoImmediateAccess,
    PARTIAL_SUCCESS_NoExtended_OR_Immediate_Access,
    NO_SYNC,
    //INVALID_PARAMETERS,
    UNSPECIFIED_FAILURE
};

enum PID_INDEX
{
    CHANNEL_REQUEST
};

typedef struct _rsi_WAVE_MLMEX_TA_indication
{
    uint8 	Timestamp[2];
    uint8 	Local_Time[2];
    uint8 	RCPI;
    uint8 	Source_MAC_address[6];
    uint8 	ChannelIdentifier;
    //struct   TimingAdvertisementContents; //Doubt??

}_rsi_WAVE_MLMEX_TA_indication_t,*_rsi_WAVE_MLMEX_TA_indication_tp;

typedef struct  _rsi_WAVE_MLMEX_VSA_indication
{
    uint8 	Source_MAC_Address[6];
    uint8 	ManagementID;
    uint8 	Vendor_Specific_Content[4];
    uint8 	ChannelIdentifier;
    uint8 	RCPI;

}_rsi_WAVE_MLMEX_VSA_indication_t,*_rsi_WAVE_MLMEX_VSA_indication_tp;

typedef struct _rsi_WAVE_MLMEX_SCHSTART_request
{
    long type;
    int pid;
    uint8 	Channel_Identifier;
    uint8 	OperationalRateSet;
    //EDCA Parameter Set,//Doubt ??
    boolean ImmediateAccess;
    uint8 	ExtendedAccess;

}__attribute__((__packed__)) _rsi_WAVE_MLMEX_SCHSTART_request_t,*_rsi_WAVE_MLMEX_SCHSTART_request_tp;

typedef struct  _rsi_WAVE_MLMEX_SCHSTART_confirm
{
    enum SCHconfirm ResultCode;

}_rsi_WAVE_MLMEX_SCHSTART_confirm_t,*_rsi_WAVE_MLMEX_SCHSTART_confirm_tp;

typedef struct _rsi_WAVE_MLMEX_SCHEND_request
{
    long type;
    int pid;
    uint8 Channel_Identifier;

}__attribute__((__packed__)) _rsi_WAVE_MLMEX_SCHEND_request_t,*_rsi_WAVE_MLMEX_SCHEND_request_tp;

typedef struct _rsi_WAVE_MLMEX_SCHEND_confirm
{
    enum Status_Confirm ResultCode;

}_rsi_WAVE_MLMEX_SCHEND_confirm_t,*_rsi_WAVE_MLMEX_SCHEND_confirm_tp; 

typedef struct _rsi_WAVE_MLMEX_SCHEND_indication
{
    enum status_indication Reason;

} _rsi_WAVE_MLMEX_SCHEND_indication_t,*_rsi_WAVE_MLMEX_SCHEND_indication_tp; 

typedef struct  _rsi_MLMEX_REGISTERTXPROFILE_request
{
    long type;
    int pid;
    uint8 macid[6];
    uint8 	Channel_Identifier;
    boolean Adaptable;
    uint8 	TxPwr_Level;
    uint8 	DataRate;

}__attribute__((__packed__)) _rsi_MLMEX_REGISTERTXPROFILE_request_t,*_rsi_MLMEX_REGISTERTXPROFILE_request_tp; 

typedef struct _rsi_WAVE_MLMEX_REGISTERTXPROFILE_confirm
{
    enum Status_Confirm ResultCode;

} _rsi_WAVE_MLMEX_REGISTERTXPROFILE_confirm,*_rsi_WAVE_MLMEX_REGISTERTXPROFILE_confirm_tp; 

typedef struct _rsi_WAVE_MLMEX_DELETETXPROFILE_request
{
    uint8 	ChannelIdentifier;

}_rsi_WAVE_MLMEX_DELETETXPROFILE_request_t,*_rsi_WAVE_MLMEX_DELETETXPROFILE_request_tp;

typedef struct  _rsi_MLMEX_DELETETXPROFILE_confirm
{    enum 		Status_Confirm ResultCode;

}_rsi_MLMEX_DELETETXPROFILE_confirm_t,*_rsi_MLMEX_DELETETXPROFILE_confirm_tp;

typedef struct  _rsi_WAVE_MLMEX_CANCELTX_request
{
    long type;
    uint8 		Channel_Identifier;
    uint8 		ACI; //0-3

}__attribute__((__packed__)) _rsi_WAVE_MLMEX_CANCELTX_request_t,*_rsi_WAVE_MLMEX_CANCELTX_request_tp;

typedef struct _rsi_WAVE_WME_GET_request
{
    long    type;
    long    req_type;
    uint32  pid;

}__attribute__((__packed__))_rsi_WAVE_WME_GET_request_t,*_rsi_WAVE_WME_GET_request_tp;

typedef struct _rsi_MLMEX_CANCELTX_confirm
{
    enum 	Status_Confirm ResultCode;

}_rsi_MLMEX_CANCELTX_confirm_t,*_rsi_MLMEX_CANCELTX_confirm_tp;

typedef struct _rsi_MLMEX_AddressChange_request 
{
    long 		type;
    uint8 		MAC_Address[6];// (optional)

}__attribute__((__packed__)) _rsi_MLMEX_AddressChange_request_t,*_rsi_MLMEX_AddressChange_request_tp;

typedef struct _rsi_WAVE_MLMEX_AddressChange_confirm
{
    enum 		ServiceStatus ResultCode;

}  _rsi_WAVE_MLMEX_AddressChange_confirm_t,*_rsi_WAVE_MLMEX_AddressChange_confirm_tp; 

typedef struct  _rsi_WAVE_MLMEX_SendPrimitive_request
{
    uint8 		Channel_Identifier;
    enum 		ChannelInterval channelInterval;
    //PrimitiveContents//Doubt ??

} _rsi_WAVE_MLMEX_SendPrimitive_request_t,*_rsi_WAVE_MLMEX_SendPrimitive_request_tp;

typedef struct _rsi_MLMEX_SendPrimitive_confirm
{
    enum Status_Confirm ResultCode;

} _rsi_MLMEX_SendPrimitive_confirm_t,*_rsi_MLMEX_SendPrimitive_confirm_tp;

typedef struct _rsi_WAVE_WME_UserService_request
{
    long 		type;
    uint32          pid;
    uint16 		Local_Service_Index;
    enum 		Action action;//add,delete valid
    enum 		UserRequestType userRequestType;
    uint8 		ProviderServiceIdentifier[4];//doubt
    uint8 		ServicePriority;
    enum 		WsaTypes wsaType;
    uint8 		ProviderServiceContext[32];// (optional),
    uint8 		Channel_Identifier;// (optional),
    uint8 		SourceMacAddress[6];// (optional),
    uint8 		AdvertiserIdentifier[31];// (optional),
    uint8 		Link_Quality;//DOUBT ?? (optional),
    boolean 	ImmediateAccess;// (optional),
    uint16 		ExtendedAccess;// (optional)

}__attribute__((__packed__)) _rsi_WAVE_WME_UserService_request_t,*_rsi_WAVE_WME_UserService_request_tp;

typedef struct _rsi_WAVE_WME_UserService_confirm 
{
    uint16 		Local_Service_Index;
    enum 		ServiceStatus ResultCode;

}_rsi_WAVE_WME_UserService_confirm_t,*_rsi_WAVE_WME_UserService_confirm_tp;

typedef struct _rsi_WAVE_WME_WSMService_request
{
    long type;
    int pid;
    uint16 		Local_Service_Index;
    enum 		Action action;//ADD,DELETE ARE VALID
    uint8  		ProviderServiceIdentifier[4];
    uint8           secured;
    int             cmh;

}__attribute__((__packed__)) _rsi_WAVE_WME_WSMService_request_t,*_rsi_WAVE_WME_WSMService_request_tp; 

typedef struct _rsi_WME_WSMService_confirm
{
    uint16 		Local_Service_Index;
    enum 		ServiceStatus ResultCode;

}__attribute__((__packed__))  _rsi_WME_WSMService_confirm_t,*_rsi_WME_WSMService_confirm_tp; 

typedef struct  _rsi_WAVE_WME_CchService_request
{
    long 		type;
    uint32          pid;
    uint16		Local_Service_Index;
    enum 		Action action;
    uint8 		channel_interval;
    uint8 		ServicePriority;

}__attribute__((__packed__)) _rsi_WAVE_WME_CchService_request_t,*_rsi_WAVE_WME_CchService_request_tp;

typedef struct _rsi_WAVE_WME_CchService_confirm
{
    uint8 		Local_service_index[4];
    enum 		ServiceStatus ResultCode;

} _rsi_WAVE_WME_CchService_confirm_t,*_rsi_WAVE_WME_CchService_confirm_tp;

typedef struct _rsi_WME_ManagementDataService_request
{
    uint8 		LocalService_Index[4];
    enum 		Action Result;
    uint8 		Destination_MAC_address[4];
    //enum Management ID;//reffer 1609.0
    //Organization Identifier//doubt
    uint8 		Data[MAX_LENGTH];
    uint8 		Repeat_Rate;
    uint8 		Channel_Identifier;
    enum 		ChannelInterval channel_interval;
    uint8 		ServicePriority;

}_rsi_WME_ManagementDataService_request_t,*_rsi_WME_ManagementDataService_request_tp; 

typedef struct  _rsi_WME_ManagementDataService_indication
{
    uint8 		Source_MAC_Address[6];
    uint8 		Management_ID;
    uint8 		Data;
    uint8 		Channel_Identifier;
    //RCPI

}_rsi_WME_ManagementDataService_indication_t,*_rsi_WME_ManagementDataService_indication_tp;

typedef struct _rsi_ocb_channel_sync
{
    long type;
    int pid;
    uint8 cch_op_class;
    uint8 cntrl_ch;
    uint16 cch_intrvl;
    uint16 sch_intrvl;
    uint16 sync_tolerence;
    uint16 max_channel_switch_time;
    uint32  reserved;
    //uint8  reserved;
    //uint8  reserved;
    //uint8  reserved;

}__attribute__ ((packed))_rsi_ocb_channel_sync_t;

typedef union mib_info
{
    struct mib_rsp_info
    {
        uint32 pid;
        uint8  type;
        uint8  more;
        uint8  rsp_type;
    }mib_rsp;
    uint8  rsp_buf[3200];

}__attribute__ ((packed))mib_info_t;

typedef union _rsi_resp
{
    struct rsp_info_buf
    {
        uint32 pid;
        uint8  type;
        uint8  status;
        uint8  rsp_type;
    }rsp_info;
    uint8  rsp_buf[3200];

}__attribute__ ((packed))_rsi_resp_t;

typedef struct _rsi_reset_mib_info 
{
    long      type;
    int       pid;
    long      req_type;

}__attribute__ ((packed))_rsi_reset_mib_info_t,*_rsi_reset_mib_info_tp;

typedef struct _rsi_1609_2_cmh_req 
{
    long      type;
    int       pid;
    int      req_type;
    int       cmh;
    uint8     sec_type;

}__attribute__ ((packed))_rsi_1609_2_cmh_req_t,*_rsi_1609_2_cmh_req_tp;

typedef struct _rsi_1609_2_cert_req
{
    long    type;
    uint32  pid;
    uint8   req_type;
    uint8   identifier;
    int     cmh;
    int     len;
    uint8   cert_buf[1040];
    int     priv_len;
    uint8   sign_priv_key[34];
    uint8   dcrypt_priv_key[34];
    uint8   encrypted;
    uint8   dcrypt_key[32];

}__attribute__ ((packed))_rsi_1609_2_cert_req_t;

typedef struct _rsi_1609_2_wsmps_req 
{
    long      type;
    int       pid;
    int       sec_type;
    int       cmh;
    //_rsi_16092_sec_signedData_req_t         sign_data_req;
    waveShortMessage                     wsmMessage;

}__attribute__ ((packed))_rsi_1609_2_wsmps_req_t,*_rsi_1609_2_wsmps_req_tp;


int rsi_wavecombo_msgqueue_init(void);

void rsi_wavecombo_1609mib_get_request(uint8 type);
int  rsi_wavecombo_1609mib_get_response(uint8 type);
int rsi_wavecombo_reset_info(uint8 type);
int rsi_wavecombo_sch_end_request(uint8 channel_no);

void rsi_wavecombo_mac_addr_change_req(uint8 *buff);
void rsi_cancel_tx(uint8 aci);
int rsi_wavecombo_register_tx_params(uint8 txpower ,uint8 *macid,uint8 channelno,uint8 Adaptable,uint8 datarate);
int rsi_wavecombo_1609mib_init(void);
int rsi_wavecombo_sch_start_req(uint8 channelno,uint8 operational_rate,uint8 Immediate_access ,uint8 extended_access);
void rsi_wavecombo_close_app(void);
int rsi_wavecombo_wsmp_service_req(enum Action action,uint16 local_index,uint8 *psid);
int rsi_wavecombo_cch_service_req(uint16 local_index,enum Action action,uint8 channel, uint8 priority);
int rsi_wavecombo_user_service_req(_rsi_WAVE_WME_UserService_request_t *userServiceRequest);
int rsi_wavecombo_get_utc(void);
int rsi_wavecombo_set_utc(void);
int rsi_wavecombo_update_channel_sync_params(uint8 oper_class,uint8 cntrl_ch,uint16 cch_intervl,uint16 sch_intervl,uint16 sync_tol,uint16 switch_time);

int rsi_wavecombo_wsmp_msg_send(waveShortMessage * WSMMessage);
int rsi_wavecombo_receive_wsmp_packet(uint8 *buff,int len);
int rsi_wavecombo_add_cryptomaterial_handle(void);
int rsi_wavecombo_delete_cryptomaterial_handle(int cmh);
int read_certificate(char *file_name,char *certificate_buf);
int read_private_key(char *file_name,char *certificate_buf);
int rsi_wavecombo_add_certificate(_rsi_1609_2_cert_req_t *cert_req,uint8 *cert_id);
int rsi_wavecombo_send_secure_wsmp(_rsi_1609_2_wsmps_req_t  *wsmps_req);
int rsi_wavecombo_local_service_index_request(void);
//int rsi_wave_combo_recv(_rsi_resp_t *rsp);
int rsi_wavecombo_wsmp_queue_init(int local_srv_index);
int rsi_wavecombo_msgqueue_deinit(void);

#endif
