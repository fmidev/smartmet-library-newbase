// ======================================================================
/*!
 * \file NFmiDataModifierMode.h
 * \brief Interface of class NFmiDataModifierMode
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiDataModifier.h"

class NFmiDataModifierMode : public NFmiDataModifier
{
 public:
  void Clear() override;

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult() override;

  NFmiDataModifierMode();
  ~NFmiDataModifierMode() override;
  NFmiDataModifierMode(const NFmiDataModifierMode &theOther);
  NFmiDataModifierMode& operator=(const NFmiDataModifierMode&) = delete;
  NFmiDataModifier* Clone() const override;

 private:
  class Data
  {
   public:
    Data() : itsValue(kFloatMissing), itsCounter(0) {}
    Data(float theValue) : itsValue(theValue), itsCounter(1) {}
    void Increase() { itsCounter++; }
    float Value() const { return itsValue; }
    void Value(float theValue) { itsValue = theValue; }
    int Counter() const { return itsCounter; }
    bool operator==(const Data& theData) const { return itsValue == theData.itsValue; }
    bool operator<(const Data& theData) const { return itsValue < theData.itsValue; }

   private:
    float itsValue;
    int itsCounter;
  };
  std::vector<Data> itsDataVector;
};

// ======================================================================
