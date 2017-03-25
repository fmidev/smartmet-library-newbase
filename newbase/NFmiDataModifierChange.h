// ======================================================================
/*!
 * \file NFmiDataModifierChange.h
 * \brief Interface of class NFmiDataModifierChange
 *
 * Returns the difference between the last and the first value.
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierChange : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierChange() override;
  NFmiDataModifierChange();
  NFmiDataModifier* Clone() const override;

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool itsFirstValueOK;
  float itsFirstValue;
  float itsLastValue;

};  // class NFmiDataModifierChange

// ======================================================================
