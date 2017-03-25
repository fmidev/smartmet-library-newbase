// ======================================================================
/*!
 * \file NFmiDataModifierStandardDeviation.h
 * \brief Interface of class NFmiDataModifierStandardDeviation
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierStandardDeviation : public NFmiDataModifier
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
  long itsCounter;
  float itsSum;
  float itsSquaredSum;

};  // class NFmiDataModifierStandardDeviation

// ======================================================================
