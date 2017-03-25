// ======================================================================
/*!
 * \file NFmiDataModifierPrecFormUnion.h
 * \brief Interface of class NFmiDataModifierPrecFormUnion
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiWeatherAndCloudiness.h"

class _FMI_DLL NFmiDataModifierPrecFormUnion : public NFmiDataModifier
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
  float Result();

 private:
  bool fIsRain;
  bool fIsSleet;
  bool fIsSnow;
  bool fIsFreezing;

};  // class NFmiDataModifierPrecFormUnion

// ======================================================================
