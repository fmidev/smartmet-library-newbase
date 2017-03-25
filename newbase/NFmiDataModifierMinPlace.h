// ======================================================================
/*!
 * \file NFmiDataModifierMinPlace.h
 * \brief Interface of class NFmiDataModifierMinPlace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMinPlace
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtremePlace.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierMinPlace : public NFmiDataModifierExtremePlace
{
 public:
  ~NFmiDataModifierMinPlace(void) override;
  NFmiDataModifierMinPlace(void);

  void Clear(void) override;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMinPlace& operator=(const NFmiDataModifierMinPlace& theOther);

};  // class NFmiDataModifierMinPlace

// ======================================================================
