clear all;

% --------------------------------------------------------------------
fileName2 = '../results/reception/0.txt';

% read the first 5 lines of the first file and extract some data
[~, part2] = textread(fileName2,'%s %s', 5, 'delimiter', ';');

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

% preallocating matrix (for performance issues)

TimePieces = zeros(NumVs, NUMrepetitions);
NumberPieces = zeros(NumVs, NUMrepetitions);

TimePiecesAll = zeros(NUMscenarios, NumVs);
NumberPiecesAll = zeros(NUMscenarios, NumVs);

% --------------------------------------------------------------------

% now read each of the files
for fn=1:Numruns

    fileName2 = sprintf('../results/reception/%d.txt', fn-1);
    
    % check to see if the file exists
    ex = exist(fileName2);
    
    if(ex == 0)
        fprintf('file does not exists.\n');        
        continue;
    end
    
    [token1, token2] = textread(fileName2,'%s %s', 'delimiter', ';', 'headerlines', 6);

    [rows,~] = size(token1);

% --------------------------------------------------------------------

    % initializing variables
    TimeCounter = 1;
    NumberCounter = 1;

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
     
        elseif(~isempty(strfind(str,'number of pieces')))   
            NumberPieces(NumberCounter, repetition+1) = str2double(char(token2(i)));      
            NumberCounter = NumberCounter + 1;
    
        elseif(~isempty(strfind(str,'last piece in')))   
            TimePieces(TimeCounter, repetition+1) = str2double(char(token2(i)));      
            TimeCounter = TimeCounter + 1;
        
        elseif(strcmp(str,'end of section') == 1)        
            NumberCounter = 1;
            TimeCounter = 1;
        
            if( rem(run, NUMrepetitions) == 0 )
                fprintf('\n%s\n\n', param);
            end        
        
            % display some info
            fprintf('Run: %d, Scenario: %d, Repetition: %d\n', run, scenario, repetition);
        
            % if all repetitions for a scenario finishes
            if( rem(run, NUMrepetitions) == NUMrepetitions - 1 )            
                disp(' ');
            
                for j=1:NumVs
                    NumberPiecesAll(scenario+1, j) = mean(NumberPieces(j,:));
                    TimePiecesAll(scenario+1, j) = mean(TimePieces(j,:)); 
                
                    % display the results
                    fprintf('V[%d]: number of pieces:     Ave = %f , Std = %f , ci = %f\n', j-1, mean(NumberPieces(j,:)), std(NumberPieces(j,:)), 0);
                    fprintf('V[%d]: last piece in:        Ave = %f , Std = %f , ci = %f\n', j-1, mean(TimePieces(j,:)), std(TimePieces(j,:)), 0);
                end
            
                fprintf('- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n');
            end           
        end
    end
end

% --------------------------------------------------------------------

% now after the above loop, we constructed two matrixes. In both of these 
% matrixes the row -> scenario number, column -> vehicle number.

% NumberPiecesAll: in each scenario for each vehicle -> number of total 
% received CRL pieces.

% TimePiecesAll: in each scenario for each vehicle -> time of reception of
% the last CRL piece.

% ----------------------------------------------------------------------

% remove some scenarios not needed

% simple case
arg_to_remove = [2,4,6,8, 9:32];

% use shuffling
% arg_to_remove = [9:32];

% use erasure code (shuffling is off)
% arg_to_remove = [2,4,6,8, 9:16, 18,20,22,24, 25:32];

%use erasure code (shuffling is on)
% arg_to_remove = [2,4,6,8, 9:16, 17,19,21,23, 25:32];

% use magic car (no shuffle and no erasure)
% arg_to_remove = [2,4,6,8,10,12,14,16, 17:32];

% number of broadcasts (when we have magic cars)
% arg_to_remove = [1:8, 10,12,14,16, 17:32];


[~,b] = size(arg_to_remove);
NUMscenarios = NUMscenarios - b;

NumberPiecesAll(arg_to_remove,:) = [];
TimePiecesAll(arg_to_remove,:) = [];

% ----------------------------------------------------------------------

% get the maximum time for all vehicles in all scenarios
MaximumTime = ceil(max(max(TimePiecesAll))) + 5;

% get the maximum time for all vehicles in all scenarios
MinimumTime = ceil(min(min(TimePiecesAll))) - 5;

P = zeros( NUMscenarios, MaximumTime ); 
count = 0;

[n1, n2] = size(TimePiecesAll);
[~, n3] = size(P);

for i=1:n3
    for j=1:n1   % n1: NUMscenarios
        for k=1:n2   % n2: number of V           
            if(TimePiecesAll(j, k) <= i) 
                %count = count + NumberPiecesAll(j, k);
                count = count + 1;
            end          
        end
        
        P(j, i) = count;
        count = 0; 
    end  
end

% ----------------------------------------------------------------------

figure('units','normalized','outerposition',[0 0 1 1]);
figure(1);

handle = plot(P','LineWidth', 2);

% set the x-axis limit (not showing from t = 0)
set( gca, 'XLim', [MinimumTime MaximumTime] );

% set the axis labels
xlabel('Time', 'FontSize', 19);
% ylabel('Number of CRL pieces');
ylabel('Number of vehicles', 'FontSize', 19);

% set the title of the figure
% title('Number of CRL pieces received in all vehicles in each scenario over time', 'FontWeight', 'bold');
% title('Number of vehicles that have recieved all the pieces in each scenario over time', 'FontWeight', 'bold');

% set the Displayname for each scenario
%{
for i=1:NUMscenarios
    ScenarioName = sprintf('Scenario %d', i);
    set(handle(i),'Displayname', ScenarioName);  
end
%}

set(handle(1),'Displayname', 'RSU-only');
set(handle(2),'Displayname', 'C2C-Epidemic');
set(handle(3),'Displayname', 'MPB');
set(handle(4),'Displayname', 'ICE');

% set the legend
legend(handle, 'Location','NorthEastOutside');

% set LineStyle and Marker
set(handle(1),'LineStyle', '-', 'Marker', '*', 'MarkerSize', 6);
set(handle(2),'LineStyle', '-');
set(handle(3),'LineStyle', '--', 'Marker', 'o', 'MarkerSize', 6);
set(handle(4),'LineStyle', '--', 'Marker', 'v', 'MarkerSize', 6);

% reduce the marker spacing
for i=1:NUMscenarios
    xdata = get(handle(i),'XData');
    ydata = get(handle(i),'YData');
    set(handle(i),'XData',xdata(1:20:n3));
    set(handle(i),'YData',ydata(1:20:n3));
end


%for i=1:NUMscenarios-1
%    for j=i+1:NUMscenarios
%        if( get(handle(i),'Color') == get(handle(j),'Color') & strcmp(get(handle(i),'LineStyle'), get(handle(j),'LineStyle')) == 1 & strcmp(get(handle(i),'Marker'), get(handle(j),'Marker')) == 1)
%            set(handle(i),'LineStyle', '--');
%            set(handle(j),'Marker', '*', 'MarkerSize', 4);
%            break;
%        end
%    end
%end

grid on;

set(gcf, 'PaperPositionMode', 'auto'); 
print('-dpng', '-r300', 'figure7');
