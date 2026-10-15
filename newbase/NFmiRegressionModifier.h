// ======================================================================
/*!
 * \file NFmiRegressionModifier.h
 * \brief Interface of class NFmiRegressionModifier
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

//! Undocumented
class NFmiRegressionModifier : public NFmiInfoModifier
{
 public:
  ~NFmiRegressionModifier() override;
  NFmiRegressionModifier(NFmiDataIdent* theParam,
                         NFmiLevel* theLevel,
                         NFmiQueryInfo* theData = nullptr);

  std::ostream& WriteOperand(std::ostream& file) const override;
  double FloatValue() override;

 protected:
  NFmiDataModifierList* itsRegressionItems;

 private:
  NFmiRegressionModifier(const NFmiRegressionModifier& theMod) = delete;
  NFmiRegressionModifier& operator=(const NFmiRegressionModifier& theMod) = delete;

};  // class NFmiRegressionModifier

// ======================================================================
