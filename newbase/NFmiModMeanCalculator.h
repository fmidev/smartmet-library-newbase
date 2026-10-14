// ======================================================================
/*!
 * \brief Interface of class NFmiModMeanCalculator
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

class NFmiModMeanCalculator
{
 public:
  NFmiModMeanCalculator(float theModulo = 360);

  void operator()(float theValue, float theWeight = 1.0);
  float operator()() const;
  int count() const;

 private:
  float itsModulo;
  bool itsValid = true;
  int itsCount = 0;
  float itsLastValue = 0;
  float itsWeightedSum = 0;
  float itsWeightSum = 0;

};  // NFmiModMeanCalculator

// ======================================================================
