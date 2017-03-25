// ======================================================================
/*!
 * \file NFmiRegressionModifier.h
 * \brief Interface of class NFmiRegressionModifier
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

//! Undocumented
class _FMI_DLL NFmiRegressionModifier : public NFmiInfoModifier
{
 public:
  ~NFmiRegressionModifier() override;
  NFmiRegressionModifier(NFmiDataIdent* theParam, NFmiLevel* theLevel, NFmiQueryInfo* theData = 0);

  std::ostream& WriteOperand(std::ostream& file) const override;
  double FloatValue() override;

 protected:
  NFmiDataModifierList* itsRegressionItems;

 private:
  NFmiRegressionModifier(const NFmiRegressionModifier& theMod);
  NFmiRegressionModifier& operator=(const NFmiRegressionModifier& theMod);

};  // class NFmiRegressionModifier

// ======================================================================
