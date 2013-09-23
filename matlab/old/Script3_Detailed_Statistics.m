clear all;

% --------------------------------------------------------------------
fileName = '../results/detailed/0.txt';

% read the first 5 lines of the first file and extract some data
[~, part2] = textread(fileName,'%s %s', 5, 'delimiter', ';');

NUMscenarios = str2double( char(part2(1)) );
NUMrepetitions = str2double( char(part2(2)) );
Numruns = str2double( char(part2(3)) );
NumVs = str2double( char(part2(4)) );
NumRSUs = str2double( char(part2(5)) );

fprintf('Number of Scenarios:    %d\n', NUMscenarios);
fprintf('Number of Repetitions:  %d\n', NUMrepetitions);
fprintf('Number of Runs:         %d\n', Numruns);
fprintf('Number of Vehicles:     %d\n', NumVs);
fprintf('Number of RSUs:         %d\n', NumRSUs);
fprintf('- - - - - - - - - - - - - - - - - - - - - -\n');

% --------------------------------------------------------------------

% initializing variables

run = 0;  
scenario = 0;
repetition = 0;
        
% preallocating matrix (for performance issues)
convTime = zeros(1, NUMrepetitions);
NErrors = zeros(1, NUMrepetitions);
CRLI2V_Total = zeros(1, NUMrepetitions);
CRLI2V = zeros(1, NUMrepetitions);
CRLV2V_Total = zeros(1, NUMrepetitions);
CRLV2V = zeros(1, NUMrepetitions);
NoBroadcastsRSUs = zeros(1, NUMrepetitions);

% preallocating matrix (for performance issues)
X = zeros(NUMscenarios, 1);
Y = zeros(NUMscenarios, 1);
Z = zeros(NUMscenarios, 2);
W = zeros(NUMscenarios, 2);
U = zeros(NUMscenarios, 1);

% preallocating matrix (for performance issues)
Xci = zeros(NUMscenarios, 2);
Yci = zeros(NUMscenarios, 2);
Zci = zeros(NUMscenarios, 4);
Wci = zeros(NUMscenarios, 4);
Uci = zeros(NUMscenarios, 2);

% --------------------------------------------------------------------
% now read each of the files
for fn=1:Numruns

    fileName = sprintf('../results/detailed/%d.txt', fn-1);

    % check to see if the file exists
    ex = exist(fileName);
    
    if(ex == 0)
        fprintf('file does not exists.\n');        
        continue;
    end
    
    % skip the first 6 lines of the file
    [token1, token2] = textread(fileName,'%s %s', 'delimiter', ';', 'headerlines', 6);

    [rows,~] = size(token1);

    for i=1:rows
    
        str = char(token1(i));
    
        if(strcmp(str,'Parameters') == 1) 
            param = char(token2(i));
        
        elseif(strcmp(str,'Run Number') == 1) 
            run = str2double(char(token2(i)));
        
        elseif(strcmp(str,'Scenario Number') == 1)
            scenario = str2double(char(token2(i)));
    
        elseif(strcmp(str,'Repetition Number') == 1)
            repetition = str2double(char(token2(i))); 
        
        elseif(strcmp(str,'Convergence Time') == 1)
            convTime(1, repetition+1) = str2double(char(token2(i)));
            
        elseif(strcmp(str,'Number of Errors') == 1)
            NErrors(1, repetition+1) = str2double(char(token2(i)));
        
        elseif(strcmp(str,'CRLs in I2V (Dup)') == 1)
            CRLI2V_Total(1, repetition+1) = str2double(char(token2(i)));
            
        elseif(strcmp(str,'CRLs in I2V (New)') == 1)
            CRLI2V(1, repetition+1) = str2double(char(token2(i)));

        elseif(strcmp(str,'CRLs in V2V (Dup)') == 1)
            CRLV2V_Total(1, repetition+1) = str2double(char(token2(i)));
        
        elseif(strcmp(str,'CRLs in V2V (New)') == 1)
            CRLV2V(1, repetition+1) = str2double(char(token2(i)));
        
        elseif(strcmp(str,'RSUs broadcasts') == 1)
            NoBroadcastsRSUs(1, repetition+1) = str2double(char(token2(i)));        
        
        elseif(strcmp(str,'end of section') == 1)
        
            if( rem(run, NUMrepetitions) == 0 )
                fprintf('\n%s\n\n', param);
            end
        
            % display some info
            fprintf('Run: %d, Scenario: %d, Repetition: %d --> %f, %f, %f, %f, %f, %f, %f\n', run, scenario, repetition, convTime(repetition+1), NErrors(repetition+1), CRLI2V_Total(repetition+1), CRLI2V(repetition+1), CRLV2V_Total(repetition+1), CRLV2V(repetition+1), NoBroadcastsRSUs(repetition+1) );
        
            % if all repetitions for a scenario finishes
            if( rem(run, NUMrepetitions) == NUMrepetitions - 1 ) 
                % compute the average
                X(scenario + 1, 1) = mean(convTime);
            
                Y(scenario + 1, 1) = mean(NErrors);
            
                Z(scenario + 1, 1) = mean(CRLI2V_Total);
                Z(scenario + 1, 2) = mean(CRLI2V);
            
                W(scenario + 1, 1) = mean(CRLV2V_Total); 
                W(scenario + 1, 2) = mean(CRLV2V);
            
                U(scenario + 1, 1) = mean(NoBroadcastsRSUs);
            
                % calculate the confidence interval for convTime
                [~,~,ci] = ztest(convTime, mean(convTime), std(convTime));
                Xci(scenario + 1, 1) = abs( ci(1,1) - mean(convTime) );
                Xci(scenario + 1, 2) = abs( ci(1,2) - mean(convTime) );
            
                % calculate the confidence interval for NErrors
                [~,~,ci] = ztest(NErrors, mean(NErrors), std(NErrors));
                Yci(scenario + 1, 1) = abs( ci(1,1) - mean(NErrors) );
                Yci(scenario + 1, 2) = abs( ci(1,2) - mean(NErrors) );
            
                % calculate the confidence interval for CRLI2V_Total
                [~,~,ci] = ztest(CRLI2V_Total, mean(CRLI2V_Total), std(CRLI2V_Total));
                Zci(scenario + 1, 1) = abs( ci(1,1) - mean(CRLI2V_Total) );
                Zci(scenario + 1, 2) = abs( ci(1,2) - mean(CRLI2V_Total) );
            
                % calculate the confidence interval for CRLI2V
                [~,~,ci] = ztest(CRLI2V, mean(CRLI2V), std(CRLI2V));
                Zci(scenario + 1, 3) = abs( ci(1,1) - mean(CRLI2V) );
                Zci(scenario + 1, 4) = abs( ci(1,2) - mean(CRLI2V) );
            
                % calculate the confidence interval for CRLV2V_Total
                [~,~,ci] = ztest(CRLV2V_Total, mean(CRLV2V_Total), std(CRLV2V_Total));
                Wci(scenario + 1, 1) = abs( ci(1,1) - mean(CRLV2V_Total) );
                Wci(scenario + 1, 2) = abs( ci(1,2) - mean(CRLV2V_Total) );            
            
                % calculate the confidence interval for CRLV2V
                [~,~,ci] = ztest(CRLV2V, mean(CRLV2V), std(CRLV2V));
                Wci(scenario + 1, 3) = abs( ci(1,1) - mean(CRLV2V) );
                Wci(scenario + 1, 4) = abs( ci(1,2) - mean(CRLV2V) );
            
                % calculate the confidence interval for NoBroadcastsRSUs
                [~,~,ci] = ztest(NoBroadcastsRSUs, mean(NoBroadcastsRSUs), std(NoBroadcastsRSUs));
                Uci(scenario + 1, 1) = abs( ci(1,1) - mean(NoBroadcastsRSUs) );
                Uci(scenario + 1, 2) = abs( ci(1,2) - mean(NoBroadcastsRSUs) );
            
                % display the results
                fprintf('\nConvergence Time:     Ave = %f , Std = %f , ci = %f\n', X(scenario + 1, 1), std(convTime), Xci(scenario + 1, 1));
                fprintf('Number of Errors:     Ave = %f , Std = %f , ci = %f\n', Y(scenario + 1, 1), std(NErrors), Yci(scenario + 1, 1));
                fprintf('CRLs in I2V (Dup):    Ave = %f , Std = %f , ci = %f\n', Z(scenario + 1, 1), std(CRLI2V_Total), Zci(scenario + 1, 1));
                fprintf('CRLs in I2V (New):    Ave = %f , Std = %f , ci = %f\n', Z(scenario + 1, 2), std(CRLI2V), Zci(scenario + 1, 3));
                fprintf('CRLs in V2V (Dup):    Ave = %f , Std = %f , ci = %f\n', W(scenario + 1, 1), std(CRLV2V_Total), Wci(scenario + 1, 1));
                fprintf('CRLs in V2V (New):    Ave = %f , Std = %f , ci = %f\n', W(scenario + 1, 2), std(CRLV2V), Wci(scenario + 1, 3));  
                fprintf('No RSUs Broadcasts:   Ave = %f , Std = %f , ci = %f\n', U(scenario + 1, 1), std(NoBroadcastsRSUs), Uci(scenario + 1, 1));  
                fprintf('- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n');
            end           
        end
    end
end

% ----------------------------------------------------------------------

% remove some scenarios not needed

% simple case
arg_to_remove = [2,4,6,8, 9:32];
changeLable = true;

% use shuffling
% arg_to_remove = [9:32];
% changeLable = false;

% use erasure code (shuffling is off)
% arg_to_remove = [2,4,6,8, 9:16, 18,20,22,24, 25:32];
% changeLable = false;

%use erasure code (shuffling is on)
% arg_to_remove = [2,4,6,8, 9:16, 17,19,21,23, 25:32];
%changeLable = false;

% use magic car (no shuffle and no erasure)
% arg_to_remove = [2,4,6,8,10,12,14,16, 17:32];
% changeLable = false;

% number of broadcasts (when we have magic cars)
%arg_to_remove = [1:8, 10,12,14,16, 17:32];
%changeLable = false;


[~,b] = size(arg_to_remove);
NUMscenarios = NUMscenarios - b;

X(arg_to_remove) = [];
Xci(arg_to_remove,:) = [];

Y(arg_to_remove) = [];
Yci(arg_to_remove,:) = [];

Z(arg_to_remove,:) = [];
Zci(arg_to_remove,:) = [];

W(arg_to_remove,:) = [];
Wci(arg_to_remove,:) = [];

U(arg_to_remove) = [];
Uci(arg_to_remove,:) = [];

% ----------------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(1);

bar(X);

% re-number x-axis from 1 to NUMscenarios
set(gca, 'XTick', (1:NUMscenarios), 'FontSize', 19);

% set the axis labels
if(~changeLable)
    xlabel('Scenario', 'FontSize', 19);
else
    set(gca,'XTickLabel',{'RSU-only','C2C-Epidemic','MPB', 'ICE'}', 'FontSize', 19); 
    rotateXLabels(gca, 0);   
end

ylabel('Convergence time', 'FontSize', 19);

% set the title of the figure
% title('Convergence time in each scenario', 'FontWeight', 'bold', 'FontSize', 13);

grid on;

% graph the confident interval
hold on
errorbar((1:NUMscenarios), X, Xci(:,1), Xci(:,2), 'r', 'LineWidth', 2, 'LineStyle', 'none')
hold off

set(gcf, 'PaperPositionMode', 'auto'); 
print('-dpng', '-r300', 'figure3');

% -----------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(2);

subplot(2,1,1);
handle = bar(Z, 'stacked');

% re-number x-axis from 1 to NUMscenarios
set(gca, 'XTick', (1:NUMscenarios), 'FontSize', 19);

% set the axis labels
if(~changeLable)
    xlabel('Scenario', 'FontSize', 19);
else
    set(gca,'XTickLabel',{'RSU-only','C2C-Epidemic','MPB', 'ICE'}', 'FontSize', 19); 
    rotateXLabels(gca, 0);  
end

ylabel('Number of pieces', 'FontSize', 19);

% set the title of the figure
title('a) I2V communication', 'FontWeight', 'bold', 'FontSize', 19);

% set the Displayname
set(handle(1),'Displayname', 'Duplicate Pieces');
set(handle(2),'Displayname', 'New Pieces'); 

% set the legend
legend(handle, 'Location','NorthEastOutside');

grid on;


% -----------------------------------------------------

subplot(2,1,2);

handle = bar(W, 'stacked');

% re-number x-axis from 1 to NUMscenarios
set(gca, 'XTick', (1:NUMscenarios), 'FontSize', 19);

% set the axis labels
if(~changeLable)
    xlabel('Scenario', 'FontSize', 19);
else
    set(gca,'XTickLabel',{'RSU-only','C2C-Epidemic','MPB', 'ICE'}', 'FontSize', 19); 
    rotateXLabels(gca, 0);    
end

ylabel('Number of pieces', 'FontSize', 19);

% set the title of the figure
title('b) V2V communication', 'FontWeight', 'bold', 'FontSize', 19);

% set the Displayname
set(handle(1),'Displayname', 'Duplicate Pieces');
set(handle(2),'Displayname', 'New Pieces');

% set the legend
legend(handle, 'Location','NorthEastOutside');

grid on;

set(gcf, 'PaperPositionMode', 'auto');
print('-dpng', '-r300', 'figure4');


% -----------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(3);

bar(Y);

% re-number x-axis from 1 to NUMscenarios
set(gca, 'XTick', (1:NUMscenarios), 'FontSize', 19);

% set the axis labels
if(~changeLable)
    xlabel('Scenario', 'FontSize', 19);
else
    set(gca,'XTickLabel',{'RSU-only','C2C-Epidemic','MPB', 'ICE'}', 'FontSize', 19); 
    rotateXLabels(gca, 0);   
end

ylabel('Number of errors', 'FontSize', 19);

% set the title of the figure
% title('Number of total frames received with error in each scenario', 'FontWeight', 'bold', 'FontSize', 13);

grid on;

% graph the confident interval
hold on
errorbar((1:NUMscenarios), Y, Yci(:,1), Yci(:,2), 'r', 'LineWidth', 2, 'LineStyle', 'none')
hold off

set(gcf, 'PaperPositionMode', 'auto');
print('-dpng', '-r300', 'figure5');


% -----------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(4);

bar(U);

% re-number x-axis from 1 to NUMscenarios
set(gca, 'XTick', (1:NUMscenarios), 'FontSize', 19);

% set the axis labels
if(~changeLable)
    xlabel('Scenario', 'FontSize', 19);
else
    set(gca,'XTickLabel',{'RSU-only','C2C-Epidemic','MPB', 'ICE'}', 'FontSize', 19); 
    rotateXLabels(gca, 0);   
end

ylabel('Number of broadcasts', 'FontSize', 19);

% set the title of the figure
% title('Number of broadcasts in all RSUs in each scenario', 'FontWeight', 'bold', 'FontSize', 13);

grid on;

% graph the confident interval
hold on
errorbar((1:NUMscenarios), U, Uci(:,1), Uci(:,2), 'r', 'LineWidth', 2, 'LineStyle', 'none')
hold off

set(gcf, 'PaperPositionMode', 'auto');
print('-dpng', '-r300', 'figure6');


% -----------------------------------------------------


% call the second script to read the second file
%script2;





%set(gca, 'XTickLabel', {'V2V Communication (Total)', 'V2V Communication (New)'});


%baseline_handle = get(bar_handle(1),'BaseLine');
%set(baseline_handle,'LineWidth',2,'LineStyle','--','Color','red')

% set the legend
%set(bar_handle(1),'Displayname','Scenario 1');
%legend('Location','north');

