// ======================================================================
/*!
 * \file NFmiDataModifierLogical.h
 * \brief Interface of classNFmiDataModifierLogical
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBoolean.h"

//! Undocumented

class _FMI_DLL NFmiDataModifierLogical : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierLogical() override;
  NFmiDataModifierLogical(NFmiDataModifierBoolean* theCondition = 0,
                          NFmiDataModifier* thePrimaryModifier = 0,
                          NFmiDataModifier* theSecondaryModifier = 0);

  std::ostream& WriteOperand(std::ostream& file) const override;
  double FloatValue() override;

 protected:
  NFmiDataModifierBoolean* itsCondition;
  NFmiDataModifier* itsSecondaryModifier;
  NFmiDataModifier* itsPrimaryModifier;

 private:
  NFmiDataModifierLogical(const NFmiDataModifierLogical& theMod);
  NFmiDataModifierLogical& operator=(const NFmiDataModifierLogical& theMod);

};  // class NFmiDataModifierLogical

// ======================================================================
