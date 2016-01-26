%%
%

%%
%

%%
%
mtca4u_mex('set_dmap','dummies.dmap')
m = mtca4u('DUMMY1');

%% Should fail without parameter

%check_error(@()mtca4u('DUMMY1'), 'Illegal parameter excepted');

check_error(@()mtca4u(), 'Illegal parameter excepted');
check_error(@()mtca4u('DUMMY1','DUMMY1'), 'Illegal parameter excepted');
check_error(@()mtca4u('DUMMY1',1), 'Illegal parameter excepted');
