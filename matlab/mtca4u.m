classdef mtca4u < mtca4u_interface
  %mtca4u Wrapper class for the MicroTCA4u API
  %
  % mtca4u Methods (static):
  %  version - Displays the MicroTCA4u Matlab Tools library version 
  %  setDMapFilePath - Sets the DMap file which will be used
  %  getDMapFilePath - Displays the current used DMap file
  %  print_info - Displays all available boards with additional information
  %
  % mtca4u Methods (class):
  %   print_device_info - Displays all available registers of a board
  %   print_register_info - Displays all information of a certain register
  %   get_register_size - Returns the size of a register
  %   read - Reads data from the register of a board
  %   write - Writes data to the register of a board
  %   read_dma_raw - Reads raw data from a board using direct memory access
  %   read_dma - Reads data from a board using direct memory access
  %   read_seq - Reads a sequence from the dma area
  %
  % Example:
  %   mtca4u.version();
  %   mtca4u.setDMapFilePath('./devices.dmap'); 
  %   dev = mtca4u('SISL')
  %   dev.read('BOARD.0','WORD_FIRMWARE');
  %
  
  % Autor:
  %    michael.heuer@desy.de
  %
  % Last revision: 27-May-2014
  
   properties  (Access = 'private')
       device = [];
       handle = [];
	   c; % used for calling the destructor
   end
   
   properties  (Access = 'public')
       debug = 0;
   end
   
   methods (Static, Access = 'public')
		function ver = version()
        %mtca4u.version - Returns the library version
		    try
                ver = mtca4u_mex('version');
            catch ex
                error(ex.message)
            end
		end
 

		function help()
        %mtca4u.help - 
      try
        mtca4u_mex('help');
      catch ex
        error(ex.message)
      end
		end
    %mtca4u.setDMapFilePath - Sets the DMap file which will be used
    %
    % Syntax:
    %    mtca4u.setDMapFilePath(path)
    %
    % Inputs:
    %    path - path of the dmap file
    %
    % See also: mtca4u, mtca4u.getDMapFilePath
    function setDMapFilePath(varargin)
        %mtca4u.setDMapFilePath - 
		    try
          mtca4u_mex('set_dmap', varargin{:});
        catch ex
          error(ex.message)
        end
		end
        %mtca4u.getDMapFilePath - Displays all available boards with additional information
        %
        % Syntax:
        %    mtca4u.getDMapFilePath()
        %
        % See also: mtca4u, mtca4u.setDMapFilePath
    function dmap = getDMapFilePath()
        %mtca4u.setDMapFilePath - 
		    try
          damp = mtca4u_mex('get_dmap');
        catch ex
          error(ex.message)
        end
		end						
		
		function print_info(~, varargin)
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
                fprintf(['Name: ', i.name, '\t Device: ', i.device, '\t Firmware: ', num2str(i.firmware), '\t Date: ', i.date, '\t Map: ', i.map, '\n']);
            end
        end
   end
   
   methods
        function obj = delete(obj)
         %mtca4u.delete - Destructor of the Wrapper class
			try
                mtca4u_mex('close', obj.handle);
            catch ex
                error(ex.message)
            end
        end
   end
   
   methods (Access = 'public')
        %mtca4u.mtca4u - Constructor of the Wrapper class
		%
        function obj = mtca4u(board)
            obj.device = board;
            try
                %mtca4u_mex('refresh_dmap');
                obj.handle = mtca4u_mex('open', obj.device);
            catch ex
                error(ex.message)
            end
            %obj.c = onCleanup(@()obj.delete()); % "Register" the destructor
            % Todo: this works only if it is not possible to change the
            % handle after the creation!
        end

        function print_device_info(obj, varargin)
        %mtca4u.print_device_info - Displays all available registers of a board
        %
        % Syntax:
        %    mtca4u.print_device_info()
        %
        % See also: mtca4u_load_dmap, mtcau4_read
            try
                info = mtca4u_mex('device_info', obj.handle, varargin{:});
            catch ex
                error(ex.message);
            end   
            for i = info
            end
        end

        function print_register_info(obj, varargin)
        %mtca4u.print_register_info - Displays all information of a certain register
        %
        % Syntax:
        %    mtca4u.print_register_info(module, register)
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %
            try
                info = mtca4u_mex('register_info', obj.handle, varargin{:});
            catch ex
                error(ex.message);
            end
            for i = info
                fprintf(['Name: ', i.name, '\t Elements: ', num2str(i.elements), ...
                '\t Signed: ', num2str(i.signed), '\t Bits: ', num2str(i.bits),  '\t Fractional Bits: ', num2str(i.fractional_bits), ...
                '\nDescription: ', num2str(i.description),'\n']);
            end
        end
		
		function s = get_register_size(obj, varargin)
        %mtca4u.get_register_size - Return the number of elements in the register
        %
        % Syntax:
        %    % board = mtca4u('board');  
        %    board.get_register_size(module, register)
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %
            try
                s = mtca4u_mex('register_size', obj.handle, varargin{:});
            catch ex
                error(ex.message);
            end
        end


        function varargout = read(obj, varargin)
        %mtca4u.read - Reads data from the register of a board
        %
        % Syntax:
        %    % board = mtca4u('board'); 
        %    [data] = board.read(module, register)
        %    [data] = d.read(module, register, offset, elements)
        %    ...
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
	     %    offset - Start element of the reading (optional, default: 1)
        %    elements - Number of elements to be read (optional, default: 'numel(offset:end)')
        %
        % Outputs:
        %    data - Value/s of the register 
        %
        % See also: mtca4u , mtca4u.write
            try
                [varargout{1:nargout}] = mtca4u_mex('read', obj.handle, varargin{:});
            catch ex
                error(ex.message);
            end
        end

        function write(obj, varargin)
        %mtca4u.write - Writes data to the register of a board
        %
        % Syntax:
        %    % board = mtca4u('board'); 
        %    board.write(module, register, value)
        %    board.write(module, register, value, offset)
        %    ...
        %
        % Inputs:
        %    module - Name of the board
        %    register - Name of the register
        %    value - Value or Vector to be written
        %    offset - Start element of the writing (optional, default: 1)
		  %
	     % Examples:
	     %    mtca4('SISL').write('BOARD', 'WORD_REGISTER', bin2dec('00101'));
        %
        % See also: mtca4u, mtca4u.read
            try
                mtca4u_mex('write', obj.handle, varargin{:});
            catch ex
                error(ex.message);
            end
        end

        function [varargout] = read_dma_raw(obj, varargin)
        %mtca4u.read_dma_raw - Reads data from a board using direct memory access
        %
        % Syntax:
        %    % board = mtca4u('board'); 
        %    [data] = board.read_dma_raw(module, register)
        %    [data] = board.read_dma_raw(module, register, offset)
        %    [data] = board.read_dma_raw(module, register, offset, elements, mode)
        %    [data] = board.read_dma_raw(module, register, offset, elements, mode, singed, bit, fracbit)
        %    ...
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %    offset - Start element of the writing (optional, default: 1)
        %    elements - Number of elements to be read (optional, default: 'numel(offset:end)')
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
            [varargout{1:nargout}] = mtca4u_mex('read_dma_raw', obj.handle, varargin{:});
          catch ex
            error(ex.message);
          end
        end
         
        function [varargout] = read_dma(obj, varargin)
        %mtca4u.read_dma - Reads data from a board using direct memory access
        %             
        % Syntax:
        %    % board = mtca4u('board'); 
        %    [data] = board.read_dma(module, register, channel)
        %    [data] = board.read_dma(module, register, channel, offset, elements)
        %    [data] = board.read_dma(module, register, channel, offset, elements)
        %    [data] = board.read_dma(module, register, channel, offset, elements, mode, signed, bit, fracbit)
        %    [channel1, channel2, ...] = board.read_dma(module, register, [1, 2 ...], offset, elements)
        %    [channel1, channel2, ...] = board.read_dma(module, register, [1, 2 ...], offset, elements, mode, signed, bit, fracbit)
        %    ...
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %    channel - Channel of the DAQ Block
        %    sample - Amount of sample to read (optional, default: all available)
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
		    warning('Deprecated function. Consider using read_seq');
            [varargout{1:nargout}] = mtca4u_mex('read_dma', obj.handle, varargin{:});
          catch ex
            error(ex.message);
          end
        end
        
        function [varargout] = read_seq(obj, varargin)
        %mtca4u.read_seq - Reads data from a multiplexed sequence
        %             
        % Syntax:
        %    % board = mtca4u('board'); 
        %    [data] = board.read_seq(module, register, sequence)
        %    [data] = board.read_seq(module, register, sequence, offset, elements)
        %    [sequence1, sequence2, ...] = mtca4u.read_seq(module, register, [1, 2 ...], offset, elements)
        %    ...
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %    sequence - Number of the sequence
        %    offset - Offset into the sequence
        %    elements - Number of samples to read (optional, default: all available)
        %
        % Outputs:
        %    data - Values of the sequence(s)
        %
        % See also: mtca4u, mtca4u.read, mtca4u.write
          try
            [varargout{1:nargout}] = mtca4u_mex('read_seq', obj.handle, varargin{:});
          catch ex
            error(ex.message);
          end
        end
    end
end

