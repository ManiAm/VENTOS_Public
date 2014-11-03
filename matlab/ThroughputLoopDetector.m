
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------- 

syms Q G_min V N Tg Tp Lv;

V = 20;      % speed of the platoon 20 m/s = 72 km/h = 44.7 mph
Tp = 3.5;    % time gap between platoons 3.5 s
Lv = 5;      % vehicle length 5 m
G_min = 2;   % minimum space gap
Tg = 0.55;   % time gap between vehicles

Q = ( (V*N) / ( N*(Lv + G_min) + (N-1)*Tg*V + Tp*V) ) * 3600;

figure(1);
subaxis(1,2,'SpacingHoriz',0.07,'MA',0.02,'MB',0.1,'MR',0.02,'ML',0.06);

subaxis(1,2,1);
hold on;

h = ezplot(Q, [1 22]);
set(h,'LineWidth',3);
ylim([2400 3000]);

%set( gca, 'XLim', [0 25] );
%set( gca, 'YLim', [0 4000] );

% set font size
%set(gca, 'FontSize', 19);

% -----------------------------------------------------------

% effect of platoon size of throughput

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

disp('throughput with different PlnSize:');
fprintf(' #Vehs  #PlnSize=5  #PlnSize=10  #PlnSize=15  #PlnSize=20\n');

for i=1:rows-1
    fprintf('%5.0f  %10.0f  %10.0f  %10.0f  %10.0f\n', q(i,1), q(i,2), q(i,3), q(i,4), q(i,5) );
    if(gcd(i+1,5) == 5)
        fprintf('\n');
    end
end

% ----------------------------------------------------------

% extract a specific row
data = q(119, :);

% remove the first column
data(:,[1]) = [];

% copy first row into second
data(2,:) = data(1,:);

% fill first row
data(1,1) = 5;
data(1,2) = 10;
data(1,3) = 15;
data(1,4) = 20;

% ----------------------------------------------------------

subaxis(1,2,1);
h = bar(data(1,:), data(2,:), 0.6);

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

