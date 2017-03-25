// ======================================================================
/*!
 * \file NFmiDataModifierConstant.h
 * \brief Interface of class NFmiDataModifierConstant
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierConstant : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierConstant() override;
  NFmiDataModifierConstant(double theConstant, FmiJoinOperator theJoinOperator = kFmiAdd);
  double FloatValue(void) override;
  std::ostream& WriteOperand(std::ostream& file) const override;

 protected:
  double itsConstant;

};  // class NFmiDataModifierConstant

// ======================================================================
