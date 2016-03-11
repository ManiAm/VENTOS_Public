
function [timeSteps_GR, totalGreenTime] = TLtotalGreenTime(timeSteps, totalPhases, phaseDurationTS, search_interval2)

rows = size(timeSteps, 1);
index = 1;
greenPortion = 0;

for j=1 : search_interval2-1 : rows-search_interval2
    
   startIndex = j;
   endIndex = startIndex + search_interval2 - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
   %greenPortion = 0;
   
   for i=1:totalPhases       
       greenStart = phaseDurationTS(1,i);
       greenEnd = phaseDurationTS(2,i);
       
       if(greenStart > endT || greenEnd < startT)
           continue;
       end
       
       if(greenStart ~= -1 && greenEnd ~= -1)
           ST = max(greenStart, startT);
           EN = min(greenEnd, endT);
           greenPortion = greenPortion + (EN - ST);                         
       end
       
   end 
   
   totalGreenTime(index) = greenPortion;
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_GR(index) = timeSteps(middleIndex);
   
   index = index + 1;   
   
end

end