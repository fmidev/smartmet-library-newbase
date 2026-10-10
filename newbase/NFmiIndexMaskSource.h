// ======================================================================
/*!
 * \file
 * \brief Interface of class NFmiIndexMaskSource
 */
// ======================================================================

#pragma once

#include "NFmiIndexMask.h"
#include "NFmiMetTime.h"
#include <map>

class NFmiIndexMaskSource
{
 public:
  ~NFmiIndexMaskSource();
  NFmiIndexMaskSource();
  NFmiIndexMaskSource(const NFmiIndexMaskSource& theSource);
  NFmiIndexMaskSource& operator=(const NFmiIndexMaskSource& theSource);

  void Insert(const NFmiMetTime& theTime, const NFmiIndexMask& theMask);
  const NFmiIndexMask& Find(const NFmiMetTime& theTime) const;
  void Clear();

 private:
  using storage_type = std::map<NFmiMetTime, NFmiIndexMask>;
  storage_type itsData;

};  // class NFmiIndexMaskSource

// ======================================================================
