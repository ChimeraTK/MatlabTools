classdef mtca4u
  %mtca4u Wrapper class for the MicroTCA4u API
  %   
  % mtca4u Methods:
  %   print_info - Displays all available boards with additional information
  %   print_device_info - Displays all available registers of a board
  %   print_register_info - Displays all information of a certain register
  %   read - Reads data from the register of a board
  %   write - Writes data to the register of a board
  %   read_dma_raw - Reads raw data from a board using direct memory access
  %   read_dma - Reads channel data from a board using direct memory access
  %
  
  % Autor:
  %    michael.heuer@desy.de
  %
  % Last revision: 27-May-2014
  
   properties
        prefix = '';
   end
    
   methods (Static, Access = 'public')
        %mtca4u.mtca4u - Constructor of the Wrapper class
        function obj = mtca4u()
        end
        
        function print_info(varargin)
        %mtca4u.print_info - Displays all available boards with additional information
        %
        % Syntax:
        %    mtca4u.print_info()
        %
        % See also: mtca4u, mtca4u.read
            try
                info = mtca4u_mex('info', varargin{:});
            catch ex
                error(ex.message)
            end
            for i = info
                fprintf(['Name: ', i.name, '\t Slot: ', num2str(i.slot), '\t Firmware: ', num2str(i.firmware), '\t Revision: ', num2str(i.revision), '\n']);
            end
        end

        function print_device_info(varargin)
        %mtca4u.print_device_info - Displays all available registers of a board
        %
        % Syntax:
        %    mtca4u.print_device_info()
        %
        % Inputs:
        %    board - Name of the board 
        %
        % See also: mtca4u_load_dmap, mtcau4_read
            try
                info = mtca4u_mex('device_info', varargin{:});
            catch ex
                error(ex.message);
            end   
            for i = info
            end
        end

        function print_register_info(varargin)
        %mtca4u.print_register_info - Displays all information of a certain register
        %
        % Syntax:
        %    mtca4u.print_register_info(board, register)
        %
        % Inputs:
        %    board - Name of the board
        %    register - Name of the register
        %
            try
                info = mtca4u_mex('register_info', varargin{:});
            catch ex
                error(ex.message);
            end
            for i = info
                fprintf(['Name: ', i.name, '\t Elements: ', num2str(i.elements), ...
                '\t Signed: ', num2str(i.signed), '\t Bits: ', num2str(i.bits),  '\t Fractional Bits: ', num2str(i.fractional_bits), ...
                '\nDescription: ', num2str(i.description),'\n']);
            end
        end

        function varargout = read(varargin)
        %mtca4u.read - Reads data from the register of a board
        %
        % Syntax:
        %    [data] = mtca4u.read(board, register)
        %    [data] = mtca4u.read(board, register, elements, offset)
        %    ...
        %
        % Inputs:
        %    board - Name of the board
        %    register - Name of the register
        %    elements - Number of elements to be read (optional, default: 'offset:end')
        %    offset - Start element of the reading (optional, default: 0)
        %
        % Outputs:
        %    data - Value/s of the register 
        %
        % See also: mtca4u , mtca4u.write
            try
                [varargout{1:nargout}] = mtca4u_mex('read', varargin{:});
            catch ex
                error(ex.message);
            end
        end

        function write(varargin)
        %mtca4u.write - Writes data to the register of a board
        %
        % Syntax:
        %    mtca4u_write(board, register, value)
        %    mtca4u_write(board, register, value, offset)
        %    ...
        %
        % Inputs:
        %    board - Name of the board
        %    register - Name of the register
        %    value - Value or Vector to be written
        %    offset - Start element of the writing (optional, default: 0)
        %
        % See also: mtca4u, mtca4u.read
            try
                mtca4u_mex('write', varargin{:});
            catch ex
                error(ex.message);
            end
        end

        function [varargout] = read_dma_raw(varargin)
        %mtca4u.read_dma_raw - Reads data from a board using direct memory access
        %
        % Syntax:
        %    [data] = mtca4u.read_dma_raw(board, area)
        %    [data] = mtca4u.read_dma_raw(board, area, sample, offset)
        %    [data] = mtca4u.read_dma_raw(board, area, sample, offset, mode)
        %    [data] = mtca4u.read_dma_raw(board, area, sample, offset, mode, singed, bit, fracbit)
        %    ...
        %
        % Inputs:
        %    board - Name of the board
        %    register - Name of the register
        %    sample - Number of sample to be read (optional, default: all available)
        %    offset - Start element of the writing (optional, default: 0)
        %    mode - Data mode of 16 or 32bit (optional, default: 32)
        %    singed - Data mode of 16 or 32bit (optional, default: false)
        %    bit - Data mode of 16 or 32bit (optional, default: mode)
        %    fracbit - Data mode of 16 or 32bit (optional, default: 0)
        %
        % Outputs:
        %    data - Value/s of the DAQ Block
        %
        % See also: mtca4u, mtcau4.read
          try
            [varargout{1:nargout}] = mtca4u_mex('read_dma_raw', varargin{:});
          catch ex
            error(ex.message);
          end
        end
         
        function [varargout] = read_dma(varargin)
        %mtca4u.read_dma - Reads data from a board using direct memory access
        %             
        % Syntax:
        %    [data] = mtca4u.read_dma(board, area, channel)
        %    [data] = mtca4u.read_dma(board, area, channel, sample, offset)
        %    [data] = mtca4u.read_dma(board, area, channel, sample, offset, channels, mode)
        %    [data] = mtca4u.read_dma(board, area, channel, sample, offset, channels, mode, signed, bit, fracbit)
        %    [channel1, channel2, ...] = mtca4u.read_dma(board, area, [1, 2 ...], sample, offset)
        %    [channel1, channel2, ...] = mtca4u.read_dma(board, area, [1, 2 ...], sample, offset, channels, mode, signed, bit, fracbit)
        %    ...
        %
        % Inputs:
        %    board - Name of the board
        %    area - Name of the dma area
        %    channel - Channel of the DAQ Block
        %    sample - Amount of sample to read (optional, default: all available)
        %    offset - Start element of the reading (optional, default: 0)
        %    channels - Amount of available channels (optional, default: 8)
        %    mode - Data mode of 16 or 32bit (optional, default: 32)
        %    singed - Data mode of 16 or 32bit (optional, default: false)
        %    bit - Data mode of 16 or 32bit (optional, default: mode)
        %    fracbit - Data mode of 16 or 32bit (optional, default: 0)
        %
        % Outputs:
        %    data - Value/s of the DAQ Block
        %
        % See also: mtca4u, mtca4u.read, mtca4u.write
          try
            [varargout{1:nargout}] = mtca4u_mex('read_dma', varargin{:});
          catch ex
            error(ex.message);
          end
        end
    end
end

