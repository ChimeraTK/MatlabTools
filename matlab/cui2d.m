function [d] = cui2d(ui,bit,fracbit)
%cui2d - Converts an unsigned int value to a double value
%
% Syntax:
%	 [d] = cui2d(i,bit,fracbit)
%      Converts the unsigned int value to a daouble value concerning the
%      specified bit and fractional bit representation 
%
% Inputs:
%    i - unsigned integer value 
%    bit - resolution of the integer value
%    fracbit - number of fractional bits
%
% Outputs:
%    d - double value
%
% See also: cui2d, cd2ui

% Author:
%    michael.heuer@desy.de
%
% May 2004; Last revision: 27-May-2014

max = 2^bit - 1;
half = 2^bit / 2 -1;

ui = round(ui);

ui = ui.*((ui <= max) & (ui >= 0)) + max .* (ui > max) + 0 .* (ui < 0);

i = ui.*(ui <= half) + (ui-max-1).*(ui>half);

d = i ./ 2^fracbit;

end