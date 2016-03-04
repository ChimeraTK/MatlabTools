classdef mtca4u_remote < mtca4u_interface
  %mtca4u_remote Remote wrapper class for the MicroTCA4u API on a remote system
  %
  % Syntax:
  %    m = mtca4u_remote(board, path, host, user)
  %
  % Inputs:
  %    board - Name of the board
  %    path - Remote Working Directory
  %    host - Address of the remote machine
  %    user - Remote User name
  %   
  % mtca4u Methods:
  %    print_info - Displays all available boards with additional information
  %    print_device_info - Displays all available registers of a board
  %    print_register_info - Displays all information of a certain register
  %    read - Reads data from the register of a board
  %    write - Writes data to the register of a board
  %    read_dma_raw - Reads raw data from a board using direct memory access
  %    read_dma - Reads channel data from a board using direct memory access
  %
  
  % Autor:
  %    michael.heuer@desy.de
  %
  % Last revision: 27-May-2014

    
    properties
        path = '';
        debug = false;
    end
    
    properties (Hidden)
		board = [];
        channel = 0;
        c;
        required_tools_version = '00.09';
        %remote_executable = strcat('mtca4u-', self.required_tools_version);
        remote_executable = 'mtca4u-00.09';
    end
    
    methods (Access = 'private')
        %mtca4u.delete - Destructor of the Wrapper class
        function delete(obj)
            if obj.channel ~= 0
              obj.channel.close();
              disp('Connection closed...')
            end
        end
        
        function res = ReadStdout(~, channel)
            res = [];
            stdout = ch.ethz.ssh2.StreamGobbler(channel.getStdout());
            br = java.io.BufferedReader(java.io.InputStreamReader(stdout));
            while(true)
                line = char(br.readLine());
                if(isempty(line)) % break if string is empty
                    break
                end
                number = str2double(line);
                if (isnan(number))
                  res = [res, line];
                else
                  res = [res, number];
                end
            end
            channel.close();
        end
        
        function res = ReadStderr(~, channel)
        	res = [];
            stdout = ch.ethz.ssh2.StreamGobbler(channel.getStderr());
            br = java.io.BufferedReader(java.io.InputStreamReader(stdout));
            while(true)
                line = br.readLine();
                if(isempty(line)), break, else res = [res, char(line)]; end
            end
            channel.close();
        end
        
        function s = createCLTString(~, d)
        s = [];
        for i = 1:length(d)
          if isnumeric(d{i})
            s = [s, '"'];
            v = d{i};
            for x = 1:length(v)-1
              s = [s, num2str(v(x)), ' '];
            end
            s = [s, num2str(v(end)), '" '];
          elseif ischar(d{i})
            s = [s, '"',d{i}, '" '];
          else
            error(['Illegal Parameter ', num2str(i)]);
          end
        end
      end
    end
    
    methods (Access = 'public')
        %mtca4u.mtca4u_remote - Constructor of the Wrapper class
        %
        % Syntax:
        %    m = mtca4u_remote(board, path, host, user)
        %
        % Inputs:
        %    board - Name of the board
        %    path - Remote Working Directory
        %    host - Address of the remote machine
        %    user - Remote User name
        %
        function obj = mtca4u_remote(board, path, hostName, userName, id_file)
            if(nargout ~= 1)
                error('One output argument is required to store the mtca4u_remote object.')
            end
			if isempty(board)
                error('Invalid board name.');
            end
			if isempty(path)
                error('Invalid path.');
            end
            if isempty(hostName)
                error('Invalid hostname.');
            end
            if isempty(userName)
                error('Invalid username.');
            end
			
            % Import the require java library      
          	try
                javaaddpath(fullfile(fileparts(mfilename('fullpath')),'ganymed-ssh2-build250.jar'));
            catch
                error('Error: Could not find the required java packages.');
            end
            % Connect to remote host
            try
                obj.c = onCleanup(@()delete(obj));
                obj.channel = ch.ethz.ssh2.Connection(hostName);
                obj.channel.connect();
                obj.board = board();
                obj.path = path;
                disp('Connection established...');
            catch
                error('Error: Could not connect to the remote machine %s.',hostName);
            end
            % Authentication
            if (~exist('id_file','var'))       
                if(~obj.channel.authenticateWithPassword(userName,passcode()))
                    error('Error: Could not authenticate the connection.');
                end
            else
                file = java.io.File(id_file);
                if(~obj.channel.authenticateWithPublicKey(userName,file,''))
                     error('Error: Could not authenticate the connection.');
                end
            end
            % Check tools version
            channel2 = obj.channel.openSession();
            channel2.execCommand([obj.remote_executable, ' version']);
            version = ReadStdout(obj, channel2);

            % strncmp(x,y,5) : only compare major and minor version
            assert(strncmp(version, obj.required_tools_version,5), ['Wrong command line tools installed on the remote side. Expected ''', obj.required_tools_version,''' but found ''', version])

            channel2.close();
        end
        
        function result = version(obj,~)
        %mtca4u_remote.version - Shows the version of the remote tools
        %
          cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' version'];
          if obj.debug, disp(['Run: ', cmd]); end
          channel2 = obj.channel.openSession();
          channel2.execCommand(cmd);
          result = ReadStdout(obj, channel2);
          % Handle an Error
          if(channel2.getExitStatus() ~= 0)
            channel2.close();
            error(['Failed to read remote value: ', ReadStderr(obj, channel2)]);
          end
          channel2.close();
        end
        
        function ver = print_info(obj, varargin)
        %mtca4u_remote.print_info - 
        %
        error('Not Implemented yet.');    
        end
        
        function ver = print_device_info(obj, varargin)
        %mtca4u_remote.print_info - 
        %
          error('Not Implemented yet.');   
        end
        
        function ver = print_register_info(obj, varargin)
        %mtca4u_remote.print_info - 
        %
          error('Not Implemented yet.');   
        end
        
        function result = get_register_size(obj, varargin)
        %mtca4u_remote.get_register_size - Return the number of elements in the register
        %
        % Syntax:
        %    % board = mtca4u('board');  
        %    board.get_register_size(module, register)
        %
        % Inputs:
        %    module - Name of the module
        %    register - Name of the register
        %
          cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' register_size ', obj.board, ' ', createCLTString(obj, varargin)];
          if obj.debug, disp(['Run: ', cmd]); end
          channel2 = obj.channel.openSession();
          channel2.execCommand(cmd);
          result = ReadStdout(obj, channel2);
          % Handle an Error
          if(channel2.getExitStatus() ~= 0)
            channel2.close();
            error(['Failed to read remote register size: ', ReadStderr(obj, channel2)]);
          end
          channel2.close();   
        end
        
        function result = read(obj, varargin)
        %mtca4u_remote.read - Reads data from the register of a board
        %
        % Syntax:
        %    % board = mtca4u(...); 
        %    [data] = board.read(module, register)
        %    [data] = board.read(module, register, offset, elements)
        %    ...
        %
        % Inputs:
        %    module - Name of the module (if none use: '')
        %    register - Name of the register
        %    offset - Start element of the reading (optional, default: 0)
        %    elements - Number of elements to be read (optional, default: 'offset:end')
        %
        % Outputs:
        %    data - Value/s of the register 
        %
        % Example:
        %
        %
        % See also: mtca4u , mtca4u.write
          cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' read ', obj.board, ' ', createCLTString(obj, varargin)];
          if obj.debug, disp(['Run: ', cmd]); end
          channel2 = obj.channel.openSession();
          channel2.execCommand(cmd);
          result = ReadStdout(obj, channel2);
         
          % Handle an Error
          if(channel2.getExitStatus() ~= 0)
            channel2.close();
            error(['Failed to read remote value: ', ReadStderr(obj, channel2)]);
          end
          channel2.close();
        end
        
        function result = write(obj, varargin)
        %mtca4u_remote.write - Writes data to the register of a board
        %
        % Syntax:
        %    % board = mtca4u(...); 
        %    board.write(module, register, value)
        %    board.write(module, register, value, offset)
        %
        % Inputs:
        %    module - Name of the module (if none use: '')
        %    register - Name of the register
        %    value - Value or Vector to be written
        %    offset - Start element of the writing (optional, default: 0)
        %
        % See also: mtca4u, mtca4u.read
            cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' write ', obj.board, ' ', createCLTString(obj, varargin)];
            if obj.debug, disp(['Run: ', cmd]); end
            channel2 = obj.channel.openSession();
            channel2.execCommand(cmd);
            result = ReadStdout(obj, channel2);
            
            % Handle an Error
            if(channel2.getExitStatus() ~= 0)
                channel2.close();
                error(['Failed to write remote value: ', ReadStderr(obj, channel2)]);
            end
            channel2.close();
        end

        function result = read_dma(obj, varargin)
        %mtca4u.read_dma - Reads data from a board using direct memory access
        %             
        % Syntax:
        %    % board = mtca4u(...); 
        %    [data] = board.read_dma(module, register)
        %    [data] = board.read_dma(module, register, channel, sample)
        %    [data] = board.read_dma(module, register, channel, sample, mode)
        %    [data] = board.read_dma(module, register, channel, sample, mode, signed, bit, fracbit)
        %    [channel1, channel2, ...] = board.read_dma(module, register, [1, 2 ...], sample)
        %    [channel1, channel2, ...] = board.read_dma(module, register, [1, 2 ...], sample, mode, signed, bit, fracbit)
        %    ...
        %
        % Inputs:
        %    module - Name of the module (if none use: '')
        %    register - Name of the register
        %    channel - Channel of the DAQ Block
        %    sample - Amount of sample to read (optional)
        %    mode - Data mode of 16 or 32bit (optional, default: 32)
        %    singed - Data mode of 16 or 32bit (optional, default: false)
        %    bit - Data mode of 16 or 32bit (optional, default: mode)
        %    fracbit - Data mode of 16 or 32bit (optional, default: 0)
        %
        % Outputs:
        %    data - Value/s of the DAQ Block
        %
        % See also: mtca4u, mtca4u.read, mtca4u.write
          cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' read_dma ', obj.board, ' ', createCLTString(obj, varargin)];
          if obj.debug, disp(['Run: ', cmd]); end
          channel2  =  obj.channel.openSession();
          channel2.execCommand(cmd);
          result  =  ReadStdout(obj, channel2);
         
          % Handle an Error
          if(channel2.getExitStatus() ~= 0)
            channel2.close();
            error(['Failed to read remote value: ', ReadStderr(obj, channel2)]);
          end
          channel2.close();
        end
        
        function result = read_seq(obj, varargin)
        %mtca4u.read_seq - Reads data from a multiplexed sequence
        %             
        % Syntax:
        %    % board = mtca4u(...); 
        %    [data] = board.read_seq(module, register)
        %    [data] = board.read_dma(module, register, sequence)
        %    [data] = board.read_dma(module, register, sequence, offset)
        %    [data] = board.read_dma(module, register, sequence, offset, elements)
        %    ...
        %
        % Inputs:
        %    module - Name of the module (if none use: '')
        %    register - Name of the register
        %    sequence - Number of the sequence to read (optional)
        %    offset - Offset into the sequence (optional)
        %    elements - Number of elements to read (optional)
        %
        % Outputs:
        %    data - Values of the sequence(s)
        %
        % See also: mtca4u, mtca4u.read, mtca4u.write
          cmd = ['cd ''', obj.path, ''' && ''', obj.remote_executable ,''' read_seq ', obj.board, ' ', createCLTString(obj, varargin)];
          if obj.debug, disp(['Run: ', cmd]); end
          channel2  =  obj.channel.openSession();
          channel2.execCommand(cmd);
          result  =  ReadStdout(obj, channel2);
          % Handle an Error
          if(channel2.getExitStatus() ~= 0)
            channel2.close();
            error(['Failed to read remote value: ', ReadStderr(obj, channel2)]);
          end
          channel2.close();
        end
        
    end
end

