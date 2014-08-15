
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   %  closes the figure window

% ----------------------------------------------------------   

file_id = fopen('../results/gui/plnStat.txt');
formatSpec = '%f %s %s %s';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

% ----------------------------------------------------------
    
timeSteps = C_text{1,1};
vehicles = C_text{1,2};    
maneuvers = C_text{1,4};

% ----------------------------------------------------------

[rows,~] = size(timeSteps);
mergeCount = 0;
splitCount = 0;
LLeaveCount = 0;
FLeaveCount = 0;

for i=1:rows    
    maneuver = char(maneuvers(i,1));
    
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
                
                mergeStat(mergeCount,1) = mergeStartT; 
                mergeStat(mergeCount,2) = mergeEndT;
                mergeStat(mergeCount,3) = mergeDuration;
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
                
                splitStat(splitCount,1) = splitStartT;
                splitStat(splitCount,2) = splitEndT;
                splitStat(splitCount,3) = splitDuration;
                break;
            end            
        end       
    end
    
    % leader leave
    if( strcmp(maneuver,'LLeave_Start') == 1 )
        LLeaveStartT = double(timeSteps(i,1));
        initiatingV = char(vehicles(i,1));
        
        for j=i+1:rows
            currManeuver = char(maneuvers(j,1));
            currInitiatingV = char(vehicles(j,1));
            
            if(strcmp(currManeuver,'LLeave_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                LLeaveEndT = double(timeSteps(j,1));
                LLeaveDuration = LLeaveEndT - LLeaveStartT;
                LLeaveCount = LLeaveCount + 1;
                
                LLeaveStat(LLeaveCount,1) = LLeaveStartT; 
                LLeaveStat(LLeaveCount,2) = LLeaveEndT;
                LLeaveStat(LLeaveCount,3) = LLeaveDuration;
                break;
            end            
        end       
    end
    
    % follower leave
    if( strcmp(maneuver,'FLeave_Start') == 1 )
        FLeaveStartT = double(timeSteps(i,1));
        initiatingV = char(vehicles(i,1));
        
        for j=i+1:rows
            currManeuver = char(maneuvers(j,1));
            currInitiatingV = char(vehicles(j,1));
            
            if(strcmp(currManeuver,'FLeave_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                FLeaveEndT = double(timeSteps(j,1));
                FLeaveDuration = FLeaveEndT - FLeaveStartT;
                FLeaveCount = FLeaveCount + 1;
                
                FLeaveStat(FLeaveCount,1) = FLeaveStartT;
                FLeaveStat(FLeaveCount,2) = FLeaveEndT;
                FLeaveStat(FLeaveCount,3) = FLeaveDuration;
                break;
            end            
        end       
    end   
end

% ---------------------------------------------------------- 
% ----------------------------------------------------------


arg_to_remove = [1:9];
mergeStat(arg_to_remove, :) = [];

%arg_to_remove = [12, 15, 16, 17];
%mergeStat(arg_to_remove, :) = [];

% ---------------------------------------------------------- 
% ----------------------------------------------------------

% multiple boxplots with vectors of different length

[n1, ~] = size(mergeStat(:,3));
[n2, ~] = size(splitStat(:,3));
[n3, ~] = size(LLeaveStat(:,3));
[n4, ~] = size(FLeaveStat(:,3));

figure(1)
group = [repmat({'Merge'}, n1, 1); repmat({'Split'}, n2, 1); repmat({'Leader Leave'}, n3, 1); repmat({'Follower Leave'}, n4, 1)];
boxplot([mergeStat(:,3); splitStat(:,3); LLeaveStat(:,3); FLeaveStat(:,3)], group)








data(1,1) = mean(mergeStat(:,3));

% calculate the confidence interval
[~,~,ci] = ztest(mergeStat(:,3), mean(mergeStat(:,3)), std(mergeStat(:,3)));
Xci(1, 1) = abs( ci(1,1) - mean(mergeStat(:,3)) );
Xci(1, 2) = abs( ci(2,1) - mean(mergeStat(:,3)) );

data(2,1) = mean(splitStat(:,3));

% calculate the confidence interval
[~,~,ci] = ztest(splitStat(:,3), mean(splitStat(:,3)), std(splitStat(:,3)));
Xci(2, 1) = abs( ci(1,1) - mean(splitStat(:,3)) );
Xci(2, 2) = abs( ci(2,1) - mean(splitStat(:,3)) );

data(3,1) = mean(LLeaveStat(:,3));

% calculate the confidence interval
[~,~,ci] = ztest(LLeaveStat(:,3), mean(LLeaveStat(:,3)), std(LLeaveStat(:,3)));
Xci(3, 1) = abs( ci(1,1) - mean(LLeaveStat(:,3)) );
Xci(3, 2) = abs( ci(2,1) - mean(LLeaveStat(:,3)) );

data(4,1) = mean(FLeaveStat(:,3));

% calculate the confidence interval
[~,~,ci] = ztest(FLeaveStat(:,3), mean(FLeaveStat(:,3)), std(FLeaveStat(:,3)));
Xci(4, 1) = abs( ci(1,1) - mean(FLeaveStat(:,3)) );
Xci(4, 2) = abs( ci(2,1) - mean(FLeaveStat(:,3)) );

% -----------------------------------------------------------

figure(3)
bar(data);

set(gca,'XTickLabel',{'Merge','Split','Leader Leave', 'Follower Leave'}', 'FontSize', 19); 
rotateXLabels(gca, 0);  

ylabel('Duration (s)', 'FontSize', 19);

grid on;

% graph the confident interval
hold on
errorbar((1:4), data, Xci(:,1), Xci(:,2), 'r', 'LineWidth', 2, 'LineStyle', 'none')
hold off


disp('done!');





