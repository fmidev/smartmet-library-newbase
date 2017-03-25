// ======================================================================
/*!
 * \file NFmiDataModifierCombinedParam.h
 * \brief Interface of class NFmiDataModifierCombinedParam
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiCombinedParam;

//! Undocumented

class _FMI_DLL NFmiDataModifierCombinedParam : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierCombinedParam() override;
  NFmiDataModifierCombinedParam(const NFmiCombinedParam &theParam);

  void Clear(void) override;

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult() override;

  NFmiCombinedParam *CombinedCalculationResult(void) override;

 protected:
  NFmiCombinedParam *itsCombinedParam;

 private:
  NFmiDataModifierCombinedParam(const NFmiDataModifierCombinedParam &theParam);
  NFmiDataModifierCombinedParam &operator=(const NFmiDataModifierCombinedParam &theParam);

};  // class NFmiDataModifierCombinedParam

// ======================================================================
