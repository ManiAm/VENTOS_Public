
function PlotLQFMWM(runNumber, timeSteps_MQ, maxQueueSize, timeSteps_D, delayPassenger, aveDelayEmergency, delayDist, runTotal)

    persistent fig_han;

    if(runNumber == 1)
        fig_han = figure('name', 'Speed', 'units', 'normalized', 'outerposition', [0 0 1 1]);
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
     
    subplot(3,2,[1,2]);
    plot(timeSteps_MQ/60, maxQueueSize, lineMark, 'LineWidth', 1, 'Color', 'k');

    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Vehicles Max Queue' , 'Size per Lane'}, 'FontSize', 20);

    grid on;
    hold on;
    
    if(runNumber == runTotal)    
        legend('LQF\_MWM (I)' , 'LQF\_MWM\_Aging (II)', 'FMSC (III)', 'Location', 'northwest');
        
        % set the x-axis limit
        set( gca, 'XLim', [0 3700/60] );
            
        % x-axis is integer
        Xlimit = get(gca,'xlim');            
        set(gca, 'xtick' , 0:3:Xlimit(2));
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:20:Ylimit(2));
    end
        
    
    subplot(3,2,3);        
    plot(timeSteps_D/60, delayPassenger/60, lineMark, 'LineWidth', 1, 'Color', 'k');
    
    annotation(fig_han,'textbox',...
    [0.138 0.573 0.118 0.041],...
    'String', {'Private Vehicles'}, 'FitBoxToText','on', 'LineStyle', 'none', 'FontSize', 15);
        
    % set font size
    set(gca, 'FontSize', 20);

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Ave Delay', '(min)'}, 'FontSize', 20);

    grid on;
    hold on;
        
    if(runNumber == runTotal) 
        % set the x-axis limit
        set( gca, 'XLim', [0 3700/60] );
            
        % x-axis is integer
        Xlimit = get(gca,'xlim');            
        set(gca, 'xtick' , 0:5:Xlimit(2));
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:4:Ylimit(2));
    end    
    
    
    
    subplot(3,2,4);        
    plot(timeSteps_D/60, aveDelayEmergency/60, lineMark, 'LineWidth', 1, 'Color', 'k');
    
    annotation(fig_han,'textbox',...
    [0.588 0.577 0.126 0.041],...
    'String', {'Service Vehicles'}, 'FitBoxToText', 'on', 'LineStyle', 'none', 'FontSize', 15);
        
    % set font size
    set(gca, 'FontSize', 20);
    %set(gca, 'YAxisLocation', 'right');

    xlabel('Time (min)', 'FontSize', 20);
    ylabel({'Ave Delay', '(min)'}, 'FontSize', 20);

    grid on;
    hold on;
        
    if(runNumber == runTotal) 
        % set the x-axis limit
        set( gca, 'XLim', [0 3700/60] );
            
        % x-axis is integer
        Xlimit = get(gca,'xlim');            
        set(gca, 'xtick' , 0:5:Xlimit(2));
        
        % Y-axis for delay          
        Ylimit = get(gca,'ylim');            
        set(gca, 'ytick' , 0:4:Ylimit(2));
    end 
    
    
    
    subplot(3,2,5); 
        
    if(runNumber == runTotal)        
        % distribution of average veh delay        
        data = [cell2mat(delayDist{1,1})'; cell2mat(delayDist{1,2})'; cell2mat(delayDist{1,3})'; 
                cell2mat(delayDist{2,1})'; cell2mat(delayDist{2,2})'; cell2mat(delayDist{2,3})'];

        n1 = size(delayDist{1,1},2);   % passenger
        n5 = size(delayDist{2,1},2);   % emergency
        
        n2 = size(delayDist{1,2},2);   % passenger
        n6 = size(delayDist{2,2},2);   % emergency
        
        n3 = size(delayDist{1,3},2);   % passenger
        n4 = size(delayDist{2,3},2);   % emergency
        
        group = [repmat({'I (P)'}, n1, 1); 
                 repmat({'I (S)'}, n5, 1);
                 repmat({'II (P)'}, n2, 1);  
                 repmat({'II (S)'}, n6, 1);
                 repmat({'III (P)'}, n3, 1);  
                 repmat({'III (S)'}, n4, 1)];

        boxplot(data, group, 'colors', 'k');
        set(gca, 'XTickLabelRotation', 40);

        set(findobj(gca,'Type','text'), 'FontSize', 20);
        
        % set font size
        set(gca, 'FontSize', 20);

        ylabel({'Vehicles Ave Delay', 'Distribution (min)'}, 'FontSize', 20);

        grid on;   
        
    end
    

    subplot(3,2,6);
        
    if(runNumber == runTotal)
        % distribution of max bike delay       
        data = [cell2mat(delayDist{3,1})'; 
                cell2mat(delayDist{3,2})';
                cell2mat(delayDist{3,3})'];

        n1 = size(delayDist{3,1},2);
        n2 = size(delayDist{3,2},2);
        n3 = size(delayDist{3,3},2);
        group = [repmat({'I'}, n1, 1); repmat({'II'}, n2, 1); repmat({'III'}, n3, 1)];

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
            subplot(3,2,[1,2]);
            
            Ylimit = get(gca,'ylim');
            arrowYLoc = Ylimit(2) + (0.05 * Ylimit(2));
            textYLoc = arrowYLoc + (0.2 * arrowYLoc);
            
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
            
            
            % mark light/medium/heavy traffic with arrows
            subplot(3,2,3);
            
            Ylimit = get(gca,'ylim');
            arrowYLoc = Ylimit(2) + (0.05 * Ylimit(2));
            textYLoc = arrowYLoc + (0.2 * arrowYLoc);
            
            Start = [0 arrowYLoc];
            Stop = [800/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3);  
            text((400-150)/60, textYLoc, 'Light', 'FontSize', 15);
            
            Start = [800/60 arrowYLoc];
            Stop = [1600/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((1200-150)/60, textYLoc, 'Medium', 'FontSize', 15);
            
            Start = [1600/60 arrowYLoc];
            Stop = [2400/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2000-150)/60, textYLoc, 'Heavy', 'FontSize', 15);
            
            Start = [2400/60 arrowYLoc];
            Stop = [3200/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2800-150)/60, textYLoc, 'Medium', 'FontSize', 15);
            
            Start = [3200/60 arrowYLoc];
            Stop = [Xlimit(2) arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((3470-150)/60, textYLoc, 'Light', 'FontSize', 15);
            
            
            % mark light/medium/heavy traffic with arrows
            subplot(3,2,4);
            
            Ylimit = get(gca,'ylim');
            arrowYLoc = Ylimit(2) + (0.05 * Ylimit(2));
            textYLoc = arrowYLoc + (0.2 * arrowYLoc);
            
            Start = [0 arrowYLoc];
            Stop = [800/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3);  
            text((400-150)/60, textYLoc, 'Light', 'FontSize', 15);
            
            Start = [800/60 arrowYLoc];
            Stop = [1600/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((1200-150)/60, textYLoc, 'Medium', 'FontSize', 15);
            
            Start = [1600/60 arrowYLoc];
            Stop = [2400/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2000-150)/60, textYLoc, 'Heavy', 'FontSize', 15);
            
            Start = [2400/60 arrowYLoc];
            Stop = [3200/60 arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((2800-150)/60, textYLoc, 'Medium', 'FontSize', 15);
            
            Start = [3200/60 arrowYLoc];
            Stop = [Xlimit(2) arrowYLoc];
            arrow(Start, Stop, 'Ends', 3); 
            text((3470-150)/60, textYLoc, 'Light', 'FontSize', 15);
    end   
    
end
