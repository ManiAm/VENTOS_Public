
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window
addpath ../libs

% path to folder
basePATH = '../../results/cmd/5_MWM_10Percent/3_full_poisson_balanced_routeDist_70_30';

TLqueuingData = dir([basePATH, '/*_TLqueuingData.txt']);

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
[entityIDs, crossed, indexTS, VehNumbers, vehicleType] = ReadVehDelay(basePATH, vehDelay(runNumber).name);

% making a cell array that contains the complete view of the system
disp('making uitable ...');
%PresentData(entityIDs, crossed, indexTS);

disp('calculating intersection delay for vehicle/bike/ped ...');
% 700 means in each 70 seconds interval, we measure delay for 
% each vehicle and then take an average
[delayPassenger, delayEmergency, delayBike, timeSteps_D] = EntityDelay(timeSteps, VehNumbers, vehicleType, crossed, indexTS, 700);

delayDist{1,runNumber} = num2cell(delayPassenger / 60.);
delayDist{2,runNumber} = num2cell(delayEmergency / 60.);
delayDist{3,runNumber} = num2cell(delayBike);

disp('calculating throughput ...');
[throughput, timeSteps_T] = TLthroughput(timeSteps, VehNumbers, crossed, indexTS, 900);

option = 3;

if(option == 1)
    disp('plotting the benefits of active detection ...');
    PlotBenefitOfActiveDetection(runNumber, timeSteps_MQ, maxQueueSize, timeSteps_D, delayPassenger, timeSteps_T, throughput, runTotal);
elseif(option == 2)
    disp('plotting the performance of VANET-enables TSC under multi-modal traffic ...');
    PlotPerfTSCmultiModal(runNumber, timeSteps_MQ, maxQueueSize, delayDist, runTotal);
elseif(option == 3)
    disp('plotting the performance of FMSC ...');
    PlotFMSC(runNumber, timeSteps_MQ, maxQueueSize, delayDist, runTotal);
end


end

