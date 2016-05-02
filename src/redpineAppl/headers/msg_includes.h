/****************************************************************************/
/// @file    msg_include.h
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

#include "wave_util.h"
#include "Dot3MIB.h"
#include "wsm_include.h"

void* receive_response();
void*  insertInMIBTable(uint8 *current,long type);
void show_result();
_rsi_16093_WSMServiceRequestTableEntry_t* searchInMIBTable(uint8 *psid,uint8 type);
_rsi_16093_CchServiceRequestTableEntry_t* deleteCchEntryInMIBTable(uint16 local_index);
_rsi_16093_UserAvailableServiceTableEntry_t* searchPSIDMIB(uint8 *psid,int *pos);
_rsi_16093_UserAvailableServiceTableEntry_t*  update16093MIB(uint8 *mib_buf);
void free_queues(long type);
void update_user_service_status(void);
void queue_response_pkt(uint8 *rxPkt,uint8 offset);
void update_available_services(WSA_Frame_t *wsa);
void print_service_info(_rsi_16093_UserAvailableServiceTableEntry_t * node_psid);

typedef union 
{
    long type;
    _rsi_MLMEX_REGISTERTXPROFILE_request_t tx_register_profile ;
    _rsi_WAVE_WME_WSMService_request_t WSMServiceRequest;
    _rsi_WAVE_MLMEX_SCHEND_request_t sch_end_req;
    _rsi_WAVE_MLMEX_SCHSTART_request_t sch_start_req;
    _rsi_WAVE_WME_CchService_request_t cchService_req;
    _rsi_WAVE_WME_UserService_request_t userServiceRequest;
    _rsi_MLMEX_AddressChange_request_t mac_addr_req ;
    _rsi_WAVE_MLMEX_CANCELTX_request_t cancel_tx_req;
    _rsi_WAVE_WME_GET_request_t        get_req;
    waveShortMessage wsmMessage;
    _rsi_ocb_channel_sync_t ocb_channel_sync;
    _rsi_reset_mib_info_t              reset_req;
    uint8 cpy_buffer[1300];

}_rsi_messagePassing_t;

typedef union
{
    long type;
    _rsi_WAVE_WME_WSMService_request_t WSMServiceRequestMIBEntry;
    _rsi_WAVE_WME_CchService_request_t CchServiceRequestMIBEntry;
    _rsi_WAVE_WME_UserService_request_t UserServiceRequestMIBEntry;
    uint8 mib_buffer[1300];

}_rsi_16093_MIB_Entry_t;

