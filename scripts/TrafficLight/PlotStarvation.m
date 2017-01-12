
function PlotStarvation(runNumber, timeSteps_D, delayDist, perClassFairness, perLaneFairness, runTotal)

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
    
    subplot(1,2,1);
    
    % max waiting for bikes
    plot(timeSteps_D, cell2mat(delayDist{3,runNumber}), lineMark, 'LineWidth', 1, 'Color', 'k');
    
    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel('Bike Waiting Time (s)', 'FontSize', 20);
    
    % set the axis limit
    set(gca, 'XLim', [50 130]);
    set(gca, 'YLim', [0 50]);

    grid on;
    hold on;
    
    if(runNumber == runTotal)
        legend('LQF-MWM', 'FMSC', 'Location', 'northwest');
    end    
    
    if(runNumber == runTotal)    
        subplot(1,2,2);
    
        b = bar([cell2mat(perClassFairness{2,1}), ...
            cell2mat(perClassFairness{2,2}) ; ...
            
            cell2mat(perLaneFairness{2,1}),  ...
            cell2mat(perLaneFairness{2,2})], 'k');       
        
        %b(1).BarWidth = 0.3;
        
        b(1).FaceColor = hsv2rgb(0,0,0);
        b(2).FaceColor = hsv2rgb(0,0,0.5);
        
        set(gca, 'XTickLabel', {'Inter-class'; 'Inter-lane'});
    
        % set the axis limit
        set(gca, 'YLim', [0.4 1]);
    
        % set font size
        set(gca, 'FontSize', 20);

        ylabel('Fairness Measure', 'FontSize', 20);
        
        legend('LQF-MWM', 'FMSC', 'Location', 'northeast');
    
        grid on;
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
