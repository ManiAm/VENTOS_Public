
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   %  closes the figure window

% ---------------------------------------------------------- 

mergeCount = 0;
splitCount = 0;
LLeaveCount = 0;
LFLeaveCount = 0;   
MFLeaveCount = 0;

for run=0:2
    filePath = sprintf('../results/%d_plnStat.txt', run);
    file_id = fopen(filePath);
    formatSpec = '%f %s %s %s';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
    fclose(file_id);

    % ----------------------------------------------------------
    
    timeSteps = C_text{1,1};
    vehicles = C_text{1,2};    
    maneuvers = C_text{1,4};

    % ----------------------------------------------------------

    [rows,~] = size(timeSteps);

    for i=1:rows    
        maneuver = char(maneuvers(i,1));
        time = double(timeSteps(i,1));
        
        % ignore warm-up
        if(time < 73)
            continue;
        end
    
        % mege
        if( strcmp(maneuver,'Merge_Start') == 1 )
            mergeStartT = double(timeSteps(i,1));
            initiatingV = char(vehicles(i,1));
        
            for j=i+1:rows
                currManeuver = char(maneuvers(j,1));
                currInitiatingV = char(vehicles(j,1));
            
                if(strcmp(currManeuver,'Merge_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                    mergeEndT = double(timeSteps(j,1));
                    mergeDuration = mergeEndT - mergeStartT;
                    mergeCount = mergeCount + 1;
                
                    mergeStat(mergeCount,1) = run;
                    mergeStat(mergeCount,2) = mergeStartT; 
                    mergeStat(mergeCount,3) = mergeEndT;
                    mergeStat(mergeCount,4) = mergeDuration;
                    break;
                end            
            end       
        end
    
        % split
        if( strcmp(maneuver,'Split_Start') == 1 )
            splitStartT = double(timeSteps(i,1));
            initiatingV = char(vehicles(i,1));
        
            for j=i+1:rows
                currManeuver = char(maneuvers(j,1));
                currInitiatingV = char(vehicles(j,1));
            
                if(strcmp(currManeuver,'Split_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                    splitEndT = double(timeSteps(j,1));
                    splitDuration = splitEndT - splitStartT;
                    splitCount = splitCount + 1;
                
                    splitStat(splitCount,1) = run;
                    splitStat(splitCount,2) = splitStartT;
                    splitStat(splitCount,3) = splitEndT;
                    splitStat(splitCount,4) = splitDuration;
                    break;
                end            
            end       
        end
    end
end

% ---------------------------------------------------------- 
% ----------------------------------------------------------

% merge
data(1,1) = mergeStat(1,4);
data(1,2) = mergeStat(2,4);
data(1,3) = mergeStat(3,4);

% split
data(2,1) = splitStat(1,4);
data(2,2) = splitStat(2,4);
data(2,3) = splitStat(3,4);

figure(1);
h = bar(data);

set(gca,'XTickLabel',{'Merge','Split'}', 'FontSize', 19);

ylabel('Duration (s)', 'FontSize', 20);

grid on;

set(h(1),'Displayname', 'T_p = 2 s');
set(h(2),'Displayname', 'T_p = 3.5 s');
set(h(3),'Displayname', 'T_p = 5 s');

set(h(1), 'FaceColor', [0.2 0.2 0.2]);
set(h(2), 'FaceColor', [0.5 0.5 0.5]);
set(h(3), 'FaceColor', [0.92 0.92 0.92]);

handle = legend('Location','northeast');

disp('done!');


