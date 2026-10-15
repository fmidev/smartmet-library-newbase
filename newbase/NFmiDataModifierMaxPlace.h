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
class NFmiDataModifierMaxPlace : public NFmiDataModifierExtremePlace
{
 public:
  ~NFmiDataModifierMaxPlace() override;
  NFmiDataModifierMaxPlace();
  NFmiDataModifierMaxPlace(const NFmiDataModifierMaxPlace&) = default;

  void Clear() override;

  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;

 protected:
  bool IsNewExtreme(float value) override;

 private:
  NFmiDataModifierMaxPlace& operator=(const NFmiDataModifierMaxPlace& theOther);

};  // class NFmiDataModifierMaxPlace

// ======================================================================
