
% Sheet1 contains detailed information of 10 vehicles, 
% Sheet2 gives the speed profile you need (marked yellow).

clear all;
close all;
clc;

fileName = '10_veh_Trajectory2.xlsx';

vehID = xlsread(fileName, 1, 'A:A');
time = xlsread(fileName, 1, 'B:B');
locationY = xlsread(fileName, 1, 'D:D');
speed = xlsread(fileName, 1, 'F:F');
spacing = xlsread(fileName, 1, 'K:K');

% find the rows that have positive time
indexRemove = (time > 0);

% remove those rows
vehID(indexRemove) = [];
time(indexRemove) = [];
locationY(indexRemove) = [];
speed(indexRemove) = [];

% change units
time = (time + 3572300) / 1000;

% stores vehicle IDs
vIDs = unique(vehID);
VehNumbers = size(vIDs, 1);

minTime = min(time);
maxTime = max(time);
entriesCount = size(minTime:0.1:maxTime, 2);

vehiclesTS = (minTime:0.1:maxTime)';
vehiclesSpeed = zeros(entriesCount,VehNumbers) - 1;
vehiclesLocation = zeros(entriesCount,VehNumbers) - 1;
vehiclesSpacing = zeros(entriesCount,VehNumbers) - 1;

[rows,~] = size(vehID);

for i=1:rows
    
    % get the current vehicle ID
    vehicle = int32(vehID(i,1));    
    vNumber = find(ismember(vIDs,vehicle));
    
    % get the current time
    currentTime = double(time(i,1));    
    index = (currentTime * 10) + 1;
    
    % get the current speed
    currentSpeed = double(speed(i,1));
    
    vehiclesSpeed(index,vNumber) = currentSpeed;  
    
    % get the current location
    currentLocation = double(locationY(i,1));
    vehiclesLocation(index,vNumber) = currentLocation;
    
    % get the current spacing
    currentSpace = double(spacing(i,1));
    vehiclesSpacing(index,vNumber) = currentSpace;
end

figure(1);
%subplot(3,1,1);

for i=1:1
    X = vehiclesTS;
    Y = vehiclesSpeed(:,i);
    
    t = (Y < 0);
    
    X(t) = [];
    Y(t) = [];
    
    name = sprintf('%d', vIDs(i));
    handle(i) = plot(X, Y, 'Displayname', name);
    
    grid on;
    
    title('Speed Profile diagram', 'FontSize', 17);
    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Speed ()', 'FontSize', 17);
    
    hold all;
end

legend(handle, 'Location','NorthEastOutside'); 

figure(2);
%subplot(3,1,2);

for i=1:VehNumbers
    X = vehiclesTS;
    Y = vehiclesLocation(:,i);
    
    t = (Y < 0);
    
    X(t) = [];
    Y(t) = [];
    
    name = sprintf('%d', vIDs(i));
    handle(i) = plot(X, Y, 'Displayname', name);
    
    grid on;
    
    title('Time-Space diagram', 'FontSize', 17);
    xlabel('Time (s)', 'FontSize', 17);
    ylabel('LocationY (ft)', 'FontSize', 17);
    
    hold on;
end

legend(handle, 'Location','NorthEastOutside');

figure(3);
%subplot(3,1,3);

for i=1:VehNumbers
    X = vehiclesSpacing(:,i);
    Y = vehiclesSpeed(:,i);
    
    t = (Y < 0);
    
    X(t) = [];
    Y(t) = [];
    
    name = sprintf('%d', vIDs(i));
    handle(i) = plot(X, Y, 'Displayname', name);
    
    grid on;
    
    title('Spacing-Speed diagram', 'FontSize', 17);
    xlabel('Spacing ()', 'FontSize', 17);
    ylabel('Speed ()', 'FontSize', 17);
    
    hold on;
end

legend(handle, 'Location','NorthEastOutside');


