
function PlotBenefitOfActiveDetection (runNumber, timeSteps_MQ, maxQueueSize, timeSteps_D, delayPassenger, timeSteps_T, throughput, runTotal)

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
       
    subplot(3,1,1);
    plot(timeSteps_MQ/60, maxQueueSize, lineMark, 'LineWidth', 1, 'Color', 'k');

    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Vehicles Max Queue' , 'Size per Lane'}, 'FontSize', 20);

    grid on;
    hold on;
    
    if(runNumber == runTotal)        
        legend('Adaptive Webster (active)', 'Traffic-actuated (active)' , 'Traffic-actuated', 'LQF', 'LQF (active)', 'Location', 'northwest');

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

    subplot(3,1,3); 
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
        for g=1:3
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