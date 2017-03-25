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
  ~NFmiDataModifierMaxPlace() override;
  NFmiDataModifierMaxPlace();

  void Clear() override;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMaxPlace& operator=(const NFmiDataModifierMaxPlace& theOther);

};  // class NFmiDataModifierMaxPlace

// ======================================================================
