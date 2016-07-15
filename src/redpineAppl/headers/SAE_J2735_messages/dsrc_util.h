#ifndef __DSRC_UTIL_H__
#define __DSRC_UTIL_H__

typedef enum dsrc_msgID {
    _DSRCmsgID_reserved	= 0,
    _DSRCmsgID_alaCarteMessage	= 1,
    _DSRCmsgID_basicSafetyMessage	= 2,
    _DSRCmsgID_basicSafetyMessageVerbose	= 3,
    _DSRCmsgID_commonSafetyRequest	= 4,
    _DSRCmsgID_emergencyVehicleAlert	= 5,
    _DSRCmsgID_intersectionCollisionAlert	= 6,
    _DSRCmsgID_mapData	= 7,
    _DSRCmsgID_nmeaCorrections	= 8,
    _DSRCmsgID_probeDataManagement	= 9,
    _DSRCmsgID_probeVehicleData	= 10,
    _DSRCmsgID_roadSideAlert	= 11,
    _DSRCmsgID_rtcmCorrections	= 12,
    _DSRCmsgID_signalPhaseAndTimingMessage	= 13,
    _DSRCmsgID_signalRequestMessage	= 14,
    _DSRCmsgID_signalStatusMessage	= 15,
    _DSRCmsgID_travelerInformation	= 16
    /*
     * Enumeration is extensible
     */
} e_dsrc_msgID;


/* RSA message extent*/
typedef enum dsrcExtent {
    _Extent_useInstantlyOnly	= 0,
    _Extent_useFor3meters	= 1,
    _Extent_useFor10meters	= 2,
    _Extent_useFor50meters	= 3,
    _Extent_useFor100meters	= 4,
    _Extent_useFor500meters	= 5,
    _Extent_useFor1000meters	= 6,
    _Extent_useFor5000meters	= 7,
    _Extent_useFor10000meters	= 8,
    _Extent_useFor50000meters	= 9,
    _Extent_useFor100000meters	= 10,
    _Extent_forever	= 127
} dsrc_e_Extent;


typedef enum dsrc_TimeConfidence {
    _TimeConfidence_unavailable	= 0,
    _TimeConfidence_time_100_000	= 1,
    _TimeConfidence_time_050_000	= 2,
    _TimeConfidence_time_020_000	= 3,
    _TimeConfidence_time_010_000	= 4,
    _TimeConfidence_time_002_000	= 5,
    _TimeConfidence_time_001_000	= 6,
    _TimeConfidence_time_000_500	= 7,
    _TimeConfidence_time_000_200	= 8,
    _TimeConfidence_time_000_100	= 9,
    _TimeConfidence_time_000_050	= 10,
    _TimeConfidence_time_000_020	= 11,
    _TimeConfidence_time_000_010	= 12,
    _TimeConfidence_time_000_005	= 13,
    _TimeConfidence_time_000_002	= 14,
    _TimeConfidence_time_000_001	= 15,
    _TimeConfidence_time_000_000_5	= 16,
    _TimeConfidence_time_000_000_2	= 17,
    _TimeConfidence_time_000_000_1	= 18,
    _TimeConfidence_time_000_000_05	= 19,
    _TimeConfidence_time_000_000_02	= 20,
    _TimeConfidence_time_000_000_01	= 21,
    _TimeConfidence_time_000_000_005	= 22,
    _TimeConfidence_time_000_000_002	= 23,
    _TimeConfidence_time_000_000_001	= 24,
    _TimeConfidence_time_000_000_000_5	= 25,
    _TimeConfidence_time_000_000_000_2	= 26,
    _TimeConfidence_time_000_000_000_1	= 27,
    _TimeConfidence_time_000_000_000_05	= 28,
    _TimeConfidence_time_000_000_000_02	= 29,
    _TimeConfidence_time_000_000_000_01	= 30,
    _TimeConfidence_time_000_000_000_005	= 31,
    _TimeConfidence_time_000_000_000_002	= 32,
    _TimeConfidence_time_000_000_000_001	= 33,
    _TimeConfidence_time_000_000_000_000_5	= 34,
    _TimeConfidence_time_000_000_000_000_2	= 35,
    _TimeConfidence_time_000_000_000_000_1	= 36,
    _TimeConfidence_time_000_000_000_000_05	= 37,
    _TimeConfidence_time_000_000_000_000_02	= 38,
    _TimeConfidence_time_000_000_000_000_01	= 39
} dsrc_e_TimeConfidence;


/******** TIM message **********/

typedef enum travelerInfoType {
    travelerInfoType_unknown	= 0,
    travelerInfoType_advisory	= 1,
    travelerInfoType_roadSignage	= 2,
    travelerInfoType_commercialSignage	= 3
    /*
     * Enumeration is extensible
     */
} e_travelerInfoType;


typedef enum MsgId_PR {
    MsgId_PR_NOTHING,	/* No components present */
    MsgId_PR_furtherInfoID,
    MsgId_PR_roadSignID
} MsgId_PR;


typedef enum Content_PR {
    Content_PR_NOTHING,	/* No components present */
    Content_PR_advisory,
    Content_PR_workZone,
    Content_PR_genericSign,
    Content_PR_speedLimit,
    Content_PR_exitService
} Content_PR;


typedef enum mUTCDCode {
    mUTCDCode_none	= 0,
    mUTCDCode_regulatory	= 1,
    mUTCDCode_warning	= 2,
    mUTCDCode_maintenance	= 3,
    mUTCDCode_motoristService	= 4,
    mUTCDCode_guide	= 5,
    mUTCDCode_rec	= 6
    /*
     * Enumeration is extensible
     */
} e_mUTCDCode;


typedef enum directionOfUse {
    directionOfUse_forward	= 0,
    directionOfUse_reverse	= 1,
    directionOfUse_both	= 2
    /*
     * Enumeration is extensible
     */
} e_directionOfUse;


typedef enum extent {
    extent_useInstantlyOnly	= 0,
    extent_useFor3meters	= 1,
    extent_useFor10meters	= 2,
    extent_useFor50meters	= 3,
    extent_useFor100meters	= 4,
    extent_useFor500meters	= 5,
    extent_useFor1000meters	= 6,
    extent_useFor5000meters	= 7,
    extent_useFor10000meters	= 8,
    extent_useFor50000meters	= 9,
    extent_useFor100000meters	= 10,
    extent_forever	= 127
} e_extent;


typedef enum Area_PR {
    Area_PR_NOTHING,	/* No components present */
    Area_PR_shapePointSet,
    Area_PR_circle,
    Area_PR_regionPointSet
} Area_PR;


typedef enum Raduis_PR {
    Raduis_PR_NOTHING,	/* No components present */
    Raduis_PR_radiusSteps,
    Raduis_PR_miles,
    Raduis_PR_km
} Raduis_PR;


typedef enum Item_PR {
    Item_PR_NOTHING,	/* No components present */
    Item_PR_itis,
    Item_PR_text
} Item_PR;

/********     MAP message    ****************/

typedef enum Data_PR {
    Data_PR_NOTHING,	/* No components present */
    Data_PR_laneSet,
    Data_PR_zones
} Data_PR;

/*******   SPAT message  *******************/

typedef enum specialSignalState {
    specialSignalState_unknown	= 0,
    specialSignalState_notInUse	= 1,
    specialSignalState_arriving	= 2,
    specialSignalState_present	= 3,
    specialSignalState_departing	= 4
    /*
     * Enumeration is extensible
     */
} e_specialSignalState;


typedef enum pedestrianSignalState {
    pedestrianSignalState_unavailable	= 0,
    pedestrianSignalState_stop	= 1,
    pedestrianSignalState_caution	= 2,
    pedestrianSignalState_walk	= 3
    /*
     * Enumeration is extensible
     */
} e_pedestrianSignalState;


typedef enum stateConfidence {
    stateConfidence_unKnownEstimate	= 0,
    stateConfidence_minTime	= 1,
    stateConfidence_maxTime	= 2,
    stateConfidence_timeLikeklyToChange	= 3
    /*
     * Enumeration is extensible
     */
} e_stateConfidence;


typedef enum pedestrianDetect {
    pedestrianDetect_none	= 0,
    pedestrianDetect_maybe	= 1,
    pedestrianDetect_one	= 2,
    pedestrianDetect_some	= 3
    /*
     * Enumeration is extensible
     */
} e_pedestrianDetect;


/****************** BSM  message *****************/

typedef enum CrumbData_PR {
    CrumbData_PR_NOTHING,	/* No components present */
    CrumbData_PR_pathHistoryPointSets_01,
    CrumbData_PR_pathHistoryPointSets_02,
    CrumbData_PR_pathHistoryPointSets_03,
    CrumbData_PR_pathHistoryPointSets_04,
    CrumbData_PR_pathHistoryPointSets_05,
    CrumbData_PR_pathHistoryPointSets_06,
    CrumbData_PR_pathHistoryPointSets_07,
    CrumbData_PR_pathHistoryPointSets_08,
    CrumbData_PR_pathHistoryPointSets_09,
    CrumbData_PR_pathHistoryPointSets_10
} CrumbData_PR;


typedef enum gPSstatus {
    gPSstatus_unavailable	= 0,
    gPSstatus_isHealthy	= 1,
    gPSstatus_isMonitored	= 2,
    gPSstatus_baseStationType	= 3,
    gPSstatus_aPDOPofUnder5	= 4,
    gPSstatus_inViewOfUnder5	= 5,
    gPSstatus_localCorrectionsPresent	= 6,
    gPSstatus_networkCorrectionsPresent	= 7
} e_gPSstatus;


#endif
