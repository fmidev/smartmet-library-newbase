// ======================================================================
/*!
 * \file NFmiDataModifierMin.h
 * \brief Interface of class NFmiDataModifierMin
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtreme.h"

class _FMI_DLL NFmiDataModifierMin : public NFmiDataModifierExtreme
{
 public:
  ~NFmiDataModifierMin() override;
  NFmiDataModifierMin();
  NFmiDataModifierMin(const NFmiDataModifierMin& theOther);
  NFmiDataModifier* Clone() const override;

  //  virtual void Calculate(float theValue);

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMin& operator=(const NFmiDataModifierMin& theOther);

};  // class NFmiDataModifierMin

// ======================================================================
