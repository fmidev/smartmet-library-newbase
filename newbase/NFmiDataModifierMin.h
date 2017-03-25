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
  ~NFmiDataModifierMin(void) override;
  NFmiDataModifierMin(void);
  NFmiDataModifierMin(const NFmiDataModifierMin& theOther);
  NFmiDataModifier* Clone(void) const override;

  //  virtual void Calculate(float theValue);

  void Clear(void) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMin& operator=(const NFmiDataModifierMin& theOther);

};  // class NFmiDataModifierMin

// ======================================================================
