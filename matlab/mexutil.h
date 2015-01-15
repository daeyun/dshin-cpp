////////////////////////////////////////////////////////////////////////////////
//
//  Name:        mexutil.h
//  Purpose:     Macros and helper functions for writing MATLAB MEX-files.
//  Author:      Daeyun Shin <daeyun@dshin.org>
//  Created:     01.15.2015
//
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "mex.h"
#include <string>
#include <algorithm>
#include <cctype>

#ifndef N_LHS_VAR
#define N_LHS_VAR nargout
#endif
#ifndef N_RHS_VAR
#define N_RHS_VAR nargin
#endif
#ifndef MEX_COMPONENT_NAME
#define MEX_COMPONENT_NAME "MATLAB"
#endif

#define BOLD(str) "<strong>" str "</strong>"
#define ORANGE(str) "[\b" str "]\b"

namespace mexutil {
enum CompOp { EQ, GT, LT, NEQ, GE, LE };

enum ArgType {
  kDouble,
  kSingle,
  kStruct,
  kLogical,
  kChar,
  kInt8,
  kUint8,
  kInt16,
  kUint16,
  kInt32,
  kUint32
};

// e.g. double* mat = GetArg<kDouble,EQ,GT>(0, prhs, 3, 3); Throws an error if
// prhs[0] doesn't have exactly 3 rows or have less than 3 columns. 0s are
// ignored.
template <ArgType argtype, CompOp row_comp = EQ, CompOp col_comp = EQ>
void *GetArg(const mwSize index, const mxArray *input[], mwSize nrows = 0,
             mwSize ncols = 0);

// Constructs the identifier token used in error messages.
std::string MatlabIdStringFromPath(std::string str);
std::string FilenameFromPath(std::string str);

// Retrieves the workspace global variable mexVerboseLevel (default: 1).
int VerboseLevel();

const int kDefaultVerboseLevel = 1;
const std::string kFilename = FilenameFromPath(__FILE__);
const std::string kFunctionIdentifier = MatlabIdStringFromPath(kFilename);
const int kVerboseLevel = VerboseLevel();

// Force pass-by-value behavior to prevent accidentally modifying shared
// memory content in-place. Undocumented.
// http://undocumentedmatlab.com/blog/matlab-mex-in-place-editing
extern "C" bool mxUnshareArray(mxArray *array_ptr, bool noDeepCopy);

mxArray *UnshareArray(int index, const mxArray *prhs[]) {
  mxArray *unshared = const_cast<mxArray *>(prhs[index]);
  mxUnshareArray(unshared, true);
  return unshared;
}

std::string MatlabIdStringFromFilename(std::string str) {
  (void)(MatlabIdStringFromPath);
  auto is_invalid_id_char =
      [](char ch) { return !(isalnum((int)ch) || ch == '_'); };
  if (int i = str.find_first_of('.')) str = str.substr(0, i);
  if (!isalpha(str[0])) str = "mex_" + str;
  std::replace_if(str.begin(), str.end(), is_invalid_id_char, '_');
  return str;
}

std::string FilenameFromPath(std::string str) {
  (void)(FilenameFromPath);
  if (int i = str.find_last_of('/')) str = str.substr(i + 1, str.length());
  return str;
}

int VerboseLevel() {
  (void)(VerboseLevel);
  mxArray *ptr = mexGetVariable("global", "mexVerboseLevel");
  if (ptr == NULL) return kDefaultVerboseLevel;
  return mxGetScalar(ptr);
}

// e.g. LEVEL(2, MPRINTF("Not printed if logging level is less than 2."))
#define LEVEL(verbose_level, expr)            \
  {                                           \
    if (kVerboseLevel >= verbose_level) expr; \
  }

// Construct an identifier string e.g.  MATLAB:mexutil:myErrorIdentifier
#define MEX_IDENTIFIER(mnemonic)                               \
  (std::string(MEX_COMPONENT_NAME ":") + kFunctionIdentifier + \
   std::string(":" mnemonic)).c_str()

// Assert number of input variables.
#define N_IN_RANGE(min, max)                                                \
  {                                                                         \
    if (N_RHS_VAR < min || N_RHS_VAR > max) {                               \
      mexErrMsgIdAndTxt(MEX_IDENTIFIER("InputSizeError"),                   \
                        "Number of inputs must be between %d and %d.", min, \
                        max);                                               \
    }                                                                       \
  }

// Assert number of output variables.
#define N_OUT_RANGE(min, max)                                                \
  {                                                                          \
    if (N_LHS_VAR < min || N_LHS_VAR > max) {                                \
      mexErrMsgIdAndTxt(MEX_IDENTIFIER("InputSizeError"),                    \
                        "Number of outputs must be between %d and %d.", min, \
                        max);                                                \
    }                                                                        \
  }

#define N_IN(num)                                             \
  {                                                           \
    if (N_RHS_VAR != num) {                                   \
      mexErrMsgIdAndTxt(MEX_IDENTIFIER("OutputSizeError"),    \
                        "Number of inputs must be %d.", num); \
    }                                                         \
  }

#define N_OUT(num)                                             \
  {                                                            \
    if (N_LHS_VAR != num) {                                    \
      mexErrMsgIdAndTxt(MEX_IDENTIFIER("OutputSizeError"),     \
                        "Number of outputs must be %d.", num); \
    }                                                          \
  }

// Print message to MATLAB console.
// e.g.MPRINTF(BOLD("%d"), argc);
#define MPRINTF(...)             \
  {                              \
    if (kVerboseLevel > 0) {     \
      mexPrintf(__VA_ARGS__);    \
      mexEvalString("drawnow;"); \
    }                            \
  }

// Display error and exit.
#define ERR_EXIT(errname, ...) \
  { mexErrMsgIdAndTxt(MEX_IDENTIFIER(errname), ##__VA_ARGS__); }

// Macros starting with an underscore are internal.
#define _ASSERT(condition)                                                   \
  {                                                                          \
    if (!(condition)) {                                                      \
      MPRINTF("[ERROR] (%s:%d %s) ", kFilename.c_str(), __LINE__, __func__); \
      mexErrMsgTxt("assertion " #condition " failed\n", );                   \
    }                                                                        \
  }

#define _ASSERT_MSG(condition, msg)                                          \
  {                                                                          \
    if (!(condition)) {                                                      \
      MPRINTF("[ERROR] (%s:%d %s) ", kFilename.c_str(), __LINE__, __func__); \
      mexErrMsgTxt("assertion " #condition " failed\n%s\n", msg);            \
    }                                                                        \
  }

#define _CHOOSE_MACRO(a, x, func, ...) func

#define ASSERT(condition, ...) \
  _CHOOSE_MACRO(, ##__VA_ARGS__, _ASSERT_MSG(__VA_ARGS__), _ASSERT(__VA_ARGS__))

#define ASSERT_FMT(condition, fmt, ...)                                      \
  {                                                                          \
    if (!(condition)) {                                                      \
      MPRINTF("[ERROR] (%s:%d %s) ", kFilename.c_str(), __LINE__, __func__); \
      mexErrMsgIdAndTxt(MEX_IDENTIFIER("AssertionError"),                    \
                        "assertion " #condition " failed\n" fmt "\n",        \
                        ##__VA_ARGS__);                                      \
    }                                                                        \
  }

template <ArgType argtype, CompOp row_comp, CompOp col_comp>
void *GetArg(mwSize index, const mxArray *input[], mwSize nrows, mwSize ncols) {
  if (nrows > 0) {
    switch (row_comp) {
      case EQ:
        ASSERT_FMT(mxGetM(input[index]) == nrows,
                   "size(input[%d], 1) must be %d.", index, nrows);
        break;
      case GT:
        ASSERT_FMT(mxGetM(input[index]) > nrows,
                   "size(input[%d], 1) must be greater than %d.", index, nrows);
        break;
      case LT:
        ASSERT_FMT(mxGetM(input[index]) < nrows,
                   "size(input[%d], 1) must be less than %d.", index, nrows);
        break;
      case NEQ:
        ASSERT_FMT(mxGetM(input[index]) != nrows,
                   "size(input[%d], 1) must be not equal %d.", index, nrows);
        break;
      case GE:
        ASSERT_FMT(mxGetM(input[index]) >= nrows,
                   "size(input[%d], 1) must be at least %d.", index, nrows);
        break;
      case LE:
        ASSERT_FMT(mxGetM(input[index]) <= nrows,
                   "size(input[%d], 1) can be at most %d.", index, nrows);
        break;
      default:
        break;
    }
  }
  if (ncols > 0) {
    switch (col_comp) {
      case EQ:
        ASSERT_FMT(mxGetN(input[index]) == ncols,
                   "size(input[%d], 2) must be %d.", index, ncols);
        break;
      case GT:
        ASSERT_FMT(mxGetN(input[index]) > ncols,
                   "size(input[%d], 2) must be greater than %d.", index, ncols);
        break;
      case LT:
        ASSERT_FMT(mxGetN(input[index]) < ncols,
                   "size(input[%d], 2) must be less than %d.", index, ncols);
        break;
      case NEQ:
        ASSERT_FMT(mxGetN(input[index]) != ncols,
                   "size(input[%d], 2) must be not equal %d.", index, ncols);
        break;
      case GE:
        ASSERT_FMT(mxGetN(input[index]) >= ncols,
                   "size(input[%d], 2) must be at least %d.", index, ncols);
        break;
      case LE:
        ASSERT_FMT(mxGetN(input[index]) <= ncols,
                   "size(input[%d], 2) can be at most %d.", index, ncols);
        break;
      default:
        break;
    }
  }

  switch (argtype) {
    case kDouble:
      ASSERT_FMT(mxIsDouble(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetPr(input[index]);  // double*
    case kSingle:
      ASSERT_FMT(mxIsSingle(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // float*
    case kStruct:
      ASSERT_FMT(mxIsStruct(input[index]),
                 "Invalid data type for input index %d.", index);
      // TODO
      ERR_EXIT("UnknownDataTypeError", "Not implemented");
      break;
    case kLogical:
      ASSERT_FMT(mxIsLogical(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetLogicals(input[index]);  // mxLogical*
    case kChar:
      ASSERT_FMT(mxIsChar(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetChars(input[index]);  // char*
    case kInt8:
      ASSERT_FMT(mxIsInt8(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // int8_t*
    case kUint8:
      ASSERT_FMT(mxIsUint8(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // uint8_t*
    case kInt16:
      ASSERT_FMT(mxIsInt16(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // int16_t*
    case kUint16:
      ASSERT_FMT(mxIsUint16(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // uint16_t*
    case kInt32:
      ASSERT_FMT(mxIsInt32(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // int32_t*
    case kUint32:
      ASSERT_FMT(mxIsUint32(input[index]),
                 "Invalid data type for input index %d.", index);
      return mxGetData(input[index]);  // uint32_t*
    default:
      ERR_EXIT("UnknownDataTypeError", "Unknown argtype");
  }
}
}
