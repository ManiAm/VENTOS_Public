
% each column of 'delayAllEntity' represents the total delay for each entity

function [delayAllLanes, allLanes] = DelayPerLane(delayAllEntity, vehicleLane)

   allLanes = unique(vehicleLane);
   NumLanes = size(allLanes, 2);
   VehNumbers = size(vehicleLane, 2);
   allDelaysPerLane = cell(NumLanes, VehNumbers);
   index = zeros(1, NumLanes) + 1;
   
   for i=1:VehNumbers
       laneNumber = find(ismember(allLanes,vehicleLane(i)));
       allDelaysPerLane(laneNumber, index(laneNumber)) = num2cell(delayAllEntity(i));
       index(laneNumber) = index(laneNumber) + 1;
   end   
   
   delayAllLanes = zeros(NumLanes,3) - 1;
   
   for i=1:NumLanes
       delayAllLanes(i,1) = min(cell2mat(allDelaysPerLane(i,:)));
       delayAllLanes(i,2) = max(cell2mat(allDelaysPerLane(i,:)));
       delayAllLanes(i,3) = mean(cell2mat(allDelaysPerLane(i,:)));
   end
   
   fprintf('Delay per lane is: \n');
   for j=1:NumLanes
       fprintf('    %s : min=%0.2f, max=%0.2f, ave=%0.2f \n', char(allLanes(j)), delayAllLanes(j, 1), delayAllLanes(j, 2), delayAllLanes(j, 3));
   end

end
