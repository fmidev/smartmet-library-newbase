// ======================================================================
/*!
 * \file NFmiDataModifierStandardDeviation.h
 * \brief Interface of class NFmiDataModifierStandardDeviation
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierStandardDeviation : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierStandardDeviation() override;
  NFmiDataModifierStandardDeviation();

  void Clear() override;

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult() override;

 protected:
  long itsCounter = 0;
  float itsSum = 0;
  float itsSquaredSum = 0;

};  // class NFmiDataModifierStandardDeviation

// ======================================================================
