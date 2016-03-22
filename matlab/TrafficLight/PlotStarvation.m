
function PlotStarvation(runNumber, timeSteps_D, delayDist, runTotal)

    % make a windows figure only once
    if(runNumber == 1)
        figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
    end
    
    if(runNumber == 1)
        lineMark = '-.x';    
    elseif(runNumber == 2)
        lineMark = '-.v';
    elseif(runNumber == 3)
        lineMark = '-';
    elseif(runNumber == 4)
        lineMark = '--';
    end
    
    % max waiting for bikes
    plot(timeSteps_D, cell2mat(delayDist{3,runNumber}), lineMark, 'LineWidth', 1, 'Color', 'k');
    
    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel('Bike Waiting Time (s)', 'FontSize', 20);
    
    % set the x-axis limit
    set( gca, 'XLim', [50 150] );

    grid on;
    hold on;
    
    if(runNumber == runTotal)
        legend('LQF-MWM', 'LQF-MWM2', 'Location', 'northwest');
    end        
    
%     subplot(2,2,runNumber+2);
%         
%     % waiting for vehicles
%     plot(timeSteps_D, cell2mat(delayDist{1,runNumber}), '-.x', 'LineWidth', 1, 'Color', 'k');
%     
%     hold on;
%     
%     plot(timeSteps_D, cell2mat(delayDist{2,runNumber}), 'LineWidth', 1, 'Color', 'k');
%     
%     % set font size
%     set(gca, 'FontSize', 20);
% 
%     xlabel('Time (min)', 'FontSize', 20);
%     ylabel('Vehicles Delay (s)', 'FontSize', 20);
%     
%     grid on;
%     legend('LQF-MWM', 'LQF-MWM2', 'Location', 'northwest');
end
