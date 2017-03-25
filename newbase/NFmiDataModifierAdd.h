// ======================================================================
/*!
 * \file NFmiDataModifierAdd.h
 * \brief Interface of class NFmiDataModifierAdd
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierAdd : public NFmiDataModifier
{
 public:
  NFmiDataModifierAdd(float theAddValue = 0);
  float FloatOperation(float theValue) override;

 private:
  float itsAddValue;

};  // class NFmiDataModifierAdd

// ======================================================================
