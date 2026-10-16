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

class NFmiDataModifierChange : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierChange() override;
  NFmiDataModifierChange();
  NFmiDataModifierChange(const NFmiDataModifierChange&) = default;
  NFmiDataModifierChange& operator=(const NFmiDataModifierChange&) = default;
  NFmiDataModifier* Clone() const override;

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool itsFirstValueOK = false;
  float itsFirstValue;
  float itsLastValue;

};  // class NFmiDataModifierChange

// ======================================================================
