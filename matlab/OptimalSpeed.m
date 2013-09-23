clear all;
clc;

% -------------------------------------------------------------------

index = 0;

for g=0:0.5:70
    index = index + 1;
    v(index,1) = max( 0 , min( (g-2)/1.64 , 30 ) );
    v(index,2) = max( 0 , min( (g-2)/1.4 , 30 ) );
    v(index,3) = max( 0 , min( (g-2)/0.7 , 30 ) );
end

figure(1);
handle1 = plot(0:0.5:70,v(:,1),'LineWidth', 3);

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Gap (m)', 'FontSize', 19);
ylabel('Speed (m/s)', 'FontSize', 19);

grid on;
 
% save the figure as a png file
set(gcf, 'PaperPositionMode', 'auto'); 
print('-dpng', '-r300', 'figurex1');

% --------------------------------------------------------------------

figure(2);
handle1 = plot(0:0.5:70,v,'LineWidth', 3);

% set font size
set(gca, 'FontSize', 19);

% set the axis labels
xlabel('Gap (m)', 'FontSize', 19);
ylabel('Speed (m/s)', 'FontSize', 19);

grid on;

 % set the name for each line
 set(handle1(1),'Displayname', 'Manual');
 set(handle1(2),'Displayname', 'ACC');
 set(handle1(3),'Displayname', 'CACC');

 % set the legend
 legend(handle1, 'Location','NorthEastOutside');
 
 % save the figure as a png file
 set(gcf, 'PaperPositionMode', 'auto'); 
 print('-dpng', '-r300', 'figurex2');

 
 
 
