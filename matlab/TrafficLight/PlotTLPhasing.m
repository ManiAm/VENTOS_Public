function PlotTLPhasing(runNumber, timeSteps_SW, totalCycles, timeSteps_GR, totalGreenTime, runTotal)

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
    plot(timeSteps_SW/60, totalCycles, lineMark, 'LineWidth', 1, 'Color', 'k');

    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel('Cycle Count', 'FontSize', 20);

    grid on;
    hold on;
    
    subplot(2,1,2);
    plot(timeSteps_GR/60, totalGreenTime/60, lineMark, 'LineWidth', 1, 'Color', 'k');
            
    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel('Total Green Time (min)', 'FontSize', 20);

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
        legend('Fix-time' , 'Traffic-actuated', 'LQF', 'OJF', 'Location', 'northwest');
    
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
            text((400-150)/60, textYLoc, 'Light Traffic', 'FontSize', 20);
            
            Start = [800/60 arrowYLoc];
            Stop = [1600/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((1200-150)/60, textYLoc, 'Medium Traffic', 'FontSize', 20);
            
            Start = [1600/60 arrowYLoc];
            Stop = [2400/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2000-150)/60, textYLoc, 'Heavy Traffic', 'FontSize', 20);
            
            Start = [2400/60 arrowYLoc];
            Stop = [3200/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2800-150)/60, textYLoc, 'Medium Traffic', 'FontSize', 20);
            
            Start = [3200/60 arrowYLoc];
            Stop = [Xlimit(2) arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((3400-150)/60, textYLoc, 'Light Traffic', 'FontSize', 20);
        end 
    
    end
end
