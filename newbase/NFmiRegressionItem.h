// ======================================================================
/*!
 * \file NFmiRegressionItem.h
 * \brief Interface of class NFmiRegressionItem
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

class NFmiSuperSmartInfo;
class NFmiDataIdent;

//! Undocumented
class NFmiRegressionItem : public NFmiInfoModifier
{
 public:
  ~NFmiRegressionItem() override;
  NFmiRegressionItem(double theCoefficient = 1.0,
                     NFmiDataIdent* theDataIdent = nullptr,
                     NFmiLevel* theLevel = nullptr,
                     NFmiQueryInfo* theData = nullptr);

  using NFmiDataModifier::CalculationResult;
  void Calculate(NFmiQueryInfo* theData) override;
  void Calculate(float theValue) override { NFmiInfoModifier::Calculate(theValue); }
  float CalculationResult() override;
  double FloatValue() override;

 protected:
  double itsReturnValue = 0.0;

 private:
  double itsCoefficient;
  double itsConstant = 0.0;

};  // class NFmiRegressionItem

// ======================================================================
