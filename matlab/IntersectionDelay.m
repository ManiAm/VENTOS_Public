
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

% total number of simulation runs
runTotal = 5;

% path to folder
basePATH = '../results/cmd/full_fix_web_act_delay_balanced';

for runNumber = 0:runTotal-1

fprintf('\n>>> runNumber %d:\n', runNumber);
    
% clear variables at the begining of each run
clearvars -except runNumber runTotal basePATH

% ----------------------------------------------------------------

disp('calculating average queue size ...');

path2 = sprintf('%s/%d_TLqueuingData.txt', basePATH, runNumber);
file_id = fopen(path2);
formatSpec = '%d %f %s %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 3);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
totalQs = C_text{1,4};
laneCounts = C_text{1,5};

% average queue size in each time step
averageQueueSize_all = double(totalQs) ./ double(laneCounts);

% aggregate every 400 values together
aggregateInterval_queue = 600;

rows = size(averageQueueSize_all, 1);
index = 1;
for i=1 : aggregateInterval_queue : rows-aggregateInterval_queue
    
    startIndex = i;
    endIndex = startIndex + aggregateInterval_queue - 1;
    
    averageQueueSize(index) = double(sum(averageQueueSize_all(startIndex:endIndex))) / double(aggregateInterval_queue);  
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_Q(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end

% -----------------------------------------------------------------

disp('reading vehicleDelay.txt ...');

path = sprintf('%s/%d_vehDelay.txt', basePATH, runNumber);
file_id = fopen(path);
formatSpec = '%s %s %s %f %d %f %f %f %f %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

vehicles = C_text{1,1};
entrance = C_text{1,4};
crossed = C_text{1,5};
startDecel = C_text{1,7};
startStopping = C_text{1,8};
crossedTime = C_text{1,9};
startAccel = C_text{1,10};
endDelay = C_text{1,11};

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
    
    if(double(startAccel(i,1)) == -1 && double(startDecel(i,1)) ~= -1)
        error('startAccel can not be -1');
    end
    
    if(crossed(i) == 1 && double(crossedTime(i,1)) == -1)
        error('crossedTime can not be -1');
    end

end

fprintf( 'totalVeh: %d, crossed: %d, noSlowDown: %d, noWaiting: %d \n', VehNumbers, sum(crossed(:,1) == 1), sum(indexTS(2,:) == -1), sum(indexTS(3,:) == -1) );

% ---------------------------------------------------------------

% making a cell array that contains the complete view of the system

if(false)
    
    disp('making uitable ...');
    
    cell1 = vIDs;
    cell2 = num2cell(crossed(:,1));
    cell3 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') num2cell(indexTS(5,:)') num2cell(indexTS(6,:)') num2cell(indexTS(7,:)')];

    data = [ cell1 cell2 cell3 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'entrance', 'deccel start', 'stopping start', 'cross time', 'accel start', 'end delay', 'YorR'}; 

end

% ---------------------------------------------------------------------

disp('calculating intersection delay ...');

interval_delay = 400;

rows = size(timeSteps, 1);
index = 1;
for j=1 : interval_delay-1 : rows-interval_delay
    
   startIndex = j;
   endIndex = startIndex + interval_delay - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
    
   delayedVehCount = 0;
   nonDelayedVehCount = 0;
   totalDelay = 0;

   for i=1:VehNumbers
       
       if(crossed(i) == 0)
           % this vehicle does not cross the intersection at all
           % We ignore this vehicle
           
       elseif(indexTS(1,i) > endT)
           % veh entrance is after this interval
           % We ignore this vehicle   
           
       elseif(indexTS(2,i) == -1)
               % this vehicle does not slow down because of getting r or Y.
               % In other words it is getting G or g and has right of way.
               % NOTE: in SUMO a vehicle slows down on g, but this slowing down is not due to
               % red or yellow light thus, indexTS(2,i) is still -1 
               
               % we need to check if the vehicle crossess the intersection at this interval
               
               if(indexTS(4,i) >= startT && indexTS(4,i) < endT)
                   % this vehicle crosses the intersection at this interval
                   nonDelayedVehCount = nonDelayedVehCount + 1;
               end
           
       elseif(indexTS(2,i) ~= -1)
           % this vehicle slows down due to getting a r or y signal
           % we need to check if this slowing down happens in this period
           % or has happend in a previous interval.
           % NOTE: if slowing down happens after this interval we don't care
               
               if(indexTS(2,i) >= startT && indexTS(2,i) < endT)
                   % slowing down occurs in this interval           
                   delayedVehCount = delayedVehCount + 1;
                   startDelay = indexTS(2,i);
                   endDelay = min( endT, indexTS(5,i) );
                   totalDelay = totalDelay + (endDelay - startDelay);   
           
               elseif(indexTS(2,i) < startT)
                   % slowing down occured in a previous interval                  
                   delayedVehCount = delayedVehCount + 1;
                   startDelay = indexTS(2,i);
                   endDelay = min( endT, indexTS(5,i) );
                   totalDelay = totalDelay + (endDelay - startDelay);
               end
       end
   end

   totalVehCount = nonDelayedVehCount + delayedVehCount;
   if(totalVehCount ~= 0)
       delay(index) = totalDelay / totalVehCount;
   else
       delay(index) = 0;
   end
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_D(index) = timeSteps(middleIndex);
    
   index = index + 1;

end
   
% -----------------------------------------------------------------

disp('calculating throughput ...');

interval_throughput = 900;

rows = size(timeSteps, 1);
index = 1;
for i=1 : interval_throughput-1 : rows-interval_throughput
    
    startIndex = i;
    endIndex = startIndex + interval_throughput - 1;
    
    startT = timeSteps(startIndex);
    endT = timeSteps(endIndex);
    
    vehCounter = 0;

    for j=1:VehNumbers       
        if( crossed(j) == 1 && indexTS(4,j) >= startT && indexTS(4,j) < endT )
            vehCounter = vehCounter + 1;
        end       
    end 
    
    throughput(index) = vehCounter;
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_T(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end

% -----------------------------------------------------------------

% now we make a single plot for all the scenarios

if(true)

    disp('plotting ...');

    if(runNumber == 0)
        figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    end

    if(runNumber == 0)
        lineMark = '-.';
    elseif(runNumber == 1)
        lineMark = '-.x';    
    elseif(runNumber == 2)
        lineMark = '-';
    elseif(runNumber == 3)
        lineMark = '-.v';
    elseif(runNumber == 4)
        lineMark = '--';
    else
        lineMark = '--';
    end
    
    subplot(3,1,1);    
    plot(timeSteps_Q, averageQueueSize, lineMark, 'LineWidth', 1);

    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Average Queue Size', 'FontSize', 17);

    %grid on;
    hold on;
    
    subplot(3,1,2);    
    plot(timeSteps_D, delay, lineMark, 'LineWidth', 1);

    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Average Delay (s)', 'FontSize', 17);

    %grid on;
    hold on;

    subplot(3,1,3);
    plot(timeSteps_T, throughput, lineMark, 'LineWidth', 1);

    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Throughput', 'FontSize', 17);

    %grid on;
    hold on;

    % at the end of the last iteration
    if(runNumber == runTotal-1)       

        for g=1:3
            subplot(3,1,g);
            Xlimit = get(gca,'xlim');
            set(gca, 'xtick' , 0:300:Xlimit(2)); 
        end   
        
        subplot(3,1,1);
        legend('fix-time' , 'adaptive webster', 'traffic-actuated', 'OJF', 'highest queue', 'Location', 'northwest');
    
        % mark change of demand with vertical lines
        for threshold=400:400:Xlimit(2)            
            for g=1:3
                subplot(3,1,g);
                % draw vertical line
                line([threshold threshold], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'k');
            end          
        end
    
    end

end

end


% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------
% -----------------------------------------------------------------


for runNumber = 0:runTotal-1

fprintf('\n>>> runNumber %d:\n', runNumber);

% clear variables at the begining of each run
clearvars -except runNumber runTotal basePATH timeSteps

% ----------------------------------------------------------------

disp('reading phasing information ...');

path3 = sprintf('%s/%d_TLphasingData.txt', basePATH, runNumber);
file_id = fopen(path3);
formatSpec = '%s %d %d %s %f %f %f %f %f %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 2);
fclose(file_id);

phaseNumbers = C_text{1,2};
cycleNumber = C_text{1,3};
greenLength = C_text{1,5};
greenStart = C_text{1,6};
yellowStart = C_text{1,7};
redStart = C_text{1,8};
endTime = C_text{1,9};
lanesCount = C_text{1,10};
queueSize = C_text{1,11};

totalPhases = size(phaseNumbers,1);
phaseDurationTS = zeros(4,totalPhases);
averageQueuePerPhase = zeros(1,totalPhases);

for i=1:totalPhases
    phaseDurationTS(1,i) = double(greenStart(i,1));
    phaseDurationTS(2,i) = double(yellowStart(i,1)); 
    phaseDurationTS(3,i) = double(redStart(i,1));
    phaseDurationTS(4,i) = double(endTime(i,1));
    
    % store average queue size per phase
    if( double(queueSize(i,1)) ~= -1 && double(lanesCount(i,1)) > 0 )
        averageQueuePerPhase(1,i) = double(queueSize(i,1)) / double(lanesCount(i,1));
    end
end

% if the last phase is incomplete
if(phaseDurationTS(1,totalPhases) == -1 || phaseDurationTS(2,totalPhases) == -1 || phaseDurationTS(3,totalPhases) == -1 || phaseDurationTS(4,totalPhases) == -1)
    phaseDurationTS(:,totalPhases) = [];
    totalPhases = totalPhases - 1;
end

% -----------------------------------------------------------------

disp('total cycles ...');

search_interval = 400;

rows = size(timeSteps, 1);
index = 1;
for j=1 : search_interval-1 : rows-search_interval
    
   startIndex = j;
   endIndex = startIndex + search_interval - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
   
   % find the last phase in this interval
   result = find(phaseDurationTS(4,:) <= endT);
   phaseNumber = result(end);
   
   totalCycles(index) = cycleNumber(phaseNumber);
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_SW(index) = timeSteps(middleIndex);
   
   index = index + 1;
   
end

% -----------------------------------------------------------------

disp('total green time ...');

search_interval2 = 400;

rows = size(timeSteps, 1);
index = 1;
greenPortion = 0;
for j=1 : search_interval2-1 : rows-search_interval2
    
   startIndex = j;
   endIndex = startIndex + search_interval2 - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
   %greenPortion = 0;
   
   for i=1:totalPhases       
       greenStart = phaseDurationTS(1,i);
       greenEnd = phaseDurationTS(2,i);
       
       if(greenStart > endT || greenEnd < startT)
           continue;
       end
       
       if(greenStart ~= -1 && greenEnd ~= -1)
           ST = max(greenStart, startT);
           EN = min(greenEnd, endT);
           greenPortion = greenPortion + (EN - ST);                         
       end
       
   end 
   
   totalGreenTime(index) = greenPortion;
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_GR(index) = timeSteps(middleIndex);
   
   index = index + 1;   
   
end

% -----------------------------------------------------------------

% now we make a single plot for all the scenarios

if(true)

    disp('plotting ...');

    if(runNumber == 0)
        figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    end

    if(runNumber == 0)
        lineMark = '-.';
    elseif(runNumber == 1)
        lineMark = '-.x';    
    elseif(runNumber == 2)
        lineMark = '-';
    elseif(runNumber == 3)
        lineMark = '-.v';
    elseif(runNumber == 4)
        lineMark = '--';
    else
        lineMark = '--';
    end
    
    subplot(2,1,1);
    plot(timeSteps_SW, totalCycles, lineMark, 'LineWidth', 1);

    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Cycle Count', 'FontSize', 17);

    %grid on;
    hold on;
    
    subplot(2,1,2);
    plot(timeSteps_GR, totalGreenTime, lineMark, 'LineWidth', 1);
            
    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (s)', 'FontSize', 17);
    ylabel('Total Green Time (s)', 'FontSize', 17);

    %grid on;
    hold on;

    % at the end of the last iteration
    if(runNumber == runTotal-1)       

        for g=1:2
            subplot(2,1,g);
            Xlimit = get(gca,'xlim');
            set(gca, 'xtick' , 0:300:Xlimit(2)); 
        end   
        
        subplot(2,1,1);
        legend('fix-time' , 'adaptive webster', 'traffic-actuated', 'OJF', 'highest queue', 'Location', 'northwest');
    
        % mark change of demand with vertical lines
        for threshold=400:400:Xlimit(2)            
            for g=1:2
                subplot(2,1,g);
                % draw vertical line
                line([threshold threshold], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'k');
            end          
        end
    
    end

end

end



