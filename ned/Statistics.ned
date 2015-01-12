package c3po.ned;

simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
        
        bool collectMAClayerData;
        bool collectVehiclesData;
        bool collectInductionLoopData;
        bool collectPlnManagerData;
        bool printBeaconsStatistics;
        bool printIncidentDetection;
        int LaneCostsMode;
        int HysteresisCount;
}
