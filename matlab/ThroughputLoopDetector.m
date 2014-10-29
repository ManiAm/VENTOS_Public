
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------- 

for run=0:3
    filePath = sprintf('../results/cmd/plnSize_on_throu/%d_loopDetector.txt', run);
    file_id = fopen(filePath);
    formatSpec = '%s %s %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
    fclose(file_id);

    % --------------------------
    
    vehicles = C_text{1,2};    
    vehEntry = C_text{1,3};

    % --------------------------
    
    vehCount = 0;
    timeStart = double(vehEntry(1,1));
    vehCount = vehCount + 1;

    [rows,~] = size(vehicles);

    for i=2:rows    
        vehCount = vehCount + 1;
        time = double(vehEntry(i,1));
        duration = time - timeStart;
        
        q(i-1, 1) = vehCount; 
        q(i-1, run+2) = (3600 * vehCount) / duration;        
    end
end

disp(q);

% ----------------------------------------------------------

% extract a specific row
data = q(119, :);

% remove the first column
data(:,[1]) = [];

% ----------------------------------------------------------

figure(1);
subaxis(1,2,'SpacingHoriz',0.07,'MA',0.02,'MB',0.1,'MR',0.02,'ML',0.06);

subaxis(1,2,1);
h = bar(data, 0.6);

set(gca,'XTickLabel',{'5','10', '15', '20'}', 'FontSize', 19);

xlabel('Platoon Size', 'FontSize', 20);
ylabel('Throughput (veh/h)', 'FontSize', 20);

% 2400 to 3000
ylim([2400 3000]);

set(h(1), 'FaceColor', [0.5 0.5 0.5]);

grid on;

% ----------------------------------------------------------
% ----------------------------------------------------------

for run=0:2
    filePath = sprintf('../results/cmd/TP_on_throu/%d_loopDetector.txt', run);
    file_id = fopen(filePath);
    formatSpec = '%s %s %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
    fclose(file_id);

    % --------------------------
    
    vehicles = C_text{1,2};    
    vehEntry = C_text{1,3};

    % --------------------------
    
    vehCount = 0;
    timeStart = double(vehEntry(1,1));
    vehCount = vehCount + 1;

    [rows,~] = size(vehicles);

    for i=2:rows    
        vehCount = vehCount + 1;
        time = double(vehEntry(i,1));
        duration = time - timeStart;
        
        r(i-1, 1) = vehCount; 
        r(i-1, run+2) = (3600 * vehCount) / duration;        
    end
end

disp(r);

% ------------------------------------------------

% extract a specific row
data = r(104, :);

% remove the first column
data(:,[1]) = [];

% ----------------------------------------------------------

subaxis(1,2,2);
h = bar(data, 0.6);

set(gca,'XTickLabel',{'2','3.5', '5'}', 'FontSize', 19);

xlabel('Inter-platoon spacing (T_P)', 'FontSize', 20);
ylabel('Throughput (veh/h)', 'FontSize', 20);

% 2400 to 3000
% ylim([2400 3000]);

set(h(1), 'FaceColor', [0.5 0.5 0.5]);

grid on;

% -----------------------------------------------------------

disp('done!');

