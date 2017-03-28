// ======================================================================
/*!
 * \file NFmiValueString.h
 * \brief Interface of class NFmiValueString
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include "NFmiString.h"

enum FmiValues
{
  eNull,
  eChar,
  eShort,
  eInt,
  eLong,
  eFloat,
  eDouble
};

//! Undocumented
class _FMI_DLL NFmiValueString : public NFmiString
{
 public:
  ~NFmiValueString() override;
  NFmiValueString();

  NFmiValueString(short theValue, const char *theFormat = "%hd");
  NFmiValueString(int theValue, const char *theFormat = "%d");
  NFmiValueString(float theValue, const char *theFormat = "%f");
  NFmiValueString(long theValue, const char *theFormat = "%ld");
  NFmiValueString(double theValue, const char *theFormat = "%lf");

  NFmiValueString(const char * /*theValue*/);
  NFmiValueString(const unsigned char * /*theValue*/);
  NFmiValueString(const NFmiString & /*theValue*/);
  NFmiValueString(const NFmiValueString & /*theValue*/);

  void SetValue(short theValue, const char *theFormat = "%hd");
  void SetValue(int theValue, const char *theFormat = "%d");
  void SetValue(float theValue, const char *theFormat = "%f");
  void SetValue(long theValue, const char *theFormat = "%ld");
  void SetValue(double theValue, const char *theFormat = "%lf");

  bool ConvertToInt(int &theValue, int theStart = 0, int theSize = 0) const;
  bool ConvertToFloat(float &theValue, int theStart = 0, int theSize = 0) const;
  bool ConvertToLong(long &theValue, int theStart = 0, int theSize = 0) const;
  bool ConvertToDouble(double &theValue, int theStart = 0, int theSize = 0) const;

  unsigned int SignificantDecimals() const;

  bool IsNumeric(int theStart = 0, int theSize = 0) const;
  unsigned long SearchNumeric(unsigned long start = 1) const;

  bool IsShort() const;
  bool IsInt(int theStart = 0, int theSize = 0) const;
  bool IsFloat(int theStart = 0, int theSize = 0) const;
  bool IsLong(int theStart = 0, int theSize = 0) const;
  bool IsDouble(int theStart = 0, int theSize = 0) const;

  operator float() const;
  operator int() const;
  operator short() const;
  operator long() const;
  operator double() const;

  // palauttaa stringin, jossa on maksimissaan haluttu määrä desimaaleja. Mutta jos perään tulisi
  // jostain alkaen vain nollia, ne jätetään pois stringistä.
  // HUOM! static funktio eli käyttö koodissa: NFmiString str =
  // NFmiValueString::GetStringWithMaxDecimalsSmartWay(12.54, 4);
  // Palauttaa "12.54" eikä "12.5400"

  static const NFmiString GetStringWithMaxDecimalsSmartWay(double value, int maxDecimals);

 private:
  FmiValues itsValueType;

};  // NFmiValueString

// ======================================================================
