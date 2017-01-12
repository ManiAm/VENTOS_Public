
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   %  closes the figure window

% -------------------------------------------------------------------

% X limit of each figure
Xmin = 0;
Xmax = 600;

disp('reading and parsing the vehData.txt file ...');

path = '../results/000_vehicleData.txt';    
file_id = fopen(path);
formatSpec = '%d %f %s %f %f %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);
    
indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3};   
pos = C_text{1,4};
speeds = C_text{1,5};
accel = C_text{1,6};
spaceGaps = C_text{1,7};

% ---------------------------------------------------------------

% stores vehicle IDs
vIDs = unique(vehicles);
vIDs = sort_nat(vIDs);
VehNumbers = size(vIDs,1);

n = indices(end,1);

% preallocating and initialization with -1
vehiclesTS = zeros(n,1) - 1;
vehiclesSpeed = zeros(n,VehNumbers) - 1;
vehiclesAccel = zeros(n,VehNumbers) - 1;
vehiclesSpaceGap = zeros(n,VehNumbers) - 1;

rows = size(vehicles,1);

for i=1:rows   
        
    index = int32(indices(i,1));        

    % store the current TS
    vehiclesTS(index,1) = double(timeSteps(i,1));
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));
    vNumber = find(ismember(vIDs,vehicle)); 

    vehiclesSpeed(index,vNumber) = speeds(i,1); 
    vehiclesAccel(index,vNumber) = accel(i,1);
    vehiclesSpaceGap(index,vNumber) = spaceGaps(i,1); 
end

% ---------------------------------------------------------------
    
% plot speed profile of vehicles    

disp('plotting the speed profile ...');

figure('name', 'Speed Profile', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle1 = plot(vehiclesTS,vehiclesSpeed,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [Xmin Xmax] );
    
% set the y-axis limit
set( gca, 'YLim', [-1 32] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Time (s)', 'FontSize', 19);
ylabel('Speed (m/s)', 'FontSize', 19);

grid on;
  
% speed profile of first vehicle (dashed line)
set(handle1(1), 'LineStyle', '-.');
    
% set the name for each line
for i=1:VehNumbers
    name = sprintf('veh %2d', i);
    set(handle1(i),'Displayname', name);   
end
    
% set the legend
legend(handle1, 'Location','NorthEastOutside');

% --------------------------------------------------------------

% plot space gaps of vehicles 

disp('plotting the space gap ..');

figure('name', 'Space Gap', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle2 = plot(vehiclesTS,vehiclesSpaceGap,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [Xmin Xmax] );
    
% set the y-axis limit
%set( gca, 'YLim', [-1 120] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Time (s)', 'FontSize', 19);
ylabel('Gap (m)', 'FontSize', 19);

grid on;   
    
% speed profile of first vehicle (dashed line)
set(handle2(1), 'LineStyle', '-.');
      
% set the name for each line
for i=1:VehNumbers
    name = sprintf('veh %2d', i);
    set(handle2(i),'Displayname', name);   
end    
    
% set the legend
legend(handle2, 'Location','NorthEastOutside');
    
% ---------------------------------------------------------------
    
% plot acceleration of vehicles 

disp('plotting acceleration ...');
        
figure('name', 'Acceleration', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle3 = plot(vehiclesTS,vehiclesAccel,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [Xmin Xmax] );
    
% set the y-axis limit
% set( gca, 'YLim', [-1 30] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Time (s)', 'FontSize', 19);
ylabel('Acceleration (m/s^2)', 'FontSize', 19);

grid on;
    
% set the name for each line
for i=1:VehNumbers
    name = sprintf('veh %2d', i);
    set(handle3(i),'Displayname', name);   
end    
    
% set the legend
legend(handle3, 'Location','NorthEastOutside');   
    
% ---------------------------------------------------------------    
    
% plot the distance to the first vehicle of the stream (CACC1)

disp('plotting distance to the first vehicle ...');

distanceToFirstVehicle = vehiclesSpaceGap;
rows = size(distanceToFirstVehicle,1);

for i=1:rows
    for j=3:VehNumbers
        if(distanceToFirstVehicle(i,j) ~= -1 && distanceToFirstVehicle(i,j-1) ~= -1)
            distanceToFirstVehicle(i,j) =  distanceToFirstVehicle(i,j) + distanceToFirstVehicle(i,j-1);      
        end
    end    
end

figure('name', 'Distance to the First Vehicle', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle4 = plot(vehiclesTS,distanceToFirstVehicle,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [Xmin Xmax] );
    
% set the y-axis limit
% set( gca, 'YLim', [-1 120] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Time (s)', 'FontSize', 19);
ylabel('Distance to First Vehicle (m)', 'FontSize', 19);

grid on;   
    
% speed profile of first vehicle (dashed line)
set(handle4(1), 'LineStyle', '-.');
    
% set the name for each line
for i=1:VehNumbers
    name = sprintf('veh %2d', i);
    set(handle4(i),'Displayname', name);   
end    
    
% set the legend
legend(handle4, 'Location','NorthEastOutside');

% -------------------------------------------------------------------

disp('done!');



%figs2subplots([hf(1) hf(7)],[2 1],{1,2});


%figs2subplots([hf(4) hf(7)],[2 1],{1,2});


