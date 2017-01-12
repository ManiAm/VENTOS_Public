
function [averageQueueSize, timeSteps_Q] = AveVehQueueSize(laneCounts, totalQs, timeSteps, aggregateInterval_queue)

% out of 16, 4 are crosswalks and 4 are bike lanes
laneCounts = laneCounts - 8;

% average queue size in each time step
averageQueueSize_all = double(totalQs) ./ double(laneCounts);

rows = size(averageQueueSize_all, 1);
index = 1;

for i=1 : aggregateInterval_queue : rows-aggregateInterval_queue
    
    startIndex = i;
    endIndex = startIndex + aggregateInterval_queue - 1;
    
    averageQueueSize(index) = double(sum(averageQueueSize_all(startIndex:endIndex))) / double(aggregateInterval_queue);  
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_Q(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end

end