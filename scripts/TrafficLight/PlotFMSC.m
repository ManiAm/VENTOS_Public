
function PlotFMSC(runNumber, timeSteps_MQ, maxQueueSize, delayDist, runTotal)

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
    else
        lineMark = '--';
    end
     
    subplot(2,2,[1,2]);
    plot(timeSteps_MQ/60, maxQueueSize, lineMark, 'LineWidth', 1, 'Color', 'k');

    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Vehicles Max Queue' , 'Size per Lane'}, 'FontSize', 20);

    grid on;
    hold on;
    
    if(runNumber == runTotal)    
        legend('OJF' , 'LQF\_MWM2', 'Location', 'northwest');
        
        % set the x-axis limit
        set( gca, 'XLim', [0 3700/60] );
            
        % x-axis is integer
        Xlimit = get(gca,'xlim');            
        set(gca, 'xtick' , 0:3:Xlimit(2));
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:20:Ylimit(2));
    end
        
    subplot(2,2,3); 
        
    if(runNumber == runTotal)        
        % distribution of average veh delay        
        data = [cell2mat(delayDist{1,1})'; cell2mat(delayDist{1,2})'; 
                cell2mat(delayDist{2,1})'; cell2mat(delayDist{2,2})'];

        n1 = size(delayDist{1,1},2);   % passenger
        n5 = size(delayDist{2,1},2);   % emergency
        
        n2 = size(delayDist{1,2},2);   % passenger
        n6 = size(delayDist{2,2},2);   % emergency
        
        group = [repmat({'OJF (P)'}, n1, 1); 
                 repmat({'OJF (S)'}, n5, 1);
                 repmat({'LQF_MWM2 (P)'}, n2, 1);  
                 repmat({'LQF_MWM2 (S)'}, n6, 1)];

        boxplot(data, group, 'colors', 'k');
        set(gca, 'XTickLabelRotation', 25);

        set(findobj(gca,'Type','text'), 'FontSize', 20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Vehicles Ave Delay','Distribution (min)'}, 'FontSize', 20);

        grid on;   
        
    end

    subplot(2,2,4);
        
    if(runNumber == runTotal)
        % distribution of max bike delay       
        data = [cell2mat(delayDist{3,1})'; 
                cell2mat(delayDist{3,2})'];

        n1 = size(delayDist{3,1},2);
        n2 = size(delayDist{3,2},2);
        group = [repmat({'OJF'}, n1, 1); repmat({'LQF_MWM2'}, n2, 1)];

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
            subplot(2,2,[1,2]);
            
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
