// ======================================================================
/*!
 * \file NFmiStatusString.h
 * \brief Interface of class NFmiStatusString
 */
// ======================================================================

#pragma once

#include "NFmiString.h"

//! Undocumented
class _FMI_DLL NFmiStatusString : public NFmiString
{
 public:
  ~NFmiStatusString() override;
  NFmiStatusString();
  NFmiStatusString(const NFmiStatusString& theStatusString);
  NFmiStatusString(const NFmiString& theString, long theStatus);

  NFmiStatusString& operator=(const NFmiStatusString& theStatusString);

  long Status();
  const NFmiString& String();
  unsigned long ClassId() const override { return kNFmiStatusString; };
  NFmiString* Clone() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 private:
  long itsStatus;

};  // class NFmiStatusString

// ======================================================================
