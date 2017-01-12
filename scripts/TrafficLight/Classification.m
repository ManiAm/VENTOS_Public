
clear all;
close all;
clc;

% ---------------------------------------------------------------

for iteration=1:3

if (iteration == 1)
     basePATH = '../results/02_trainError0m';
elseif (iteration == 2)
     basePATH = '../results/03_trainError3m';
elseif(iteration == 3)
     basePATH = '../results/04_trainError5m';
end

repetition = 5;

TLqueuingData = dir([basePATH, '/*_classificationResults.txt']);
runTotal = length(TLqueuingData);  % total number of simulation runs
row = 1;
column = 1;

classifyResult = zeros(runTotal/repetition, 1);
for runNumber = 1:runTotal

path = sprintf('%s/%s', basePATH, TLqueuingData(runNumber).name);
file_id = fopen(path);
C_text = textscan(file_id, '%s %[^|] %c %d %d', 'HeaderLines', 6);
fclose(file_id);

name = C_text{1,1};
predictions = C_text{1,2};
total_prediction = C_text{1,4};
correct_prediction = C_text{1,5};  

% ---------------------------------------------------------------

% stores vehicle IDs
vIDs = unique(name);
vIDs = sort_nat(vIDs);
VehNumbers = size(vIDs,1);

for i=1:VehNumbers 
    classifyResult(row, column) = double(correct_prediction(i)) / double(total_prediction(i)); 
    column = column + 1;
end

if(rem(runNumber, repetition) == 0)
    column = 1;
    row = row + 1; 
end

end

% ---------------------------------------------------------------

% plot speed profile of vehicles    

if(iteration == 1)
    figure('name', 'Classification', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    subaxis(2,2,1,'SpacingVert',0.1,'SpacingHoriz',0.06,'MA',0.02,'MB',0.1,'MR',0.02,'ML',0.01);
end

subaxis(2,2,iteration);
%subplot(2,2,iteration);

handle1 = boxplot([classifyResult(1,:)', classifyResult(2,:)', classifyResult(3,:)', classifyResult(4,:)', classifyResult(5,:)', classifyResult(6,:)', classifyResult(7,:)'], 'labels', {'0','0.5','1','3','5','7','10'}, 'colors', 'k');

% set font size
set(gca, 'FontSize', 20);

% set the axis labels
xlabel('Maximum Location Error (m)', 'FontSize', 20);
ylabel('Classification Success Ratio', 'FontSize', 20);

grid on;   

end

       