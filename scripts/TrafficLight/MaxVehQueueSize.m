
function [maxQueueSize, timeSteps_MQ] = MaxVehQueueSize(maxQs, timeSteps, aggregateInterval_queue)

rows = size(maxQs, 1);
index = 1;

for i=1 : aggregateInterval_queue : rows-aggregateInterval_queue
    
    startIndex = i;
    endIndex = startIndex + aggregateInterval_queue - 1;
    
    maxQueueSize(index) = double(sum(maxQs(startIndex:endIndex))) / double(aggregateInterval_queue);  
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_MQ(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end

end