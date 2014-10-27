clc;

syms Q G_min V N Tg Tp Lv;

V = 20;      % speed of the platoon 20 m/s = 72 km/h = 44.7 mph
Tp = 3.5;    % time gap between platoons 3.5 s
Lv = 5;      % vehicle length 5 m
G_min = 2;   % minimum space gap

Q = ( (V*N) / ( N*(Lv + G_min) + (N-1)*Tg*V + Tp*V) ) * 3600;

handle = figure(1);
hold on;

counter = 1;

for i=[0.55 0.6 0.65 0.7]
   p = subs(Q, Tg, i);
   h = ezplot(p, [1 , 20]);
   set(h,'LineWidth',3);
   
   if(counter == 1)
      set(h, 'LineStyle', '-');
      set(h,'Displayname', 'T_g = 0.55 s');
   elseif(counter == 2)
      set(h, 'LineStyle', '--');
      set(h,'Displayname', 'T_g = 0.6 s');
   elseif(counter == 3)
      set(h, 'LineStyle', '-.');
      set(h,'Displayname', 'T_g = 0.65 s');
   elseif(counter == 4)
      set(h, 'LineStyle', ':');
      set(h,'Displayname', 'T_g = 0.7 s');
   end
   
   counter = counter + 1;
end

grid on;
set( gca, 'XLim', [0 25] );
set( gca, 'YLim', [0 4000] );

% set font size
set(gca, 'FontSize', 19);

title('');
xlabel('Platoon Size N', 'FontSize', 19);
ylabel('Throughput Q (veh/h)', 'FontSize', 19);

handle = legend('Location','SouthEast');
set(handle, 'FontSize', 19);

set(gcf, 'PaperPositionMode', 'auto'); 
print('-dpng', '-r300', 'fig1');

