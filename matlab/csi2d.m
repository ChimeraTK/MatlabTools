function [d] = csi2d(i,bit,fracbit)
%csi2d - Converts an signed int value to a double value
%
% Syntax:
%	 [d] = csi2d(i,bit,fracbit)
%      Converts the signed int value to a daouble value concerning the
%      specified bit and fractional bit representation 
%
% Inputs:
%    i - signed integer value 
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

max = 2^bit / 2 - 1;
min = - 2^bit / 2;

i = round(i);

i = i.*((i <= max) & (i >= min)) + max .* (i > max) + min .* (i < min);

d = i ./ 2^fracbit;

end