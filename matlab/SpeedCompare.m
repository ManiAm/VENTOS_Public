clear all;
clc;   % position the cursor at the top of the screen
%clf;   %  closes the figure window

% -------------------------------------------------------------------

path = '';

%font size
Fsize = 25;

%x-axis limit
%x1 = 38.9;
%x2 = 180;

%x1 = 77;
%x2 = 150;

x1 = 39.5;
x2 = 75;

for s=1:2    
    
    if(s == 1)
        path = '../results/gui/speed-gap1.2.txt';
    elseif(s == 2)
        path = '../results/gui/speed-gap0.8.txt';
    end
    
    file_id = fopen(path);
    formatSpec = '%d %s %f %f %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 4);
    fclose(file_id);

    % ---------------------------------------------------------------
    
    indices = C_text{1,1};
    vehicles = C_text{1,2};
    timeSteps = C_text{1,3};
    speeds = C_text{1,4};

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
    end
    
    vehicleSpeedAll(:,:,s) = vehiclesSpeed;    
end

% -------------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(1);
set(gcf,'name','Speed');
% set(gcf,'name','Myfigure','numbertitle','off')

subaxis(1,2,1,'SpacingHoriz',0.06,'MA',0.02,'MB',0.1,'MR',0.02,'ML',0.06); 

handle1 = plot(vehiclesTS,vehicleSpeedAll(:,:,1),'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [x1 x2] );
   
% set the y-axis limit
set( gca, 'YLim', [0 30] );

% set font size
set(gca, 'FontSize', Fsize);

% set the axis labels
xlabel('Simulation Time (s)', 'FontSize', Fsize);
ylabel('Speed (m/s)', 'FontSize', Fsize);

grid on;

set(handle1(1), 'LineStyle', '-.');

annotation('textbox',...
    [0.85 0.86 0.08 0.07],...
    'String',{'Tg=0.8'},...
    'FitBoxToText','on',...
    'FontSize',Fsize,...
    'FontName','Arial',...
    'BackgroundColor',[0.9  0.9 0.9]);

% set the name for each line
%set(handle1(1),'Displayname', 'Manual');
%set(handle1(2),'Displayname', 'ACC');
%set(handle1(3),'Displayname', 'CACC');

% set the legend
%legend(handle1, 'Location','NorthEastOutside');

% save the figure as fig to restore it later
% saveas(gcf,'figure1.fig');



subaxis(1,2,2);

handle1 = plot(vehiclesTS,vehicleSpeedAll(:,:,2),'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [x1 x2] );
   
% set the y-axis limit
set( gca, 'YLim', [0 30] );

% set font size
set(gca, 'FontSize', Fsize);

% set the axis labels
xlabel('Simulation Time (s)', 'FontSize', Fsize);
ylabel('Speed (m/s)', 'FontSize', Fsize);

grid on;

set(handle1(1), 'LineStyle', '-.');

annotation('textbox',...
    [0.36 0.86 0.08 0.07],...
    'String',{'Tg=1.2'},...
    'FitBoxToText','on',...
    'FontSize',Fsize,...
    'FontName','Arial',...
    'BackgroundColor',[0.9  0.9 0.9]);


% set the name for each line
%set(handle1(1),'Displayname', 'Manual');
%set(handle1(2),'Displayname', 'ACC');
%set(handle1(3),'Displayname', 'CACC');

% set the legend
%legend(handle1, 'Location','NorthEastOutside');

% save the figure as fig to restore it later
% saveas(gcf,'figure1.fig');




