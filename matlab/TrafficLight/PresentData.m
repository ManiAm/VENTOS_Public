function PresentData (vIDs, crossed, indexTS)
    
    cell1 = vIDs;
    cell2 = num2cell(crossed(:,1));
    cell3 = [ num2cell(indexTS(1,:)') num2cell(indexTS(2,:)') num2cell(indexTS(3,:)') num2cell(indexTS(4,:)') num2cell(indexTS(5,:)') num2cell(indexTS(6,:)') ];

    data = [ cell1 cell2 cell3 ]';

    f = figure();

    t = uitable('Parent', f, 'Units', 'normalized', 'Position', [0.08 0.6 0.85 0.3], 'Data', data );
    t.Data = data;
    t.RowName = {'Veh ID', 'crossed?', 'entrance', 'deccel start', 'stopping start', 'cross time', 'accel start', 'end delay'}; 

end