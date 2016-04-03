
% indexTS(1,vNumber): entrance time for vehicle vNumber        
% indexTS(2,vNumber): start of decel for vehicle vNumber
% indexTS(3,vNumber): start of stopping delay for vehicle vNumber
% indexTS(4,vNumber): crossing time for vehicle vNumber 
% indexTS(5,vNumber): start of accel for vehicle vNumber
% indexTS(6,vNumber): end of delay for vehicle vNumber

function delayAllEntity = DelayPerEntity(timeSteps, indexTS)

   VehNumbers = size(indexTS, 2);
   delayAllEntity = zeros(4,VehNumbers) - 1;
   
   for i=1:VehNumbers
       
       % this vehicle never decelerates
       if(indexTS(2,i) == -1)
           delayAllEntity(1,i) = 0;  % decelerating delay is 0
           delayAllEntity(2,i) = 0;  % stoping delay is 0
           delayAllEntity(3,i) = 0;  % accelerating delay is 0
           
       % this vehicle stops
       elseif(indexTS(3,i) ~= -1)
           delayAllEntity(1,i) = indexTS(3,i) - indexTS(2,i);  % decelerating delay
           
           % now we calculate stopping delay
           if(indexTS(5,i) ~= -1)
               delayAllEntity(2,i) = indexTS(5,i) - indexTS(3,i);
           else
               % acceleration startT is not known!
               delayAllEntity(2,i) = timeSteps(end) - indexTS(3,i);
           end
           
           % now we calculate acceleration delay
           if(indexTS(5,i) ~= -1)
               if(indexTS(6,i) ~= -1)
                   delayAllEntity(3,i) = indexTS(6,i) - indexTS(5,i);
               else
                   % end of delay is not known!
                   delayAllEntity(3,i) = timeSteps(end) - indexTS(5,i);
               end
           else
               delayAllEntity(3,i) = 0;
           end
           
       % this vehicle does not stop
       elseif(indexTS(3,i) == -1)
           delayAllEntity(2,i) = 0;  % stoping delay is 0
           
           % now we calculate deceleration delay
           if(indexTS(5,i) ~= -1)           
               delayAllEntity(1,i) = indexTS(5,i) - indexTS(2,i);
           else
               delayAllEntity(1,i) = timeSteps(end) - indexTS(2,i);
           end
           
           % now we calculate acceleration delay
           if(indexTS(5,i) == -1)
               delayAllEntity(3,i) = 0;
           else
               if(indexTS(6,i) ~= -1)
                   delayAllEntity(3,i) = indexTS(6,i) - indexTS(5,i);
               else
                   % end of delay is not known!
                   delayAllEntity(3,i) = timeSteps(end) - indexTS(5,i);
               end
           end
       end
       
       if(delayAllEntity(1,i) < 0 || delayAllEntity(2,i) < 0 || delayAllEntity(3,i) < 0)
           error('delay is negative!');
       end
       
       % total delay is the summation of all delays
       % NOTE: we DO NOT consider the acceleration delay
       delayAllEntity(4,i) = delayAllEntity(1,i) + delayAllEntity(2,i);
   end

end
