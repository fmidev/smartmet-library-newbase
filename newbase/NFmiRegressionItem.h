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
class _FMI_DLL NFmiRegressionItem : public NFmiInfoModifier
{
 public:
  ~NFmiRegressionItem() override;
  NFmiRegressionItem(double theCoefficient = 1.0,
                     NFmiDataIdent* theDataIdent = 0,
                     NFmiLevel* theLevel = 0,
                     NFmiQueryInfo* theData = 0);

  using NFmiDataModifier::CalculationResult;
  void Calculate(NFmiQueryInfo* theData) override;
  void Calculate(float theValue) override { NFmiInfoModifier::Calculate(theValue); }
  float CalculationResult() override;
  double FloatValue() override;

 protected:
  double itsReturnValue;

 private:
  double itsCoefficient;
  double itsConstant;

};  // class NFmiRegressionItem

// ======================================================================
