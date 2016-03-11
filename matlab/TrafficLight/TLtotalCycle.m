function [timeSteps_SW, totalCycles] = TLtotalCycle(timeSteps, phaseDurationTS, cycleNumber, search_interval)

rows = size(timeSteps, 1);
index = 1;

for j=1 : search_interval-1 : rows-search_interval
    
   startIndex = j;
   endIndex = startIndex + search_interval - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
   
   % find the last phase in this interval
   result = find(phaseDurationTS(4,:) <= endT);
   phaseNumber = result(end);
   
   totalCycles(index) = cycleNumber(phaseNumber);
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_SW(index) = timeSteps(middleIndex);
   
   index = index + 1;
   
end

end