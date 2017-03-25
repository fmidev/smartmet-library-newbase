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
  ~NFmiDataModifierSum(void) override;
  NFmiDataModifierSum(FmiJoinOperator theJoinOperator = kFmiAdd, bool missingValuesAlloved = true);
  NFmiDataModifierSum(const NFmiDataModifierSum& theOther);
  NFmiDataModifier* Clone(void) const override;

  float Sum(void);
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear(void) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;

 protected:
  float itsSum;

};  // class NFmiDataModifierSum

// ======================================================================
