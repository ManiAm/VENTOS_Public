
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

for run=0:12
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
                
                    LLeaveStat(LLeaveCount,1) = run; 
                    LLeaveStat(LLeaveCount,2) = LLeaveStartT; 
                    LLeaveStat(LLeaveCount,3) = LLeaveEndT;
                    LLeaveStat(LLeaveCount,4) = LLeaveDuration;
                    break;
                end            
            end       
        end
    
        % last follower leave
        if( strcmp(maneuver,'LFLeave_Start') == 1 )
            LFLeaveStartT = double(timeSteps(i,1));
            initiatingV = char(vehicles(i,1));
        
            for j=i+1:rows
                currManeuver = char(maneuvers(j,1));
                currInitiatingV = char(vehicles(j,1));
            
                if(strcmp(currManeuver,'FLeave_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                    LFLeaveEndT = double(timeSteps(j,1));
                    LFLeaveDuration = LFLeaveEndT - LFLeaveStartT;
                    LFLeaveCount = LFLeaveCount + 1;
                
                    LFLeaveStat(LFLeaveCount,1) = run;
                    LFLeaveStat(LFLeaveCount,2) = LFLeaveStartT;
                    LFLeaveStat(LFLeaveCount,3) = LFLeaveEndT;
                    LFLeaveStat(LFLeaveCount,4) = LFLeaveDuration;
                    break;
                end            
            end       
        end 
        
        % middle follower leave
        if( strcmp(maneuver,'MFLeave_Start') == 1 )
            MFLeaveStartT = double(timeSteps(i,1));
            initiatingV = char(vehicles(i,1));
        
            for j=i+1:rows
                currManeuver = char(maneuvers(j,1));
                currInitiatingV = char(vehicles(j,1));
            
                if(strcmp(currManeuver,'FLeave_End') == 1 && strcmp(currInitiatingV,initiatingV) == 1)
                    MFLeaveEndT = double(timeSteps(j,1));
                    MFLeaveDuration = MFLeaveEndT - MFLeaveStartT;
                    MFLeaveCount = MFLeaveCount + 1;
                
                    MFLeaveStat(MFLeaveCount,1) = run;
                    MFLeaveStat(MFLeaveCount,2) = MFLeaveStartT;
                    MFLeaveStat(MFLeaveCount,3) = MFLeaveEndT;
                    MFLeaveStat(MFLeaveCount,4) = MFLeaveDuration;
                    break;
                end            
            end       
        end        
    end
end

% ---------------------------------------------------------- 
% ----------------------------------------------------------

%arg_to_remove = [1:9];
%mergeStat(arg_to_remove, :) = [];

%arg_to_remove = [12, 15, 16, 17];
%mergeStat(arg_to_remove, :) = [];

% ---------------------------------------------------------- 
% ----------------------------------------------------------

% multiple boxplots with vectors of different length

%{
[n1, ~] = size(mergeStat(:,4));
[n2, ~] = size(splitStat(:,4));
[n3, ~] = size(LLeaveStat(:,4));
[n4, ~] = size(LFLeaveStat(:,4));
[n5, ~] = size(MFLeaveStat(:,4));

figure(1)
group = [repmat({'Merge'}, n1, 1); repmat({'Split'}, n2, 1); repmat({'Leader Leave'}, n3, 1); repmat({'Last Follower Leave'}, n4, 1); repmat({'Middle Follower Leave'}, n5, 1)];
boxplot([mergeStat(:,4); splitStat(:,4); LLeaveStat(:,4); LFLeaveStat(:,4); MFLeaveStat(:,4)], group);

set(findobj(gca,'Type','text'),'FontSize',19);

ylabel('Average duration (s)', 'FontSize', 19);

grid on;
%}

% ---------------------------------------------------------- 
% ----------------------------------------------------------

data(1,1) = mean(mergeStat(:,4));

% calculate the confidence interval
[~,~,ci] = ztest(mergeStat(:,4), mean(mergeStat(:,4)), std(mergeStat(:,4)));
Xci(1, 1) = abs( ci(1,1) - mean(mergeStat(:,4)) );
Xci(1, 2) = abs( ci(2,1) - mean(mergeStat(:,4)) );

data(2,1) = mean(splitStat(:,4));

% calculate the confidence interval
[~,~,ci] = ztest(splitStat(:,4), mean(splitStat(:,4)), std(splitStat(:,4)));
Xci(2, 1) = abs( ci(1,1) - mean(splitStat(:,4)) );
Xci(2, 2) = abs( ci(2,1) - mean(splitStat(:,4)) );

data(3,1) = mean(LLeaveStat(:,4));

% calculate the confidence interval
[~,~,ci] = ztest(LLeaveStat(:,4), mean(LLeaveStat(:,4)), std(LLeaveStat(:,4)));
Xci(3, 1) = abs( ci(1,1) - mean(LLeaveStat(:,4)) );
Xci(3, 2) = abs( ci(2,1) - mean(LLeaveStat(:,4)) );

data(4,1) = mean(LFLeaveStat(:,4));

% calculate the confidence interval
[~,~,ci] = ztest(LFLeaveStat(:,4), mean(LFLeaveStat(:,4)), std(LFLeaveStat(:,4)));
Xci(4, 1) = abs( ci(1,1) - mean(LFLeaveStat(:,4)) );
Xci(4, 2) = abs( ci(2,1) - mean(LFLeaveStat(:,4)) );

data(5,1) = mean(MFLeaveStat(:,4));

% calculate the confidence interval
[~,~,ci] = ztest(MFLeaveStat(:,4), mean(MFLeaveStat(:,4)), std(MFLeaveStat(:,4)));
Xci(5, 1) = abs( ci(1,1) - mean(MFLeaveStat(:,4)) );
Xci(5, 2) = abs( ci(2,1) - mean(MFLeaveStat(:,4)) );

% -------------------------------------

figure(2)
h = bar(data, 0.6);

set(gca,'XTickLabel',{'Merge','Split','Leader Leave', 'Last Follower Leave', 'Middle Follower Leave'}', 'FontSize', 19); 
rotateXLabels(gca, 0);  

ylabel('Duration (s)', 'FontSize', 20);

grid on;

set(h(1), 'FaceColor', [0.7 0.7 0.7]);

% graph the confident interval
hold on
errorbar((1:5), data, Xci(:,1), Xci(:,2), 'r', 'LineWidth', 2, 'LineStyle', 'none')
hold off

disp('done!');


