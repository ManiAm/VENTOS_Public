
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

% total number of simulation runs
runTotal = 3;

for runNumber = 0:runTotal-1

basePATH = '../results/full_fix_web_adap_balanced_newFormat';
path = sprintf('%s/%d_vehicleData.txt', basePATH, runNumber);
path2 = sprintf('%s/%d_intersectionData.txt', basePATH, runNumber);

% ---------------------------------------------------------------

disp('reading vehicleData.txt ...');

file_id = fopen(path);
formatSpec = '%d %f %s %s %f %s';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3}; 
lanes = C_text{1,4};
speeds = C_text{1,5};
signal = C_text{1,6};

% ---------------------------------------------------------------

disp('parsing file ...');
    
% stores vehicle IDs
vIDs = unique(vehicles);
vIDs = sort_nat(vIDs);
VehNumbers = size(vIDs,1);

[rows,~] = size(vehicles);
n = indices(end,1);

% preallocating and initialization with -1
vehiclesTS = zeros(n,1) - 1;
vehiclesSpeed = zeros(n,VehNumbers) - 1;
vehiclesLane = cell(n,VehNumbers);
vehicleSignal = cell(n,VehNumbers);

for i=1:rows   
       
    index = int32(indices(i,1));        

    % store the current TS
    vehiclesTS(index,1) = double(timeSteps(i,1));
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));
        
    vNumber = find(ismember(vIDs,vehicle)); 

    vehiclesSpeed(index,vNumber) = double(speeds(i,1));         
    vehiclesLane(index, vNumber) = lanes(i,1);
    vehicleSignal(index, vNumber) = signal(i,1);
end

%clear C_text indices timeSteps lanes speeds signal
    
[rows,~] = size(vehiclesLane);

% ---------------------------------------------------------------

disp('get the last lane before the intersection ...');

% vehiclesLane contains the lanes (controlled by a TL) that a vehicle traveres 
% before reaching to the intersection. We are only interested into the last lane
% that the vehicle is on before entering into the intersection.

indexLane = zeros(3,VehNumbers) - 1;  % initialize to -1
crossed = zeros(2,VehNumbers);        % does the vehicle cross the intersection?
changedLane = zeros(1,VehNumbers);    % does the vehicle change lane at least once

for i=1:VehNumbers    
    stoppingLane = '';
    startingIndex = -1;
    endingIndex = -1;
    
    for j=1:rows         
        currentLane = char( vehiclesLane{j,i} );
        if( isempty(strfind(currentLane, ':C_')) )  % if not in the center of intersection
            if( strcmp(stoppingLane,currentLane) ~= 1 )
                if(strcmp(stoppingLane,'') ~= 1)
                    changedLane(i) = 1;
                end
                stoppingLane = currentLane;
                startingIndex = j;
            end
        else
            endingIndex = j-1;
            
            indexLane(1,i) = startingIndex;
            indexLane(2,i) = endingIndex;
            indexLane(3,i) = endingIndex - startingIndex + 1;
            
            crossed(1,i) = 1;              % the vehicle crossed the intersection
            crossed(2,i) = endingIndex+1;  % when crossing happens?
            
            break;
        end   
    end
    
    if(crossed(1,i) == 0)
        indexLane(1,i) = startingIndex;
        indexLane(2,i) = rows; 
        indexLane(3,i) = rows - startingIndex + 1;        
    end
    
end

% ---------------------------------------------------------------

disp('extract the speed in the last lane before the intersection ...');

% indexLane(1,i)/indexLane(2,i) show the section of the last lane that each
% vehicle traverses before reaching to the intersection

% extracting the vehicle speed between indexLane(1,i) and indexLane(2,i)

vehiclesSpeedBeforeIntersection = zeros( size(vehiclesTS,1), VehNumbers ) - 1;   % initialize to -1

for i=1:VehNumbers  
    if( indexLane(1,i) ~= -1 && indexLane(2,i) ~= -1 )
        for j=indexLane(1,i):indexLane(2,i)
            vehiclesSpeedBeforeIntersection(j,i) = vehiclesSpeed(j,i)';
        end
    end
end

% ---------------------------------------------------------------

disp('looking for the start of each delay component ...');

% looking for the start of 'deceleration delay'
smoothDelay = zeros(size(vehiclesSpeedBeforeIntersection,1)-1,1);
for i=1:VehNumbers
    index(1,i) = -1;
    
    smoothDelay(:,1) = diff( vehiclesSpeedBeforeIntersection(:,i) );    
    for j=1:(rows-11)
        if( all(smoothDelay(j+1:j+10,1) < -0.05) )
            index(1,i) = j;   % save the starting point
            break;
        end
    end
end

% looking for the start of 'stopping delay'
% Note that a vehicle might not have any 'stopping delay'
for i=1:VehNumbers
    index(2,i) = -1;
    
    % if no deceleration is found then skip
    if(index(1,i) == -1)
        continue; 
    else
        % we keep going from the starting point until speed < 0.5
        found = false;
        for j=index(1,i):rows
            if( vehiclesSpeedBeforeIntersection(j,i) <= 0.5 && vehiclesSpeedBeforeIntersection(j,i) ~= -1)         
                index(2,i) = j;   % save the starting point
                found = true;
                break;
            end            
        end
        % if not found and the vehicle did not cross
        if(~found && crossed(1,i) == 0)
            index(2,i) = rows;  % mark the end of simulation as the starting point of stopping delay
        end
    end    
end

% looking for the start of 'acceleration delay'
for i=1:VehNumbers
    index(3,i) = -1;
    
    % if no deceleration is found then skip
    if(index(1,i) == -1)
        continue; 
    % if the start of stopping delay was found in previous step
    elseif(index(2,i) ~= -1)    
        found = false;
        for j=index(2,i):(rows-11)  % start from index(2,i)
            if( all(vehiclesSpeed(j+1:j+10,i) > 0.5) )  % as soon as the speed > 0.5
                index(3,i) = j;   % save the starting point
                found = true;
                break;
            end
        end
        if(~found && crossed(1,i) == 0)
            index(3,i) = rows;   % mark the end as starting point of acceleration
        end
    % if stopping delay was not found (it is zero)
    elseif(index(2,i) == -1)
        for j=index(1,i):(rows-11)  % start from deceleration point
            smoothDelay(:,1) = diff( vehiclesSpeed(:,i) );            
            if( all(smoothDelay(j+1:j+10,1) > 0) )  % as soon as the slope > 0
                index(3,i) = j;   % save the starting point
                break;
            end
        end
        index(2,i) = index(3,i); 
    end    
end

% looking for the end of 'acceleration delay'
for i=1:VehNumbers
    index(4,i) = -1;
    
    % if we could not find any starting point then skip
    if(index(1,i) == -1 || index(3,i) == -1)
        continue; 
    else    
        % we keep going from the starting point until we reach the old speed
        oldVehicleSpeed = vehiclesSpeed(index(1,i), i);
        
        found = false;
        for j=index(3,i):rows
            if( vehiclesSpeed(j,i) >= oldVehicleSpeed )
                index(4,i) = j;   % save the end point
                found = true;
                break;
            end
        end
        if(~found)
            index(4,i) = rows;             
        end
    end    
end

% making sure that index is in the correct format
for i=1:VehNumbers
    if(index(1,i) ~= -1)
        % none of these indices shold be negative
        if( index(2,i) == -1 || index(3,i) == -1 || index(4,i) == -1 )
            error('one of the entries in index is -1');
        end   
    end
end

% -----------------------------------------------------------------

%{
At this point, we have the starting point of all delay components:
- index(1,i): start of the deceleration delay
- index(2,i): start of the stopping delay
- index(3,i): start of the acceleration delay
- index(4,i): end of the acceleration delay
%}

disp('reading IntersectionData.txt and get phasing information ...');

% we need to get TL phasing information + queue size
    
file_id = fopen(path2);
formatSpec = '%s %d %s %f %f %f %f %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

phaseNumbers = C_text{1,2};
greenStart = C_text{1,4};
yellowStart = C_text{1,5};
redStart = C_text{1,6};
endTime = C_text{1,7};
lanesCount = C_text{1,8};
queueSize = C_text{1,9};

totalPhases = size(phaseNumbers,1);
phaseDurationTS = zeros(4,totalPhases);
phaseData = zeros(2,totalPhases);

for i=1:totalPhases
    phaseDurationTS(1,i) = double(greenStart(i,1));
    phaseDurationTS(2,i) = double(yellowStart(i,1)); 
    phaseDurationTS(3,i) = double(redStart(i,1));
    phaseDurationTS(4,i) = double(endTime(i,1));
    
    % store average queue size
    if( double(queueSize(i,1)) ~= -1 && double(lanesCount(i,1)) > 0 )
        phaseData(1,i) = double(queueSize(i,1)) / double(lanesCount(i,1));
    else
        phaseData(1,i) = phaseData(1,i-1);
    end
end

% remove possible -1 in the last phase
for i=1:4
    if(phaseDurationTS(i,totalPhases) == -1)
        phaseDurationTS(i,totalPhases) = vehiclesTS(end,1);
    end
end

% it might be usefull to get the index of phaseDurationTS
phaseDuration = int32(zeros(4, totalPhases) - 1);
for i=1:totalPhases
    for j=1:4
        if( phaseDurationTS(j,i) ~= -1 )
            phaseDuration(j,i) = find(vehiclesTS >= phaseDurationTS(j,i),1);            
        end
    end
end

% -----------------------------------------------------------------

% draw speed profile of vehicles and mark delay components with colors
% as well as showing TL states for each movement

disp('draw speed profiles ...');

figLimit = 0;   % limit the number of windows shown (-1: show all)
counter = 1;
figNum = 0; 
figNum_old = -1;

for i=1:VehNumbers

    figNum = floor( (i-1)/6 ) + 1;
    
    % show only one figure
    if(figLimit ~= -1 && figNum > figLimit)
        break;    
    elseif(figNum ~= figNum_old)
        figure('name','Speed', 'units', 'normalized', 'outerposition', [0 0 1 1], 'outerposition', [0 0 1 1]);
        figNum_old = figNum;
    end

    subplot(3,2,counter); 
    
    % get rid of the negative speed
    X = vehiclesTS;
    Y = vehiclesSpeed(:,i);    
    t = (Y < 0);    
    X(t) = [];
    Y(t) = [];    
    
    plot(X, Y,'LineWidth', 3);

    counter = counter + 1;
    % reset the counter
    if(counter == 7)
        counter = 1;
    end
    
    % set the x-axis limit
    set( gca, 'XLim', [0 vehiclesTS(end)] );
    
    % set the y-axis limit
    set( gca, 'YLim', [-2 36] );

    % set font size
    set(gca, 'FontSize', 17);

    grid on;
    
    title( char(vIDs{i}) );
    
    % if the vehicle did not cross the intersection at all
    if( crossed(1,i) == 0 )        
        text(4, 13, 'Not crossed!');
    end  
    
    hold on;
    
    % mark deceleration delay with red
    if( index(1,i) ~= -1 && index(2,i) ~= -1 )
        X = vehiclesTS(index(1,i):index(2,i), 1);
        Y = vehiclesSpeed(index(1,i):index(2,i), i);
        plot(X, Y, 'LineWidth', 3, 'Color','red');
    end
    
    % mark stopping delay with black
    if( index(2,i) ~= -1 && index(3,i) ~= -1 )
        X = vehiclesTS(index(2,i):index(3,i), 1);
        Y = vehiclesSpeed(index(2,i):index(3,i), i);
        plot(X, Y, 'LineWidth', 3, 'Color','black');
    end
    
    % mark acceleration delay with green
    if( index(3,i) ~= -1 && index(4,i) ~= -1 )
        X = vehiclesTS(index(3,i):index(4,i), 1);
        Y = vehiclesSpeed(index(3,i):index(4,i), i);
        plot(X, Y, 'LineWidth', 3, 'Color','green');
    end
    
    hold off;
    
    % show TL status ahead for this vehicle
    for k=1:size(vehicleSignal,1)    
        signalStatus = vehicleSignal(k,i);
        
        if(strcmp(signalStatus,'g') == 1)
            line([vehiclesTS(k) vehiclesTS(k)+0.09],[32 32], 'LineWidth', 3, 'Color', rgb('LightGreen'));            
        elseif(strcmp(signalStatus,'G') == 1)
            line([vehiclesTS(k) vehiclesTS(k)+0.09],[32 32], 'LineWidth', 3, 'Color', rgb('DarkGreen'));     
        elseif(strcmp(signalStatus,'y') == 1)
            line([vehiclesTS(k) vehiclesTS(k)+0.09],[32 32], 'LineWidth', 3, 'Color', 'y'); 
        elseif(strcmp(signalStatus,'r') == 1)
            line([vehiclesTS(k) vehiclesTS(k)+0.09],[32 32], 'LineWidth', 3, 'Color', 'r'); 
        elseif(strcmp(signalStatus,'n') == 1)
            line([vehiclesTS(k) vehiclesTS(k)+0.09],[32 32], 'LineWidth', 3, 'Color', 'k'); 
        end       
    end
    
    % show phase numbering for this vehicle
    phasesCount = size(phaseDurationTS,2);
    for p=1:phasesCount        
        if(phaseDurationTS(1,p) ~= -1 && phaseDurationTS(4,p) ~= -1)
            Start = [phaseDurationTS(1,p) 34];
            Stop = [phaseDurationTS(4,p) 34];
            arrow(Start, Stop, 'Ends', 3); 
        end
    end
    
end

% -----------------------------------------------------------------

% convert index to TS

indexTS = zeros(4, size(index,2)) - 1;

for i=1:VehNumbers 
    
    if(index(1,i) ~= -1)
        indexTS(1,i) = vehiclesTS( index(1,i) );
    end
    
    if(index(2,i) ~= -1)
        indexTS(2,i) = vehiclesTS( index(2,i) );
    end
    
    if(index(3,i) ~= -1)
        indexTS(3,i) = vehiclesTS( index(3,i) );
    end
    
    if(index(4,i) ~= -1)
        indexTS(4,i) = vehiclesTS( index(4,i) );
    end    
    
end

% -----------------------------------------------------------------

% making a cell array that contains the complete view of the system

disp('making uitable ...');

if(false)
    
    cell1 = vIDs;
    cell2 = num2cell(crossed(1,:)');
    cell3 = [ num2cell(index(1,:)') num2cell(index(2,:)') num2cell(index(3,:)') num2cell(index(4,:)') ];
    cell4 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') ];

    data = [ cell1 cell2 cell3 cell4 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'deceleration start index', 'stopping start index', 'acceleration start index', 'acceleration end index', 'deceleration start TS', 'stopping start TS', 'acceleration start TS', 'acceleration end TS'}; 

end

% -----------------------------------------------------------------

disp('calculating the delay ...');

fprintf( '\ntotalVeh: %d, crossedVehCount: %d\n\n', VehNumbers, sum(crossed(1,:) == 1) );

phasesCount = size(phaseDurationTS,2);

crossedVehCount = zeros(phasesCount);   % number of crossed vehicles in each phase
delayedVehCount = zeros(phasesCount);   % number of delayed vehicles in each phase
   
totalDelay = zeros(phasesCount);        % total delay in each phase
   
for j=1:phasesCount
    
   phaseStart = phaseDurationTS(1,j);
   phaseEnd = phaseDurationTS(4,j);   
   
   phaseStartIndex = phaseDuration(1,j);
   phaseEndIndex = phaseDuration(4,j);

   for i=1:VehNumbers
       
       if( all(vehiclesSpeedBeforeIntersection(phaseStartIndex:phaseEndIndex,i) == -1) )
           % this vehicle is not present in phase j
           % We ignore this vehicle   
           
       elseif( indexTS(1,i) == -1 )
           % this vehicle does not slow down in this phase
           % 1) it is getting green (G) and has right of way or
           % 2) the distance to TL is high and there is no need to break
           
           if( crossed(1,i) == 1 && vehiclesTS(crossed(2,i)) < phaseEnd )
              % this vehicle crossed the intersection at this phase 
              crossedVehCount(j) = crossedVehCount(j) + 1;
           end      
           
       elseif( indexTS(1,i) ~= -1 && indexTS(1,i) <= phaseEnd && indexTS(1,i) >= phaseStart )
           % this vehicle slows down in this phase
           
           range = index(1,i):phaseDuration(4,j);
           vec1 = find( strcmp(vehicleSignal(range,i), 'y') );
           vec2 = find( strcmp(vehicleSignal(range,i), 'r') );
           if( ~isempty(vec1) || ~isempty(vec2) )
               % this slowing down is due to a red/yellow light 
               delayedVehCount(j) = delayedVehCount(j) + 1;
               startDelay = indexTS(1,i);
               endDelay = min( phaseEnd, indexTS(4,i) );
               totalDelay(j) = totalDelay(j) + (endDelay - startDelay); 
           else
               % this slowing down is due to getting 'g'
               crossedVehCount(j) = crossedVehCount(j) + 1;
           end     
           
       elseif(indexTS(1,i) ~= -1 && indexTS(1,i) < phaseStart )
           % this vehicle slowed down in previous phases
                  
           delayedVehCount(j) = delayedVehCount(j) + 1;
           startDelay = indexTS(1,i);
           endDelay = min( phaseEnd, indexTS(4,i) );
           totalDelay(j) = totalDelay(j) + (endDelay - startDelay);
       end       
   end

   tot = crossedVehCount(j) + delayedVehCount(j);
   if(tot ~= 0)
       phaseData(2,j) = totalDelay(j) / tot;
   else
       phaseData(2,j) = 0;
   end
   
   fprintf( 'phase %d: crossedVehCount=%2d, delayedVehCount=%2d, totalDelay=%0.2f, aveDelay=%0.2f, aveQueueSize=%0.2f\n', j, crossedVehCount(j), delayedVehCount(j), totalDelay(j), phaseData(2,j), phaseData(1,j) );
    
end

fprintf('\n');
   
% -----------------------------------------------------------------

% now we make a single plot for all the scenarios
disp('drawing queue size and average delay per phase ...');

if(runNumber == 0)
    figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
end

subplot(2,1,1);
handle1 = plot(phaseData(1,:),'LineWidth', 3);

% set font size
set(gca, 'FontSize', 17);

xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Queue Size', 'FontSize', 17);

grid on;
hold on;
    
subplot(2,1,2);
handle1 = plot(phaseData(2,:),'LineWidth', 3);

% set font size
set(gca, 'FontSize', 17);

xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Delay (s)', 'FontSize', 17);

grid on;
hold on;

% % set the name for each line
% set(handle1(1),'Displayname', 'Adaptive-time (R&B)');
%     
% % set the legend
% legend(handle1, 'Location','NorthEastOutside');

% at the end of the last iteration
if(runNumber == runTotal-1)

    % % set the x-axis limit
% set(gca, 'XLim', [0 size(phaseData,2)+1]);
% 
% % x axis should be integer
% set(gca, 'xtick' , 0:2:size(phaseDurationTS,2)+1);
%     
% % set the y-axis limit
% set(gca, 'YLim', [-0.1 max(phaseData(1,:))+0.2]);



% % set the x-axis limit
% set(gca, 'XLim', [0 size(phaseData,2)+1]);
% 
% % x axis should be integer
% set(gca, 'xtick' , 0:2:size(phaseDurationTS,2)+1);
%     
% % set the y-axis limit
% set( gca, 'YLim', [-0.1 max(phaseData(2,:))+0.2] );
    
    
    
    
    
    % specify TS interval with vertical lines
    for threshold=100:100:vehiclesTS(end)
        for i=1:size(phaseDurationTS,2)         
            if(threshold >= phaseDurationTS(1,i) && threshold <= phaseDurationTS(2,i))            
                subplot(2,1,1);
                % draw vertical line
                line([i i], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'r');
       
                subplot(2,1,2);
                % draw vertical line
                line([i i], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'r');            
                continue;
            end        
        end
    end 
    
end

end

