clear all;
clc;   % position the cursor at the top of the screen
%clf;   % closes the figure window

% -------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(1);
subplot(2,1,1);

Z(:, 1) = [12188 10251 10454 10492 10407 10157 10082 10121 9935 6912];
Z(:, 2) = [0 1937 1707 1611 1612 1746 1681 1468 1458 875];
handle = bar(Z, 'stacked');

% range of y axis
range = ylim;

grid on;

xlabel('Vehicle', 'FontSize', 19);
ylabel('Number of beacons', 'FontSize', 19);

% set the Displayname
set(handle(1),'Displayname', 'Other');
set(handle(2),'Displayname', 'Preceeding'); 

% set the legend
legend(handle, 'Location','NorthEastOutside');

% --------------------------------------------------------------

subplot(2,1,2);

Z(:, 1) = [0 0 0 0 0 0 0 0 0 3016];
Z(:, 2) = [0 0 0 0 0 0 0 0 0 366];
handle = bar(Z, 'stacked');

ylim([1 range(2)])

grid on;

xlabel('Vehicle', 'FontSize', 19);
ylabel('Number of dropped beacons', 'FontSize', 19);

% set the Displayname
set(handle(1),'Displayname', 'Other');
set(handle(2),'Displayname', 'Preceeding'); 

% set the legend
legend(handle, 'Location','NorthEastOutside');

% ---------------------------------------------------------------

file_id = fopen('../results/06.beacon_interval_droped_p.txt');
formatSpec = '%s %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

timeSteps2 = C_text{1,2};
beacons2 = C_text{1,3};

max(beacons2)

figure('units','normalized','outerposition',[0 0 1 1]);
figure(2);
subplot(2,1,1);
bar(timeSteps2, beacons2);

grid on;

xlabel('Simulation Step', 'FontSize', 19);
ylabel('Dropped beacons (proceeding)', 'FontSize', 19);

 % set the x-axis limit
 %set( gca, 'XLim', [25.7 29] );
 set( gca, 'XLim', [75 90] );
 
 set(gca,'ytick',0:20);
 
 set(gca,'YLim', [0 7]);
 
% ---------------------------------------------------------------

file_id = fopen('../results/05.beacon_interval_droped_o.txt');
formatSpec = '%s %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

timeSteps1 = C_text{1,2};
beacons1 = C_text{1,3};

max(beacons1)

subplot(2,1,2);

bar(timeSteps1, beacons1);

grid on;

xlabel('Simulation Step', 'FontSize', 19);
ylabel('Dropped beacons (others)', 'FontSize', 19);

 % set the x-axis limit
 set(gca, 'XLim', [75 90]);
 
 set(gca,'ytick',0:20);
 
 set(gca,'YLim', [0 7]);
 
% ---------------------------------------------------------------


%bar(timeSteps1,[beacons1 beacons2],'stacked');




