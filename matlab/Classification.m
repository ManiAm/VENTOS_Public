
clear all;
close all;
clc;

% ---------------------------------------------------------------

basePATH = '../results/ML/cmd';
repetition = 5;

TLqueuingData = dir([basePATH, '/*_classificationResults.txt']);
runTotal = length(TLqueuingData);  % total number of simulation runs
row = 1;
column = 1;

classifyResult = zeros(runTotal/repetition, 1);
for runNumber = 1:runTotal

path = sprintf('%s/%s', basePATH, TLqueuingData(runNumber).name);
file_id = fopen(path);
C_text = textscan(file_id, '%s %d %d %[^\n]', 'HeaderLines', 6);
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

figure('name', 'Classification', 'units', 'normalized', 'outerposition', [0 0 1 1]);

handle1 = boxplot([classifyResult(1,:)', classifyResult(2,:)', classifyResult(3,:)', classifyResult(4,:)', classifyResult(5,:)', classifyResult(6,:)', classifyResult(7,:)'], 'labels', {'0','0.5','1','3','5','7','10'});

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Maximum GPS Error (m)', 'FontSize', 19);
ylabel('Classification Ratio', 'FontSize', 19);

grid on;   

       