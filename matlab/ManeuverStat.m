
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   %  closes the figure window

% ----------------------------------------------------------   

file_id = fopen('../results/gui/plnManage.txt');
formatSpec = '%f %s %s %s %s %s';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

% ----------------------------------------------------------
    
timeSteps = C_text{1,1};
vehicles = C_text{1,2};    
name = C_text{1,4};

% ----------------------------------------------------------

[rows,~] = size(timeSteps);

for i=1:rows    
    vehicle = char(vehicles(i,1));        
        

end


