
function [delayPassenger, delayEmergency, delayBike, timeSteps_D] = EntityDelay(timeSteps, VehNumbers, vehicleType, crossed, indexTS, interval_delay)

indexCounter = 1;

rows = size(timeSteps, 1);
for j=1 : interval_delay-1 : rows-interval_delay
    
   startIndex = j;
   endIndex = startIndex + interval_delay - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
    
   delayedCountVehPassenger = 0;
   nonDelayedCountVehPassenger = 0;
   totalDelayVehPassenger = 0;
   
   delayedCountVehEmergency = 0;
   nonDelayedCountVehEmergency = 0;
   totalDelayVehEmergency = 0;
   
   maxDelayBike = 0;
   maxDelayPed = 0;

   for i=1:VehNumbers
       
       % check the class of the current entity
       typeId = char(vehicleType(i)); 
       
       if(~isempty(strfind(typeId, 'passenger')))
           mode = 1;
       elseif(~isempty(strfind(typeId, 'emergency')))
           mode = 2;
       elseif(~isempty(strfind(typeId, 'bicycle')))
           mode = 3;
       elseif(~isempty(strfind(typeId, 'pedestrian')))
           mode = 4;
       else
           continue;
       end       
       
       if(crossed(i) == 0)
           % this vehicle does not cross the intersection at all
           % We ignore this vehicle
           
       elseif(indexTS(1,i) > endT)
           % veh entrance is after this interval
           % We ignore this vehicle   
           
       elseif(indexTS(2,i) == -1)
               % this vehicle does not slow down because of getting r or Y.
               % In other words it is getting G or g and has right of way.
               % NOTE: in SUMO a vehicle slows down on g, but this slowing down is not due to
               % red or yellow light thus, indexTS(2,i) is still -1 
               
               % we need to check if the vehicle crossess the intersection at this interval
               
               if(indexTS(4,i) >= startT && indexTS(4,i) < endT)
                   % this vehicle crosses the intersection at this interval
                   if(mode == 1)
                       nonDelayedCountVehPassenger = nonDelayedCountVehPassenger + 1;
                   elseif(mode == 2)
                       nonDelayedCountVehEmergency = nonDelayedCountVehEmergency + 1;
                   end
               end
           
       elseif(indexTS(2,i) ~= -1)
           % this vehicle slows down due to getting a r or y signal
           % we need to check if this slowing down happens in this period
           % or has happend in a previous interval.
           % NOTE: if slowing down happens after this interval we don't care
               
               if(indexTS(2,i) >= startT && indexTS(2,i) < endT)
                   % slowing down occurs in this interval     
                   if(mode == 1)
                       delayedCountVehPassenger = delayedCountVehPassenger + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehPassenger = totalDelayVehPassenger + (endDelay - startDelay); 
                   elseif(mode == 2)
                       delayedCountVehEmergency = delayedCountVehEmergency + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehEmergency = totalDelayVehEmergency + (endDelay - startDelay); 
                   elseif(mode == 3 && indexTS(3,i) ~= -1 && indexTS(3,i) < endT)
                       % bike
                       startDelay = indexTS(3,i);
                       endDelay = min( endT, indexTS(5,i) );
                       maxDelayBike = max(maxDelayBike, (endDelay - startDelay));
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
           
               elseif(indexTS(2,i) < startT && indexTS(4,i) >= startT)
                   % slowing down occured in a previous interval                  
                   if(mode == 1)
                       delayedCountVehPassenger = delayedCountVehPassenger + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehPassenger = totalDelayVehPassenger + (endDelay - startDelay); 
                   elseif(mode == 2)
                       delayedCountVehEmergency = delayedCountVehEmergency + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min( endT, indexTS(5,i) );
                       totalDelayVehEmergency = totalDelayVehEmergency + (endDelay - startDelay); 
                   elseif(mode == 3 && indexTS(3,i) ~= -1 && indexTS(5,i) >= startT)
                       % bike
                       startDelay = max( startT, indexTS(3,i) );
                       endDelay = min( endT, indexTS(5,i) );
                       maxDelayBike = max(maxDelayBike, (endDelay - startDelay));                       
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
               end
       end
   end

   % passenger vehicle
   totalCountVehPassenger = nonDelayedCountVehPassenger + delayedCountVehPassenger;
   if(totalCountVehPassenger ~= 0)
       delayPassenger(indexCounter) = totalDelayVehPassenger / totalCountVehPassenger;
   else
       delayPassenger(indexCounter) = 0;
   end
   
   % emergency vehicle
   totalCountVehEmergency = nonDelayedCountVehEmergency + delayedCountVehEmergency;
   if(totalCountVehEmergency ~= 0)
       delayEmergency(indexCounter) = totalDelayVehEmergency / totalCountVehEmergency;
   else
       delayEmergency(indexCounter) = 0;
   end
   
   % bike
   delayBike(indexCounter) = maxDelayBike;
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_D(indexCounter) = timeSteps(middleIndex);
    
   indexCounter = indexCounter + 1;

end

end