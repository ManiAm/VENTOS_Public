
function [phaseDurationTS, cycleNumber, totalPhases] = ReadPhasingData(basePATH, name)

path3 = sprintf('%s/%s', basePATH, name);
file_id = fopen(path3);
formatSpec = '%s %d %d %s %f %f %f %f %f %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);

phaseNumbers = C_text{1,2};
cycleNumber = C_text{1,3};
greenLength = C_text{1,5};
greenStart = C_text{1,6};
yellowStart = C_text{1,7};
redStart = C_text{1,8};
endTime = C_text{1,9};
lanesCount = C_text{1,10};
queueSize = C_text{1,11};

totalPhases = size(phaseNumbers,1);
phaseDurationTS = zeros(4,totalPhases);
averageQueuePerPhase = zeros(1,totalPhases);

for i=1:totalPhases
    phaseDurationTS(1,i) = double(greenStart(i,1));
    phaseDurationTS(2,i) = double(yellowStart(i,1)); 
    phaseDurationTS(3,i) = double(redStart(i,1));
    phaseDurationTS(4,i) = double(endTime(i,1));
    
    % store average queue size per phase
    if( double(queueSize(i,1)) ~= -1 && double(lanesCount(i,1)) > 0 )
        averageQueuePerPhase(1,i) = double(queueSize(i,1)) / double(lanesCount(i,1));
    end
end

% if the last phase is incomplete
if(phaseDurationTS(1,totalPhases) == -1 || phaseDurationTS(2,totalPhases) == -1 || phaseDurationTS(3,totalPhases) == -1 || phaseDurationTS(4,totalPhases) == -1)
    phaseDurationTS(:,totalPhases) = [];
    totalPhases = totalPhases - 1;
end

end