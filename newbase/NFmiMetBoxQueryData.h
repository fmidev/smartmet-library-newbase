// ======================================================================
/*!
 * \file NFmiMetBoxQueryData.h
 * \brief Interface of class NFmiMetBoxQueryData
 */
// ======================================================================

#pragma once

#include "NFmiMetBox.h"
#include "NFmiQueryData.h"

//! Undocumented
class _FMI_DLL NFmiMetBoxQueryData : public NFmiQueryData
{
 public:
  ~NFmiMetBoxQueryData() override;
  NFmiMetBoxQueryData();
  NFmiMetBoxQueryData(const NFmiMetBoxQueryData& theData);
  NFmiMetBoxQueryData(NFmiQueryInfo& theInfo);

  virtual NFmiMetBox* Value();

 private:
  NFmiMetBoxQueryData& operator=(const NFmiMetBoxQueryData& theData);

  NFmiMetBox* itsMetBox;

};  // class NFmiMetBoxQueryData

// ======================================================================
