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
  ~NFmiDataModifierMax(void) override;
  NFmiDataModifierMax(void);
  NFmiDataModifierMax(const NFmiDataModifierMax& theOther);
  NFmiDataModifier* Clone(void) const override;

  void Clear(void) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMax& operator=(const NFmiDataModifierMax& theOther);

};  // class NFmiDataModifierMax

// ======================================================================
