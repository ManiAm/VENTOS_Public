
% indexTS(1,vNumber): entrance time for vehicle vNumber        
% indexTS(2,vNumber): start of decel for vehicle vNumber
% indexTS(3,vNumber): start of stopping delay for vehicle vNumber
% indexTS(4,vNumber): crossing time for vehicle vNumber 
% indexTS(5,vNumber): start of accel for vehicle vNumber
% indexTS(6,vNumber): end of delay for vehicle vNumber

% NOTE: we do not count the acceleration delay

function [aveDelayPassenger, aveDelayEmergency, maxDelayBike, timeSteps_D] = DelayPerInterval(timeSteps, VehNumbers, vehicleType, indexTS, interval_delay)

indexCounter = 1;

rows = size(timeSteps, 1);
for j=1 : interval_delay-1 : rows-interval_delay
    
   startIndex = j;
   endIndex = startIndex + interval_delay - 1;
    
   startT = timeSteps(startIndex);
   endT = timeSteps(endIndex);
    
   vehPassengerCountDelayed = 0;
   vehPassengerCountNonDelayed = 0;
   vehPassengerTotalDelay = 0;
   
   vehEmergencyCountDelayed = 0;
   vehEmergencyCountNonDelayed = 0;
   vehEmergencyTotalDelay = 0;
   
   bikeMaxDelay = 0;
   pedMaxDelay = 0;

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
           
       if(indexTS(1,i) > endT)
           % veh entrance is after this interval
           % We ignore this vehicle   
           
       elseif(indexTS(2,i) == -1)
               % this vehicle does not slow down because of getting r or Y.
               % In other words it is getting G or g and has right of way.
               % NOTE: in SUMO a vehicle slows down on g, but this slowing down is not due to
               % red or yellow light thus, indexTS(2,i) is still -1 
               
               % if the vehicle crossess the intersection at this interval               
               if(indexTS(4,i) >= startT && indexTS(4,i) < endT)
                   if(mode == 1)
                       vehPassengerCountNonDelayed = vehPassengerCountNonDelayed + 1;
                   elseif(mode == 2)
                       vehEmergencyCountNonDelayed = vehEmergencyCountNonDelayed + 1;
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
                       vehPassengerCountDelayed = vehPassengerCountDelayed + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min(endT, getLastTime(timeSteps, indexTS(3,i), indexTS(5,i)));
                       vehPassengerTotalDelay = vehPassengerTotalDelay + (endDelay - startDelay); 
                   elseif(mode == 2)
                       vehEmergencyCountDelayed = vehEmergencyCountDelayed + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min(endT, getLastTime(timeSteps, indexTS(3,i), indexTS(5,i)));
                       vehEmergencyTotalDelay = vehEmergencyTotalDelay + (endDelay - startDelay); 
                   elseif(mode == 3)
                       % we only measure the waiting time for bikes
                       if(indexTS(3,i) ~= -1 && indexTS(3,i) < endT)
                           startDelay = indexTS(3,i);
                           endDelay = min(endT, getLastTime(timeSteps, -1, indexTS(5,i)));
                           bikeMaxDelay = max(bikeMaxDelay, (endDelay - startDelay));
                       end
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
           
               elseif(indexTS(2,i) < startT && (indexTS(4,i) == -1 || indexTS(4,i) >= startT))
                   % slowing down occured in a previous interval, but not crossed yet!                  
                   if(mode == 1)
                       vehPassengerCountDelayed = vehPassengerCountDelayed + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min(endT, getLastTime(timeSteps, indexTS(3,i), indexTS(5,i)));
                       vehPassengerTotalDelay = vehPassengerTotalDelay + (endDelay - startDelay); 
                   elseif(mode == 2)
                       vehEmergencyCountDelayed = vehEmergencyCountDelayed + 1;
                       startDelay = indexTS(2,i);
                       endDelay = min(endT, getLastTime(timeSteps, indexTS(3,i), indexTS(5,i)));
                       vehEmergencyTotalDelay = vehEmergencyTotalDelay + (endDelay - startDelay); 
                   elseif(mode == 3)
                       % we only measure the waiting time for bikes
                       last = getLastTime(timeSteps, -1, indexTS(5,i));
                       if(indexTS(3,i) ~= -1 && last >= startT)
                           startDelay = indexTS(3,i);
                           endDelay = min(endT, last);
                           bikeMaxDelay = max(bikeMaxDelay, (endDelay - startDelay));   
                       end
                   elseif(mode == 4)
                       % pedestrian
                       
                   end
               end
       end
   end

   % passenger vehicle
   totalCountVehPassenger = vehPassengerCountNonDelayed + vehPassengerCountDelayed;
   if(totalCountVehPassenger ~= 0)
       aveDelayPassenger(indexCounter) = vehPassengerTotalDelay / totalCountVehPassenger;
   else
       aveDelayPassenger(indexCounter) = 0;
   end
   
   % emergency vehicle
   totalCountVehEmergency = vehEmergencyCountNonDelayed + vehEmergencyCountDelayed;
   if(totalCountVehEmergency ~= 0)
       aveDelayEmergency(indexCounter) = vehEmergencyTotalDelay / totalCountVehEmergency;
   else
       aveDelayEmergency(indexCounter) = 0;
   end
   
   % bike
   maxDelayBike(indexCounter) = bikeMaxDelay;
   
   middleIndex = floor( double((startIndex + endIndex)) / 2. );
   timeSteps_D(indexCounter) = timeSteps(middleIndex);
    
   indexCounter = indexCounter + 1;

end

end


function last = getLastTime(timeSteps, startStopping, startAccel)

if(startAccel ~= -1)
    last = startAccel;
    return;
elseif(startAccel == -1 && startStopping ~= -1)
    last = startStopping;
    return;
elseif(startAccel == -1 && startStopping == -1)
    last = timeSteps(end);
    return;    
end

end
