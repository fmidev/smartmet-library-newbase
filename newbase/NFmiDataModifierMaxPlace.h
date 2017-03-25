// ======================================================================
/*!
 * \file NFmiDataModifierMaxPlace.h
 * \brief Interface of class NFmiDataModifierMaxPlace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMaxPlace
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtremePlace.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierMaxPlace : public NFmiDataModifierExtremePlace
{
 public:
  ~NFmiDataModifierMaxPlace(void) override;
  NFmiDataModifierMaxPlace(void);

  void Clear(void) override;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMaxPlace& operator=(const NFmiDataModifierMaxPlace& theOther);

};  // class NFmiDataModifierMaxPlace

// ======================================================================
