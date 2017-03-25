// ======================================================================
/*!
 * \file NFmiDataModifierAllValidAvg.h
 * \brief Interface of class NFmiDataModifierAllValidAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class _FMI_DLL NFmiDataModifierAllValidAvg : public NFmiDataModifierAvg
{
 public:
  ~NFmiDataModifierAllValidAvg() override;
  NFmiDataModifierAllValidAvg();
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

 private:
  NFmiDataModifierAllValidAvg& operator=(const NFmiDataModifierAllValidAvg& theOther);

};  // class NFmiDataModifierAllValidAvg

// ======================================================================
