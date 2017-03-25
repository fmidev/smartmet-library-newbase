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
class _FMI_DLL NFmiDataModifierExtreme : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierExtreme(void) override;
  NFmiDataModifierExtreme(void);
  NFmiDataModifierExtreme(const NFmiDataModifierExtreme &theOther);

  void SetTime(NFmiQueryInfo *theQI);
  const NFmiTime GetTime(void);
  void Calculate(NFmiQueryInfo *theQI) override;
  void Calculate(float theValue) override;
  virtual void Calculate(float theValue, NFmiQueryInfo *theQI);

 protected:
  virtual bool IsNewExtreme(float) = 0;

 public:
  float itsExtremeValue;
  NFmiTime itsExtremeTime;

};  // class NFmiDataModifierExtreme

// ======================================================================
