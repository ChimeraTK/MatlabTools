function [si] = cd2si(d,bit,fracbit)
%cd2si - Converts double data to sgned int
%
% Syntax:
%	 [si] = cd2ui(d,bit,fracbit)
%      Converts the double value d to an sigend int value with the
%      specified bit and fractional bit representation 
%
% Inputs:
%    d - double value
%    bit - resolution of the integer value
%    fracbit - number of fractional bits
%
% Outputs:
%    i - signed int value
%
% See also: cui2d, csi2d

% Author:
%    michael.heuer@desy.de
%
% May 2004; Last revision: 27-May-2014

max = 2^bit / 2 - 1;
min = - 2^bit / 2;

d = round(d.*2^fracbit);

si = d.*((d <= max) & (d >= min)) + max .* (d > max) + min .* (d < min);

end
