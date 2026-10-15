// ======================================================================
/*!
 * \file NFmiMaskedDataIterator.h
 * \brief Interface of class NFmiMaskedDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"

class NFmiSuperSmartInfo;
class NFmiIntegrationSelector;

//! Undocumented
class NFmiMaskedDataIterator : public NFmiDataIterator
{
 public:
  ~NFmiMaskedDataIterator() override;
  NFmiMaskedDataIterator(NFmiSuperSmartInfo* theData,
                         bool isCombinedParam = false,
                         NFmiIntegrationSelector* theIntegrationSelector = nullptr);

  void DoForEach(NFmiDataModifier* theModifier) override;

  bool fIsCombinedParam;
  NFmiIntegrationSelector* itsIntegrationSelector;

 private:
  NFmiMaskedDataIterator(const NFmiMaskedDataIterator& theIter) = delete;
  NFmiMaskedDataIterator& operator=(const NFmiMaskedDataIterator& theIter) = delete;

};  // class NFmiMaskedDataIterator

// ======================================================================
