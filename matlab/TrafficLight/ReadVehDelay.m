
function [entityIDs, crossed, indexTS, VehNumbers, vehicleType] = ReadVehDelay(basePATH, name)

path = sprintf('%s/%s', basePATH, name);
file_id = fopen(path);
formatSpec = '%s %s %s %s %f %d %f %f %f %f %f %f';
C_text = textscan(file_id, formatSpec, 'HeaderLines', 8);
fclose(file_id);

vehicles = C_text{1,1};
vehiclesType = C_text{1,2};
entrance = C_text{1,5};
crossed = C_text{1,6};
startDecel = C_text{1,8};
startStopping = C_text{1,9};
crossedTime = C_text{1,10};
startAccel = C_text{1,11};
endDelay = C_text{1,12};

% stores the ids of all entities
entityIDs = unique(vehicles);
entityIDs = sort_nat(entityIDs);
VehNumbers = size(entityIDs,1);

[rows,~] = size(vehicles);

% preallocating and initialization
indexTS = zeros(6,rows) - 1;

for i=1:rows 
    
    % get the current vehicle name
    vehicle = char(vehicles(i,1));        
    vNumber = find(ismember(entityIDs,vehicle)); 
    
    vehicleType(vNumber) = (vehiclesType(i,1));

    indexTS(1,vNumber) = double(entrance(i,1));       
    indexTS(2,vNumber) = double(startDecel(i,1));
    indexTS(3,vNumber) = double(startStopping(i,1));
    indexTS(4,vNumber) = double(crossedTime(i,1));
    indexTS(5,vNumber) = double(startAccel(i,1));
    indexTS(6,vNumber) = double(endDelay(i,1));
    
    if(double(startAccel(i,1)) == -1 && double(startDecel(i,1)) ~= -1)
        error('startAccel can not be -1');
    end
    
    if(crossed(i) == 1 && double(crossedTime(i,1)) == -1)
        error('crossedTime can not be -1');
    end

end

fprintf( 'totalEntity: %d, crossed: %d, noSlowDown: %d, noWaiting: %d \n', VehNumbers, sum(crossed(:,1) == 1), sum(indexTS(2,:) == -1), sum(indexTS(3,:) == -1) );

end
