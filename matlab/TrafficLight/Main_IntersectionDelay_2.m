
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window
addpath ../libs

% path to folder
basePATH = '../../results/cmd/3_existing_multiClass/3_full_poisson_balanced_routeDist_70_30';

TLqueuingData = dir([basePATH, '/*_TLqueuingData.txt']);
TLphasingData = dir([basePATH, '/*_TLphasingData.txt']);

% total number of simulation runs
runTotal = length(TLqueuingData);

for runNumber = 1:runTotal

fprintf('\n>>> runNumber %d:\n', runNumber);

disp('reading phasing information ...');
[phaseDurationTS, cycleNumber, totalPhases] = ReadPhasingData(basePATH, TLphasingData(runNumber).name);

% read the current TLqueuingData.txt file
[indices, timeSteps, totalQs, maxQs, laneCounts] = ReadTLqueuingData(basePATH, TLqueuingData(runNumber).name);

disp('calculate TL total cycles ...');
[timeSteps_SW, totalCycles] = TLtotalCycle(timeSteps, phaseDurationTS, cycleNumber, 400);

disp('calculate TL total green time ...');
[timeSteps_GR, totalGreenTime] = TLtotalGreenTime(timeSteps, totalPhases, phaseDurationTS, 400);

disp('plotting TL phasing information ...');
PlotTLPhasing(runNumber, timeSteps_SW, totalCycles, timeSteps_GR, totalGreenTime, runTotal);

end



