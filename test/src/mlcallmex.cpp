/* http://stackoverflow.com/questions/11220250/how-do-i-profile-a-mex-function-in-matlab
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "engine.h"

//#include <execinfo.h>
//#include <signal.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <ucontext.h>

typedef void (*mexFunction_t)(int nargout, mxArray *pargout[], int nargin,
                              const mxArray *pargin[]);

int main(int argc, const char *argv[]) {
  Engine *ep;
  char buff[1024];
  unsigned int iteration = 1;

  /* matlab must be in the PATH! */
  if (!(ep = engOpen("matlab -nodisplay"))) {
    fprintf(stderr, "Can't start MATLAB engine\n");
    return -1;
  }
  engOutputBuffer(ep, buff, 1023);

  /* load the mex file */
  if (argc < 2) {
    fprintf(stderr,
            "Error. Give full path to the MEX file as input parameter.\n");
    return -1;
  }

  void *handle = dlopen(argv[1], RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "Error loading MEX file: %s\n", strerror(errno));
    return -1;
  }

  /* grab mexFunction handle */
  mexFunction_t mexfunction;
  // Sorry, verry ugly conversion to get rid off all compiler warnings.
  // See 'man 3 dlsym'
  *(void **)(&mexfunction) = dlsym(handle, "mexFunction");
  if (!(mexfunction)) {
    fprintf(stderr, "MEX file does not contain mexFunction\n");
    return -1;
  }

  /* Get number of iteration */
  if (argc >= 3) {
    try {
      iteration = std::stoul(argv[2]);
    } catch (...) {
      // catch (const std::invalid_argument& ia) {
      fprintf(stderr, "Exception\n");
      return -1;
    }
  }

  /* load input data - for convenience do that using MATLAB engine */
  /* NOTE: parameters are MEX-file specific, so one has to modify this*/
  /* to fit particular needs */
  engEvalString(ep,
                "cmd = 'read'; dev = 'SISL10'; reg = 'BOARD_WORD_FIRMWARE'");
  mxArray *arg1 = engGetVariable(ep, "cmd");
  mxArray *arg2 = engGetVariable(ep, "dev");
  mxArray *arg3 = engGetVariable(ep, "reg");
  mxArray *pargout[1] = {0};
  const mxArray *pargin[3] = {arg1, arg2, arg3};

  for (unsigned int i = 0; i < iteration; i++) {

    if (pargout[0] != 0) {
      mxDestroyArray(pargout[0]);
      pargout[0] = NULL;
    }

    /* execute the mex function */
    (*mexfunction)(1, pargout, 3, pargin);
  }

  /* execute the mex function */
  /* (*mexfunction)(0, pargout, 0, pargin); */

  /* print the results using MATLAB engine */
  engPutVariable(ep, "result", pargout[0]);
  engEvalString(ep, "result");
  printf("%s\n", buff);

  /* cleanup */
  mxDestroyArray(pargout[0]);
  engEvalString(ep, "clear all;");
  dlclose(handle);
  engClose(ep);

  return 0;
}
