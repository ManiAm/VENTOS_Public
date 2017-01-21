clear all;
clc;   % position the cursor at the top of the screen
%clf;   %  closes the figure window

NoRun = 7;

% -------------------------------------------------------------------

path = '';

for s=1:NoRun
    path = sprintf('%s_%d.txt','../results/diffPLR,Tg=0.71/speed-gap', s-1);    
    file_id = fopen(path);
    formatSpec = '%d %s %f %f %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
    fclose(file_id);

    % ---------------------------------------------------------------
    
    indices = C_text{1,1};
    vehicles = C_text{1,2};
    timeSteps = C_text{1,3};
    speeds = C_text{1,4};
    accel = C_text{1,5};
    timeGaps = C_text{1,8};

    % ---------------------------------------------------------------
    
    % stores vehicle IDs
    vIDs = {};

    [rows,~] = size(vehicles);
    n = indices(end,1);

    % preallocating and initialization with -1
    vehiclesTS = zeros(n,1) - 1;
    vehiclesSpeed = zeros(n,1) - 1;
    vehiclesAccel = zeros(n,1) - 1;
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
        vehiclesAccel(index,vNumber) = accel(i,1);
        vehiclesTimeGap(index,vNumber) = timeGaps(i,1);  
    end
    
    % we only need the last vehicle datas
    lastV(:,s) = vehiclesSpeed(:,end);
    lastAccel(:,s) = vehiclesAccel(:,end);
    
end

% add trajectory data
lastV(:,end+1) = vehiclesSpeed(:,1);
lastAccel(:,end+1) = vehiclesAccel(:,1);

% remove some data
arg_to_remove = [2,4,7];
lastV(:,arg_to_remove) = [];
lastAccel(:,arg_to_remove) = [];


figure('units','normalized','outerposition',[0 0 1 1]);
figure(1);
set(gcf,'name','Packet Loss Impact on Stability');
% set(gcf,'name','Myfigure','numbertitle','off')

handle = plot(vehiclesTS,lastV,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [0 400] );
    
% set the y-axis limit
%set( gca, 'YLim', [0 30] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Simulation Time (s)', 'FontSize', 19);
ylabel('Speed (m/s)', 'FontSize', 19);

grid on;

set(handle(5), 'LineStyle', '-.');

% set the name for each line
set(handle(1),'Displayname', 'PLR = 0%');
set(handle(2),'Displayname', 'PLR = 40%');
set(handle(3),'Displayname', 'PLR = 60%');
set(handle(4),'Displayname', 'PLR = 80%');
set(handle(5),'Displayname', 'Veh0 Speed Profile');

% set the legend
legend(handle, 'Location','NorthEastOutside');


% ----------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(2);
set(gcf,'name','Packet Loss Impact on Stability');
% set(gcf,'name','Myfigure','numbertitle','off')

handle = plot(vehiclesTS,lastAccel,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [0 400] );
    
% set the y-axis limit
%set( gca, 'YLim', [0 30] );

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Simulation Time (s)', 'FontSize', 19);
ylabel('Accel (m/s^2)', 'FontSize', 19);

grid on;

set(handle(5), 'LineStyle', '-.');

% set the name for each line
set(handle(1),'Displayname', 'PLR = 0');
set(handle(2),'Displayname', 'PLR = 40');
set(handle(3),'Displayname', 'PLR = 60');
set(handle(4),'Displayname', 'PLR = 80');
set(handle(5),'Displayname', 'Trajectory');

% set the legend
legend(handle, 'Location','NorthEastOutside');



  

   