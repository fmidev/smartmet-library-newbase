// ======================================================================
/*!
 * \file NFmiDataModifierDummy.h
 * \brief Interface of class NFmiDataModifierDummy
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

// tallettaa vain arvon. Voidaan käyttää windChill-modifierista jos ei
// mitään varsinaista modifieria/Lasse

//! Undocumented
class _FMI_DLL NFmiDataModifierDummy : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierDummy() override;
  NFmiDataModifierDummy();
  using NFmiDataModifier::Calculate;
  void Calculate(float theValue) override;

  // Lasse ei kutsuta mistään, aikaisemmat kutsut QI::CalcInterpolatedTimeData ja
  // QI::CalcTimeData:sta jouduttu poistamaan

  void Calculate(NFmiQueryInfo* theQI) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  void Clear() override;

 protected:
  float itsValue;

};  // class NFmiDataModifierDummy

// ======================================================================
