
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

disp('reading file ...');

folderPath = '../results/cmd/test3_noRightTurns';

path = strcat(folderPath,'/0_vehicleData.txt');    
file_id = fopen(path);
formatSpec = '%d %f %s %s %s %f %f %f %s %f %f %f %s';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3}; 
lanes = C_text{1,5};
speeds = C_text{1,7};
signal = C_text{1,13};

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

clear C_text indices timeSteps lanes speeds signal
    
[rows,~] = size(vehiclesLane);

% ---------------------------------------------------------------

disp('calculation ...');

% vehiclesLane contains the lanes (controlled by a TL) that a vehicle traveres 
% before reaching to the intersection. We are only interested into the last lane
% that the vehicle is on before entering into the intersection.

indexLane = zeros(3,VehNumbers) - 1;  % initialize to -1
crossed = zeros(1,VehNumbers);  % did the vehicle cross the intersection?

for i=1:VehNumbers    
    stoppingLane = '';
    startingIndex = -1;
    endingIndex = -1;
    
    for j=1:rows         
        currentLane = char( vehiclesLane{j,i} );
        if( isempty(strfind(currentLane, ':C_')) )  % if not in the center of intersection
            if( strcmp(stoppingLane,currentLane) ~= 1 )
                stoppingLane = currentLane;
                startingIndex = j;        
            end
        else
            endingIndex = j-1;
            
            indexLane(1,i) = startingIndex;
            indexLane(2,i) = endingIndex;
            indexLane(3,i) = endingIndex - startingIndex + 1;
            
            crossed(i) = 1; % the vehicle crossed the intersection
            
            break;
        end   
    end
end

% ---------------------------------------------------------------

% indexLane(1,i)/indexLane(2,i) show the section of the last lane that each
% vehicle traverses before reaching to the intersection

% extracting the vehicle speed between indexLane(1,i) and indexLane(2,i)

waitingSpeeds = zeros( size(vehiclesTS,1), VehNumbers ) - 1;   % initialize to -1

for i=1:VehNumbers  
    if( indexLane(1,i) ~= -1 && indexLane(2,i) ~= -1 )
        for j=indexLane(1,i):indexLane(2,i)
            waitingSpeeds(j,i) = vehiclesSpeed(j,i)';
        end
    end
end

% ---------------------------------------------------------------

% looking for the start of 'deceleration delay'
smoothDelay = zeros(size(waitingSpeeds,1)-1,1);
for i=1:VehNumbers
    index(1,i) = -1;
    smoothDelay(:,1) = diff( waitingSpeeds(:,i) );
    
    for j=1:(rows-11)
        if( all(smoothDelay(j+1:j+10,1) ~= 0) && all(smoothDelay(j+1:j+10,1) < -0.05) )
            index(1,i) = j;   % save the starting point
            break;
        end
    end
end

% looking for the start of 'stopping delay'
% Note that a vehicle might not have any 'stopping delay'
for i=1:VehNumbers
    index(2,i) = -1;
    
    % if we could not find any starting point then skip
    if(index(1,i) == -1)
        continue; 
    else
        % we keep going from the starting point until speed < 0.5
        for j=index(1,i):rows
            if( waitingSpeeds(j,i) <= 0.5 && waitingSpeeds(j,i) ~= -1)         
                index(2,i) = j;   % save the starting point
                break;
            end            
        end        
    end    
end

% looking for the start of 'acceleration delay'
for i=1:VehNumbers
    index(3,i) = -1;
    
    % if we could not find any starting point then skip
    if(index(1,i) == -1)
        continue; 
    % if stopping delay was found in previous step
    elseif(index(2,i) ~= -1)    
        for j=index(2,i):(rows-11)  % start from index(2,i)
            if( all(vehiclesSpeed(j+1:j+10,i) > 0.5) )
                index(3,i) = j;   % save the starting point
                break;
            end
        end
    % if stopping delay is zero
    elseif(index(2,i) == -1)
        for j=index(1,i):(rows-11)  % start from index(1,i)
            smoothDelay(:,1) = diff( vehiclesSpeed(:,i) );
            
            if( all(smoothDelay(j+1:j+10,1) > 0) )
                index(3,i) = j;   % save the starting point
                break;
            end
        end
    end    
end

% looking for the end of 'acceleration delay'
for i=1:VehNumbers
    index(4,i) = -1;
    
    % if we could not find any starting point then skip
    if(index(3,i) == -1)
        continue; 
    else    
        % we keep going from the starting point until we reach the old speed
        oldVehicleSpeed = vehiclesSpeed(index(1,i), i);
        
        for j=index(3,i):rows
            if( vehiclesSpeed(j,i) >= oldVehicleSpeed )
                index(4,i) = j;   % save the end point
                break;
            end
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

% we need to get TL phasing information + queue size

path = strcat(folderPath, '/0_intersectionData.txt');    
file_id = fopen(path);
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
phaseDuration = zeros(4,totalPhases);
phaseData = zeros(2,totalPhases);

for i=1:totalPhases
    phaseDuration(1,i) = double(greenStart(i,1));
    phaseDuration(2,i) = double(yellowStart(i,1)); 
    phaseDuration(3,i) = double(redStart(i,1));
    phaseDuration(4,i) = double(endTime(i,1));
    
    % store average queue size
    if( double(queueSize(i,1)) ~= -1)
        phaseData(1,i) = double(queueSize(i,1)) / double(lanesCount(i,1));
    else
        phaseData(1,i) = -1;
    end
end

% -----------------------------------------------------------------

% draw speed profile of vehicles and mark delay components with colors
% as well as showing TL states for each movement

figLimit = 2;    % limit the number of windows shown (-1: show all)
counter = 1;
figNum = 0; 
figNum_old = -1;

for i=1:VehNumbers

    figNum = floor( (i-1)/8 ) + 1;
    
    % show only one figure
    if(figLimit ~= -1 && figNum > figLimit)
        break;    
    elseif(figNum ~= figNum_old)
        figure('name','Speed', 'units', 'normalized', 'outerposition', [0 0 1 1], 'outerposition', [0 0 1 1]);
        figNum_old = figNum;
    end

    subplot(4,2,counter); 
    
    % get rid of the negative speed
    X = vehiclesTS;
    Y = vehiclesSpeed(:,i);    
    t = (Y < 0);    
    X(t) = [];
    Y(t) = [];    
    
    plot(X, Y,'LineWidth', 3);

    counter = counter + 1;
    % reset the counter
    if(counter == 9)
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
    if( crossed(i) == 0 )        
        text(4, 13, 'Not crossed!');
    end  
    
    hold on;
    
    % mark deceleration delay with red
    if( index(1,i) ~= -1 )
        endPoint = index(2,i);
        if(endPoint == -1)        
            endPoint = index(3,i);
        end        
        X = vehiclesTS(index(1,i):endPoint, 1);
        Y = vehiclesSpeed(index(1,i):endPoint, i);
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
    
    % show TL status ahead for this vehicle
    
    % TODO: currently it is 0 or 1. I should change it to g G y r later.
    % I should wait for the implementation of the TraCI command that
    % gives the TL status ahead!
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
    phasesCount = size(phaseDuration,2);
    for p=1:phasesCount        
        if(phaseDuration(1,p) ~= -1 && phaseDuration(4,p) ~= -1)
            Start = [phaseDuration(1,p) 34];
            Stop = [phaseDuration(4,p) 34];
            arrow(Start, Stop, 'Ends', 3); 
        end
        
        % if the finish time of the last phase is -1
        if(p == phasesCount && phaseDuration(1,p) ~= -1 && phaseDuration(4,p) == -1)
            Start = [phaseDuration(1,p) 34];
            Stop = [vehiclesTS(end) 34];
            arrow(Start, Stop, 'Ends', 2);             
        end
    end
    
    hold off;
    
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

if(false)
    
    cell1 = vIDs;
    cell2 = num2cell(crossed');
    cell3 = [ num2cell(index(1,:)') num2cell(index(2,:)') num2cell(index(3,:)') num2cell(index(4,:)') ];
    cell4 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') ];

    data = [ cell1 cell2 cell3 cell4 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'deceleration start index', 'stopping start index', 'acceleration start index', 'acceleration end index', 'deceleration start TS', 'stopping start TS', 'acceleration start TS', 'acceleration end TS'}; 

end

% -----------------------------------------------------------------

%{ 

if(vehicle does not slow down before the intersection)
{
    // it is getting green and has right of way
    // this happens when a movement gets G
    // the delay is zero for this vehicle.
}
else if(vehicle slows down)
{
    if(isGreen and g)
    {
        // vehicle slows down and the TL ahead is green.

        // for example it is slowing down before turning right or
        // a preceeding vehicle is blocking its way.
        // the delay is zero for this vehicle.
    }
    else
    {
        //

    }
}


%}


i = 0;






% the slowing down can be due to many reasons other than red/yellow TL.
% for example the vehicle slows down when turning at intersection
% we are only interested into those slowing downs that are due to red or
% yellow signal

% for i=1:VehNumbers  
%     if( index(1,i) ~= -1)
%         target = index(2,i);
%         if(target == -1)
%             target = index(3,i);
%         end
%         % index(1,i) and target specify the acceleration delay interval        
%         newStart = index(1,i);
%         for j=index(1,i):target
%             if( vehicleSignal(j,i) ~= 1 )  % if the signal is not yellow or red
%                 newStart = newStart + 1;
%             end
%         end
%         
%         if(newStart-1 == target)
%             index(1,i) = -1;
%             index(2,i) = -1;
%             index(3,i) = -1;
%             index(4,i) = -1;
%         else
%             index(1,i) = newStart;            
%         end        
%     end
% end        
         


% calculate the average delay at the end of each phase

for j=1:size(phaseDuration,2)  % j is phase number
   
   crossedVehCount = 0;   % number of crossed vehicles in this phase
   delayedVehCount = 0;   % number of delayed vehicles
   vehCount = 0;
   totalDelay = 0;
   phaseStart = phaseDuration(1,j);
   phaseEnd = phaseDuration(2,j);
   
   for i=1:VehNumbers
       if(indexTS(1,i) ~= -1 && indexTS(4,i) ~= -1)
           d1 = max(phaseStart,indexTS(1,i));
           d2 = min(phaseEnd,indexTS(4,i));
           difference = d2 - d1;
           if(difference > 0)
               totalDelay = totalDelay + difference;
               delayedVehCount = delayedVehCount + 1;
           end
       end
   end
   
   phaseData(2,j) = totalDelay/vehCount;
   fprintf( 'phase %d: crossedVehCount=%d, delayedVehCount=%d, totalDelay=%0.2f, aveDelay=%0.2f\n', j, 0, delayedVehCount, totalDelay, totalDelay/vehCount);
    
end

% -----------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
set(gcf,'name','Speed');

subplot(2,1,1);
handle1 = plot(phaseData(1,:),'LineWidth', 3);
  
% set the x-axis limit
set( gca, 'XLim', [0 size(phaseData,2)+1] );

% x axis should be integer
set(gca, 'xtick' , 0:2:size(phaseDuration,2)+1);
    
% set the y-axis limit
set( gca, 'YLim', [-0.1 max(phaseData(1,:))+0.2] );

% set font size
set(gca, 'FontSize', 17);

grid on;
    
xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Queue Size', 'FontSize', 17);

subplot(2,1,2);
handle1 = plot(phaseData(2,:),'LineWidth', 3);
  
% set the x-axis limit
set( gca, 'XLim', [0 size(phaseData,2)+1] );

% x axis should be integer
set(gca, 'xtick' , 0:2:size(phaseDuration,2)+1);
    
% set the y-axis limit
set( gca, 'YLim', [-0.1 max(phaseData(2,:))+0.2] );

% set font size
set(gca, 'FontSize', 17);

grid on;
    
xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Delay (s)', 'FontSize', 17);

% % set the name for each line
% set(handle1(1),'Displayname', 'Adaptive-time (R&B)');
%     
% % set the legend
% legend(handle1, 'Location','NorthEastOutside'); 

% -----------------------------------------------------------------

% specify TS interval in previous figure with vertical lines

for threshold=100:100:vehiclesTS(end)
    for i=1:size(phaseDuration,2)
        
        if(threshold >= phaseDuration(1,i) && threshold <= phaseDuration(2,i))            
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



% ----------------------------------------------------------------

% print delay for each component

% delayT = [];
% delayVeh = [];
% delayBike = [];
% delayPed = [];
% 
% for i=1:VehNumbers 
%     
%     if( delayTotal(3,i) ~= -1 )  
%         delayT(end+1) = delayTotal(3,i);
%         name = lower( char(vIDs{i}) );
%         
%         if( ~isempty(strfind(name, 'veh')) )
%             delayVeh(end+1) = delayTotal(3,i);
%         elseif( ~isempty(strfind(name, 'bike')) )
%             delayBike(end+1) = delayTotal(3,i);
%         elseif( ~isempty(strfind(name, 'ped')) )
%             delayPed(end+1) = delayTotal(3,i);
%         end
%     end
%     
% end
% 
% fprintf( 'Average total delay: %0.2f s\n', mean(delayT) );
% fprintf( 'Average vehicle delay: %0.2f s\n', mean(delayVeh) );
% fprintf( 'Average bike delay: %0.2f s\n', mean(delayBike) );
% fprintf( 'Average pedestrian delay: %0.2f s\n', mean(delayPed) );


