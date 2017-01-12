
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window
addpath ../libs

% path to folder
basePATH = '../../results/4_LQF_MWM_starvation';

% what to plot?
option = 4;

TLqueuingData = dir([basePATH, '/*_TLqueuingData.txt']);
TLphasingData = dir([basePATH, '/*_TLphasingData.txt']);

% total number of simulation runs
runTotal = length(TLqueuingData);

for runNumber = 1:runTotal

fprintf('\n>>> runNumber %d:\n', runNumber);

% read the current TLqueuingData.txt file
[indices, timeSteps, totalQs, maxQs, laneCounts] = ReadTLqueuingData(basePATH, TLqueuingData(runNumber).name);

disp('calculating average queue size for vehicles ...');
% 700 means we aggregate every 150 values together
[averageQueueSize, timeSteps_Q] = AveVehQueueSize(laneCounts, totalQs, timeSteps, 700);

disp('calculating max queue size for vehicles ...');
% 700 means we aggregate every 150 values together
[maxQueueSize, timeSteps_MQ] = MaxVehQueueSize(maxQs, timeSteps, 700);

disp('reading vehicleDelay.txt file ...');
vehDelay = dir([basePATH, '/*_vehDelay.txt']);
[entityIDs, indexTS, VehNumbers, vehicleType, vehicleLane] = ReadVehDelay(basePATH, vehDelay(runNumber).name);
% indexTS containsvaluable information for each entity
%     indexTS(1,vNumber): entrance time for vehicle vNumber        
%     indexTS(2,vNumber): start of decel for vehicle vNumber
%     indexTS(3,vNumber): start of stopping delay for vehicle vNumber
%     indexTS(4,vNumber): crossing time for vehicle vNumber 
%     indexTS(5,vNumber): start of accel for vehicle vNumber
%     indexTS(6,vNumber): end of delay for vehicle vNumber

% making a cell array that contains the complete view of the system
disp('making uitable ...');
%PresentData(entityIDs, crossed, indexTS);

disp('calculating intersection delay for each class in each interval ...');
% 700 means in each 70 seconds interval, we measure delay for 
% each vehicle and then take an average
if(option == 4)
    interval = 100;
else
    interval = 700;
end
[aveDelayPassenger, aveDelayEmergency, maxDelayBike, timeSteps_D] = DelayPerInterval(timeSteps, VehNumbers, vehicleType, indexTS, interval);

% save delay for all entities in each time step
delayDist{1,runNumber} = num2cell(aveDelayPassenger / 60.);
delayDist{2,runNumber} = num2cell(aveDelayEmergency / 60.);
delayDist{3,runNumber} = num2cell(maxDelayBike);

disp('calculating deceleration/waiting/acceleration delay for each entity ...');
delayAllEntity = DelayPerEntity(timeSteps, indexTS);

disp('calculating min/max/average delay for each class ...');
[delayAllClass, allClasses] = DelayPerClass(delayAllEntity, vehicleType);

disp('calculating min/max/average delay for each lane ...');
[delayAllLanes, allLanes] = DelayPerLane(delayAllEntity(4,:), vehicleLane);

disp('calculating per-class fairness index ...');
% sending average delay in each class
[stDeviation, jain] = FairnessIndex(delayAllClass(:,3));

% save both fairness measures in each time step
perClassFairness{1, runNumber} = num2cell(stDeviation);
perClassFairness{2, runNumber} = num2cell(jain);

disp('calculating per-lane fairness index ...');
% sending average delay in each lane
[stDeviation, jain] = FairnessIndex(delayAllLanes(:,3));

% save both fairness measures in each time step
perLaneFairness{1, runNumber} = num2cell(stDeviation);
perLaneFairness{2, runNumber} = num2cell(jain);

disp('calculating throughput ...');
[throughput, timeSteps_T] = TLthroughput(timeSteps, VehNumbers, indexTS, 900);

disp('reading phasing information ...');
[phaseDurationTS, cycleNumber, totalPhases] = ReadPhasingData(basePATH, TLphasingData(runNumber).name);

disp('calculate TL total cycles ...');
[timeSteps_SW, totalCycles] = TLtotalCycle(timeSteps, phaseDurationTS, cycleNumber, 400);

disp('calculate TL total green time ...');
[timeSteps_GR, totalGreenTime] = TLtotalGreenTime(timeSteps, totalPhases, phaseDurationTS, 400);

if(option == 1)
    disp('plotting the benefits of active detection ...');
    PlotBenefitOfActiveDetection(runNumber, timeSteps_MQ, maxQueueSize, timeSteps_D, aveDelayPassenger, timeSteps_T, throughput, runTotal);
elseif(option == 2)
    disp('plotting the performance of VANET-enables TSC under multi-modal traffic ...');
    PlotPerfTSCmultiModal(runNumber, timeSteps_MQ, maxQueueSize, delayDist, perClassFairness, perLaneFairness, runTotal);
elseif(option == 3)  
    disp('plotting TL phasing information ...');
    PlotTLPhasing(runNumber, timeSteps_SW, totalCycles, timeSteps_GR, totalGreenTime, runTotal); 
elseif(option == 4)
    disp('plotting the LQF-MWM starvation problem ...');
    PlotStarvation(runNumber, timeSteps_D, delayDist, perClassFairness, perLaneFairness, runTotal);
elseif(option == 5)
    disp('plotting the performance of LQF_MWM, LQF_MWM_Aging and LQF_MWM_Cycle ...');
    PlotLQFMWM(runNumber, timeSteps_MQ, maxQueueSize, timeSteps_D, aveDelayPassenger, aveDelayEmergency, delayDist, runTotal);
elseif(option == 6)
    disp('plotting the performance of FMSC ...');
    PlotFMSC(runNumber, timeSteps_MQ, maxQueueSize, delayDist, runTotal);
end

end

