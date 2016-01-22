/**
 * @file mtca4u_matlab.cpp
 *
 * @brief Main file of the MicroTCA 4 You Matlab Library
 *
 * In this file the main entry point of the mex file is declared
 */

/*
 * Copyright (c) 2014 michael.heuer@desy.de
 *
 */

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

#include <mex.h>

#include <mtca4u/DMapFilesParser.h>
#include <mtca4u/Device.h>
#include <mtca4u/MultiplexedDataAccessor.h>

#include "../include/version.h"

using namespace mtca4u;
using namespace std;

typedef MultiplexedDataAccessor<double> dma_accessor;
typedef boost::shared_ptr<MultiplexedDataAccessor<double>> dma_Accessor_ptr;

// Some c++ wrapper and utility functions

void mexPrintf(const std::string &s) { mexPrintf(s.c_str()); }
void mexWarnMsgTxt(const std::string &s) { mexWarnMsgTxt(s.c_str()); }
void mexErrMsgTxt(const std::string &s) { mexErrMsgTxt(s.c_str()); }

bool mxIsRealScalar(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == 1); }
bool mxIsPositiveRealScalar(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == 1) && (mxGetScalar(a) > 0); }
bool mxIsRealVector(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == mxGetN(a)); }
bool mxIsPositiveRealVector(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == mxGetN(a)) && (mxGetScalar(a) >= 0); }
bool mxGetBoolScalar(const mxArray* a) { return  mxGetScalar(a) != 0; }

std::string getOrdinalNumerString(const unsigned int &n) {
  const vector<string> arrayOfOrdinalNumberStrings = {
    "", "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth",
	"ninth", "tenth", "eleventh", "twelfth" };
  if (n > arrayOfOrdinalNumberStrings.size())
  {
    stringstream ss; ss << n << "th";
	return ss.str();
  }
  else return arrayOfOrdinalNumberStrings[n];
}


string mxArrayToStdString(const mxArray *pa)  { char *ps = mxArrayToString(pa); string s = ps; mxFree(ps); return s; }
//char* mxArrayToString(const mxArray*) = delete; // do not use this function cause one could forget the mxFree. Use the one above instead!

// Function declaration

//static void CleanUp(void); // Should me declared static (See Matlab MEX Manual)
boost::shared_ptr<mtca4u::Device> getDevice(const mxArray *plhsDevice);
//boost::shared_ptr< devMap<devPCIE> > getDevice(const mxArray *plhsDevice);

// Global Parameter

//std::vector<boost::shared_ptr< devMap<devPCIE> > > openDevicesVector;
std::vector<boost::shared_ptr<mtca4u::Device> > openDevicesVector;

// Command Function declarations and stuff

typedef void (*CmdFnc)(unsigned int, mxArray**, unsigned int, const mxArray **);

struct Command {
  string Name; CmdFnc pCallback; string Description; string Example;
  Command(string n,CmdFnc p, string d, string e) : Name(n), pCallback(p), Description(d), Example(e) {};
};

void PrintHelp(unsigned int, mxArray**, unsigned int, const mxArray **);
void getVersion(unsigned int, mxArray**, unsigned int, const mxArray **);
void openDevice(unsigned int, mxArray**, unsigned int, const mxArray **);
void closeDevice(unsigned int, mxArray**, unsigned int, const mxArray **);
void getInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void getDeviceInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void getRegisterInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void getRegisterSize(unsigned int, mxArray**, unsigned int, const mxArray **);
//void refreshDmap(unsigned int, mxArray**, unsigned int, const mxArray **);
void readRegister(unsigned int, mxArray**, unsigned int, const mxArray **);
void writeRegister(unsigned int, mxArray**, unsigned int, const mxArray **);
void readDmaRaw(unsigned int, mxArray**, unsigned int, const mxArray **);
//void readDmaChannel(unsigned int, mxArray**, unsigned int, const mxArray **);
void readSequence(unsigned int, mxArray**, unsigned int, const mxArray **);

vector<Command> vectorOfCommands = {
  Command("help", &PrintHelp, "", ""),
  Command("version", &getVersion, "", ""),
  Command("nop", NULL, "", ""),
  Command("open", &openDevice, "", ""),
  Command("close", &closeDevice, "", ""),
  Command("info", &getInfo, "", ""),
  Command("device_info", &getDeviceInfo, "", ""),
  Command("register_info", &getRegisterInfo, "", ""),
  Command("register_size", &getRegisterSize, "", ""),
  //Command("refresh_dmap", &refreshDmap, "", ""),
  Command("read", &readRegister, "", ""),
  Command("write", &writeRegister, "", ""),
  Command("read_dma_raw", &readDmaRaw, "", ""),
  //Command("read_dma", &readDmaChannel, "", ""),
  Command("read_seq", &readSequence, "", ""),
};

/**
 * @brief Mex Entry Function
 *
 * @param[in] nlhs Number of left hand side parameter
 * @param[inout] phls Pointer to the left hand side parameter
 * @param[in] rhhs Numer of the right hand side parameter
 * @param[inout] prhs Pointer to the right hand side parameter
 *
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
 // mexAtExit(CleanUp); // register Cleanup Function

  if(nrhs == 0) { mexErrMsgTxt("Not enough input arguments."); return; }
  if (!mxIsChar(prhs[0])) { mexErrMsgTxt("Invalid input arguments."); return; }

  string cmd = mxArrayToStdString(prhs[0]);
  transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

  try {
    vector<Command>::iterator it = vectorOfCommands.begin();

    // Look for the right command
    for (; it != vectorOfCommands.end(); ++it)
    {
      if (it->Name == cmd)
        break;
    }

    // Check if search was successful
    if(it == vectorOfCommands.end()) {
      mexErrMsgTxt("Unknown command '" + cmd + "'. Use mtca4u('help') to show some help information.");
      return;
    }

    // Check if the method is implemented
    else if (NULL == it->pCallback) {
      mexErrMsgTxt("Command not implemented yet.");
      return;
    }

    // Ok run method
    else it->pCallback(nlhs, plhs, nrhs - 1, &prhs[1]);
  }

  catch ( Exception &e )
  {
    mexErrMsgTxt(e.what());
  }
}

/**
 * @brief CleanUp
 *
 */
//static void CleanUp(void)
//{
//#ifdef __MEX_DEBUG_MODE
//  mexPrintf("CleanUp\n");
//#endif
//  //mapOfDevices.clear();
//}

/**
 * @brief getDevice
 *
 * @param[in] dmapFileName File to be loaded or all in the current directory if empty
 *
 */
/* boost::shared_ptr< devMap<devPCIE> > getDevice(const string& deviceName) //, const string &dmapFileName = "")
{
  if (g_dmapFilesParser.getdMapFileSize() == 0) {
    g_dmapFilesParser.parse_dir(".");
  }
  // Return already open device
  if ( !openDevicesMap[deviceName] ){

    // Look for device
    dmapFilesParser::iterator it = g_dmapFilesParser.begin();
    for (; it != g_dmapFilesParser.end(); ++it)
    {
      if (deviceName == it->first.dev_name)
        break;
    }

    if(it == g_dmapFilesParser.end())
      mexErrMsgTxt("Unknown device '" + deviceName + "'.");

	//open the devPCIE
    boost::shared_ptr<devPCIE> pdev( new devPCIE );
    pdev->openDev(it->first.dev_file, O_RDWR, NULL);

	//open the mapFile
    openDevicesMap[deviceName].reset( new devMap< devPCIE > );
    openDevicesMap[deviceName]->openDev( pdev, it->second ); //never thows

//#ifdef __MEX_DEBUG_MODE
    mexPrintf("Successfully opened "+ deviceName + " at " + it->first.dev_file + "\n");
//#endif
  }
  return openDevicesMap[deviceName];
} */

/**
 * @brief getDevice
 *
 * @param[in] dmapFileName File to be loaded or all in the current directory if empty
 *
 */
//boost::shared_ptr< devMap<devPCIE> > getDevice(const mxArray *prhsDevice) //, const string &dmapFileName = "")
boost::shared_ptr<mtca4u::Device> getDevice(const mxArray *prhsDevice)
{
  	 
  if (!mxIsRealScalar(prhsDevice))
    mexErrMsgTxt("Invalid device handle.");

  const size_t deviceHandle = mxGetScalar(prhsDevice);

  if (deviceHandle >= openDevicesVector.size())
    mexErrMsgTxt("Invalid device handle.");
 
  if (!openDevicesVector[deviceHandle])
    mexErrMsgTxt("Device closed.");

  return openDevicesVector[deviceHandle];
}

/**
 * @brief PrintHelp shows the help text on the console
 *
 */
void PrintHelp(unsigned int, mxArray **, unsigned int, const mxArray **)
{
  mexPrintf(std::string("mtca4u Matlab library revision: ") + gVersion + "\n");
  mexPrintf("Available Commands are: \n");

  for (vector<Command>::iterator it = vectorOfCommands.begin(); it != vectorOfCommands.end(); ++it)
  {
    mexPrintf("\t" + it->Name + "\n");
  }
  mexPrintf("\n\nFor further help or bug reports please contact michael.heuer@desy.de");
}

/**
 * @brief Returns the version as a number
 *
 */
void getVersion(unsigned int, mxArray *plhs[], unsigned int, const mxArray **)
{
  plhs[0] = mxCreateString(gVersion.c_str());
}

/**
 * @brief Force to open the device
 *
 */
void openDevice(unsigned int, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 1) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
  
  //dmapFilesParser parser(".");
  DMapFilesParser parser(".");
  
  std::string deviceName = mxArrayToStdString(prhs[0]);
  
  // Look for device
  //dmapFilesParser::iterator it = parser.begin();
  DMapFilesParser::iterator it = parser.begin();
  for (; it != parser.end(); ++it)
  {
    if (deviceName == it->first.deviceName)
      break;
  }

  if(it == parser.end())
    mexErrMsgTxt("Unknown device '" + deviceName + "'.");
    

  //open the devPCIE
  //boost::shared_ptr<devPCIE> pdev( new devPCIE );
  //pdev->openDev(it->first.dev_file, O_RDWR, NULL);
  
  //open the mapFile  
  //openDevicesVector.push_back( boost::shared_ptr< devMap<devPCIE> > (new devMap< devPCIE >) );
  openDevicesVector.push_back( boost::shared_ptr<Device> ( new mtca4u::Device()) );
  //openDevicesVector.back()->openDev( pdev, it->second ); //never thows
  //openDevicesVector.back()->open( pdev, it->second ); //never thows
  openDevicesVector.back()->open(it->first.deviceName);
  
  plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
  (*mxGetPr(plhs[0])) = openDevicesVector.size()-1;
  
  #ifdef __MEX_DEBUG_MODE
  mexPrintf("Successfully opened "+ deviceName + " at " + it->first.uri + "\n");
  #endif
}

/**
 * @brief Closes a open device
 *
 */
void closeDevice(unsigned int, mxArray **, unsigned int nrhs, const mxArray *prhs[])
{
	mexPrintf("closeDevice");
  if (nrhs < 1) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 1) mexWarnMsgTxt("Too many input arguments.");

  if (!mxIsRealScalar(prhs[0])) mexErrMsgTxt("Invalid device handle.");

  const size_t deviceHandle = mxGetScalar(prhs[0]);

  if (deviceHandle >= openDevicesVector.size())
    mexErrMsgTxt("Invalid device handle.");

  openDevicesVector[deviceHandle].reset();

  #ifdef __MEX_DEBUG_MODE
  mexPrintf("Device closed\n");
  #endif
}

/**
 * @brief refreshDmap
 *
 * Parameter: dmap
 */
/*void refreshDmap(unsigned int, mxArray **, unsigned int nrhs, const mxArray **)
{
  if (nrhs > 0) mexWarnMsgTxt("To many input arguments.");

  //g_lastDmapFile = mxArrayToStdString(prhs[0]);
  g_dmapFilesParser.parse_dir(".");
} */

/**
 * @brief getInfo
 *
 */
void getInfo(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray **)
{
  if(nrhs > 0) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  //dmapFilesParser parser(".");
  DMapFilesParser parser(".");

  mwSize dims[2] = {1, (int)parser.getdMapFileSize()};
  const char *field_names[] = {"name", "device", "firmware", "date", "map"};
  plhs[0] = mxCreateStructArray(2, dims, (sizeof(field_names)/sizeof(*field_names)), field_names);

  unsigned int i = 0;

  //for (dmapFilesParser::iterator it = parser.begin(); it != parser.end(); ++it, ++i)
  for (DMapFilesParser::iterator it = parser.begin(); it != parser.end(); ++it, ++i)
  {
    mxArray *firmware_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(firmware_value) = 0;

    std::string date;

    try {
      //devMap<devPCIE> tempDevice;
      
      std::shared_ptr<Device> tempDevice( new Device());
      //tempDevice.openDev(it->first.dev_file, it->first.map_file_name);
      tempDevice->open(it->first.deviceName);

      int firmware = 0;
      tempDevice->readReg("BOARD0", "WORD_FIRMWARE", &firmware);
      *mxGetPr(firmware_value) = firmware;

      int timestamp = 0;
      tempDevice->readReg("BOARD0", "WORD_TIMESTAMP", &timestamp);
      date = timestamp;
    }
    catch(...) { }

    mxSetFieldByNumber(plhs[0], i, 0, mxCreateString(it->first.deviceName.c_str()));
    mxSetFieldByNumber(plhs[0], i, 1, mxCreateString(it->first.uri.c_str()));
    mxSetFieldByNumber(plhs[0], i, 2, firmware_value);
    mxSetFieldByNumber(plhs[0], i, 3, mxCreateString(date.c_str()));
    mxSetFieldByNumber(plhs[0], i, 4, mxCreateString(it->first.mapFileName.c_str()));
  }
}

/**
 * @brief getDeviceInfo
 *
 */
void getDeviceInfo(unsigned int nlhs, mxArray ** plhs, unsigned int nrhs, const mxArray *prhs[])
{
std::cout<<"getDeviceInfo"<<std::endl;
  if(nrhs < 1) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");
std::cout<<"no error"<<std::endl;
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[0]);
  boost::shared_ptr<mtca4u::Device> device = getDevice(prhs[0]);

  

  const boost::shared_ptr<const mtca4u::RegisterInfoMap> map = device->getRegisterMap();

  const char *field_names[] = {"name", "elements", "signed", "bits", "fractional_bits", "description"};
  
  plhs[0] = mxCreateStructMatrix(map->getMapFileSize(), 1, (sizeof(field_names)/sizeof(*field_names)), field_names);

  unsigned int index = 0;
  for (std::vector<RegisterInfoMap::RegisterInfo>::const_iterator cit = map->begin(); cit != map->end(); ++cit, ++index) {
	  mxSetFieldByNumber(plhs[0], index, 0, mxCreateString(cit->name.c_str()));

    mxArray *numElements = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numElements) = cit->nElements;
    mxSetFieldByNumber(plhs[0], index, 1, numElements);

    mxArray *signedFlag = mxCreateLogicalMatrix(1,1);
    *mxGetPr(signedFlag) = cit->signedFlag;
    mxSetFieldByNumber(plhs[0], index, 2, signedFlag);

    mxArray *numBits = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numBits) = cit->width;
    mxSetFieldByNumber(plhs[0], index, 3, numBits);

    mxArray *numFractionalBits = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numFractionalBits) = cit->nFractionalBits;
    mxSetFieldByNumber(plhs[0], index, 4, numFractionalBits);

    mxSetFieldByNumber(plhs[0], index, 5, mxCreateString(""));
  }
  std::cout<<"index:"<<index<<std::endl;
}

/**
 * @brief getRegisterInfo
 *
 */
void getRegisterInfo(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 3) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");
  
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[0]);
  boost::shared_ptr<Device> device = getDevice(prhs[0]);

  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(2) + " input argument.");
  if (!mxIsChar(prhs[2])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(2) + " input argument.");

  //boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[2]), mxArrayToStdString(prhs[1]));
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[2]), mxArrayToStdString(prhs[1]));
  RegisterInfoMap::RegisterInfo regInfo = reg->getRegisterInfo();

  //mwSize dims[2] = {1, 1};
  const char *field_names[] = {"name", "elements", "signed", "bits", "fractional_bits", "description"};
  plhs[0] = mxCreateStructMatrix(1, 1, (sizeof(field_names)/sizeof(*field_names)), field_names);

  mxSetFieldByNumber(plhs[0], 0, 0, mxCreateString(regInfo.name.c_str()));

  mxArray *numElements = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numElements) = regInfo.nElements;
  mxSetFieldByNumber(plhs[0], 0, 1, numElements);

  mxArray *signedFlag = mxCreateLogicalMatrix(1,1);
  *mxGetPr(signedFlag) = regInfo.signedFlag;
  mxSetFieldByNumber(plhs[0], 0, 2, signedFlag);

  mxArray *numBits = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numBits) = regInfo.width;
  mxSetFieldByNumber(plhs[0], 0, 3, numBits);

  mxArray *numFractionalBits = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numFractionalBits) = regInfo.nFractionalBits;
  mxSetFieldByNumber(plhs[0], 0, 4, numFractionalBits);

  mxSetFieldByNumber(plhs[0], 0, 5, mxCreateString(""));
}

/**
 * @brief getRegisterSize
 *
 */
void getRegisterSize(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 3) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");
  
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[0]);
  boost::shared_ptr<Device> device = getDevice(prhs[0]);
  
  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(1) + " input argument.");
  if (!mxIsChar(prhs[2])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(2) + " input argument.");

  //boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[2]), mxArrayToStdString(prhs[1]));
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[2]), mxArrayToStdString(prhs[1]));

  plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
  (*mxGetPr(plhs[0])) = reg->getRegisterInfo().nElements;
}

/**
 * @brief readRegister
 *
 * Parameter: device, module, register, [offset], [elements]
 */
void readRegister(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_offset = 3, pp_elements = 4;
  std::cout<<"readRegister"<<std::endl;
  if (nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 5) mexWarnMsgTxt("To many input arguments.");
  if (nlhs > 1) mexErrMsgTxt("To many output arguments.");
  
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[pp_device]);
  boost::shared_ptr<Device> device = getDevice(prhs[pp_device]);
  
  if (!mxIsChar(prhs[pp_module])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_module) + " input argument.");
  if (!mxIsChar(prhs[pp_register])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_register) + " input argument.");
	
  if ((nrhs > pp_offset) && ( !mxIsRealScalar(prhs[pp_offset]) || (mxGetScalar(prhs[pp_offset]) < 0) ))
    mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_offset) + " input argument.");
    
  if ((nrhs > pp_elements) && !mxIsPositiveRealScalar(prhs[pp_elements]))
    mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_elements) + " input argument.");
	
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]),mxArrayToStdString(prhs[pp_module]));

  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;
  //if(offset == 0) mexErrMsgTxt("Subscript indices must be real positive integers. Instead of C/C++ the first element in MATLAB is 1.");
  
  const uint32_t elements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : (reg->getRegisterInfo().nElements - offset);

  plhs[0] = mxCreateDoubleMatrix(1, elements, mxREAL);
  double *plhsValue = mxGetPr(plhs[0]);
  reg->read(plhsValue, elements, offset);
}

/**
 * @brief writeRegister
 *
 * Parameter: device, module, register, value, [offset]
 */
void writeRegister(unsigned int, mxArray **, unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_value = 3, pp_offset = 4;
  
  if (nrhs < 4) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 5) mexWarnMsgTxt("To many input arguments.");
  
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[pp_device]);
  boost::shared_ptr<Device> device = getDevice(prhs[pp_device]);
	
  if (!mxIsChar(prhs[pp_module]))  mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_module) + " input argument.");
  if (!mxIsChar(prhs[pp_register]))  mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_register) + " input argument.");

  if (!mxIsNumeric(prhs[pp_value]) || mxIsComplex(prhs[pp_value]))
    mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_value) + " input argument.");
  
  if ((nrhs > pp_offset) && (!mxIsRealScalar(prhs[pp_offset]) ||(mxGetScalar(prhs[pp_offset]) < 0)))
    mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_offset) + " input argument.");

  //boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]), mxArrayToStdString(prhs[pp_module]));
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]), mxArrayToStdString(prhs[pp_module]));

  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;

  mxClassID arrayType = mxGetClassID(prhs[pp_value]);
  size_t prhsValueElements = mxGetNumberOfElements(prhs[pp_value]);
  void *data = mxGetData(prhs[pp_value]);

  // TODO: Extract this to a method if this has to be used else where
  if (arrayType == mxDOUBLE_CLASS) {
    reg->write(static_cast<double *>(data), prhsValueElements, offset);
  } else if (arrayType == mxUINT8_CLASS) {
    reg->write(static_cast<uint8_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxINT8_CLASS) {
    reg->write(static_cast<int8_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxINT16_CLASS) {
    reg->write(static_cast<int16_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxUINT16_CLASS) {
    reg->write(static_cast<uint16_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxINT32_CLASS) {
    reg->write(static_cast<int32_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxUINT32_CLASS) {
    reg->write(static_cast<uint32_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxINT64_CLASS) {
    reg->write(static_cast<int64_t *>(data), prhsValueElements, offset);
  } else if (arrayType == mxUINT64_CLASS) {
    reg->write(static_cast<uint64_t *>(data), prhsValueElements, offset);
  } else {
    mexErrMsgTxt("Data type unsupported.");
  }
}

/**
 * @brief readRawDmaData
 *
 */
//template<class T> void innerRawDMARead(const boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> &reg, double* dest, const size_t nElements, const size_t nOffset, const FixedPointConverter &conv)
template<class T> void innerRawDMARead(const boost::shared_ptr<Device::RegisterAccessor> &reg, double* dest, const size_t nElements, const size_t nOffset, const FixedPointConverter &conv)
{
  std::vector<T> dmaValue(nElements);
  reg->readDMA(reinterpret_cast<int32_t*>(&dmaValue[0]), nElements*sizeof(T), nOffset);

  // copy and transform data
  for(unsigned int i = 0; i < nElements; i++)
    dest[i] = conv.toDouble(dmaValue[i]);
}

/**
 * @brief readRawDmaData
 *
 * Parameter: device, module, register, [offset], [elements], [mode], [singed], [bit], [fracbit]
 */
void readDmaRaw(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_offset = 3, pp_elements = 4;
  static const unsigned int pp_mode = 5, pp_signed = 6, pp_bit = 7, pp_fracbit = 8;
	
  if (nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 9) mexWarnMsgTxt("Too many input arguments.");
  if (nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[pp_device]);
  boost::shared_ptr<Device> device = getDevice(prhs[pp_device]);
	
  if (!mxIsChar(prhs[pp_module])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_module) + " input argument."); 
  if (!mxIsChar(prhs[pp_register])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_register) + " input argument."); 
	
  if ((nrhs > pp_offset) && !mxIsRealScalar(prhs[pp_offset])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_offset) + " input argument.");
  if ((nrhs > pp_elements) && !mxIsPositiveRealScalar(prhs[pp_elements])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_elements) + " input argument.");
  
  if ((nrhs > pp_mode) && !mxIsPositiveRealScalar(prhs[pp_mode])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_mode) + " input argument.");
  if ((nrhs > pp_signed) && !mxIsRealScalar(prhs[pp_signed])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_signed) + " input argument.");
  if ((nrhs > pp_bit) && !mxIsPositiveRealScalar(prhs[pp_bit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_bit) + " input argument.");
  if ((nrhs > pp_fracbit) && !mxIsRealScalar(prhs[pp_fracbit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_fracbit) + " input argument.");
  
  //boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]), mxArrayToStdString(prhs[pp_module]));
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]), mxArrayToStdString(prhs[pp_module]));
  
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;
  const uint32_t elements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : (reg->getRegisterInfo().nElements - offset);
    
  const uint32_t mode = (nrhs > pp_mode) ? mxGetScalar(prhs[pp_mode]): 32;
  if ((mode != 32) && (mode != 16)) mexErrMsgTxt("Invalid data mode.");
  
  const bool signedFlag = (nrhs > pp_signed) ? mxGetBoolScalar(prhs[pp_signed]): 0;
  const uint32_t bits = (nrhs > pp_bit) ? mxGetScalar(prhs[pp_bit]) : mode;
  const uint32_t fracBits = (nrhs > pp_fracbit) ? mxGetScalar(prhs[pp_fracbit]) : 0;
  FixedPointConverter conv(bits, fracBits, signedFlag);
  
  plhs[0] = mxCreateDoubleMatrix(elements, 1, mxREAL);
  double *plhsValue = mxGetPr(plhs[0]);
  
  if (mode == 32)
    innerRawDMARead<int32_t>(reg, plhsValue, elements, offset, conv);
  else
    innerRawDMARead<int16_t>(reg, plhsValue, elements, offset, conv);
}
//Not supported any more. 
#ifdef _OLD
/**
 * @brief readDmaChannel
 *
 * Parameter: device, module, register, [channel], [offset], [elements], [channel], [mode], [singed], [bit], [fracbit]
 */
void readDmaChannel(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_channel = 3, pp_offset = 4, pp_elements = 5;
  static const unsigned int pp_channel_cnt = 6, pp_mode = 7, pp_signed = 8, pp_bit = 9, pp_fracbit = 10;
  
  if (nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 11) mexWarnMsgTxt("Too many input arguments.");
  
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[pp_device]);
  boost::shared_ptr<Device> device = getDevice(prhs[pp_device]);
   
  if (!mxIsChar(prhs[pp_module])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_module) + " input argument.");
  if (!mxIsChar(prhs[pp_register])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_register) + " input argument.");
  
  if (!mxIsPositiveRealVector(prhs[pp_channel])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_channel) + " input argument.");
  
  if ((nlhs > 1) && !(nrhs > pp_channel)) mexErrMsgTxt("Not enough input arguments.");
  if ((nlhs > 1) && (nrhs > pp_channel) && (nlhs > mxGetNumberOfElements(prhs[pp_channel]))) mexErrMsgTxt("Too many output arguments.");
  if ((nlhs > 1) && (nrhs > pp_channel) && (nlhs < mxGetNumberOfElements(prhs[pp_channel]))) mexErrMsgTxt("Not enough output arguments.");

  if ((nrhs > pp_offset) && !mxIsRealScalar(prhs[pp_offset])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_offset) + " input argument.");
  if ((nrhs > pp_elements) && !mxIsPositiveRealScalar(prhs[pp_elements])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_elements) + " input argument.");
  if ((nrhs > pp_channel_cnt) && !mxIsPositiveRealScalar(prhs[pp_channel_cnt])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_channel_cnt) + " input argument.");
  if ((nrhs > pp_mode) && !mxIsPositiveRealScalar(prhs[pp_mode])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_mode) + " input argument.");
  if ((nrhs > pp_signed) && !mxIsRealScalar(prhs[pp_signed])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_signed) + " input argument.");
  if ((nrhs > pp_bit) && !mxIsPositiveRealScalar(prhs[pp_bit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_bit) + " input argument.");
  if ((nrhs > pp_fracbit) && !mxIsRealScalar(prhs[pp_fracbit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_fracbit) + " input argument.");
  
  //boost::shared_ptr<devMap<devPCIE>::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]),mxArrayToStdString(prhs[pp_module]));
  boost::shared_ptr<Device::RegisterAccessor> reg = device->getRegisterAccessor(mxArrayToStdString(prhs[pp_register]),mxArrayToStdString(prhs[pp_module]));
  
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]): 0;
  const uint32_t elements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : (reg->getRegisterInfo().nElements - offset);
  
  const uint32_t totalChannels = (nrhs > pp_channel_cnt) ? mxGetScalar(prhs[pp_channel_cnt]) : 8;
  //if ((totalChannels != 8) && (totalChannels != 16)) mexErrMsgTxt("Invalid number of channels.");
  
  const uint32_t mode = (nrhs > pp_mode) ? mxGetScalar(prhs[pp_mode]) : 32;
  if ((mode != 32) && (mode != 16)) mexErrMsgTxt("Invalid data mode.");
 
  const uint32_t size = elements*totalChannels;
  std::vector<double> dmaValue(size);
  
  const bool signedFlag = (nrhs > pp_signed) ? mxGetBoolScalar(prhs[pp_signed]): 0;
  const uint32_t bits = (nrhs > pp_bit) ? mxGetScalar(prhs[pp_bit]) : mode;
  const uint32_t fracBits = (nrhs > pp_fracbit) ? mxGetScalar(prhs[pp_fracbit]) : 0;
  FixedPointConverter conv(bits, fracBits, signedFlag);
  
  if (mode == 32) 
    innerRawDMARead<int32_t>(reg, &dmaValue[0], size, offset, conv);
  else 
    innerRawDMARead<int16_t>(reg, &dmaValue[0], size, offset, conv);

  // Store data in different vectors passed over lhs
  if (nlhs == mxGetNumberOfElements(prhs[pp_channel]))
  {
    for(unsigned int ic = 0; ic < nlhs; ic++)
    {
      plhs[ic] = mxCreateDoubleMatrix(elements, 1, mxREAL);
      double *plhsValue = mxGetPr(plhs[ic]);
      unsigned int currentChannel = (mxGetPr(prhs[pp_channel])[ic] - 1);

      for(unsigned int is = 0; is < elements; is++)
      {
        unsigned int dmaIndex = is*totalChannels + currentChannel;
        if (dmaIndex >= size) mexErrMsgTxt("Data storage indexing went wrong!");
        plhsValue[is] = (currentChannel < totalChannels) ? dmaValue[dmaIndex] : 0;
      }
    }
  }
  // Store data in lhs matrix
  else
  {
    unsigned int selectedChannels = (nrhs > pp_channel) ? mxGetNumberOfElements(prhs[pp_channel]) : totalChannels; 
    plhs[0] = mxCreateDoubleMatrix(elements, selectedChannels, mxREAL);
    double *plhsValue = mxGetPr(plhs[0]);
    
    // copy and transform data
    for(unsigned int ic = 0; ic < selectedChannels; ic++)
    {
      unsigned int currentChannel = (nrhs > pp_channel) ? (mxGetPr(prhs[pp_channel])[ic] - 1) : ic;
      for(unsigned int is = 0; is < elements; is++)
      {
        unsigned int plhsIndex = ic*elements + is;
        unsigned int dmaIndex = is*totalChannels + currentChannel;

        if (dmaIndex >= size) mexErrMsgTxt("Data storage indexing went wrong! (1)");
        if (plhsIndex >= elements*selectedChannels) mexErrMsgTxt("Data storage indexing went wrong! (2)");

        plhsValue[plhsIndex] = (currentChannel < totalChannels) ? dmaValue[dmaIndex] : 0;
      }
    }
  }
}
#endif
/**
 * @brief readSequence
 *
 * Parameter: device, module, register, [channel], [offset], [elements]
 */
void readSequence(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_channel = 3, pp_offset = 4, pp_elements = 5;
  
  if (nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 6) mexWarnMsgTxt("Too many input arguments.");
    
  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[pp_device]);
  boost::shared_ptr<Device> device = getDevice(prhs[pp_device]);
   
  if (!mxIsChar(prhs[pp_module])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_module) + " input argument.");
  if (!mxIsChar(prhs[pp_register])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_register) + " input argument.");
  
  if ((nrhs > pp_channel) && !mxIsPositiveRealVector(prhs[pp_channel])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_channel) + " input argument.");
  
  if ((nlhs > 1) && !(nrhs > pp_channel)) mexErrMsgTxt("Not enough input arguments.");
  if ((nlhs > 1) && (nrhs > pp_channel) && (nlhs > mxGetNumberOfElements(prhs[pp_channel]))) mexErrMsgTxt("Too many output arguments.");
  if ((nlhs > 1) && (nrhs > pp_channel) && (nlhs < mxGetNumberOfElements(prhs[pp_channel]))) mexErrMsgTxt("Not enough output arguments.");

  if ((nrhs > pp_offset) && !mxIsRealScalar(prhs[pp_offset])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_offset) + " input argument.");
  if ((nrhs > pp_elements) && !mxIsPositiveRealScalar(prhs[pp_elements])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_elements) + " input argument.");
  
  boost::shared_ptr<MultiplexedDataAccessor<double>> reg = device->getCustomAccessor<MultiplexedDataAccessor<double>>(mxArrayToStdString(prhs[pp_register]),mxArrayToStdString(prhs[pp_module]));
  reg->read();
  
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]): 0;
  const uint32_t elements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : ((*reg)[0].size());
  
  const uint32_t totalChannels = reg->getNumberOfDataSequences();
   
  const uint32_t size = elements*totalChannels;
  std::vector<double> dmaValue(size);
   
  unsigned int selectedChannels = (nrhs > pp_channel) ? mxGetNumberOfElements(prhs[pp_channel]) : totalChannels;    
  // Store data in different vectors passed over lhs
  if (nlhs == selectedChannels)
  {
    for(unsigned int ic = 0; ic < nlhs; ic++)
    {
      plhs[ic] = mxCreateDoubleMatrix(elements, 1, mxREAL);
      double *plhsValue = mxGetPr(plhs[ic]);
      unsigned int currentChannel = (nrhs > pp_channel) ? (mxGetPr(prhs[pp_channel])[ic] - 1) : ic;

      for(unsigned int is = 0; is < elements; is++)
      {
        unsigned int seqIndex = (is+offset);
        if (seqIndex >= size) mexErrMsgTxt("Data storage indexing went wrong!");
        plhsValue[is] = (*reg)[currentChannel][seqIndex];
      }
    }
  }
  // Store data in lhs matrix
  else
  {
    plhs[0] = mxCreateDoubleMatrix(elements, selectedChannels, mxREAL);
    double *plhsValue = mxGetPr(plhs[0]);
    
    // copy and transform data
    for(unsigned int ic = 0; ic < selectedChannels; ic++)
    {
      unsigned int currentChannel = (nrhs > pp_channel) ? (mxGetPr(prhs[pp_channel])[ic] - 1) : ic;
	  
	  if (currentChannel < 0 || currentChannel >= totalChannels) mexErrMsgTxt("Illegal Channel Index");
		  
      for(unsigned int is = 0; is < elements; is++)
      {
        unsigned int plhsIndex = ic*elements + is;
        unsigned int seqIndex = (is+offset);        

        if (seqIndex >= size) mexErrMsgTxt("Data storage indexing went wrong! (1)");
        if (plhsIndex >= elements*selectedChannels) mexErrMsgTxt("Data storage indexing went wrong! (2)");

        plhsValue[plhsIndex] = (*reg)[currentChannel][seqIndex];
      }
    }
  }
}
