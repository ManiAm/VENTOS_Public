clear all;
close all;
clc;   % position the cursor at the top of the screen
%clf;   %  closes the figure window

% -------------------------------------------------------------------

A = importdata('../results/AID_tableCount.txt');

Z(:,1) = A(1,:);
Z(:,2) = A(2,:);  
Z(:,3) = A(3,:);

handle = plot(Z,'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [0 1000] );
    
% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Road Segment', 'FontSize', 19);
ylabel('Vehicles per Segment', 'FontSize', 19);

grid on;

    % set the name for each line
set(handle(1),'Displayname', 'lane 0');
set(handle(2),'Displayname', 'lane 1');  
set(handle(3),'Displayname', 'lane 2');  
    
% set the legend
legend(handle, 'Location','NorthEastOutside');  