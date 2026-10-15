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
class NFmiDataModifierCombi : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierCombi() override;
  NFmiDataModifierCombi(const NFmiDataModifierCombi& other);

  NFmiDataModifierCombi(unsigned int theNumberOfValues = 0,
                        float theProbabilityScale = 100.,
                        FmiJoinOperator theJoinOperator = kFmiAdd,
                        bool missingValuesAllowed = true,
                        NFmiCombinedParam* theCombinedParam = nullptr);

  void Clear() override;
  using NFmiDataModifier::Calculate;
  void Calculate(float theValue) override;
  float CalcResult(const NFmiIntegrationSelector& theSelector);

 protected:
  float ProbValue(const NFmiIntegrationSelector& theSelector);
  bool CheckFrequency(float theValue, const NFmiIntegrationSelector& theSelector);
  bool CheckParams(double theValue) const;

 private:
  NFmiDataModifierCombi& operator=(const NFmiDataModifierCombi& theCombi) = delete;

  float itsProbabilityScale;
  int itsNumberOfValues;
  unsigned long itsTotalCounter = 0;
  float itsMin = 0.0F;
  float itsMax = 0.0F;
  float itsSum = 0.0F;
  int* itsFrequencies = nullptr;

};  // class NFmiDataModifierCombi

// ======================================================================
