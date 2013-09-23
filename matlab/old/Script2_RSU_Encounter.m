clear all;
fileName3 = '../results/2.RSUencounter-84 nodes.txt';

% -------------------------------------------------------------------

[part1, part2] = textread(fileName3,'%s %s', 'delimiter', ';', 'headerlines', 2);

[rows,~] = size(part1);

for i=1:rows
    
    str = char(part1(i));   
    
    if(~isempty(strfind(str,'vehicle')))        
        RSUs(i,1) = str2double( char( part2(i) ) );
    end
end

disp(RSUs);

fprintf('Mean: %d \nStandard Deviation: %d \n', mean(RSUs) ,std(RSUs));

histfit(RSUs);

set(gca,'FontSize',13)

% set the axis labels
xlabel('Number of RSUs encountered', 'FontSize', 13);
ylabel('Number of vehicles', 'FontSize', 13);

print('-dpng', '-r300', 'figure2');

