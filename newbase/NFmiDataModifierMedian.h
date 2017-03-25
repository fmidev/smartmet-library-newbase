// ======================================================================
/*!
 * \file NFmiDataModifierMedian.h
 * \brief Interface of class NFmiDataModifierMedian (8.10.03/EL)
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMedian
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierMedian : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierMedian() override;
  NFmiDataModifierMedian(float theLimitProsent = 50.f);
  NFmiDataModifierMedian(const NFmiDataModifierMedian& theOther);
  NFmiDataModifier* Clone() const override;

  float Median();
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  float LimitProsent() const { return itsLimitProsent; }
  void LimitProsent(float newValue);

 protected:
  checkedVector<float> itsMedianArray;
  float itsLimitProsent;  // ok median filter nimensä puolesta palauttaa puolivälistä, mutta lisäsin
                          // kuitenkin
  // säädettävän rajan, jonka mukaan arvo palautetaan. Defaulttina raja on 50% eli juuri puoliväli

};  // class NFmiDataModifierMedian

// ======================================================================
