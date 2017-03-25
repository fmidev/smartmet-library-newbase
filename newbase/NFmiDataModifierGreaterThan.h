// ======================================================================
/*!
 * \file NFmiDataModifierGreaterThan.h
 * \brief Interface of class NFmiDataModifierGreaterThan
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierGreaterThan : public NFmiDataModifier
{
 public:
  NFmiDataModifierGreaterThan(float theValue = kFloatMissing);
  bool BoolOperation(float theValue) override;

 private:
  double itsLimit;

};  // class NFmiDataModifierGreaterThan

// ======================================================================
