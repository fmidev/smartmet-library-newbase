// ======================================================================
/*!
 * \file NFmiStreamQueryData.h
 * \brief Interface of class NFmiStreamQueryData
 */
// ======================================================================

#pragma once

#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"

//! Undocumented
class NFmiStreamQueryData
{
 public:
  ~NFmiStreamQueryData();
  NFmiStreamQueryData();
  NFmiStreamQueryData(NFmiQueryData* theQueryData, bool isOwnerData = false);

  // Marko Muutin rajapinnan niin, että theQueryData pointterita tuli pointterin pointteri.
  // Tällöin voin muuttaa koodin niin, että luettu data annetaan pointterin pointteriin ja omistus
  // siirtyy ulos NFmiStreamQueryData-oliolta.

  bool ReadData(const NFmiString& theFileName, NFmiQueryData** theQueryData = nullptr);
  bool ReadLatestData(const NFmiString& theFileName, NFmiQueryData** theQueryData = nullptr);
  bool SafeReadLatestData(const NFmiString& theFileName, NFmiQueryData** theQueryData = nullptr);
  bool WriteData(const NFmiString& theFileName,
                 NFmiQueryData* theQueryData = nullptr,
                 long theLibVersion = DefaultFmiInfoVersion) const;

  bool ReadIn(NFmiQueryData* theQueryData = nullptr);
  bool ReadIn(NFmiQueryInfo* theQueryInfo);
  bool WriteCout(NFmiQueryData* theQueryData = nullptr) const;

  bool IsData();

  NFmiQueryData* QueryData(bool theOwnerData = false);
  NFmiFastQueryInfo* QueryInfoIter();

 private:
  NFmiStreamQueryData(const NFmiStreamQueryData& theData) = delete;
  NFmiStreamQueryData& operator=(const NFmiStreamQueryData& theData) = delete;

  NFmiQueryData* itsQueryData;
  NFmiFastQueryInfo* itsQueryDataIter;
  bool itsOwnerData;  // Jos false, omistaa itsQueryData:n ja tuhoaa sen destruktorissa, jos true,
                      // ei omista eikä tuhoa.

};  // class NFmiStreamQueryData

// ======================================================================
