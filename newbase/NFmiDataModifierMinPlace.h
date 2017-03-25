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
  ~NFmiDataModifierMinPlace() override;
  NFmiDataModifierMinPlace();

  void Clear() override;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMinPlace& operator=(const NFmiDataModifierMinPlace& theOther);

};  // class NFmiDataModifierMinPlace

// ======================================================================
