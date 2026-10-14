// ======================================================================
/*!
 * \file NFmiDataModifierAllValidClipSum.h
 * \brief Interface of class NFmiDataModifierAllValidClipSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAllValidSum.h"

class NFmiDataModifierAllValidClipSum : public NFmiDataModifierAllValidSum
{
 public:
  ~NFmiDataModifierAllValidClipSum() override;
  NFmiDataModifierAllValidClipSum();

  void Calculate(float theValue) override { NFmiDataModifierAllValidSum::Calculate(theValue); }
  void Calculate(NFmiQueryInfo* theQI) override;

 private:
  float itsHalfValue = 0;

};  // class NFmiDataModifierAllValidClipSum

// ======================================================================
