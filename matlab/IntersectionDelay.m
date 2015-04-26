
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   %  closes the figure window

% ---------------------------------------------------------------

path = '../results/gui/TLData.txt';    
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
% that the vehicle is on before entering to the intersection.

% mark the end of each column
for i=1:VehNumbers 
    for j=rows:-1:1
        currentLane = char(vehiclesLane{j,i});
        
        if( strcmp(currentLane,'') == 1 )
            continue;
        else
            vehiclesLane{j+1,i} = ':end';
            break;
        end        
    end    
end

for i=1:VehNumbers    
    stoppingLane = '';
    startingIndex = -1;
    endingIndex = -1;
    
    for j=1:rows+1            
        currentLane = char(vehiclesLane{j,i});
        if( isempty(strfind(currentLane, ':end')) )  % if not in the center of intersection
            if( strcmp(stoppingLane,currentLane) ~= 1 )
                stoppingLane = currentLane;
                startingIndex = j;        
            end
        else
            endingIndex = j-1;
            indexDB(1,i) = startingIndex;
            indexDB(2,i) = endingIndex;
            indexDB(3,i) = endingIndex - startingIndex + 1;
            break;
        end   
    end
end

% ---------------------------------------------------------------

% extracting the vehicle speed between indexDB(1,i) and indexDB(2,i) 

waitingSpeeds = zeros( size(vehiclesTS,1), VehNumbers ) - 1;

for i=1:VehNumbers    
    for j=indexDB(1,i):indexDB(2,i)    
       % if( vehicleSignal(j,i) == 1 )
            waitingSpeeds(j,i) = vehiclesSpeed(j,i)';
      %  end
    end
end

% ---------------------------------------------------------------

% speed profile of vehicles before passing the intersection   

figure('units','normalized','outerposition',[0 0 1 1]);

counter = 1;

for i=1:VehNumbers

    figNum = floor( (i-1)/12 ) + 1;
    hf(1) = figure(figNum);
    set(gcf,'name','Speed');

    subplot(4,3,counter);    
    handle1 = plot(vehiclesTS(:,1),waitingSpeeds(:,i),'LineWidth', 3);

    counter = counter + 1;
    % reset the counter
    if(counter == 13)
        counter = 1;
    end
    
    % set the x-axis limit
    set( gca, 'XLim', [0 95] );
    
    % set the y-axis limit
    set( gca, 'YLim', [-2 15] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('Simulation Time (s)', 'FontSize', 19);
    ylabel('Speed (m/s)', 'FontSize', 19);

    grid on;
    
    name = lower( char(vIDs{i}) );
    title(name);

end

