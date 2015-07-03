
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

% total number of simulation runs
runTotal = 3;
endSim = 0;

for runNumber = 0:runTotal-1
    
clearvars -except runNumber runTotal endSim

basePATH = '../results/cmd/full_fix_web_adap_balanced';
path = sprintf('%s/%d_vehicleDelay.txt', basePATH, runNumber);
path2 = sprintf('%s/%d_intersectionData.txt', basePATH, runNumber);

% ---------------------------------------------------------------

disp('reading vehicleDelay.txt ...');

file_id = fopen(path);
formatSpec = '%s %s %s %f %d %f %d %f %f %f %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

vehicles = C_text{1,1};
entrance = C_text{1,4};
crossed = C_text{1,5};
YorR = C_text{1,7};
startDecel = C_text{1,8};
startStopping = C_text{1,9};
crossedTime = C_text{1,10};
startAccel = C_text{1,11};
endDelay = C_text{1,12};

% ---------------------------------------------------------------

disp('parsing file ...');
    
% stores vehicle IDs
vIDs = unique(vehicles);
vIDs = sort_nat(vIDs);
VehNumbers = size(vIDs,1);

[rows,~] = size(vehicles);

% preallocating and initialization with -1
indexTS = zeros(7,rows) - 1;

for i=1:rows 
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));
        
    vNumber = find(ismember(vIDs,vehicle)); 

    indexTS(1,vNumber) = double(entrance(i,1));       
    indexTS(2,vNumber) = double(startDecel(i,1));
    indexTS(3,vNumber) = double(startStopping(i,1));
    indexTS(4,vNumber) = double(crossedTime(i,1));
    indexTS(5,vNumber) = double(startAccel(i,1));
    indexTS(6,vNumber) = double(endDelay(i,1));
    indexTS(7,vNumber) = int32(YorR(i,1));
    
    if(double(startAccel(i,1)) == -1 && double(startDecel(i,1)) ~= -1)
        error('startAccel can not be -1');
    end
    
    if(crossed(i) == 1 && double(crossedTime(i,1)) == -1)
        error('crossedTime can not be -1');
    end

end

% ---------------------------------------------------------------

% making a cell array that contains the complete view of the system

disp('making uitable ...');

if(false)
    
    cell1 = vIDs;
    cell2 = num2cell(crossed(:,1));
    cell3 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') num2cell(indexTS(5,:)') num2cell(indexTS(6,:)') num2cell(indexTS(7,:)')];

    data = [ cell1 cell2 cell3 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'entrance', 'deccel start', 'stopping start', 'cross time', 'accel start', 'end delay', 'YorR'}; 

end

% ----------------------------------------------------------------

disp('reading IntersectionData.txt and get phasing information ...');

% we need to get TL phasing information + queue size
    
file_id = fopen(path2);
formatSpec = '%s %d %s %f %f %f %f %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

phaseNumbers = C_text{1,2};
greenStart = C_text{1,4};
yellowStart = C_text{1,5};
redStart = C_text{1,6};
endTime = C_text{1,7};
lanesCount = C_text{1,8};
queueSize = C_text{1,9};

totalPhases = size(phaseNumbers,1);

for i=1:totalPhases
    
    if(double(greenStart(i,1)) ~= -1 && double(yellowStart(i,1)) ~= -1 && double(redStart(i,1)) ~= -1 && double(endTime(i,1)) ~= -1 && double(queueSize(i,1)) ~= -1 && double(lanesCount(i,1)) > 0)
    
        phaseDurationTS(1,i) = double(greenStart(i,1));
        phaseDurationTS(2,i) = double(yellowStart(i,1)); 
        phaseDurationTS(3,i) = double(redStart(i,1));
        phaseDurationTS(4,i) = double(endTime(i,1));
    
        % store average queue size
        phaseData(1,i) = double(queueSize(i,1)) / double(lanesCount(i,1));

    end
    
end

endSim = max( endSim, phaseDurationTS(4,end) );

% -----------------------------------------------------------------

disp('calculating throughput ...');

phasesCount = size(phaseDurationTS,2);

throughput = zeros(phasesCount,1);   % number of crossed vehicles in each phase
   
for j=1:phasesCount
    
   phaseStart = phaseDurationTS(1,j);
   phaseEnd = phaseDurationTS(4,j);
   vehCounter = 0;

   for i=1:VehNumbers       
       if( crossed(i) == 1 && indexTS(4,i) >= phaseStart && indexTS(4,i) <= phaseEnd)
           vehCounter = vehCounter + 1;
       end       
   end
   
   throughput(j) = vehCounter;
   vehCounter = 0;   
end

% -----------------------------------------------------------------

disp('calculating the delay per phase ...');

fprintf( '\ntotalVeh: %d, crossedVehCount: %d\n\n', VehNumbers, sum(crossed(:,1) == 1) );

phasesCount = size(phaseDurationTS,2);

crossedVehCount = zeros(phasesCount);   % number of crossed vehicles in each phase
delayedVehCount = zeros(phasesCount);   % number of delayed vehicles in each phase
   
totalDelay = zeros(phasesCount);        % total delay in each phase
   
for j=1:phasesCount
    
   phaseStart = phaseDurationTS(1,j);
   phaseEnd = phaseDurationTS(4,j);

   for i=1:VehNumbers
       
       if(crossed(i) == 0)
           % this vehicle does not cross the intersection
           % We ignore this vehicle
           
       elseif(indexTS(1,i) > phaseEnd)
           % veh entrance is after phase j
           % We ignore this vehicle   
           
       elseif(indexTS(2,i) == -1)
               % this vehicle does not slow down at this phase
           
               if(indexTS(4,i) > phaseEnd)
                   % this vehicle does not cross the intersection at this phase
                   % the distance to TL is high and there is no need to break   
                   % We ignore this vehicle since it crosses in a future phase
               
               elseif(indexTS(4,i) <= phaseEnd)
                   % crosses the intersection at this phase
                   % it is getting green (G) and has right of way
                   crossedVehCount(j) = crossedVehCount(j) + 1;  
               
               end
           
       elseif(indexTS(2,i) ~= -1)
           % this vehicle slows down
           
               if(indexTS(2,i) > phaseEnd)
                   % slowing down occurs after this phase
                   % We ignore this vehicle
               
               elseif(indexTS(2,i) <= phaseEnd && indexTS(2,i) >= phaseStart)
                   % slowing down occurs in this phase           
           
                       if(indexTS(3,i) ~= -1)
                           % this slowing down is due to a red/yellow light 
                           delayedVehCount(j) = delayedVehCount(j) + 1;
                           startDelay = indexTS(2,i);
                           endDelay = min( phaseEnd, indexTS(5,i) );
                           totalDelay(j) = totalDelay(j) + (endDelay - startDelay);            
                       else
                           % this slowing down is due to getting 'g'
                           crossedVehCount(j) = crossedVehCount(j) + 1;
                       end     
           
               elseif(indexTS(2,i) < phaseStart)
                   % slowing down occured in previous phases                  
                   delayedVehCount(j) = delayedVehCount(j) + 1;
                   startDelay = indexTS(2,i);
                   endDelay = min( phaseEnd, indexTS(5,i) );
                   totalDelay(j) = totalDelay(j) + (endDelay - startDelay);
               end
       end
   end

   tot = crossedVehCount(j) + delayedVehCount(j);
   if(tot ~= 0)
       phaseData(2,j) = totalDelay(j) / tot;
   else
       phaseData(2,j) = 0;
   end
   
   fprintf( 'phase %d: crossedVehCount=%2d, delayedVehCount=%2d, totalDelay=%0.2f, aveDelay=%0.2f, aveQueueSize=%0.2f\n', j, crossedVehCount(j), delayedVehCount(j), totalDelay(j), phaseData(2,j), phaseData(1,j) );
    
end

fprintf('\n');
   
% -----------------------------------------------------------------

% now we make a single plot for all the scenarios
disp('drawing queue size / average delay / throughput per phase ...');

if(runNumber == 0)
    figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
end

subplot(3,1,1);
plot(phaseData(1,:),'LineWidth', 3);

% set font size
set(gca, 'FontSize', 17);

xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Queue Size', 'FontSize', 17);

grid on;
hold on;
    
subplot(3,1,2);
plot(phaseData(2,:),'LineWidth', 3);

% set font size
set(gca, 'FontSize', 17);

xlabel('Phase Number', 'FontSize', 17);
ylabel('Average Delay (s)', 'FontSize', 17);

grid on;
hold on;

subplot(3,1,3);
plot(throughput,'LineWidth', 3);

% set font size
set(gca, 'FontSize', 17);

xlabel('Phase Number', 'FontSize', 17);
ylabel('Throughput', 'FontSize', 17);

grid on;
hold on;

% at the end of the last iteration
if(runNumber == runTotal-1)

    % X-axis should be integer
    for g=1:3
        subplot(3,1,g);
        Xlimit = get(gca,'xlim');
        set(gca, 'xtick' , 0:20:Xlimit(2));
        legend('fix-time' , 'adaptive Webster', 'traffic-actuated');
    end   
    
end

end

