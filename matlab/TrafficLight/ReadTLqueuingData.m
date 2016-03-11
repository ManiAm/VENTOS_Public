
% Sample format of the TLqueuingData.txt file

% configName      TrafficSignalControl
% totalRun        16
% currentRun      3
% currentConfig   $TLControl=3, $passage=2s, $maxQ=-1, $addEntity=11, $routeDist="70,30,0", $repetition=0
% 
% 
% index     timeStep  TLid      totalQueue     maxQueue       laneCount 
% 
% 
% 1         0.10      C         0              0              16        
% 
% 2         0.20      C         0              0              16        
% 
% 3         0.30      C         0              0              16        
% 
% 4         0.40      C         0              0              16

function [indices, timeSteps, totalQs, maxQs, laneCounts] = ReadTLqueuingData(basePATH, name)

path2 = sprintf('%s/%s', basePATH, name);
file_id = fopen(path2);
formatSpec = '%d %f %s %d %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
totalQs = C_text{1,4};
maxQs = C_text{1,5};
laneCounts = C_text{1,6};

end

