
clear all;
close all;
clc;

% ---------------------------------------------------------------

basePATH = '../results/ML/cmd';
TLqueuingData = dir([basePATH, '/*_classificationResults.txt']);
runTotal = length(TLqueuingData);  % total number of simulation runs

classifyResult = zeros(1,runTotal);
for runNumber = 1:runTotal

path = sprintf('%s/%s', basePATH, TLqueuingData(runNumber).name);
file_id = fopen(path);
C_text = textscan(file_id, '%s %d %d %[^\n]');
fclose(file_id);

name = C_text{1,1};
total_prediction = C_text{1,2};
correct_prediction = C_text{1,3};  
predictions = C_text{1,4};

% ---------------------------------------------------------------

% stores vehicle IDs
vIDs = unique(name);
vIDs = sort_nat(vIDs);
VehNumbers = size(vIDs,1);

tot = 0.0;
count = 0;
for i=1:VehNumbers 
    tot = tot + double(correct_prediction(i)) / double(total_prediction(i));
    count = count + 1;    
end

classifyResult(runNumber) = tot / count;

end

% ---------------------------------------------------------------
    
% plot speed profile of vehicles    

figure('name', 'Classification', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle1 = plot(vehiclesTS, vehiclesSpeed, 'LineWidth', 3);

% set the x-axis limit
set( gca, 'XLim', [0 vehiclesTS(end)] );
    
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

       