%%
%

ver = mtca4u.version();

requ_ver = '00.01.01';

assert(strcmp(ver,requ_ver) ~= 0, ['Wrong version. (Returned ''', ver ,''' but ''', requ_ver ,''' expected)']);
