
function [stDeviation, jain] = FairnessIndex(delayGroups)

       % calculate standard deviation
       stDeviation = std(delayGroups);
       fprintf('Standard deviation is %0.2f \n', stDeviation);
       
       % calculate jain's fairness index
       
       %delayGroups = 1 / delayGroups;
       
       top = sum(delayGroups)^2;
       n = size(delayGroups,1);

       S = 0;
       for i=1:n
           S = S + (delayGroups(i))^2;
       end
   
       jain = top / (n * S);
       fprintf('Jains fairness index is %0.2f \n', jain);
    
end
