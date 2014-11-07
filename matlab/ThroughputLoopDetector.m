
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------- 

% effect of platoon size of throughput

for run=0:3
    filePath = sprintf('../results/cmd/plnSize_on_throu/%d_loopDetector.txt', run);
    file_id = fopen(filePath);
    formatSpec = '%s %s %f %f %f %f';
    C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
    fclose(file_id);

    % --------------------------
    
    if(run == 0)
        plnSize = 5;
    elseif(run == 1)
        plnSize = 10;
    elseif(run == 2)
        plnSize = 15;
    elseif(run == 3)
        plnSize = 20;
    end    
    
    % --------------------------
    
    vehicles = C_text{1,2};    
    vehEntry = C_text{1,3};

    % --------------------------
    
    timeStart = double(vehEntry(1,1));
    vehCount = 1;
    index = 1;

    [rows,~] = size(vehicles);

    for i=2:rows    
        vehCount = vehCount + 1;
        time = double(vehEntry(i,1));
        duration = time - timeStart;
        
        if(gcd(i-1,plnSize) == plnSize)
            q(index, run+1) = (3600 * (vehCount-1)) / duration;   
            index = index + 1;
        end       
    end
end

disp('Throughput with different PlnSize:');
fprintf(' #PlnSize=5  #PlnSize=10  #PlnSize=15  #PlnSize=20\n');

[rows, ~] = size(q);

for i=1:rows
    fprintf('%10.0f  %10.0f  %10.0f  %10.0f\n', q(i,1), q(i,2), q(i,3), q(i,4) );
end

% ----------------------------------------------------------

figure(1);
subaxis(1,2,'SpacingHoriz',0.09,'MA',0.02,'MB',0.1,'MR',0.02,'ML',0.09);

subaxis(1,2,1);
h = bar([5 10 15 20], [q(24,1) q(11,2) q(7,3) q(5,4)], 0.6);

set(h(1), 'FaceColor', [0.5 0.5 0.5]);

hold on;

% ----------------------------------------------------------

syms Q G_min V N Tg Tp Lv;

V = 20;      % speed of the platoon 20 m/s = 72 km/h = 44.7 mph
Tp = 3.5;    % time gap between platoons 3.5 s
Lv = 5;      % vehicle length 5 m
G_min = 2;   % minimum space gap
Tg = 0.55;   % time gap between vehicles
% platoon size (N) is changing

Q = ( (V*N) / ( N*(Lv + G_min) + (N-1)*Tg*V + Tp*V) ) * 3600;

h = ezplot(Q, [1 22]);
set(h,'LineWidth',3);
set(h, 'LineStyle', '-.');

xlabel('Platoon Size', 'FontSize', 20);
ylabel('Throughput (veh/h)', 'FontSize', 20);

% set font size
set(gca, 'FontSize', 20);

% only show these values
set(gca,'XTick',[5 10 15 20] );

ylim([2300 3500]);

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
h = bar([2 3.5 5], data, 0.6);

set(h(1), 'FaceColor', [0.5 0.5 0.5]);

hold on;

% -----------------------------------------------------------

syms Q G_min V N Tg Tp Lv;

V = 20;      % speed of the platoon 20 m/s = 72 km/h = 44.7 mph
N = 5;       % platoon size is 5
Lv = 5;      % vehicle length 5 m
G_min = 2;   % minimum space gap
Tg = 0.55;   % time gap between vehicles
% Tp is changing

Q = ( (V*N) / ( N*(Lv + G_min) + (N-1)*Tg*V + Tp*V) ) * 3600;

h = ezplot(Q, [1 6]);
set(h, 'LineWidth', 3);
set(h, 'LineStyle', '-.');

xlabel('Inter-platoon Spacing (T_P)', 'FontSize', 20);
ylabel('Throughput (veh/h)', 'FontSize', 20);

% set font size
set(gca, 'FontSize', 20);

% only show these values
set(gca,'XTick',[2 3.5 5] );

ylim([1800 3200]);

grid on;

% -----------------------------------------------------------

disp('done!');

