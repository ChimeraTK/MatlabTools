%%
%

ver = mtca4u.version();

requ_ver = '00.02.00';

assert(strcmp(ver,requ_ver) ~= 0, ['Wrong version. (Returned ''', ver ,''' but ''', requ_ver ,''' expected)']);
