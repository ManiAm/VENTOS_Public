clear all;
fileName3 = '../results/1.SnapShots-10 nodesS.txt';

NoPieces = 20;

% -------------------------------------------------------------------

[part1, part2] = textread(fileName3,'%s %s', 'delimiter', ';');

[rows,~] = size(part1);

snapshots = 0;
run = zeros(100, NoPieces) - 1;    % initialized to -1

for i=1:rows
    
    str = char(part1(i));
    
    if(strcmp(str,'SnapShot at t') == 1)
        
        snapshots = snapshots + 1;
        time(1,snapshots) = str2double(char(part2(i)));
    
    elseif(strcmp(str,'Pieces of vehicle 0') == 1) 
        
        remain = char( part2(i) );
        [~,n] = size(remain);  % n == 0, for loop does not run
        
        % start tockenizing
        for j=1:n
            [str, remain] = strtok(remain, ',');
            if(strcmp(str,'') == 1) 
                break;
            end
            run(snapshots, str2double(str)+1) = str2double(str);            
        end
        
    elseif(strcmp(str,'end of section') == 1)        
        % display some info
        fprintf('SnapShot at time %d:\n', time(1,snapshots));        
        fprintf('pieces of vehicle 0: ');     
       
        for h=1:NoPieces          
            if(run(snapshots,h) ~= -1)
                 fprintf('%d,', run(snapshots,h));               
            end        
        end
        
        fprintf('\n\n');
        
    end
end


% dimention. one Z for each snapshot
Z = zeros(snapshots, NoPieces, snapshots);  

for d=1:snapshots
    for k=1:NoPieces
        Z(d, k, d) = 1;        
    end
    
    bar_handle = bar(Z(:,:,d), 'stacked');

    for j=1:NoPieces        
        if(run(d,j) ~= -1)
            set(bar_handle(j),'facecolor','black');
            set(bar_handle(j),'edgecolor','white');
        else
            set(bar_handle(j),'facecolor','white');
            set(bar_handle(j),'edgecolor','black');
        end        
    end

    hold on;
end

set(gca, 'YTick', (0:NoPieces-1), 'FontSize', 13);
set(gca, 'XTick', (1:snapshots), 'FontSize', 13);

Xlables = cell(1,snapshots);

for j=1:6:snapshots
   Xlables{1,j} = time(1,j); 
end

set(gca,'XTickLabel',Xlables);

% set the axis labels
xlabel('Time of SnapShots (second)', 'FontSize', 13);
ylabel('Piece Number', 'FontSize', 13);

% set the title of the figure
title('List of possessed CRL pieces by a node', 'FontWeight', 'bold', 'FontSize', 13);

print('-dpng', '-r300', 'figure1');

