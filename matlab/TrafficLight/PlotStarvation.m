
function PlotStarvation(runNumber, timeSteps_MQ, delayDist, runTotal)

    % make a windows figure only once
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
    
    % max waiting for bikes
    plot(timeSteps_MQ, cell2mat(delayDist{3,:}), '-.', 'LineWidth', 1, 'Color', 'k');
    grid on;
    
    subplot(2,1,2);
        
    % waiting for vehicles
    plot(timeSteps_MQ, cell2mat(delayDist{1,:}), '-.x', 'LineWidth', 1, 'Color', 'k');
    
    hold on;
    grid on;
    
    plot(timeSteps_MQ, cell2mat(delayDist{2,:}), '-.v', 'LineWidth', 1, 'Color', 'k');

end