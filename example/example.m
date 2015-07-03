%
%   Minimalistic example for using the mtca4u library in Matlab
%   Before running this example, you need to copy the correct map files
%   into your current directory. They can be taken from the test
%   subdirectory of the matlab_tools repository. You will need at least the
%   files "dummies.dmap" and all *.map/*.mapp files referenced inside.
%   Also, you need to add the "bin" and "matlab" subdirectories to the path
%   of your Matlab installation (click "Set Path" in "Environment" in
%   Matlab main window).
%
%   Author: Martin Hierholzer <martin.hierholzer@desy.de>
%

%
%   Open dummy device
%   Use DUMMY2 to have the modern interface with modules
%
m = mtca4u('DUMMY2');

%
%   Print some device info
%
m.print_device_info();

%
%   Read FIRMWARE and COMPILATION values and print them
%
fprintf('WORD_FIRMWARE = %d\n',m.read('BOARD', 'WORD_FIRMWARE'));
fprintf('WORD_COMPILATION = %d\n',m.read('BOARD', 'WORD_COMPILATION'));

%
%   Write 1 to WORD_ADC_ENA (so AREA_DMAABLE will be filled)
%
m.write('ADC', 'WORD_ADC_ENA',1);

%
%   Print some register info of AREA_DMAABLE_FIXEDPOINT16_3
%
m.print_register_info('ADC','AREA_DMAABLE_FIXEDPOINT16_3');

%
%   Obtain number of elements in AREA_DMAABLE_FIXEDPOINT16_3 register
%
nelements = m.get_register_size('ADC','AREA_DMAABLE_FIXEDPOINT16_3');

%
%   Create figure
%
fig = figure('Name','ADC data', 'NumberTitle','off', 'Position',[360,500,900,900]);

%
%   Read array from AREA_DMAABLE_FIXEDPOINT16_3 and plot it
%   Interpretation as fixed point values will be performed automatically
%
adcdata = m.read('ADC', 'AREA_DMAABLE_FIXEDPOINT16_3', 0, nelements);
subplot(2,1,1);
plot(adcdata);
title('Normal read with fixed point data');

%
%   Alternatively read data via DMA and plot it
%   The register is not configured to have fractional bits, so no
%   conversion can be done here.
%
dmadata = m.read_dma_raw('ADC', 'AREA_DMA_VIA_DMA', 0, nelements);
subplot(2,1,2);
plot(dmadata);
title('DMA read with integer data');