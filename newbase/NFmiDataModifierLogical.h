// ======================================================================
/*!
 * \file NFmiDataModifierLogical.h
 * \brief Interface of classNFmiDataModifierLogical
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierBoolean.h"

//! Undocumented

class NFmiDataModifierLogical : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierLogical() override;
  NFmiDataModifierLogical(NFmiDataModifierBoolean* theCondition = nullptr,
                          NFmiDataModifier* thePrimaryModifier = nullptr,
                          NFmiDataModifier* theSecondaryModifier = nullptr);

  std::ostream& WriteOperand(std::ostream& file) const override;
  double FloatValue() override;

 protected:
  NFmiDataModifierBoolean* itsCondition;
  NFmiDataModifier* itsSecondaryModifier;
  NFmiDataModifier* itsPrimaryModifier;

 private:
  NFmiDataModifierLogical(const NFmiDataModifierLogical& theMod) = delete;
  NFmiDataModifierLogical& operator=(const NFmiDataModifierLogical& theMod) = delete;

};  // class NFmiDataModifierLogical

// ======================================================================
