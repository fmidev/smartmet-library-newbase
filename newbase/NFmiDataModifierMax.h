// ======================================================================
/*!
 * \file NFmiDataModifierMax.h
 * \brief Interface of class NFmiDataModifierMax
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMax
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtreme.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierMax : public NFmiDataModifierExtreme
{
 public:
  ~NFmiDataModifierMax() override;
  NFmiDataModifierMax();
  NFmiDataModifierMax(const NFmiDataModifierMax& theOther);
  NFmiDataModifier* Clone() const override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMax& operator=(const NFmiDataModifierMax& theOther);

};  // class NFmiDataModifierMax

// ======================================================================
