// ======================================================================
/*!
 * \file NFmiDataModifierSum.h
 * \brief Interface of class NFmiDataModifierSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierSum : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierSum() override;
  NFmiDataModifierSum(FmiJoinOperator theJoinOperator = kFmiAdd, bool missingValuesAlloved = true);
  NFmiDataModifierSum(const NFmiDataModifierSum& theOther);
  NFmiDataModifier* Clone() const override;

  float Sum();
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  float itsSum;

};  // class NFmiDataModifierSum

// ======================================================================
