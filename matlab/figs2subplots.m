function newfig = figs2subplots(handles,tiling,arr)
% FIGS2SUBLPLOTS Combine axes in many figures into subplots in one figure
%
%   The syntax:
%
%       >> newfig = figs2subplots(handles,tiling,arr);
%   
%   creates a new figure with handle "newfig", in which the axes specified
%   in vector "handles" are reproduced and aggregated as subplots. 
%
%   Vector "handles" is a vector of figure and/or axes handles. If an axes
%   handle is encountered, the corresponding axes is simply reproduced as
%   a subplot in the new figure; if a figure handle is encountered, all its
%   children axes are reproduced as subplots in the figure.
%
%   Vector "tiling" is an optional subplot tiling vector of the form 
%   [M N], where M and N specify the number of rows and columns for the
%   subplot tiling. M and N correspond to the first two arguments of the
%   SUBPLOT command. By default, the tiling is such that all subplots are
%   stacked in a column.
%
%   Cell array "arr" is an optional subplot arrangement cell array. For
%   the k-th axes handle encountered, the subplot command issued is
%   actually:
%
%       subplot(tiling(1),tiling(2),arr{k})
%
%   By default, "arr" is a cell array {1,2,...}, which means that each axes
%   found in the figures is reproduced in a neatly tiled grid.
%
%   Example:
%
%       figs2subplots([a1 a2 a3],[2 2],{[1 3],2,4})
%
%   copies the three axes a1, a2 and a3 as subplots in a new figure with a 
%   2x2 tiling arangement. Axes a1 will be reproduced as a subplot 
%   occupying tiles 1 and 3 (thus covering the left part of the figure), 
%   while axes a2 will be reproduced as a subplot occupying tile 2 (upper
%   right corner) and a3 occupying tile 4 (lower right corner).

%   Original version by François Bouffard (fbouffard@gmail.com)
%   Legend copy code by Zoran Pasaric (pasaric@rudjer.irb.hr)

% Parsing handles vector
av = [];
for k = 1:length(handles)
    if strcmp(get(handles(k),'Type'),'axes')
        av = [av handles(k)];
    elseif strcmp(get(handles(k),'Type'),'figure');
        fc = get(handles(k),'Children');
        for j = length(fc):-1:1
            if strcmp(get(fc(j),'Type'),'axes') && ~strcmp(get(fc(j),'Tag'),'legend')
                av = [av fc(j)];
            end;
        end;
    end;
end;

% --- find all legends
hAxes = findobj('type','axes');
tags = get(hAxes,'tag');
iLeg = strmatch('legend',tags);
hLeg = hAxes(iLeg); % only legend axes
userDat = get(hLeg,'UserData');

% Extract axes handles that own particular legend, and corresponding strings
for i1 = 1:length(userDat)
    hLegParAxes(i1) = userDat{i1}.PlotHandle;
    hLegString{i1} = userDat{i1}.lstrings;
end

% Setting the subplots arrangement
Na = length(av);
if nargin < 2
    tiling = [Na 1];
    Ns = Na;
else
    Ns = prod(tiling);
end;

if nargin < 3
    arr = mat2cell((1:Ns)',ones(1,Ns));
end;
if ~iscell(arr)
    error('Arrangement must be a cell array');
end;

% Creating new figure
da = zeros(1,Ns);
newfig = figure;
for k = 1:min(Ns,Na)
    da(k) = subplot(tiling(1),tiling(2),arr{k});
    na = copyobj(av(k),newfig);
    set(na,'Position',get(da(k),'Position'));
    % Produce legend if it exists in original axes
    [ii jj] = ismember(av(k),hLegParAxes);
    if(jj>0)
        axes(na);
        legend(hLegString{jj});
    end
    delete(da(k));
end;