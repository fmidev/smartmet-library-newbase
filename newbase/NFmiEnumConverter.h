// ======================================================================
/*!
 * \file NFmiEnumConverter.h
 * \brief Interface of class NFmiEnumConverter
 */
// ----------------------------------------------------------------------

#pragma once

#include "NFmiDef.h"
#include "NFmiGlobals.h"
#include "NFmiParameterName.h"

#include <list>
#include <memory>
#include <string>

class _FMI_DLL NFmiEnumConverter
{
 public:
  ~NFmiEnumConverter();
  NFmiEnumConverter(FmiEnumSpace theEnumspace = kParamNames);

  const char *ToCharPtr(int theValue);
  const std::string ToString(int theValue);

  int ToEnum(const char *s);
  int ToEnum(const std::string &theString) { return ToEnum(theString.c_str()); }
  std::list<std::string> Names();

 private:
  class Impl;
  std::unique_ptr<Impl> impl;

};  // class NFmiEnumConverter

// ======================================================================
