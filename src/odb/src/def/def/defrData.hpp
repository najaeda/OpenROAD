// *****************************************************************************
// *****************************************************************************
// Copyright 2013 - 2015, Cadence Design Systems
//
// This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
// Distribution,  Product Version 5.8.
//
// Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
// For updates, support, or to become part of the LEF/DEF Community,
// check www.openeda.org for details.
//
//  $Author: arakhman $
//  $Revision: #6 $
//  $Date: 2013/08/09 $
//  $State:  $
// *****************************************************************************
// *****************************************************************************

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "defrCallBacks.hpp"
#include "defrReader.hpp"
#include "defrSettings.hpp"

#ifndef defrData_h
#define defrData_h

#define CURRENT_VERSION 5.8
#define RING_SIZE 10
#define IN_BUF_SIZE 16384
#define TOKEN_SIZE 4096
#define MSG_SIZE 100

BEGIN_DEF_PARSER_NAMESPACE

struct defCompareStrings
{
  bool operator()(const std::string& lhs, const std::string& rhs) const
  {
    return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
  }
};

using defAliasMap = std::map<std::string, std::string, defCompareStrings>;
using defDefineMap = std::map<std::string, std::string, defCompareStrings>;

union YYSTYPE
{
  double dval;
  int integer;
  char* string;
  int keyword;  // really just a nop
  defPOINT pt;
  defTOKEN* tk;
};

#define YYSTYPE_IS_DECLARED

class defrData
{
 public:
  defrData(const defrCallbacks* pCallbacks,
           const defrSettings* pSettings,
           defrSession* pSession);
  ~defrData();

  inline int defGetKeyword(const char* name, int* result);
  inline int defGetAlias(const std::string& name, std::string& result);
  inline int defGetDefine(const std::string& name, std::string& result);
  void reload_buffer();
  int GETC();

  void UNGETC(char ch);
  char* ringCopy(const char* string);
  int DefGetTokenFromStack(char* s);
  inline void print_lines(long long lines);
  const char* lines2str(long long lines);
  static inline void IncCurPos(char** curPos, char** buffer, int* bufferSize);
  int DefGetToken(char** buffer, int* bufferSize);
  static void uc_array(char* source, char* dest);
  void StoreAlias();
  int defyylex(YYSTYPE* pYylval);
  int sublex(YYSTYPE* pYylval);
  int amper_lookup(YYSTYPE* pYylval, char* tkn);
  void defError(int msgNum, const char* s);
  void defyyerror(const char* s);
  void defInfo(int msgNum, const char* s);
  void defWarning(int msgNum, const char* s);

  void defiError(int check, int msgNum, const char* mess);
  const char* DEFCASE(const char* ch);
  void pathIsDone(int shield, int reset, int netOsnet, int* needCbk);
  const char* upperCase(const char* str);

  inline int checkErrors();
  int validateMaskInput(int input, int warningIndex, int getWarningsIndex);
  int validateMaskShiftInput(const char* shiftMask,
                             int warningIndex,
                             int getWarningsIndex);

  static double convert_defname2num(char* versionName);

  static int numIsInt(char* volt);
  int defValidNum(int values);

  inline static const char* defkywd(int num);

  int assertionWarnings{0};
  int bit_is_keyword{0};
  int bitsNum{0};  // Scanchain Bits value
  int blockageWarnings{0};
  int by_is_keyword{0};
  int caseSensitiveWarnings{0};
  int componentWarnings{0};
  int constraintWarnings{0};
  int cover_is_keyword{0};
  int defMsgCnt{0};
  int defMsgPrinted{0};  // number of msgs output so far
  int defRetVal{0};
  int def_warnings{0};
  int do_is_keyword{0};
  int dumb_mode{0};
  int errors{0};
  int fillWarnings{0};
  int first_buffer{0};
  int fixed_is_keyword{0};
  int gcellGridWarnings{0};
  int hasBlkLayerComp{0};      // only 1 BLOCKAGE/LAYER/COMP
  int hasBlkLayerSpac{0};      // only 1 BLOCKAGE/LAYER/SPACING
  int hasBlkLayerTypeComp{0};  // SLOTS or FILLS
  int hasBlkPlaceComp{0};      // only 1 BLOCKAGE/PLACEMENT/COMP
  int hasBlkPlaceTypeComp{0};  // SOFT or PARTIAL
  int hasBusBit{0};            // keep track BUSBITCHARS is in the file
  int hasDes{0};               // keep track DESIGN is in the file
  int hasDivChar{0};           // keep track DIVIDERCHAR is in the file
  int hasDoStep{0};
  int hasNameCase{0};  // keep track NAMESCASESENSITIVE is in the file
  int hasOpenedDefLogFile{0};
  int hasPort{0};      // keep track is port defined in a Pin
  int hadPortOnce{0};  // to restrict implicit ports if the Pin already has any
                       // port
  int hasVer{0};       // keep track VERSION is in the file
  int hasFatalError{0};  // don't report errors after the file end.
  int mask_is_keyword{0};
  int mustjoin_is_keyword{0};
  int names_case_sensitive{0};  // always true in 5.6
  int needNPCbk{0};             // if cbk for net path is needed
  int needSNPCbk{0};            // if cbk for snet path is needed
  int nl_token{0};
  int no_num{0};
  int nonDefaultWarnings{0};
  int nondef_is_keyword{0};
  int ntokens{0};
  int orient_is_keyword{0};
  int pinExtWarnings{0};
  int pinWarnings{0};
  int real_num{0};
  int rect_is_keyword{0};
  int regTypeDef{0};  // keep track that region type is defined
  int regionWarnings{0};
  int ringPlace{0};
  int routed_is_keyword{0};
  int scanchainWarnings{0};
  int specialWire_mask{0};
  int step_is_keyword{0};
  int stylesWarnings{0};
  int trackWarnings{0};
  int unitsWarnings{0};
  int versionWarnings{0};
  int viaRule{0};  // keep track the viarule has called first
  int viaWarnings{0};

  std::vector<char> History_text;
  defAliasMap def_alias_set;
  defDefineMap def_defines_set;

  char* specialWire_routeStatus;
  char* specialWire_routeStatusName;
  char* specialWire_shapeType;
  double VersionNum{5.7};

  // defrParser vars.
  defiPath PathObj;
  defiProp Prop;
  defiSite Site;
  defiComponent Component;
  defiComponentMaskShiftLayer ComponentMaskShiftLayer;
  defiNet Net;
  defiPinCap PinCap;
  defiSite CannotOccupy;
  defiSite Canplace;
  defiBox DieArea;
  defiPin Pin;
  defiRow Row;
  defiTrack Track;
  defiGcellGrid GcellGrid;
  defiVia Via;
  defiRegion Region;
  defiGroup Group;
  defiAssertion Assertion;
  defiScanchain Scanchain;
  defiIOTiming IOTiming;
  defiFPC FPC;
  defiTimingDisable TimingDisable;
  defiPartition Partition;
  defiPinProp PinProp;
  defiBlockage Blockage;
  defiSlot Slot;
  defiFill Fill;
  defiNonDefault NonDefault;
  defiStyles Styles;
  defiGeometries Geometries;

  int msgLimit[DEF_MSGS];
  char buffer[IN_BUF_SIZE];
  char* ring[RING_SIZE];
  int ringSizes[RING_SIZE];
  std::string stack[20]; /* the stack itself */

  YYSTYPE yylval;
  char lineBuffer[MSG_SIZE];

  FILE* File{nullptr};
  defrSession* session;
  defiSubnet* Subnet{nullptr};
  const defrSettings* settings;
  int aOxide{0};  // keep track for oxide
  int defInvalidChar{0};
  int defIgnoreVersion{0};  // ignore checking version number
  char* defMsg{nullptr};
  int defPrintTokens{0};
  char defPropDefType{'\0'};  // save the current type of the property
  int defaultCapWarnings{0};
  FILE* defrLog{nullptr};
  int input_level{-1};
  char* last{nullptr};  // points to the last valid char in the buffer, or null
  int new_is_keyword{0};
  long long nlines{1};
  char* rowName{nullptr};  // to hold the rowName for message
  int iOTimingWarnings{0};
  char* magic;
  int netWarnings{0};
  double save_x{0.0};
  double save_y{0.0};
  int sNetWarnings{0};
  int netOsnet{0};  // net = 1 & snet = 2
  char* next{nullptr};
  int rowWarnings{0};
  char* shieldName{nullptr};  // to hold the shieldNetName
  int deftokenLength{TOKEN_SIZE};
  double xStep{0.0};
  double yStep{0.0};
  // Flags to control what happens
  int NeedPathData{0};
  int shield{0};  // To identify if the path is shield for 5.3
  char* shiftBuf{nullptr};
  int shiftBufLength{0};
  int virtual_is_keyword{0};
  char* warningMsg{nullptr};
  double lVal{0.0};
  double rVal{0.0};
  char* deftoken;
  int doneDesign{0};  // keep track if the Design is done parsing
  char* uc_token;
  char* pv_deftoken;
  const defrCallbacks* callbacks;
};

class defrContext
{
 public:
  defrContext(int ownConf = 0);

  defrSettings* settings{nullptr};
  defrSession* session{nullptr};
  defrData* data{nullptr};
  int ownConfig;
  const char* init_call_func{nullptr};
  defrCallbacks* callbacks{nullptr};
};

int defrData::checkErrors()
{
  if (errors > 20) {
    defError(6011, "Too many syntax errors have been reported.");
    errors = 0;
    return 1;
  }

  return 0;
}

END_DEF_PARSER_NAMESPACE

#endif
