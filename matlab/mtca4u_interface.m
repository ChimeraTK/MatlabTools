classdef mtca4u_interface
  %mtca4u_interface Interface definition for the MATLAB MicroTCA4 Tools
  %   
  % mtca4u Methods:
  %   print_info - Displays all available boards with additional information
  %   print_device_info - Displays all available registers of a board
  %   print_register_info - Displays all information of a certain register
  %   get_register_size - Returns the size of a register
  %   read - Reads data from the register of a board
  %   write - Writes data to the register of a board
  %   read_dma_raw - Reads raw data from a board using direct memory access
  %   read_dma - Reads channel data from a board using direct memory access
  %
  
  % Autor:
  %    michael.heuer@desy.de
  %
  % Last revision: 27-May-2014
   
   methods (Abstract)
      ver = version(obj);
      print_info(obj, varargin);
      print_device_info(obj, varargin);
      print_register_info(obj, varargin);
      s = get_register_size(obj, varargin);
      varargout = read(obj, varargin);
      write(obj, varargin);
      read_dma_raw(obj, varargin);
      read_dma(obj, varargin);
      read_seq(obj, varargin);
    end
end

