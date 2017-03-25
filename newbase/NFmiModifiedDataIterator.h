// ======================================================================
/*!
 * \file NFmiModifiedDataIterator.h
 * \brief Interface of class NFmiModifierDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiRelativeDataIterator.h"

class NFmiInfoModifier;

//! Undocumented
class _FMI_DLL NFmiModifiedDataIterator : public NFmiRelativeDataIterator
{
 public:
  ~NFmiModifiedDataIterator() override;
  NFmiModifiedDataIterator(NFmiInfoModifier* theData, long dx = 0, long dy = 0, long dt = 0);
  void DoForEach(NFmiDataModifier* theDataModifier) override;

 protected:
  NFmiModifiedDataIterator(const NFmiModifiedDataIterator& theIter);
  NFmiModifiedDataIterator& operator=(const NFmiModifiedDataIterator& theIter);
  NFmiInfoModifier* itsPrimaryModifier;

};  // class NFmiModifiedDataIterator

// ======================================================================
