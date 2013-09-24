
clear all;
clc;

% -------------------------------------------------------------------

path = '';
fig = 0;
hf = zeros(1,4);

for s=1:1    
    
    if(s == 1)
        path = '../results/speed-gap.txt';
        figureName = 'Optimal Speed';
    elseif(s == 2)
        path = '../2.Result/speed-gap-21MSCFModel_KraussFixed.txt';
        figureName = 'Krauss Fixed';
    elseif(s == 3)
        path = '../2.Result/speed-gap-21MSCFModel_KraussOrig1.txt';
        figureName = 'KraussOrig1';
    end
    
    file_id = fopen(path);
    formatSpec = '%s %f %f %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
    fclose(file_id);

    % ---------------------------------------------------------------
    
    vehicles = C_text{1,1};
    timeSteps = C_text{1,2};
    speeds = C_text{1,3};
    accel = C_text{1,4};
    timeGaps = C_text{1,7};

    % ---------------------------------------------------------------

    [rows,~] = size(vehicles);

    % preallocating and initialization with -1
    vehiclesTS = zeros(rows/4,1) - 1;
    vehiclesSpeed = zeros(rows/4,3) - 1;
    vehiclesAccel = zeros(rows/4,3) - 1;
    vehiclesTimeGap = zeros(rows/4,3) - 1;

    timeStep_old = -1.0;
    index = 0;

    for i=1:rows   
        
        timeStep = double(timeSteps(i,1));    
        
        if(timeStep ~= timeStep_old)
            index = index + 1;
            vehiclesTS(index,1) = timeStep;
            timeStep_old = timeStep;
        end
    
        vehicle = char(vehicles(i,1));
    
        if(strcmp(vehicle,'Manual') == 1)  
            vehiclesSpeed(index,1) = speeds(i,1); 
            vehiclesAccel(index,1) = accel(i,1);
            vehiclesTimeGap(index,1) = timeGaps(i,1);
            
        elseif(strcmp(vehicle,'ACC') == 1)
            vehiclesSpeed(index,2) = speeds(i,1); 
            vehiclesAccel(index,2) = accel(i,1);
            vehiclesTimeGap(index,2) = timeGaps(i,1);
            
        elseif(strcmp(vehicle,'CACC') == 1)
            vehiclesSpeed(index,3) = speeds(i,1); 
            vehiclesAccel(index,3) = accel(i,1);
            vehiclesTimeGap(index,3) = timeGaps(i,1);
        end    
    end

    % ---------------------------------------------------------------

    figure('units','normalized','outerposition',[0 0 1 1]);
    fig = fig + 1;
    hf(fig) = figure(fig);
    set(gcf,'name',figureName);
    % set(gcf,'name','Myfigure','numbertitle','off')

    handle1 = plot(vehiclesTS,vehiclesSpeed,'LineWidth', 3);

    % set the x-axis limit
    set( gca, 'XLim', [0 110] );
    
    % set the y-axis limit
    set( gca, 'YLim', [0 30] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('StepTime (s)', 'FontSize', 19);
    ylabel('Speed (m/s)', 'FontSize', 19);

    grid on;

    % set the name for each line
    set(handle1(1),'Displayname', 'Manual');
    set(handle1(2),'Displayname', 'ACC');
    set(handle1(3),'Displayname', 'CACC');

    % set the legend
    legend(handle1, 'Location','NorthEastOutside');

    % save the figure as fig to restore it later
    % saveas(gcf,'figure1.fig');

    % save the figure as a png file
    set(gcf, 'PaperPositionMode', 'auto'); 
    figName = sprintf('figure%d',fig);
    print('-dpng', '-r300', figName);

    % ----------------------------------------------------------------

    figure('units','normalized','outerposition',[0 0 1 1]);
    fig = fig + 1;
    hf(fig) = figure(fig);
    set(gcf,'name',figureName);

    handle2 = plot(vehiclesTS,vehiclesTimeGap,'LineWidth', 3);

    % set the x-axis limit
    set( gca, 'XLim', [0 110] );
    
    % set the y-axis limit
    set( gca, 'YLim', [0 120] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('StepTime (s)', 'FontSize', 19);
    ylabel('TimeGap (m)', 'FontSize', 19);

    grid on;

    % set the name for each line
    set(handle2(1),'Displayname', 'Manual');
    set(handle2(2),'Displayname', 'ACC');
    set(handle2(3),'Displayname', 'CACC');

    % set the legend
    legend(handle2, 'Location','NorthEastOutside');

    % save the figure as a png file
    set(gcf, 'PaperPositionMode', 'auto');
    figName = sprintf('figure%d',fig);
    print('-dpng', '-r300', figName);
    
    % ---------------------------------------------------------------
        
    figure('units','normalized','outerposition',[0 0 1 1]);
    fig = fig + 1;
    hf(fig) = figure(fig);
    set(gcf,'name',figureName);

    handle3 = plot(vehiclesTS,vehiclesAccel,'LineWidth', 3);

    % set the x-axis limit
    set( gca, 'XLim', [0 110] );
    
    % set the y-axis limit
    set( gca, 'YLim', [0 30] );

    % set font size
    set(gca, 'FontSize', 19);

    % set the axis labels
    xlabel('StepTime (s)', 'FontSize', 19);
    ylabel('Acceleration (m/s^2)', 'FontSize', 19);

    grid on;

    % set the name for each line
    set(handle3(1),'Displayname', 'Manual');
    set(handle3(2),'Displayname', 'ACC');
    set(handle3(3),'Displayname', 'CACC');

    % set the legend
    legend(handle3, 'Location','NorthEastOutside');

    % save the figure as fig to restore it later
    % saveas(gcf,'figure1.fig');

    % save the figure as a png file
    set(gcf, 'PaperPositionMode', 'auto'); 
    figName = sprintf('figure%d',fig);
    print('-dpng', '-r300', figName);    
        
end

% -------------------------------------------------------------------

figs2subplots([hf(1) hf(7)],[2 1],{1,2});


figs2subplots([hf(4) hf(7)],[2 1],{1,2});


