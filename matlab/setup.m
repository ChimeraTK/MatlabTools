% Setup - Adds the required folder to the Matlab search path
%

f = mfilename('fullpath');
i = strfind(f, filesep);

if numel(i) < 2
    disp('Please run the setup script using ''run setup.m'' or via the run button.')
elseif ~strcmp(f(i(end-1):i(end)),'/bin/')
    disp('Please excecute the setup file in the ''bin'' directory.')  
else    
    p = f(1:i(end));
    addpath(p);
    disp('Path sucessufly added. To save the current path persistantly run ''savepath ~/pathdef.m''.')
end

