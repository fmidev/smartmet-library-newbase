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
class _FMI_DLL NFmiMaskedDataIterator : public NFmiDataIterator
{
 public:
  ~NFmiMaskedDataIterator() override;
  NFmiMaskedDataIterator(NFmiSuperSmartInfo* theData,
                         bool isCombinedParam = false,
                         NFmiIntegrationSelector* theIntegrationSelector = 0);

  void DoForEach(NFmiDataModifier* theModifier) override;

  bool fIsCombinedParam;
  NFmiIntegrationSelector* itsIntegrationSelector;

 private:
  NFmiMaskedDataIterator(const NFmiMaskedDataIterator& theIter);
  NFmiMaskedDataIterator& operator=(const NFmiMaskedDataIterator& theIter);

};  // class NFmiMaskedDataIterator

// ======================================================================
