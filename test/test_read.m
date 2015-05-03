%%
%

m = mtca4u('DUMMY1');

fw = m.read('', 'WORD_FIRMWARE');

assert(fw == 0, 'Wrong firmware id returned.');

c = m.read('', 'WORD_COMPILATION');

assert(c == 9, 'Wrong compilation returned.');

clk = m.read('', 'WORD_CLK_DUMMY');

assert(clk == 1145916761, 'Wrong clk_dummy returned.');

%%
% 

check_error(@()m.read(), 'Illegal parameter excepted');
check_error(@()m.read('WORD_FIRMWARE'), 'Illegal parameter excepted');
check_error(@()m.read('WORD_FIRMWARE',1), 'Illegal parameter excepted');

check_error(@()m.read('', 'WORD_CLK_DUMMY',''), 'Illegal parameter excepted');
check_error(@()m.read('', 'WORD_CLK_DUMMY',500), 'Illegal parameter excepted');

check_error(@()m.read('', 'CLK_DUMMY'), 'Illegal parameter excepted');

