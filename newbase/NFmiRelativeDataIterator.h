// ======================================================================
/*!
 * \file NFmiRelativeDataIterator.h
 * \brief Interface of class NFmiRelativeDataIterator
 */
// ======================================================================

#pragma once

#include "NFmiDataIterator.h"

//! Undocumented

class _FMI_DLL NFmiRelativeDataIterator : public NFmiDataIterator
{
 public:
  ~NFmiRelativeDataIterator() override;
  NFmiRelativeDataIterator(NFmiQueryInfo* theData, long dx = 0, long dy = 0, long dt = 0);

  NFmiRelativeDataIterator(NFmiQueryInfo* theData,
                           long dxStart,
                           long dyStart,
                           long dtStart,
                           long dxEnd,
                           long dyEnd,
                           long dtEnd);

  void SetDimensions(long dx, long dy, long dt) override;
  void DoForEach(NFmiDataModifier* theDataModifier = 0) override;

 protected:
  long itsDtStart;
  long itsDyStart;
  long itsDxStart;
  long itsDtEnd;
  long itsDyEnd;
  long itsDxEnd;

};  // class NFmiRelativeDataIterator

// ======================================================================
