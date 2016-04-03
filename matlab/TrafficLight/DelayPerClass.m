
% delayAllEntity(1,vNumber): decelerating delay for vehicle vNumber
% delayAllEntity(2,vNumber): stoping delay for vehicle vNumber
% delayAllEntity(3,vNumber): accelerating delay for vehicle vNumber
% delayAllEntity(4,vNumber): total delay for vehicle vNumber

function [delayAllClass, allClasses] = DelayPerClass(delayAllEntity, vehicleType)

       allClasses = unique(vehicleType);
       NoClasses = size(allClasses, 2);
       VehNumbers = size(delayAllEntity, 2);
       GroupDelayByClass = cell(NoClasses, VehNumbers);
       index = zeros(1,NoClasses) + 1;
       
       for i=1:VehNumbers           
           classNum = find(ismember(allClasses, vehicleType(i)));
           GroupDelayByClass(classNum, index(classNum)) = num2cell(delayAllEntity(4,i));
           index(classNum) = index(classNum) + 1;
       end
       
       delayAllClass = zeros(NoClasses,3) - 1;
       
       for i=1:NoClasses
           validEntries = cell2mat(GroupDelayByClass(i,:));
           
           delayAllClass(i,1) = min(validEntries);
           delayAllClass(i,2) = max(validEntries);
           delayAllClass(i,3) = mean(validEntries);
       end     
       
       fprintf('Delay per class is: \n');
       for j=1:NoClasses
           fprintf('    %s : min=%0.2f, max=%0.2f, ave=%0.2f \n', char(allClasses(j)), delayAllClass(j, 1), delayAllClass(j, 2), delayAllClass(j, 3));
       end
       
end
