// ======================================================================
/*!
 * \file NFmiDataModifierPrecFormUnion.h
 * \brief Interface of class NFmiDataModifierPrecFormUnion
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiWeatherAndCloudiness.h"

class NFmiDataModifierPrecFormUnion : public NFmiDataModifier
{
  // HUOM missingValuesAlloved = true koska pouta tuo puuttuvan !!!

 public:
  ~NFmiDataModifierPrecFormUnion() override;
  NFmiDataModifierPrecFormUnion(FmiJoinOperator theJoinOperator = kFmiAdd,
                                bool missingValuesAlloved = true);

  void Clear() override;
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  float Result() const;

 private:
  bool fIsRain = false;
  bool fIsSleet = false;
  bool fIsSnow = false;
  bool fIsFreezing = false;

};  // class NFmiDataModifierPrecFormUnion

// ======================================================================
