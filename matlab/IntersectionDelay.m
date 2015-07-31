
clear all;
close all;
clc;    % position the cursor at the top of the screen
%clf;   % closes the figure window

% ---------------------------------------------------------------

% path to folder
basePATH = '../results/cmd/2_effect_active_detection/3_full_poisson_balanced_routeDist_70_30';

option = 1;  % 1: benefits of active detection   2: existing_multiClass   3: MWM

TLqueuingData = dir([basePATH, '/*_TLqueuingData.txt']);
TLphasingData = dir([basePATH, '/*_TLphasingData.txt']);
vehDelay = dir([basePATH, '/*_vehDelay.txt']);

% total number of simulation runs
runTotal = length(TLqueuingData);

for runNumber = 1:runTotal

fprintf('\n>>> runNumber %d:\n', runNumber);
    
% clear variables at the begining of each run
clearvars -except runNumber runTotal basePATH TLqueuingData TLphasingData vehDelay delayDist option

% ----------------------------------------------------------------

path2 = sprintf('%s/%s', basePATH, TLqueuingData(runNumber).name);
file_id = fopen(path2);
formatSpec = '%d %f %s %d %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);

indices = C_text{1,1};
timeSteps = C_text{1,2};
totalQs = C_text{1,4};
maxQs = C_text{1,5};
laneCounts = C_text{1,6};

disp('calculating average queue size for vehicles ...');

% out of 16, 4 are crosswalks and 4 are bike lanes
laneCounts = laneCounts - 8;

% average queue size in each time step
averageQueueSize_all = double(totalQs) ./ double(laneCounts);

% aggregate every 150 values together
aggregateInterval_queue = 700;

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

disp('calculating max queue size for vehicles ...');

% aggregate every 150 values together
aggregateInterval_queue = 700;

rows = size(maxQs, 1);
index = 1;
for i=1 : aggregateInterval_queue : rows-aggregateInterval_queue
    
    startIndex = i;
    endIndex = startIndex + aggregateInterval_queue - 1;
    
    maxQueueSize(index) = double(sum(maxQs(startIndex:endIndex))) / double(aggregateInterval_queue);  
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_MQ(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end


% -----------------------------------------------------------------

disp('reading vehicleDelay.txt file ...');

path = sprintf('%s/%s', basePATH, vehDelay(runNumber).name);
file_id = fopen(path);
formatSpec = '%s %s %s %s %f %d %f %f %f %f %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);

vehicles = C_text{1,1};
vehiclesType = C_text{1,2};
entrance = C_text{1,5};
crossed = C_text{1,6};
startDecel = C_text{1,8};
startStopping = C_text{1,9};
crossedTime = C_text{1,10};
startAccel = C_text{1,11};
endDelay = C_text{1,12};

% stores the ids of all entities
entityIDs = unique(vehicles);
entityIDs = sort_nat(entityIDs);
VehNumbers = size(entityIDs,1);

[rows,~] = size(vehicles);

% preallocating and initialization
indexTS = zeros(6,rows) - 1;

for i=1:rows 
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));        
    vNumber = find(ismember(entityIDs,vehicle)); 
    
    vehicleType(vNumber) = (vehiclesType(i,1));

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

fprintf( 'totalEntity: %d, crossed: %d, noSlowDown: %d, noWaiting: %d \n', VehNumbers, sum(crossed(:,1) == 1), sum(indexTS(2,:) == -1), sum(indexTS(3,:) == -1) );

% ---------------------------------------------------------------

% making a cell array that contains the complete view of the system

if(false)
    
    disp('making uitable ...');
    
    cell1 = vIDs;
    cell2 = num2cell(crossed(:,1));
    cell3 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') num2cell(indexTS(5,:)') num2cell(indexTS(6,:)') ];

    data = [ cell1 cell2 cell3 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'entrance', 'deccel start', 'stopping start', 'cross time', 'accel start', 'end delay'}; 

end

% ---------------------------------------------------------------------

disp('calculating intersection delay for vehicle/bike/ped ...');

% in each 70 seconds interval, we measure delay for each vehicle and then take an average
interval_delay = 700;

indexCounter = 1;

rows = size(timeSteps, 1);
for j=1 : interval_delay-1 : rows-interval_delay
    
   startIndex = j;
   endIndex = startIndex + interval_delay - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
    
   delayedCountVehPassenger = 0;
   nonDelayedCountVehPassenger = 0;
   totalDelayVehPassenger = 0;
   
   delayedCountVehEmergency = 0;
   nonDelayedCountVehEmergency = 0;
   totalDelayVehEmergency = 0;
   
   maxDelayBike = 0;
   maxDelayPed = 0;

   for i=1:VehNumbers
       
       % check the class of the current entity
       typeId = char(vehicleType(i)); 
       if(~isempty(strfind(typeId, 'passenger')))
           mode = 1;
       elseif(~isempty(strfind(typeId, 'emergency')))
           mode = 2;
       elseif(~isempty(strfind(typeId, 'bicycle')))
           mode = 3;
       elseif(~isempty(strfind(typeId, 'pedestrian')))
           mode = 4;
       else
           continue;
       end       
       
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
                   if(mode == 1)
                       nonDelayedCountVehPassenger = nonDelayedCountVehPassenger + 1;
                   elseif(mode == 2)
                       nonDelayedCountVehEmergency = nonDelayedCountVehEmergency + 1;
                   end
               end
           
       elseif(indexTS(2,i) ~= -1)
           % this vehicle slows down due to getting a r or y signal
           % we need to check if this slowing down happens in this period
           % or has happend in a previous interval.
           % NOTE: if slowing down happens after this interval we don't care
               
               if(indexTS(2,i) >= startT && indexTS(2,i) < endT)
                   % slowing down occurs in this interval     
                   if(mode == 1)
                       delayedCountVehPassenger = delayedCountVehPassenger + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehPassenger = totalDelayVehPassenger + (endDelay - startDelay); 
                   elseif(mode == 2)
                       delayedCountVehEmergency = delayedCountVehEmergency + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehEmergency = totalDelayVehEmergency + (endDelay - startDelay); 
                   elseif(mode == 3 && indexTS(3,i) ~= -1 && indexTS(3,i) < endT)
                       % bike
                       startDelay = indexTS(3,i);
                       endDelay = min( endT, indexTS(5,i) );
                       maxDelayBike = max(maxDelayBike, (endDelay - startDelay));
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
           
               elseif(indexTS(2,i) < startT && indexTS(4,i) >= startT)
                   % slowing down occured in a previous interval                  
                   if(mode == 1)
                       delayedCountVehPassenger = delayedCountVehPassenger + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehPassenger = totalDelayVehPassenger + (endDelay - startDelay); 
                   elseif(mode == 2)
                       delayedCountVehEmergency = delayedCountVehEmergency + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehEmergency = totalDelayVehEmergency + (endDelay - startDelay); 
                   elseif(mode == 3 && indexTS(3,i) ~= -1 && indexTS(5,i) >= startT)
                       % bike
                       startDelay = max( startT, indexTS(3,i) );
                       endDelay = min( endT, indexTS(5,i) );
                       maxDelayBike = max(maxDelayBike, (endDelay - startDelay));                       
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
               end
       end
   end

   % passenger vehicle
   totalCountVehPassenger = nonDelayedCountVehPassenger + delayedCountVehPassenger;
   if(totalCountVehPassenger ~= 0)
       delayPassenger(indexCounter) = totalDelayVehPassenger / totalCountVehPassenger;
   else
       delayPassenger(indexCounter) = 0;
   end
   
   % emergency vehicle
   totalCountVehEmergency = nonDelayedCountVehEmergency + delayedCountVehEmergency;
   if(totalCountVehEmergency ~= 0)
       delayEmergency(indexCounter) = totalDelayVehEmergency / totalCountVehEmergency;
   else
       delayEmergency(indexCounter) = 0;
   end
   
   % bike
   delayBike(indexCounter) = maxDelayBike;
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_D(indexCounter) = timeSteps(middleIndex);
    
   indexCounter = indexCounter + 1;

end

delayDist{1,runNumber} = num2cell(delayPassenger / 60.);
delayDist{2,runNumber} = num2cell(delayEmergency / 60.);
delayDist{3,runNumber} = num2cell(delayBike);

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

    if(runNumber == 1)
        figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    end

    if(runNumber == 1)
        lineMark = '-.';
    elseif(runNumber == 2)
        lineMark = '-.x';    
    elseif(runNumber == 3)
        lineMark = '-.v';
    elseif(runNumber == 4)
        lineMark = '-';
    elseif(runNumber == 5)
        lineMark = '--';
    else
        lineMark = '--';
    end
    
    subplot(3,1,1);    
    plot(timeSteps_MQ/60, maxQueueSize, lineMark, 'LineWidth', 1, 'Color', 'k');

    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Vehicles Max Queue' , 'Size per Lane'}, 'FontSize', 20);

    grid on;
    hold on;
    
    if(runNumber == runTotal)    
        if(option == 1)
            legend('Adaptive Webster (active)', 'Traffic-actuated (active)' , 'Traffic-actuated', 'Longest queue', 'Longest queue (active)', 'Location', 'northwest');
        elseif(option == 2)
            legend('Fix-time', 'Traffic-actuated', 'Longest queue', 'OJF', 'Location', 'northwest');
        elseif(option == 3)
            legend('LQF\_MWM' , 'OJF\_MWM', 'Location', 'northwest');
        end
        
        % set the x-axis limit
        set( gca, 'XLim', [0 3700/60] );
            
        % x-axis is integer
        Xlimit = get(gca,'xlim');            
        set(gca, 'xtick' , 0:3:Xlimit(2));
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:20:Ylimit(2));
    end
    
    subplot(3,1,2);    
    
    if(option == 1)    
        plot(timeSteps_D/60, delayPassenger/60, lineMark, 'LineWidth', 1, 'Color', 'k');
        
        % set font size
        set(gca, 'FontSize', 20);

        xlabel('Time (min)', 'FontSize', 20);
        ylabel({'Ave Delay per', 'Vehicle (min)'}, 'FontSize', 20);

        grid on;
        hold on;
        
        if(runNumber == runTotal) 
            % set the x-axis limit
            set( gca, 'XLim', [0 3700/60] );
            
            % x-axis is integer
            Xlimit = get(gca,'xlim');            
            set(gca, 'xtick' , 0:3:Xlimit(2));
        
            % Y-axis for delay          
            Ylimit = get(gca,'ylim');            
            set(gca, 'ytick' , 0:4:Ylimit(2));
        end
        
    elseif(option == 2 && runNumber == runTotal)        
        % distribution of average veh delay        
        data = [cell2mat(delayDist{1,1})'; cell2mat(delayDist{2,1})'; 
                cell2mat(delayDist{1,2})'; cell2mat(delayDist{2,2})'; 
                cell2mat(delayDist{1,3})'; cell2mat(delayDist{2,3})'; 
                cell2mat(delayDist{1,4})'; cell2mat(delayDist{2,4})'];

        n1 = size(delayDist{1,1},2);
        n5 = size(delayDist{2,1},2);
        
        n2 = size(delayDist{1,2},2);
        n6 = size(delayDist{2,2},2);        
        
        n3 = size(delayDist{1,3},2);
        n7 = size(delayDist{2,3},2);        
        
        n4 = size(delayDist{1,4},2);        
        n8 = size(delayDist{2,4},2);
        
        group = [repmat({'Fix-Time (P)'}, n1, 1); repmat({'Fix-Time (S)'}, n5, 1);
                 repmat({'T-Act (P)'}, n2, 1); repmat({'T-Act (S)'}, n6, 1);
                 repmat({'L-Queue (P)'}, n3, 1); repmat({'L-Queue (S)'}, n7, 1);
                 repmat({'OJF (P)'}, n4, 1); repmat({'OJF (S)'}, n8, 1)];
             
        boxplot(data,group,'colors', 'k');

        set(findobj(gca,'Type','text'),'FontSize',20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Vehicles Ave Delay','Distribution (min)'}, 'FontSize', 20);

        grid on;  
        
    elseif(option == 3 && runNumber == runTotal)        
        % distribution of average veh delay        
        data = [cell2mat(delayDist{1,1})'; cell2mat(delayDist{1,2})'; 
                cell2mat(delayDist{2,1})'; cell2mat(delayDist{2,2})'];

        n1 = size(delayDist{1,1},2);
        n5 = size(delayDist{2,1},2);        
        
        n2 = size(delayDist{1,2},2);
        n6 = size(delayDist{2,2},2);
        
        group = [repmat({'LQF_MWM (P)'}, n1, 1); 
                 repmat({'LQF_MWM (S)'}, n5, 1);
                 repmat({'OJF_MWM (P)'}, n2, 1);  
                 repmat({'OJF_MWM (S)'}, n6, 1)];

        boxplot(data,group,'colors', 'k');

        set(findobj(gca,'Type','text'),'FontSize',20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Vehicles Ave Delay','Distribution (min)'}, 'FontSize', 20);

        grid on;   
        
    end

    subplot(3,1,3);
    
    if(option == 1) 
        plot(timeSteps_T/60, throughput, lineMark, 'LineWidth', 1, 'Color', 'k');

        % set font size
        set(gca, 'FontSize', 20);

        xlabel('Time (min)', 'FontSize', 20);
        ylabel('Throughput', 'FontSize', 20);

        grid on;
        hold on;
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:20:Ylimit(2));
        
        if(runNumber == runTotal) 
            % set the x-axis limit
            set( gca, 'XLim', [0 3700/60] );
            
            % x-axis is integer
            Xlimit = get(gca,'xlim');            
            set(gca, 'xtick' , 0:3:Xlimit(2));
        end
        
    elseif(option == 2 && runNumber == runTotal)
        % distribution of max bike delay       
        data = [cell2mat(delayDist{3,1})'; 
                cell2mat(delayDist{3,2})'; 
                cell2mat(delayDist{3,3})'; 
                cell2mat(delayDist{3,4})'];

        n1 = size(delayDist{3,1},2);
        n2 = size(delayDist{3,2},2);        
        n3 = size(delayDist{3,3},2);
        n4 = size(delayDist{3,4},2);
        
        group = [repmat({'Fix-time'}, n1, 1); repmat({'Traffic actuated'}, n2, 1); repmat({'Longest Queue'}, n3, 1); repmat({'OJF'}, n4, 1)];

        boxplot(data,group,'colors', 'k');

        set(findobj(gca,'Type','text'),'FontSize',20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Bikes Max Delay', 'Distribution (s)'}, 'FontSize', 20);

        grid on; 
        
    elseif(option == 3 && runNumber == runTotal)
        % distribution of max bike delay       
        data = [cell2mat(delayDist{3,1})'; 
                cell2mat(delayDist{3,2})'];

        n1 = size(delayDist{3,1},2);
        n2 = size(delayDist{3,2},2);
        group = [repmat({'LQF_MWM'}, n1, 1); repmat({'OJF_MWM'}, n2, 1)];

        boxplot(data,group,'colors', 'k');

        set(findobj(gca,'Type','text'),'FontSize',20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Bikes Max Delay', 'Distribution (s)'}, 'FontSize', 20);

        grid on; 
        
    end

    % at the end of the last iteration
    if(runNumber == runTotal)    
    
%         % mark change of demand with vertical lines
%         for threshold=400:400:Xlimit(2)            
%             for g=1:2
%                 subplot(2,1,g);
%                 % draw vertical line
%                 line([threshold threshold], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'k');
%             end          
%         end
        
        % mark light/medium/heavy traffic with arrows
        if(option == 1)
            endNum = 3;
        elseif(option == 2)
            endNum = 1;
        elseif(option == 3)
            endNum = 1;
        end
        
        for g=1:endNum
            subplot(3,1,g);
            
            Ylimit = get(gca,'ylim');
            arrowYLoc = Ylimit(2) + (0.05 * Ylimit(2));
            textYLoc = arrowYLoc + (0.1 * arrowYLoc);
            
            Start = [0 arrowYLoc];
            Stop = [800/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3);  
            text((400-150)/60, textYLoc, 'Light', 'FontSize', 20);
            
            Start = [800/60 arrowYLoc];
            Stop = [1600/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((1200-150)/60, textYLoc, 'Medium', 'FontSize', 20);
            
            Start = [1600/60 arrowYLoc];
            Stop = [2400/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2000-150)/60, textYLoc, 'Heavy', 'FontSize', 20);
            
            Start = [2400/60 arrowYLoc];
            Stop = [3200/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2800-150)/60, textYLoc, 'Medium', 'FontSize', 20);
            
            Start = [3200/60 arrowYLoc];
            Stop = [Xlimit(2) arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((3470-150)/60, textYLoc, 'Light', 'FontSize', 20);
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

for runNumber = 1:runTotal

fprintf('\n>>> runNumber %d:\n', runNumber);

% clear variables at the begining of each run
clearvars -except runNumber runTotal basePATH timeSteps TLqueuingData TLphasingData vehDelay delayVariations

% ----------------------------------------------------------------

disp('reading phasing information ...');

path3 = sprintf('%s/%s', basePATH, TLphasingData(runNumber).name);
file_id = fopen(path3);
formatSpec = '%s %d %d %s %f %f %f %f %f %d %d';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
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

    if(runNumber == 1)
        figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    end

    if(runNumber == 1)
        lineMark = '-.';
    elseif(runNumber == 2)
        lineMark = '-.x';    
    elseif(runNumber == 3)
        lineMark = '-.v';
    elseif(runNumber == 4)
        lineMark = '-';
    elseif(runNumber == 5)
        lineMark = '--';
    else
        lineMark = '--';
    end
    
    subplot(2,1,1);
    plot(timeSteps_SW/60, totalCycles, lineMark, 'LineWidth', 1);

    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (min)', 'FontSize', 17);
    ylabel('Cycle Count', 'FontSize', 17);

    grid on;
    hold on;
    
    subplot(2,1,2);
    plot(timeSteps_GR/60, totalGreenTime/60, lineMark, 'LineWidth', 1);
            
    % set font size
    set(gca, 'FontSize', 17);

    xlabel('Time (min)', 'FontSize', 17);
    ylabel('Total Green Time (min)', 'FontSize', 17);

    grid on;
    hold on;

    % at the end of the last iteration
    if(runNumber == runTotal)       
        
        for g=1:2
            subplot(2,1,g);   
            
            % set the x-axis limit
            set( gca, 'XLim', [0 3700/60] );
            
            % x-axis is integer
            Xlimit = get(gca,'xlim');            
            set(gca, 'xtick' , 0:5:Xlimit(2));
        end 
        
        subplot(2,1,1);
        legend('Fix-time' , 'Traffic-actuated', 'Longest queue', 'OJF', 'Location', 'northwest');
    
%         % mark change of demand with vertical lines
%         for threshold=400:400:Xlimit(2)            
%             for g=1:2
%                 subplot(2,1,g);
%                 % draw vertical line
%                 line([threshold threshold], ylim, 'LineWidth', 1, 'LineStyle', '--', 'Color', 'k');
%             end          
%         end

        % mark light/medium/heavy traffic with arrows
        for g=1:2
            subplot(2,1,g);
            
            Ylimit = get(gca,'ylim');
            arrowYLoc = Ylimit(2) + (0.05 * Ylimit(2));
            textYLoc = arrowYLoc + (0.09 * arrowYLoc);
            
            Start = [0 arrowYLoc];
            Stop = [800/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3);  
            text((400-150)/60, textYLoc, 'Light Traffic', 'FontSize', 17);
            
            Start = [800/60 arrowYLoc];
            Stop = [1600/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((1200-150)/60, textYLoc, 'Medium Traffic', 'FontSize', 17);
            
            Start = [1600/60 arrowYLoc];
            Stop = [2400/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2000-150)/60, textYLoc, 'Heavy Traffic', 'FontSize', 17);
            
            Start = [2400/60 arrowYLoc];
            Stop = [3200/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2800-150)/60, textYLoc, 'Medium Traffic', 'FontSize', 17);
            
            Start = [3200/60 arrowYLoc];
            Stop = [Xlimit(2) arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((3400-150)/60, textYLoc, 'Light Traffic', 'FontSize', 17);
        end 
    
    end

end

end



