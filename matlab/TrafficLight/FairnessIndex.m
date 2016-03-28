
% delayAllEntity(1,vNumber): decelerating delay for vehicle vNumber
% delayAllEntity(2,vNumber): stoping delay for vehicle vNumber
% delayAllEntity(3,vNumber): accelerating delay for vehicle vNumber
% delayAllEntity(4,vNumber): total delay for vehicle vNumber

function [jain, stDeviation] = FairnessIndex(delayAllEntity, vehicleType)

       classes = unique(vehicleType);
       NoClasses = size(classes, 2);
       VehNumbers = size(delayAllEntity, 2);
       GroupDelayByClass = zeros(NoClasses, VehNumbers) - 1;
       index = zeros(NoClasses) + 1;
       
       for i=1:VehNumbers           
           classNum = find(ismember(classes, vehicleType(i)));
           GroupDelayByClass(classNum, index(classNum)) = delayAllEntity(4,i);
           index(classNum) = index(classNum) + 1;
       end
       
       AveDelayByClass = zeros(NoClasses,1) - 1;
       
       for i=1:NoClasses
           validEntries = GroupDelayByClass(GroupDelayByClass(i,:) >= 0);
           AveDelayByClass(i) = mean(validEntries);
       end

       fprintf('Average delay per class is: \n');
       for j=1:NoClasses
           fprintf('    %s : %f \n', char(classes(j)), AveDelayByClass(j));
       end
       
       % calculate jain's fairness index
       
       %AveDelayByClass = 1 / AveDelayByClass;
       
       top = sum(AveDelayByClass)^2;

       S = 0;
       for i=1:NoClasses
           S = S + (AveDelayByClass(i))^2;
       end
   
       jain = top / (NoClasses * S);
       fprintf('Jains fairness index is %f \n', jain);
       
       % calculate standard deviation
       stDeviation = std(AveDelayByClass);
       fprintf('Standard deviation is %f \n', stDeviation);
       
end
