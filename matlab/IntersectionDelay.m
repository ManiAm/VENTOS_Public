
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

path = '../results/gui/TLStatePerVehicle_fixV.txt';    
file_id = fopen(path);
formatSpec = '%d %f %s %s %f %f %s %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

% ---------------------------------------------------------------
    
indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3}; 
lanes = C_text{1,4};
speeds = C_text{1,6};
signal = C_text{1,8};

% ---------------------------------------------------------------
    
% stores vehicle IDs
vIDs = {};

[rows,~] = size(vehicles);
n = indices(end,1);

% preallocating and initialization with -1
vehiclesTS = zeros(n,1) - 1;
vehiclesSpeed = zeros(n,1) - 1;

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

        vehiclesSpeed(index,vNumber) = speeds(i,1);         
        vehiclesLane(index, vNumber) = lanes(i,1);
        vehicleSignal(index, vNumber) = signal(i,1);
end
    
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

% extracting the vehicle speed between indexDB(1,i) and indexDB(2,i) 
% signal should also be yellow or red

waitingSpeeds = zeros( size(vehiclesTS,1), VehNumbers ) - 1; % initialize to -1
yellowOrRed = zeros(1,VehNumbers);  % did the traffic light became yellow or red at leat once

for i=1:VehNumbers  
    if( indexLane(1,i) ~= -1 && indexLane(2,i) ~= -1 )
        for j=indexLane(1,i):indexLane(2,i)    
            if( vehicleSignal(j,i) == 1 )  % if the signal is yellow or red
                waitingSpeeds(j,i) = vehiclesSpeed(j,i)';
                yellowOrRed(i) = 1;
            end
        end
    end
end

% ---------------------------------------------------------------

% we are only interested in the descending sections

delay(1,:) = waitingSpeeds(1,:);
rows = size(waitingSpeeds, 1) - 1;

for i=1:VehNumbers
    diffSpeeds(:,i) = diff( waitingSpeeds(:,i) );
    
    for j=1:rows
        if( diffSpeeds(j,i) < 0)
            delay(j+1,i) = waitingSpeeds(j,i);
        elseif( diffSpeeds(j,i) == 0 && waitingSpeeds(j,i) > 0 )
            delay(j+1,i) = -1;
        elseif( diffSpeeds(j,i) == 0 && waitingSpeeds(j,i) == 0 )
            delay(j+1,i) = waitingSpeeds(j,i);
        elseif( diffSpeeds(j,i) > 0)
            delay(j+1,i) = -1;
        else
            delay(j+1,i) = -1;
        end
    end
end

% -----------------------------------------------------------------

% looking for the peak with a smooth descending after that

rows = size(delay, 1);

% setting the starting point
for i=1:VehNumbers
    index(1,i) = -1;
    smoothDelay(:,i) = diff( delay(:,i) );
    
    for j=1:rows
        if( delay(j,i) > 0 )
            if( all(smoothDelay(j+1:j+10,i) ~= 0) && all(smoothDelay(j+1:j+10,i) < -0.05) )
                index(1,i) = j;   % starting point
                break;
            end
        end
    end
end

% setting the ending point
for i=1:VehNumbers
    index(2,i) = -1;
    
    % if we could not find any starting point then skip
    if(index(1,i) == -1)
        continue;
    else
        for j=rows:-1:1
            if( delay(j,i) >= 0 )
                index(2,i) = j;
                break;
            end            
        end        
    end    
end

% -----------------------------------------------------------------

% now we should look for the point that the vehicle goes back to its old speed

for i=1:VehNumbers
    indexSpeedUp(1,i) = -1;
    indexSpeedUp(2,i) = -1;
    
    if( index(1,i) ~= -1 && index(2,i) ~= -1 )
        indexSpeedUp(1,i) = index(2,i);  % starting point is where the waiting time ends
        oldSpeed = vehiclesSpeed( index(1,i), i );
        
        % now we search to see when we reach to the old speed
        % note that we might never find this point! thus indexSpeedUp(2,i) might be -1
        for j=indexLane(2,i):rows
            if( vehiclesSpeed(j,i) >= oldSpeed )
                indexSpeedUp(2,i) = j; 
                break;
            end           
        end        
    end    
end

% -----------------------------------------------------------------

% speed profile of vehicles before passing the intersection   

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
    set( gca, 'YLim', [-2 15] );

    % set font size
    set(gca, 'FontSize', 17);

    grid on;
    
    title( char(vIDs{i}) );
    
    % if the vehicle did not cross the intersection at all
    if( crossed(i) == 0 )        
        text(4, 13, 'Not crossed!');
    % if the vehicle gets green all the time (delay=0)
    elseif( yellowOrRed(i) == 0 )
        text(4, 13, 'Gets green all the time!');
    end    
    
    % slowing down and waiting time
    if( index(1,i) ~= -1 && index(2,i) ~= -1 )
        startC = [vehiclesTS(index(1,i),1), vehiclesSpeed(index(1,i),i)]; 
        endC = [vehiclesTS(index(2,i),1), vehiclesSpeed(index(1,i),i)];
        arrow(startC, endC, 10, 'BaseAngle', 20, 'Ends', 3);
    end
    
    % speeding up
    if( indexSpeedUp(1,i) ~= -1 && indexSpeedUp(2,i) ~= -1 )
        startC = [vehiclesTS(indexSpeedUp(1,i),1), vehiclesSpeed(indexSpeedUp(1,i),i)]; 
        endC = [vehiclesTS(indexSpeedUp(2,i),1), vehiclesSpeed(indexSpeedUp(1,i),i)];
        arrow(startC, endC, 10, 'BaseAngle', 20, 'Ends', 3);
    end
end

% -----------------------------------------------------------------

% at this point we have two indices
%  1) index: contains the starting index of 'slowing down' and ending index of 'waiting time'
%  2) indexSpeedUp: contains the starting/ending index of 'speed-up'

% now we calculate the delay

delayTotal = zeros(2,VehNumbers)-1;

for i=1:VehNumbers 
    
    % slowing down and waiting time
    if( index(1,i) ~= -1 && index(2,i) ~= -1 )
        delayTotal(1,i) = double(vehiclesTS(index(2,i))) - double(vehiclesTS(index(1,i)));
    else
        delayTotal(1,i) = -1;
    end
    
    % speeding up
    if( indexSpeedUp(1,i) ~= -1 && indexSpeedUp(2,i) ~= -1 )
        delayTotal(2,i) = double(vehiclesTS(indexSpeedUp(2,i))) - double(vehiclesTS(indexSpeedUp(1,i)));
    else
        delayTotal(2,i) = -1;
    end
    
    if( delayTotal(1,i) ~= -1 )
        if( delayTotal(2,i) ~= -1 )
            delayTotal(3,i) = delayTotal(1,i) + delayTotal(2,i);
        else
            delayTotal(3,i) = delayTotal(1,i);
        end        
    else
        delayTotal(3,i) = -1;
    end
    
end

% -----------------------------------------------------------------

% print delay for each component

delayT = [];
delayVeh = [];
delayBike = [];
delayPed = [];

for i=1:VehNumbers 
    
    if( delayTotal(3,i) ~= -1 )  
        delayT(end+1) = delayTotal(3,i);
        name = lower( char(vIDs{i}) );
        
        if( ~isempty(strfind(name, 'veh')) )
            delayVeh(end+1) = delayTotal(3,i);
        elseif( ~isempty(strfind(name, 'bike')) )
            delayBike(end+1) = delayTotal(3,i);
        elseif( ~isempty(strfind(name, 'ped')) )
            delayPed(end+1) = delayTotal(3,i);
        end
    end
    
end

fprintf( 'Average total delay: %0.2f s\n', mean(delayT) );
fprintf( 'Average vehicle delay: %0.2f s\n', mean(delayVeh) );
fprintf( 'Average bike delay: %0.2f s\n', mean(delayBike) );
fprintf( 'Average pedestrian delay: %0.2f s\n', mean(delayPed) );


