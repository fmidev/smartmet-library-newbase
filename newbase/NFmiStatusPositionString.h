// ======================================================================
/*!
 * \file NFmiStatusPositionString.h
 * \brief Interface of class NFmiStatusPositionString
 */
// ======================================================================

#pragma once

#include "NFmiStatusString.h"

//! Undocumented
class _FMI_DLL NFmiStatusPositionString : public NFmiStatusString
{
 public:
  ~NFmiStatusPositionString(void) override;
  NFmiStatusPositionString(void);
  NFmiStatusPositionString(const NFmiStatusPositionString& theStatusPositionString);
  NFmiStatusPositionString(const NFmiString& theString,
                           long theStatus,
                           long theStartPos,
                           long theEndPos);
  NFmiStatusPositionString(const NFmiStatusString& theString, long theStartPos, long theEndPos);

  NFmiStatusPositionString& operator=(const NFmiStatusPositionString& theStatusPositionString);

  long StartPosition(void);
  long EndPosition(void);
  unsigned long ClassId(void) const override;
  NFmiString* Clone() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 private:
  long itsStartPosition;
  long itsEndPosition;

};  // class NFmiStatusPositionString

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiStatusPositionString::ClassId(void) const
{
  return kNFmiStatusPositionString;
}

// ======================================================================
