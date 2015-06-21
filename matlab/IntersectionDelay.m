
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------
folderPath = '../results/cmd/R&B';

path = strcat(folderPath,'/0_vehicleData.txt');    
file_id = fopen(path);
formatSpec = '%d %f %s %s %s %f %f %f %s %f %f %f %s %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3}; 
lanes = C_text{1,5};
speeds = C_text{1,7};
signal = C_text{1,14};

disp('reading file is done!');

% ---------------------------------------------------------------
    
% stores vehicle IDs
vIDs = {};

[rows,~] = size(vehicles);
n = indices(end,1);

% preallocating and initialization with -1
vehiclesTS = zeros(n,1) - 1;
vehiclesSpeed = zeros(n,1) - 1;
vehiclesLane = cell(n,1);
vehicleSignal = zeros(n,1) - 1;

for i=1:rows   
       
    index = int32(indices(i,1));        

    % store the current TS
    vehiclesTS(index,1) = double(timeSteps(i,1));
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));
        
    vNumber = find(ismember(vIDs,vehicle));        
    if( isempty(vNumber) )
        vIDs{end+1} = vehicle;
        [~,vNumber] = size(vIDs);
    end   

    vehiclesSpeed(index,vNumber) = double(speeds(i,1));         
    vehiclesLane(index, vNumber) = lanes(i,1);
    vehicleSignal(index, vNumber) = int32(signal(i,1));
end

disp('parsing file is done!');
clearvars C_text indices timeSteps lanes speeds signal
    
[~,VehNumbers] = size(vIDs);
[rows,~] = size(vehiclesLane);

% ---------------------------------------------------------------

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

% TODO: I should wait for the implementation of the TraCI command that
% gives the TL status ahead!


% at this point, we have the starting point of all delay components:
% index(1,i): start of the deceleration delay
% index(2,i): start of the stopping delay
% index(3,i): start of the acceleration delay
% index(4,i): end of the acceleration delay

% the slowing down can be due to many reasons other than TL.
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
         
% -----------------------------------------------------------------

% speed profile of vehicles before passing the intersection

figNum = 0;

if(false)

    figure('units','normalized','outerposition',[0 0 1 1]);
    
    counter = 1;

    for i=1:VehNumbers

        figNum = floor( (i-1)/12 ) + 1;
        hf(1) = figure(figNum);
        set(gcf,'name','Speed');

        subplot(4,3,counter);    
        handle1 = plot(vehiclesTS(:,1),vehiclesSpeed(:,i),'LineWidth', 3);

        counter = counter + 1;
        % reset the counter
        if(counter == 13)
            counter = 1;
        end
    
        % set the x-axis limit
        set( gca, 'XLim', [0 vehiclesTS(end)] );
    
        % set the y-axis limit
        set( gca, 'YLim', [-2 34] );

        % set font size
        set(gca, 'FontSize', 17);

        grid on;
    
        title( char(vIDs{i}) );
    
        % if the vehicle did not cross the intersection at all
        if( crossed(i) == 0 )        
            text(4, 13, 'Not crossed!');
        end    
    
        hold on;
    
        % mark deceleration delay
        if( index(1,i) ~= -1 )
            endPoint = index(2,i);
            if(endPoint == -1)        
                endPoint = index(3,i);
            end        
            X = vehiclesTS(index(1,i):endPoint, 1);
            Y = vehiclesSpeed(index(1,i):endPoint, i);
            plot(X, Y, 'LineWidth', 3, 'Color','red');
        end
    
        % mark stopping delay
        if( index(2,i) ~= -1 && index(3,i) ~= -1 )
            X = vehiclesTS(index(2,i):index(3,i), 1);
            Y = vehiclesSpeed(index(2,i):index(3,i), i);
            plot(X, Y, 'LineWidth', 3, 'Color','black');
        end
    
        % mark acceleration delay
        if( index(3,i) ~= -1 && index(4,i) ~= -1 )
            X = vehiclesTS(index(3,i):index(4,i), 1);
            Y = vehiclesSpeed(index(3,i):index(4,i), i);
            plot(X, Y, 'LineWidth', 3, 'Color','green');
        end
    
        hold off;    
    
%     % mark slowing down and waiting time with an arrow
%     if( index(1,i) ~= -1 && index(2,i) ~= -1 )
%         startC = [vehiclesTS(index(1,i),1), vehiclesSpeed(index(1,i),i)]; 
%         endC = [vehiclesTS(index(2,i),1), vehiclesSpeed(index(1,i),i)];
%         arrow(startC, endC, 10, 'BaseAngle', 20, 'Ends', 3);
%     end
%     
%     % mark speeding up with an arrow
%     if( indexSpeedUp(1,i) ~= -1 && indexSpeedUp(2,i) ~= -1 )
%         startC = [vehiclesTS(indexSpeedUp(1,i),1), vehiclesSpeed(indexSpeedUp(1,i),i)]; 
%         endC = [vehiclesTS(indexSpeedUp(2,i),1), vehiclesSpeed(indexSpeedUp(1,i),i)];
%         arrow(startC, endC, 10, 'BaseAngle', 20, 'Ends', 3);
%     end

    end
end

% -----------------------------------------------------------------

% we need to get TL phasing information

path = strcat(folderPath, '/0_intersectionData.txt');    
file_id = fopen(path);
formatSpec = '%d %f %s %s %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

phaseNumbers = C_text{1,1};
endTime = C_text{1,2};
queueSize = C_text{1,5};

% store start/end time of each phase in phaseDuration

[rows,~] = size(phaseNumbers);
oldIndex = -1;
oldTime = 0;

for i=1:rows 
    phaseNumber = int32(phaseNumbers(i,1));
    time = double(endTime(i,1));
    
    if(oldIndex ~= phaseNumber)
        phaseDuration(1,phaseNumber) = oldTime;
        phaseDuration(2,phaseNumber) = time;
        
        oldIndex = phaseNumber;
        oldTime = time;
    end   
end

% -----------------------------------------------------------------

% store average queue size at the end of each phase in phaseData(1,:)

oldPhaseNumber = 1;
totalQSize = 0;
numLanes = 0;

for i=1:rows 
    phaseNumber = int32(phaseNumbers(i,1));
    
    if(oldPhaseNumber ~= phaseNumber)        
        phaseData(1,phaseNumber-1) = double(totalQSize) / numLanes;
        oldPhaseNumber = phaseNumber;
        totalQSize = 0;
        numLanes = 0;
    end
    
    qSize = int32(queueSize(i,1));
    totalQSize = totalQSize + qSize;
    numLanes = numLanes + 1; 
    
    if(i == rows)
        phaseData(1,phaseNumber) = double(totalQSize) / numLanes;
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

% calculate the average delay at the end of each phase

for j=1:size(phaseDuration,2)  % j is phase number
   
   totalDelay = 0;  % total delay in this phase
   vehCount = 0;  % total vehicles in this phase
   phaseStart = phaseDuration(1,j);
   phaseEnd = phaseDuration(2,j);
   
   for i=1:VehNumbers
       if(indexTS(1,i) ~= -1 && indexTS(4,i) ~= -1)
           d1 = max(phaseStart,indexTS(1,i));
           d2 = min(phaseEnd,indexTS(4,i));
           difference = d2 - d1;
           if(difference > 0)
               totalDelay = totalDelay + difference;
               vehCount = vehCount + 1;
           end
       end
   end
   
   phaseData(2,j) = totalDelay/vehCount;
   %fprintf( 'phase %d: totalDelay=%0.2f, vehCount=%d, aveDelay=%0.2f\n', j, totalDelay, vehCount, totalDelay/vehCount);
    
end

% -----------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);

figNum = figNum + 1;
hf(1) = figure(figNum);
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


