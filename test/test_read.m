%%
%

% Syntax of read:  module, register, [offset], [elements]

%% Check Basic read functions with know return values

assert(m.read('', 'WORD_FIRMWARE') == 0, 'Wrong firmware id returned.');
assert(m.read('', 'WORD_COMPILATION') == 9, 'Wrong compilation returned.');
%assert(m.read('', 'WORD_CLK_DUMMY') == 1145916761, 'Wrong clk_dummy returned.');

%% Check for illegal parameter

check_error(@()m.read(), 'Illegal parameter excepted');
check_error(@()m.read('WORD_FIRMWARE'), 'Illegal parameter excepted');
check_error(@()m.read('WORD_FIRMWARE',1), 'Illegal parameter excepted');

check_error(@()m.read('', 'WORD_CLK_DUMMY', ''), 'Illegal parameter excepted');
check_error(@()m.read('', 'WORD_CLK_DUMMY', 500), 'Illegal parameter excepted');

check_error(@()m.read('', 'WORD_CLK_DUMMY', 0, ''), 'Illegal parameter excepted');
check_error(@()m.read('', 'WORD_CLK_DUMMY', 0, 500), 'Illegal parameter excepted');

check_error(@()m.read('', 'CLK_DUMMY'), 'Illegal parameter excepted');

%% Check the read of arrys 

m.write('', 'WORD_ADC_ENA',1); % Fill AREA_DMAABLE with n^2
ref = (0:1:24).^2;

offset = 5;
readback = m.read('', 'AREA_DMAABLE', offset);
assert(isequal(readback(1:numel(ref)-offset), ref(offset+1:end)), 'Wrong array read back');
clear readback offset

elements = 5;
readback = m.read('', 'AREA_DMAABLE', 0, elements);
assert(numel(readback) == elements, 'Wrong number of elements read back');
assert(isequal(readback, ref(1:elements)), 'Wrong array read back');
clear readback offset

offset = 4;
elements = 10;
readback = m.read('', 'AREA_DMAABLE', offset, elements);
assert(numel(readback) == elements, 'Wrong number of elements read back');
assert(isequal(readback, ref(offset+1:offset+elements)), 'Wrong array read back');
clear readback offset

