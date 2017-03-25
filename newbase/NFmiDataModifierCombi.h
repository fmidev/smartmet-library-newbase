// ======================================================================
/*!
 * \file NFmiDataModifierCombi.h
 * \brief Interface of class NFmiDataModifierCombi
 */
// ======================================================================

#pragma once

#include "NFmiCombinedParam.h"
#include "NFmiDataModifier.h"

class NFmiIntegrationSelector;

//! Undocumented
class _FMI_DLL NFmiDataModifierCombi : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierCombi() override;
  NFmiDataModifierCombi(const NFmiDataModifierCombi& other);

  NFmiDataModifierCombi(unsigned int theNumberOfValues = 0,
                        float theProbabilityScale = 100.,
                        FmiJoinOperator theJoinOperator = kFmiAdd,
                        bool missingValuesAllowed = true,
                        NFmiCombinedParam* theCombinedParam = 0);

  void Clear() override;
  using NFmiDataModifier::Calculate;
  void Calculate(float theValue) override;
  float CalcResult(const NFmiIntegrationSelector& theSelector);

 protected:
  float ProbValue(const NFmiIntegrationSelector& theSelector);
  bool CheckFrequency(float theValue, const NFmiIntegrationSelector& theSelector);
  bool CheckParams(double theValue);

 private:
  NFmiDataModifierCombi& operator=(const NFmiDataModifierCombi& theCombi);

  float itsProbabilityScale;
  int itsNumberOfValues;
  unsigned long itsTotalCounter;
  float itsMin;
  float itsMax;
  float itsSum;
  int* itsFrequencies;

};  // class NFmiDataModifierCombi

// ======================================================================
