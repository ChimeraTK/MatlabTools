function [ui] = cd2ui(d,bit,fracbit)
%cd2ui - Converts double data to unisgned int
%
% Syntax:
%	 [ui] = cd2ui(d,bit,fracbit)
%      Converts the double value d to an unsigend int value with the
%      specified bit and fractional bit representation 
%
% Inputs:
%    d - double value
%    bit - resolution of the integer value
%    fracbit - number of fractional bits
%
% Outputs:
%    ui - unsigned int value
%
% See also: cui2d, csi2d

% Author:
%    michael.heuer@desy.de
%
% May 2004; Last revision: 27-May-2014

max = 2^bit / 2 - 1;
min = - 2^bit / 2;

d = round(d.*2^fracbit);

d = d.*((d <= max) & (d >= min)) + max .* (d > max) + min .* (d < min); 

ui = d.*(d >= 0) + (d+(2^bit)).*(d < 0);

end

