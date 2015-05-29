%% Test direct calls of the mex files
%
% This shouldn't be done by the user but we have to test race conditions here
%

%%
%

check_error(@()mtca4u_mex(), 'Illegal parameter excepted');
check_error(@()mtca4u_mex(1), 'Illegal parameter excepted');

check_error(@()mtca4u_mex('foo'), 'Illegal command excepted');
check_error(@()mtca4u_mex('nop'), 'Illegal command excepted');

mtca4u_mex('version')

