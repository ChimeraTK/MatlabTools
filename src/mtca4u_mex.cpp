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
#include <ChimeraTK/Utilities.h>
#include <ChimeraTK/BackendFactory.h>
#include <ChimeraTK/DMapFilesParser.h>
#include <ChimeraTK/Device.h>
#include <ChimeraTK/MultiplexedDataAccessor.h>
#include <ChimeraTK/RegisterPath.h>

#include "../include/version.h"

using namespace ChimeraTK;
using namespace std;

typedef MultiplexedDataAccessor<double> dma_accessor;
typedef boost::shared_ptr<MultiplexedDataAccessor<double>> dma_Accessor_ptr;

template <typename UserType>
void writeToDevice(boost::shared_ptr<Device> &device,
                     const std::string &registerPath, void *data,
                     size_t numberOfwords, size_t Offset);

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
boost::shared_ptr<Device> getDevice(const mxArray *plhsDevice);
//boost::shared_ptr< devMap<devPCIE> > getDevice(const mxArray *plhsDevice);

// Global Parameter

//std::vector<boost::shared_ptr< devMap<devPCIE> > > openDevicesVector;
bool isInit = false; // Used to initalize stuff at the first run
std::vector<boost::shared_ptr<Device> > openDevicesVector;

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
void setDMapFilePath(unsigned int, mxArray**, unsigned int, const mxArray **);
void getDMapFilePath(unsigned int, mxArray**, unsigned int, const mxArray **);

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
  Command("set_dmap", &setDMapFilePath, "", ""),
  Command("get_dmap", &getDMapFilePath, "", "")
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

  // Initalize basic stuff
  if (isInit == false) {
    ChimeraTK::setDMapFilePath("./devices.dmap");
    // mexAtExit(CleanUp); // register Cleanup Function
    isInit = true;
  }

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

void getDMapFilePath(unsigned int, mxArray *plhs[], unsigned int, const mxArray **)
{
   plhs[0] = mxCreateString((BackendFactory::getInstance().getDMapFilePath()).c_str());
}


/**
 * @brief setDMapFilePath
 *
 */
void setDMapFilePath(unsigned int, mxArray**, unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 1) mexErrMsgTxt("Not enough input params.");
  if(nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
  std::string dMapFile = mxArrayToStdString(prhs[0]);
  ChimeraTK::setDMapFilePath(dMapFile);
}

/**
 * @brief getDevice
 *
 * @param[in] dmapFileName File to be loaded or all in the current directory if empty
 *
 */
boost::shared_ptr<Device> getDevice(const mxArray *prhsDevice)
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
  mexPrintf("\n\nFor further help or bug reports please contact chimeratk-support@desy.de\n");
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

  std::string deviceName = mxArrayToStdString(prhs[0]);

  openDevicesVector.push_back( boost::shared_ptr<Device> ( new Device()) );
  openDevicesVector.back()->open(deviceName);

  plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
  (*mxGetPr(plhs[0])) = openDevicesVector.size()-1;

  #ifdef __MEX_DEBUG_MODE
  mexPrintf("Successfully opened "+ deviceName + "\n");
  #endif
}

/**
 * @brief Closes a open device
 *
 */
void closeDevice(unsigned int, mxArray **, unsigned int nrhs, const mxArray *prhs[])
{
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
      ChimeraTK::setDMapFilePath( it->first.dmapFileName );
      Device tempDevice;
      tempDevice.open(it->first.deviceName);

      int firmware = tempDevice.read<int>("BOARD0/WORD_FIRMWARE");
      *mxGetPr(firmware_value) = firmware;

      int timestamp = tempDevice.read<int>("BOARD0/WORD_TIMESTAMP");
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

std::string getFundamentalTypeString( ChimeraTK::RegisterInfo::FundamentalType fundamentalType){
  if (fundamentalType ==  RegisterInfo::FundamentalType::numeric)   return "numeric";
  if (fundamentalType ==  RegisterInfo::FundamentalType::string)    return "string";
  if (fundamentalType ==  RegisterInfo::FundamentalType::boolean)   return "boolean";
  if (fundamentalType ==  RegisterInfo::FundamentalType::nodata)    return "nodata";
  if (fundamentalType ==  RegisterInfo::FundamentalType::undefined) return "undefined";
  return "unknown";
}

/**
 * @brief getDeviceInfo
 *
 */
void getDeviceInfo(unsigned int nlhs, mxArray ** plhs, unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 1) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  //boost::shared_ptr< devMap<devPCIE> > device = getDevice(prhs[0]);
  boost::shared_ptr<Device> device = getDevice(prhs[0]);

  auto & registerCatalogue = device->getRegisterCatalogue();

  //                    index: 0       1            2            3              4                  5
  const char *field_names[] = {"name", "nElementsPerChannel", "nChannels", "nDimensions", "fundamentalType", "description"};

  plhs[0] = mxCreateStructMatrix(registerCatalogue.getNumberOfRegisters(), 1, (sizeof(field_names)/sizeof(*field_names)), field_names);

  unsigned int index = 0;
  for (RegisterInfoMap::const_iterator cit = registerCatalogue.begin(); cit != registerCatalogue.end(); ++cit, ++index) {

    mxSetFieldByNumber(plhs[0], index, 0, mxCreateString(std::string(cit->getRegisterName()).c_str()));

    mxArray *numElements = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numElements) = cit->getNumberOfElements();
    mxSetFieldByNumber(plhs[0], index, 1, numElements);

    mxArray *numChannels = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numChannels) = cit->getNumberOfChannels();
    mxSetFieldByNumber(plhs[0], index, 2, numChannels);

    mxArray *numDimensions = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(numDimensions) = cit->getNumberOfDimensions();
    mxSetFieldByNumber(plhs[0], index, 3, numDimensions);

    auto fundamentalType =  cit->getDataDescriptor().fundamentalType();
    mxSetFieldByNumber(plhs[0], index, 4, mxCreateString(getFundamentalTypeString(fundamentalType).c_str()));
    
    mxSetFieldByNumber(plhs[0], index, 5, mxCreateString(""));
  }
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

  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(2) + " input argument.");
  if (!mxIsChar(prhs[2])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(3) + " input argument.");

  auto device = getDevice(prhs[0]);
  auto & registerCatalogue = device->getRegisterCatalogue();

  RegisterPath moduleName( mxArrayToStdString(prhs[1]) );
  RegisterPath registerName( mxArrayToStdString(prhs[2]) );
  auto registerInfo = registerCatalogue.getRegister(moduleName/registerName);
  auto & dataDescriptor = registerInfo->getDataDescriptor();
 
  //                    index: 0       1                      2            3              4                  5              6          7
  const char *field_names[] = {"name", "nElementsPerChannel", "nChannels", "nDimensions", "fundamentalType", "isIntegral", "isSigned", "description"};
  plhs[0] = mxCreateStructMatrix(1, 1, (sizeof(field_names)/sizeof(*field_names)), field_names);

  mxSetFieldByNumber(plhs[0], 0, 0, mxCreateString(std::string(registerInfo->getRegisterName()).c_str()));

  mxArray *numElements = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numElements) = registerInfo->getNumberOfElements();
  mxSetFieldByNumber(plhs[0], 0, 1, numElements);

  mxArray *numChannels = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numChannels) = registerInfo->getNumberOfChannels();
  mxSetFieldByNumber(plhs[0], 0, 2, numChannels);

  mxArray *numDimensions = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numDimensions) = registerInfo->getNumberOfDimensions();
  mxSetFieldByNumber(plhs[0], 0, 3, numDimensions);

  auto fundamentalType =  dataDescriptor.fundamentalType();

  mxSetFieldByNumber(plhs[0], 0, 4, mxCreateString(getFundamentalTypeString(fundamentalType).c_str()));

  bool isIntegral =  false; // default value in case the data type is not numeric
  bool isSigned = false; // default value in case the data type is not numeric
  if (fundamentalType ==  RegisterInfo::FundamentalType::numeric){
    // These operations on the data descriptor are only valid for numeric types.
    // They will throw otherwise.
    isIntegral = dataDescriptor.isIntegral();
    isSigned = dataDescriptor.isSigned();
  }
  
  mxArray *integralFlag = mxCreateLogicalMatrix(1,1);
  *mxGetPr(integralFlag) = isIntegral;
  mxSetFieldByNumber(plhs[0], 0, 5, integralFlag);

  mxArray *signedFlag = mxCreateLogicalMatrix(1,1);
  *mxGetPr(signedFlag) = isSigned;
  mxSetFieldByNumber(plhs[0], 0, 6, signedFlag);

  mxSetFieldByNumber(plhs[0], 0, 7, mxCreateString(""));
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

  RegisterPath moduleName( mxArrayToStdString(prhs[1]) );
  RegisterPath registerName( mxArrayToStdString(prhs[2]) );
  auto reg = device->getOneDRegisterAccessor<double>(moduleName/registerName);

  plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
  (*mxGetPr(plhs[0])) = reg.getNElements();
}

/**
 * @brief readRegister
 *
 * Parameter: device, module, register, [offset], [elements]
 */
void readRegister(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  static const unsigned int pp_device = 0, pp_module = 1, pp_register = 2, pp_offset = 3, pp_elements = 4;

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

  // offset is optional. Use 0 if not set
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;

  // number of elements is optional. Use 0 (=all remaining) if not set
  const uint32_t nElements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : 0;

  RegisterPath moduleName( mxArrayToStdString(prhs[pp_module]) );
  RegisterPath registerName( mxArrayToStdString(prhs[pp_register]) );
  // we get the register content as a std::vector<double>
  auto registerContent = device->read<double>(moduleName/registerName, nElements, offset);

  // as both DeviceAccess and Matlab do their own memory allocation all we can do is memcpy :-(
  plhs[0] = mxCreateDoubleMatrix(1, registerContent.size(), mxREAL);
  double *plhsValue = mxGetPr(plhs[0]);
  memcpy( plhsValue, registerContent.data(), registerContent.size()*sizeof(double));
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

  mxClassID arrayType = mxGetClassID(prhs[pp_value]);
  size_t prhsValueElements = mxGetNumberOfElements(prhs[pp_value]);
  void *data = mxGetData(prhs[pp_value]);

  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;
  std::string registerPath = mxArrayToStdString(prhs[pp_module]) + '/' + mxArrayToStdString(prhs[pp_register]) ;


  // TODO: Extract this to a method if this has to be used else where
  if (arrayType == mxDOUBLE_CLASS) {
    writeToDevice<double>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxUINT8_CLASS) {
    writeToDevice<uint8_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxINT8_CLASS) {
    writeToDevice<int8_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxINT16_CLASS) {
    writeToDevice<int16_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxUINT16_CLASS) {
    writeToDevice<uint16_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxINT32_CLASS) {
    writeToDevice<int32_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxUINT32_CLASS) {
    writeToDevice<uint32_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxINT64_CLASS) {
    writeToDevice<int64_t>(device, registerPath, data, prhsValueElements, offset);
  } else if (arrayType == mxUINT64_CLASS) {
    writeToDevice<uint64_t>(device, registerPath, data, prhsValueElements, offset);
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

  // FIXME: These don't make sense in raw mode. Shall we just skip the checks? They are not used anyway.
  if ((nrhs > pp_signed) && !mxIsRealScalar(prhs[pp_signed])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_signed) + " input argument.");
  if ((nrhs > pp_bit) && !mxIsPositiveRealScalar(prhs[pp_bit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_bit) + " input argument.");
  if ((nrhs > pp_fracbit) && !mxIsRealScalar(prhs[pp_fracbit])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(pp_fracbit) + " input argument.");

  // offset is optional. Use 0 if not set
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]) : 0;
  // number of elements is optional. Use 0 (=all remaining) if not set
  const uint32_t nElements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : 0;
    
  // The mode can be 16 or 32 bit. This does not make sense because the real raw stuff is 32 bits,
  // it was there so we keep it in order not to break compatibility. 
  const uint32_t mode = (nrhs > pp_mode) ? mxGetScalar(prhs[pp_mode]): 32;
  if ((mode != 32) && (mode != 16)) mexErrMsgTxt("Invalid data mode.");

  // Number of 32 bit words to read through the "bus". This currently is the only mode the raw accessor knows
  uint32_t nWords32Bit=nElements;
  if (mode==16){
    // The number of 32 bit words is only half the number of elements if the are requested as 16 bit
    nWords32Bit/=2;
  }  

  // Notice: signedFlags, bits and fracBits have been removed as they were not/cannot be used anyway

  // Now that we have all parameters it's time to read the data from the device.
  RegisterPath moduleName( mxArrayToStdString(prhs[pp_module]) );
  RegisterPath registerName( mxArrayToStdString(prhs[pp_register]) );

  // we get the register content as a std::vector<double>
  auto accessor = device->getOneDRegisterAccessor<int32_t>(moduleName/registerName, nWords32Bit, offset, {AccessMode::raw});
  accessor.read();

  // now fill it to the matlab output buffer
  plhs[0] = mxCreateDoubleMatrix(nElements, 1, mxREAL);
  double *plhsValue = mxGetPr(plhs[0]);

  if (mode == 32){
    size_t i = 0;
    // we can directly loop the accessor
    for (auto value : accessor){
      plhsValue[i++]=value;
    }
  }else{
    auto values16Bit = reinterpret_cast<int16_t *>(accessor.data());
    for (size_t i = 0; i < nElements; ++i){
       plhsValue[i]=values16Bit[i];
    }
  }
}

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

  /*if (mxGetScalar(prhs[pp_channel]) == 0)
    mexErrMsgTxt("channel index cannot be 0.");*/

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
  
  auto twoDRegister = device->getTwoDRegisterAccessor<double>(mxArrayToStdString(prhs[pp_module])+"/"+mxArrayToStdString(prhs[pp_register]));
  twoDRegister.read();
  
  const uint32_t offset = (nrhs > pp_offset) ? mxGetScalar(prhs[pp_offset]): 0;
  const uint32_t elements = (nrhs > pp_elements) ? mxGetScalar(prhs[pp_elements]) : (twoDRegister[0].size());
  
  const uint32_t totalChannels = twoDRegister.getNumberOfDataSequences();
  
  /*if (mxGetScalar(prhs[pp_channel]) > totalChannels)
    mexErrMsgTxt("Requested Channel Index greater than available channels");*/
   
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
        plhsValue[is] = twoDRegister[currentChannel][seqIndex];
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
      
      int currentChannel = (nrhs > pp_channel) ? (mxGetPr(prhs[pp_channel])[ic] - 1) : ic;
      
	  if (currentChannel < 0 || uint32_t(currentChannel) >= totalChannels) mexErrMsgTxt("Illegal Channel Index");
		  
      for(unsigned int is = 0; is < elements; is++)
      {
        unsigned int plhsIndex = ic*elements + is;
        unsigned int seqIndex = (is+offset);        

        if (seqIndex >= size) mexErrMsgTxt("Data storage indexing went wrong! (1)");
        if (plhsIndex >= elements*selectedChannels) mexErrMsgTxt("Data storage indexing went wrong! (2)");

        plhsValue[plhsIndex] = twoDRegister[currentChannel][seqIndex];
      }
    }
  }
}

template <typename UserType>
void writeToDevice(boost::shared_ptr<Device> &device,
                     const std::string &registerPath, void *data_,
                     size_t numberOfwords, size_t offset) {

  auto data = static_cast<UserType*>(data_);
  auto accessor = device->getOneDRegisterAccessor<UserType>(
      registerPath, numberOfwords, offset);

  for (unsigned int i = 0; i < numberOfwords; ++i) {
    accessor[i] = data[i];
  }
  accessor.write();
}
