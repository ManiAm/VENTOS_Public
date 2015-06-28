
clear all;
close all;
clc;

path = '../results/gui/vehicleData.txt';

file_id = fopen(path);
formatSpec = '%d %f %s %s %s %f %f %f %s %f %f %f %s';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

% ---------------------------------------------------------------
    
indices = C_text{1,1};
timeSteps = C_text{1,2};
vehicles = C_text{1,3};  
pos = C_text{1,6};
speeds = C_text{1,7};
accel = C_text{1,8};
gaps = C_text{1,11};
timeGaps = C_text{1,12};

% ---------------------------------------------------------------
    
% stores vehicle IDs
vIDs = {};

[rows,~] = size(vehicles);
n = indices(end,1);

% preallocating and initialization with -1
vehiclesTS = zeros(n,1) - 1;
vehiclePos = zeros(n,1) - 1;
vehiclesSpeed = zeros(n,1) - 1;
vehiclesAccel = zeros(n,1) - 1;
vehiclesGap = zeros(n,1) - 1;
vehiclesTimeGap = zeros(n,1) - 1;

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
    vehiclePos(index,vNumber) = pos(i,1);
    vehiclesAccel(index,vNumber) = accel(i,1);
    vehiclesGap(index,vNumber) = gaps(i,1);
    vehiclesTimeGap(index,vNumber) = timeGaps(i,1);  
    
end
    
[~, VehNumbers] = size(vIDs);

    % ---------------------------------------------------------------
    
    % speed profile of vehicles    

    figure('name','Speed','units','normalized','outerposition',[0 0 1 1]);

    handle1 = plot(vehiclesTS,vehiclesSpeed,'LineWidth', 3);

    % set the x-axis limit
    set( gca, 'XLim', [0 vehiclesTS(end)] );
    
    % set the y-axis limit
    set( gca, 'YLim', [0 35] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('Simulation Time (s)', 'FontSize', 19);
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
       
    % time-space diagram of vehicles 

    figure('name','Gap','units','normalized','outerposition',[0 0 1 1]);

    handle2 = plot(vehiclesTS,vehiclePos,'LineWidth', 3);

    % set the x-axis limit
    set( gca, 'XLim', [0 vehiclesTS(end)] );
    
    % set the y-axis limit
    %set( gca, 'YLim', [0 120] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('Simulation Time (s)', 'FontSize', 19);
    ylabel('Space (m)', 'FontSize', 19);

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
       
    % Spacing-Speed diagram

    figure('name','Gap','units','normalized','outerposition',[0 0 1 1]);

    handle2 = plot(vehiclesGap,vehiclesSpeed,'LineWidth', 3);

    % set the x-axis limit
    %set( gca, 'XLim', [0 vehiclesGap(end,1)] );
    
    % set the y-axis limit
    %set( gca, 'YLim', [0 120] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('Gap (m)', 'FontSize', 19);
    ylabel('Speed (m/s)', 'FontSize', 19);

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
   
