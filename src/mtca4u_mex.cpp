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
#include <string.h>
#include <sstream>
#include <stdexcept>

#include <mex.h>

#include <MtcaMappedDevice/dmapFilesParser.h>
#include <MtcaMappedDevice/devMap.h>

#include "version.h"

using namespace mtca4u;
using namespace std;

// Some c++ wrapper and utility functions

void mexPrintf(const std::string s) { mexPrintf(s.c_str()); }
void mexWarnMsgTxt(const std::string s) { mexWarnMsgTxt(s.c_str()); }
void mexErrMsgTxt(const std::string s) { mexErrMsgTxt(s.c_str()); }

bool mxIsRealScalar(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == 1); }
bool mxIsPositiveRealScalar(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == 1) && (mxGetScalar(a) >= 0); }
bool mxIsRealVector(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == mxGetN(a)); }
bool mxIsPositiveRealVector(const mxArray* a) { return mxIsNumeric(a) && !mxIsComplex(a) && (mxGetNumberOfElements(a) == mxGetN(a)) && (mxGetScalar(a) >= 0); }
bool mxGetBoolScalar(const mxArray* a) { return  mxGetScalar(a) != 0; }

std::string getOrdinalNumerString(const unsigned int &n) {
  const vector<string> arrayOfOrdinalNumberStrings = {
    "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth",
	"ninth", "tenth", "eleventh", "twelfth" };
  if (n > arrayOfOrdinalNumberStrings.size())
  {
    stringstream ss; ss << n << "th";
	return ss.str();
  }
  else return arrayOfOrdinalNumberStrings[n-1];  
}


string mxArrayToStdString(const mxArray *pa)  { char *ps = mxArrayToString(pa); string s = ps; mxFree(ps); return s; }
//char* mxArrayToString(const mxArray*) = delete; // do not use this function cause one could forget the mxFree. Use the one above instead!

// Function declaration

static void CleanUp(void); // Should me declared static (See Matlab MEX Manual)

devMap<devPCIE> getDevice(const string& deviceName, const string &dmapFileName);

// Command Function declarations and stuff

typedef void (*CmdFnc)(unsigned int, mxArray**, unsigned int, const mxArray **);

struct Command {
  string Name; CmdFnc pCallback; string Description; string Example;
  Command(string n,CmdFnc p, string d, string e) : Name(n), pCallback(p), Description(d), Example(e) {};
};

void PrintHelp(unsigned int, mxArray**, unsigned int, const mxArray **);
//void loadDevicesFromDmap(unsigned int, mxArray**, unsigned int, const mxArray **);
//void clearDevices(unsigned int, mxArray**, unsigned int, const mxArray **);
void getInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void getDeviceInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void getRegisterInfo(unsigned int, mxArray**, unsigned int, const mxArray **);
void readRegister(unsigned int, mxArray**, unsigned int, const mxArray **);
void writeRegister(unsigned int, mxArray**, unsigned int, const mxArray **);
void readDmaRaw(unsigned int, mxArray**, unsigned int, const mxArray **);
void readDmaChannel(unsigned int, mxArray**, unsigned int, const mxArray **);

vector<Command> vectorOfCommands = {
  Command("help", &PrintHelp, "", ""), 
  Command("info", &getInfo, "", ""),
  Command("device_info", &getDeviceInfo, "", ""),
  Command("register_info", &getRegisterInfo, "", ""),
  Command("read", &readRegister, "", ""),
  Command("write", &writeRegister, "", ""),
  Command("read_dma_raw", &readDmaRaw, "", ""),
  Command("read_dma", &readDmaChannel, "", ""),
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
  mexAtExit(CleanUp); // register Cleanup Function

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

    // Check if search was successfull
    if(it == vectorOfCommands.end()) {
      mexErrMsgTxt("Unknown command. Use mtca4u('help') to show some help information.");
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

  catch ( exBase &e )
  {
    mexErrMsgTxt(e.what());
  }
}

/**
 * @brief CleanUp
 *
 */
static void CleanUp(void)
{
#ifdef __MEX_DEBUG_MODE
  mexPrintf("CleanUp\n");
#endif
  //mapOfDevices.clear();
}

/**
 * @brief getDevice
 *
 * @param[in] dmapFileName File to be loaded or all in the current directory if empty
 *
 */
devMap<devPCIE> getDevice(const string& deviceName, const string &dmapFileName = "")
{
  dmapFilesParser filesParser(".");
  
  if (dmapFileName.empty())
    filesParser.parse_dir(".");
  else
    filesParser.parse_file(dmapFileName);
  
  dmapFilesParser::iterator it = filesParser.begin();
  
  for (; it != filesParser.end(); ++it)
  {
    if (deviceName == it->first.dev_name)
      break;   
  }

  if(it == filesParser.end())
      mexErrMsgTxt("Unknown device '" + deviceName + "'.");
      
  devMap<devPCIE> tempDevice;
  tempDevice.openDev(it->first.dev_file, it->first.map_file_name);
  return tempDevice;
}

/**
 * @brief PrintHelp shows the help text on the console
 *
 */
void PrintHelp(unsigned int, mxArray **, unsigned int, const mxArray **)
{
  mexPrintf("mtca4u Matlab library revision: 0\n");
  mexPrintf("Available Commands are: \n");

  for (vector<Command>::iterator it = vectorOfCommands.begin(); it != vectorOfCommands.end(); ++it)
  {
    mexPrintf("\t" + it->Name + "\n");
  }
  mexPrintf("\n\nFor further help or bug reports please contact michael.heuer@desy.de"); 
}

/**
 * @brief loadDevicesFromDmap
 *
 */
//void loadDevicesFromDmap(unsigned int, mxArray **, unsigned int nrhs, const mxArray *prhs[])
//{
//  if ((nrhs >= 1) && (!mxIsChar(prhs[0]))) mexErrMsgTxt("Invalid " + getOrdinalNumerString(1) + " input argument.");
//  if (nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
//  if ((nrhs == 0) && lastDmapFileName.empty()) mexErrMsgTxt("Could not reload Dmap file. No previous file available.");
//  if (!lastDmapFileName.empty()) mexWarnMsgTxt("Reload Dmap File.");
//  
//  string dmapFileName = (nrhs > 0) ? mxArrayToStdString(prhs[0]) : lastDmapFileName;
//  mapOfDevices.clear();
//  dmapFilesParser filesParser(".");
//  filesParser.parse_file(dmapFileName);
//  lastDmapFileName = dmapFileName;
//  
//  for (dmapFilesParser::iterator it = filesParser.begin(); it != filesParser.end(); ++it)
//  {
//    devMap<devPCIE> tempDevice;
//    tempDevice.openDev(it->first.dev_file, it->first.map_file_name);
//    mapOfDevices.insert(pair< string, devMap<devPCIE> >(it->first.dev_name, tempDevice));
//  }
//}

/**
 * @brief clearDevices
 *
 */
//void clearDevices(unsigned int, mxArray **, unsigned int nrhs, const mxArray **)
//{
//  if(nrhs > 0) mexWarnMsgTxt("Too many input arguments.");
//  mapOfDevices.clear();
//}

/**
 * @brief getInfo
 *
 */
void getInfo(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray **)
{
  if(nrhs > 0) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  dmapFilesParser filesParser(".");
  filesParser.parse_dir(".");

  mwSize dims[2] = {1, (int)filesParser.getdMapFileSize()};
  const char *field_names[] = {"name", "slot", "device", "driver", "firmware", "revision"};
  plhs[0] = mxCreateStructArray(2, dims, (sizeof(field_names)/sizeof(*field_names)), field_names);

  unsigned int i = 0;
  
  for (dmapFilesParser::iterator it = filesParser.begin(); it != filesParser.end(); ++it)
  {
    devMap<devPCIE> tempDevice;
    tempDevice.openDev(it->first.dev_file, it->first.map_file_name);

    mxSetFieldByNumber(plhs[0], i, 0, mxCreateString(it->first.dev_name.c_str()));

    mxArray *slot_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(slot_value) = 0;
    mxSetFieldByNumber(plhs[0], i, 1, slot_value);

    mxSetFieldByNumber(plhs[0], i, 2, mxCreateString(""));

    mxArray *driver_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(driver_value) = 0;
    mxSetFieldByNumber(plhs[0], i, 3, driver_value);

    int firmware = 0;
    //it->second.readReg("WORD_FIRMWARE", &firmware);

    mxArray *firmware_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(firmware_value) = firmware;
    mxSetFieldByNumber(plhs[0], i, 4, firmware_value);

    mxArray *revision_value = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(revision_value) = 0;
    mxSetFieldByNumber(plhs[0], i, 5, revision_value);
  }
}

/**
 * @brief getDeviceInfo
 *
 */
void getDeviceInfo(unsigned int nlhs, mxArray **, unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 1) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 1) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));

  mexWarnMsgTxt("Not implemented yet.");

  //mwSize dims[2] = {1, (int)mapOfDevices.size()};
  //const char *field_names[] = {"name"};
  //plhs[0] = mxCreateStructArray(2, dims, (sizeof(field_names)/sizeof(*field_names)), field_names);

}

/**
 * @brief getRegisterInfo
 *
 */
void getRegisterInfo(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 2) mexErrMsgTxt("Not enough input arguments.");
  if(nrhs > 2) mexWarnMsgTxt("Too many input arguments.");
  if(nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  if (!mxIsChar(prhs[0])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(1) + " input argument.");
  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " +  getOrdinalNumerString(2) + " input argument.");

  devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));

  string registerName = mxArrayToStdString(prhs[1]);
  mapFile::mapElem regInfo;

  try {
    devMap<devPCIE>::RegisterAccessor reg = device.getRegisterAccessor(registerName);
    regInfo = reg.getRegisterInfo();
  }
  catch( exBase & e ) {
    mexErrMsgTxt("Failed to read register");
  }

  //mwSize dims[2] = {1, 1};
  const char *field_names[] = {"name", "elements", "signed", "bits", "fractional_bits", "description"};
  plhs[0] = mxCreateStructMatrix(1, 1, (sizeof(field_names)/sizeof(*field_names)), field_names);

  mxSetFieldByNumber(plhs[0], 0, 0, mxCreateString(registerName.c_str()));

  mxArray *numElements = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numElements) = regInfo.reg_elem_nr;
  mxSetFieldByNumber(plhs[0], 0, 1, numElements);

  mxArray *signedFlag = mxCreateLogicalMatrix(1,1);
  *mxGetPr(signedFlag) = regInfo.reg_signed;
  mxSetFieldByNumber(plhs[0], 0, 2, signedFlag);

  mxArray *numBits = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numBits) = regInfo.reg_width;
  mxSetFieldByNumber(plhs[0], 0, 3, numBits);

  mxArray *numFractionalBits = mxCreateDoubleMatrix(1,1,mxREAL);
  *mxGetPr(numFractionalBits) = regInfo.reg_frac_bits;
  mxSetFieldByNumber(plhs[0], 0, 4, numFractionalBits);

  mxSetFieldByNumber(plhs[0], 0, 5, mxCreateString(""));
}

/**
 * @brief readRegister
 *
 * Parameter: device, register, [elements], [offset]
 */
void readRegister(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if (nrhs < 2) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 4) mexWarnMsgTxt("To many input arguments.");

  if (nlhs > 1) mexErrMsgTxt("To many output arguments.");

  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(2) + " input argument.");
  if ((nrhs > 2) && !mxIsPositiveRealScalar(prhs[2])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(3) + " input argument.");
  if ((nrhs > 3) && !mxIsPositiveRealScalar(prhs[3])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(3) + " input argument.");
 
  try {
    devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));
    devMap<devPCIE>::RegisterAccessor reg = device.getRegisterAccessor(mxArrayToStdString(prhs[1]));
    mapFile::mapElem regInfo = reg.getRegisterInfo();

    const uint32_t offset = (nrhs > 3) ? static_cast<uint32_t>(*mxGetPr(prhs[3])) : 0;
    const uint32_t elements = (nrhs > 2) ? static_cast<uint32_t>(*mxGetPr(prhs[2])) : regInfo.reg_elem_nr - offset;

    plhs[0] = mxCreateDoubleMatrix(1, elements, mxREAL);
    double *plhsValue = mxGetPr(plhs[0]);
    reg.read(plhsValue, elements, offset * sizeof(uint32_t));
  }
  catch( exBase & e ) {
    mexErrMsgTxt(string("Failed to read register: ") + e.what());
  }
}

/**
 * @brief writeRegister
 *
 * Parameter: device, register, value, [offset]
 */
void writeRegister(unsigned int, mxArray **, unsigned int nrhs, const mxArray *prhs[])
{
  if(nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (!mxIsChar(prhs[0])) mexErrMsgTxt("Invalid input arguments.");
  if (!mxIsChar(prhs[1]))  mexErrMsgTxt("Invalid " + getOrdinalNumerString(2) + " input argument.");
  if (!mxIsNumeric(prhs[2]) || mxIsComplex(prhs[2])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(3) + " input argument.");
  if ((nrhs > 3) && !mxIsPositiveRealScalar(prhs[3])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(4) + " input argument.");
  if (nrhs > 4) mexWarnMsgTxt("To many input arguments.");
  
  devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));

  string registerName = mxArrayToStdString(prhs[1]);
  double *prhsValue = mxGetPr(prhs[2]);
  const unsigned int prhsValueElements = mxGetNumberOfElements(prhs[2]);
  uint32_t ValueOffset = (nrhs > 3) ? static_cast<uint32_t>(*mxGetPr(prhs[3])) : 0;
  
  try {
    devMap<devPCIE>::RegisterAccessor reg = device.getRegisterAccessor(registerName);
    mapFile::mapElem regInfo = reg.getRegisterInfo();

    //if (regInfo.reg_elem_nr < prhsValueElements)
    //  mexWarnMsgTxt("Array is larger than the register sizeof '" + registerName + "'.");
    //else if (regInfo.reg_elem_nr > prhsValueElements)
    //  mexWarnMsgTxt("Array is smaller than the register size of '" + registerName + "'.");

    reg.write(prhsValue, prhsValueElements, ValueOffset*sizeof(uint32_t));
  }
  catch( exBase & e ) {
    mexErrMsgTxt("Failed to write register '" + registerName + "': " + e.what());
    return;
  }
}

/**
 * @brief readRawDmaData
 *
 */
template<class T> void innerRawDMARead(devMap<devPCIE> &dev, const string regName, double* dest, const size_t nElements, const size_t nOffset, const bool signedFlag, const unsigned int bits, const int fracBits)
{
  std::vector<T> dmaValue(nElements);
  FixedPointConverter conv(bits, fracBits, signedFlag);
  
  try {
    dev.readDMA(regName, reinterpret_cast<int32_t*>(&dmaValue[0]), nElements*sizeof(T), nOffset);
  }
  catch( exBase & e ) {
    mexErrMsgTxt(string("Failed to read dma register: ") + e.what());
  }
  
  // copy and transform data
  for(unsigned int i = 0; i < nElements; i++)
  {
    dest[i] = conv.toDouble(dmaValue[i]);
  }
}


/**
 * @brief readRawDmaData
 *
 * Parameter: device, register, [elements], [offset], [mode], [singed], [bit], [fracbit]
 */
void readDmaRaw(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if (nrhs < 2) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 7) mexWarnMsgTxt("Too many input arguments.");
  if (nlhs > 1) mexErrMsgTxt("Too many output arguments.");

  if (!mxIsChar(prhs[0])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(1) + " input argument.");
  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(2) + " input argument."); 
  
  if ((nrhs > 2) && !mxIsPositiveRealScalar(prhs[2])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(3) + " input argument.");
  if ((nrhs > 3) && !mxIsPositiveRealScalar(prhs[3])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(4) + " input argument.");
  
  if ((nrhs > 4) && !mxIsPositiveRealScalar(prhs[4])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(5) + " input argument.");
  if ((nrhs > 5) && !mxIsRealScalar(prhs[5])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(6) + " input argument.");
  if ((nrhs > 6) && !mxIsPositiveRealScalar(prhs[6])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(7) + " input argument.");
  if ((nrhs > 7) && !mxIsRealScalar(prhs[7])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(8) + " input argument.");
  
  devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));
  const string regName = mxArrayToStdString(prhs[1]);
  devMap<devPCIE>::RegisterAccessor reg = device.getRegisterAccessor(regName);
  mapFile::mapElem regInfo = reg.getRegisterInfo();
  
  const uint32_t offset = (nrhs < 4) ? 0 : mxGetScalar(prhs[3]);
  const uint32_t size = (nrhs < 3) ? (regInfo.reg_elem_nr - offset) : mxGetScalar(prhs[2]);
    
  const uint32_t mode = (nrhs < 5) ? 32 : mxGetScalar(prhs[4]);
  if ((mode != 32) && (mode != 16)) mexErrMsgTxt("Invalid data mode.");
  
  const bool signedFlag = (nrhs < 6) ? 0 : mxGetBoolScalar(prhs[5]);
  const uint32_t bits = (nrhs < 7) ? mode : mxGetScalar(prhs[6]);
  const uint32_t fracBits = (nrhs < 8) ? 0 : mxGetScalar(prhs[7]);
  
  plhs[0] = mxCreateDoubleMatrix(size, 1, mxREAL);
  double *plhsValue = mxGetPr(plhs[0]);
  
  if (mode == 32)
    innerRawDMARead<int32_t>(device, regName, plhsValue, size, offset, signedFlag, bits, fracBits);
  else
    innerRawDMARead<int16_t>(device, regName, plhsValue, size, offset, signedFlag, bits, fracBits);
}


/**
 * @brief readDmaChannel
 *
 * Parameter: device, register, channel, [sample], [offset], [channel count], [mode], [singed], [bit], [fracbit]
 */
void readDmaChannel(unsigned int nlhs, mxArray *plhs[], unsigned int nrhs, const mxArray *prhs[])
{
  if (nrhs < 3) mexErrMsgTxt("Not enough input arguments.");
  if (nrhs > 10) mexWarnMsgTxt("Too many input arguments.");
  
  if (!mxIsChar(prhs[0])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(1) + " input argument.");
  if (!mxIsChar(prhs[1])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(2) + " input argument.");
    
  if ((nlhs > 1) && (nrhs < 2)) mexErrMsgTxt("Too many output arguments.");
  if ((nlhs > 1) && (nrhs > 1) && (nlhs > mxGetNumberOfElements(prhs[2]))) mexErrMsgTxt("Too many output arguments.");
  if ((nlhs > 1) && (nrhs > 1) && (nlhs < mxGetNumberOfElements(prhs[2]))) mexErrMsgTxt("Not enough output arguments.");

  if ((nrhs > 2) && !mxIsPositiveRealVector(prhs[2])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(3) + " input argument.");
  if ((nrhs > 3) && !mxIsPositiveRealScalar(prhs[3])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(4) + " input argument.");
  if ((nrhs > 4) && !mxIsPositiveRealScalar(prhs[4])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(5) + " input argument.");
  if ((nrhs > 5) && !mxIsPositiveRealScalar(prhs[5])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(6) + " input argument.");
  if ((nrhs > 6) && !mxIsPositiveRealScalar(prhs[6])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(7) + " input argument.");
  if ((nrhs > 7) && !mxIsRealScalar(prhs[7])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(8) + " input argument.");
  if ((nrhs > 8) && !mxIsPositiveRealScalar(prhs[8])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(9) + " input argument.");
  if ((nrhs > 9) && !mxIsRealScalar(prhs[9])) mexErrMsgTxt("Invalid " + getOrdinalNumerString(10) + " input argument.");
  
  devMap<devPCIE> device = getDevice(mxArrayToStdString(prhs[0]));
  const string regName = mxArrayToStdString(prhs[1]);
  devMap<devPCIE>::RegisterAccessor reg = device.getRegisterAccessor(regName);
  mapFile::mapElem regInfo = reg.getRegisterInfo();
  
  const uint32_t offset = (nrhs < 5) ? 0 : mxGetScalar(prhs[4]);
  const uint32_t sample = (nrhs < 4) ? (regInfo.reg_elem_nr - offset) : mxGetScalar(prhs[3]);

  const uint32_t totalChannels = (nrhs < 6) ? 8 : mxGetScalar(prhs[5]);
  if ((totalChannels != 8) && (totalChannels != 16)) mexErrMsgTxt("Invalid number of channels.");
  
  const uint32_t mode = (nrhs < 7) ? 32 : mxGetScalar(prhs[6]);
  if ((mode != 32) && (mode != 16)) mexErrMsgTxt("Invalid data mode.");
 
  const uint32_t size = sample*totalChannels;
  std::vector<double> dmaValue(size);
  
  const bool signedFlag = (nrhs < 8) ? 0 : mxGetBoolScalar(prhs[7]);
  const uint32_t bits = (nrhs < 9) ? mode : mxGetScalar(prhs[8]);
  const uint32_t fracBits = (nrhs < 10) ? 0 : mxGetScalar(prhs[9]);

  if (mode == 32) 
    innerRawDMARead<int32_t>(device, regName, &dmaValue[0], size, offset, signedFlag, bits, fracBits);
  else 
    innerRawDMARead<int16_t>(device, regName, &dmaValue[0], size, offset, signedFlag, bits, fracBits);

  // Store data in different vectors passed over lhs
  if ((nrhs >= 2) && (nlhs == mxGetNumberOfElements(prhs[2])))
  {
    for(unsigned int ic = 0; ic < nlhs; ic++)
    {
      plhs[ic] = mxCreateDoubleMatrix(sample, 1, mxREAL);
      double *plhsValue = mxGetPr(plhs[ic]);
      unsigned int currentChannel = (mxGetPr(prhs[2])[ic] - 1);

      for(unsigned int is = 0; is < sample; is++)
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
    unsigned int SelectedChannels = (nrhs > 1) ? mxGetNumberOfElements(prhs[2]) : totalChannels;
    plhs[0] = mxCreateDoubleMatrix(sample, SelectedChannels, mxREAL);
    double *plhsValue = mxGetPr(plhs[0]);
    
    // copy and transform data
    for(unsigned int ic = 0; ic < SelectedChannels; ic++)
    {
      unsigned int currentChannel = (nrhs > 1) ? (mxGetPr(prhs[2])[ic] - 1) : ic;
      for(unsigned int is = 0; is < sample; is++)
      {
        unsigned int plhsIndex = ic*sample + is;
        unsigned int dmaIndex = is*totalChannels + currentChannel;

        if (dmaIndex >= size) mexErrMsgTxt("Data storage indexing went wrong!");
        if (plhsIndex >= sample*SelectedChannels) mexErrMsgTxt("Data storage indexing went wrong!");

        plhsValue[plhsIndex] = (currentChannel < totalChannels) ? dmaValue[dmaIndex] : 0;
      }
    }
  }
}
