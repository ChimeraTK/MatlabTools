%%
%

m = mtca4u('DUMMY1');

%% Test the readback of a simple value

v = 143;
m.write('','WORD_USER', v);
rb = m.read('','WORD_USER');
assert(rb == v, 'Wrong value read back');
clear v rb size

% Write a binary string

m.write('','WORD_USER', bin2dec('0111'));
assert(m.read('','WORD_USER') == 7, 'Wrong value read back');

% Write a hex string
%v = '0x010101';
m.write('','WORD_USER', hex2dec('00F2'));
assert(m.read('','WORD_USER') == 242, 'Wrong value read back');

%% Test the write of error or bad values

check_error(@()m.write(), 'Illegal parameter excepted');
check_error(@()m.write(''), 'Illegal number of parameters excepted');
check_error(@()m.write('',''), 'Illegal number of parameters excepted');

check_error(@()m.write(1,'',1), 'Illegal parameter excepted');
check_error(@()m.write('',1,1), 'Illegal parameter excepted');
check_error(@()m.write('','WORD_USER',''), 'Illegal parameter excepted');

%% Test the readback of an array

v = 1:32; 
m.write('','AREA_DMAABLE', v);
rb = m.read('','AREA_DMAABLE');
s = m.get_register_size('','AREA_DMAABLE');

assert(numel(rb) == s, 'Wrong number of elements read back');
assert(sum(rb(1:32) ~= v) == 0, 'Wrong array read back')
clear v rb s

%% Test the readback of an array with offset

value = 1:1024;
offset = 1000;
m.write('','AREA_DMAABLE', value);
readback = m.read('','AREA_DMAABLE',offset);
register_size = m.get_register_size('','AREA_DMAABLE');

assert(numel(readback) == (register_size-offset), 'Wrong number of elements read back');
assert(sum(readback ~= value(offset+1:end)) == 0, 'Wrong array read back');
clear value readbback register_size

%% Test the readback of an array with offset and elements

value = 1:1024;
offset = 5;
elements = 10;
m.write('','AREA_DMAABLE', value);
readback = m.read('','AREA_DMAABLE',offset,elements);

assert(numel(readback) == elements, 'Wrong number of elements read back');
assert(sum(readback ~= value(offset+1:elements+offset)) == 0, 'Wrong array read back');
clear value readbback register_size elements

%% Test the write of error or bad values

check_error(@()m.write('','AREA_DMAABLE',0,-1), 'Illegal offset excepted');
check_error(@()m.write('','AREA_DMAABLE',0,0,-1), 'Illegal element number excepted');
