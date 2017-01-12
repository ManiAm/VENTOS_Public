
function [throughput, timeSteps_T] = TLthroughput(timeSteps, VehNumbers, indexTS, interval_throughput)

rows = size(timeSteps, 1);
index = 1;

for i=1 : interval_throughput-1 : rows-interval_throughput
    
    startIndex = i;
    endIndex = startIndex + interval_throughput - 1;
    
    startT = timeSteps(startIndex);
    endT = timeSteps(endIndex);
    
    vehCounter = 0;

    for j=1:VehNumbers       
        if( indexTS(4,j) ~= -1 && indexTS(4,j) >= startT && indexTS(4,j) < endT )
            vehCounter = vehCounter + 1;
        end       
    end 
    
    throughput(index) = vehCounter;
    
    middleIndex = floor( double((startIndex + endIndex)) / 2. );
    timeSteps_T(index) = timeSteps(middleIndex);
    
    index = index + 1;
    
end

end