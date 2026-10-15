// ======================================================================
/*!
 * \file NFmiDataModifierExtreme.h
 * \brief abstrakti luokka min/max:ia varten joka myös antaa äärirvon
 *  sattumisajan/Lasse
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiMetTime.h"

//! Undocumented
class NFmiDataModifierExtreme : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierExtreme() override;
  NFmiDataModifierExtreme();
  NFmiDataModifierExtreme(const NFmiDataModifierExtreme &theOther);
  NFmiDataModifierExtreme& operator=(const NFmiDataModifierExtreme&) = delete;

  void SetTime(NFmiQueryInfo *theQI);
  const NFmiTime GetTime() const;
  void Calculate(NFmiQueryInfo* theQI) override;
  void Calculate(float theValue) override;
  virtual void Calculate(float theValue, NFmiQueryInfo *theQI);

 protected:
  virtual bool IsNewExtreme(float) = 0;

 public:
  float itsExtremeValue = 0.0F;
  NFmiTime itsExtremeTime;

};  // class NFmiDataModifierExtreme

// ======================================================================
