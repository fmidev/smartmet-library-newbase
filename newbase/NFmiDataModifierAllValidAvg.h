// ======================================================================
/*!
 * \file NFmiDataModifierAllValidAvg.h
 * \brief Interface of class NFmiDataModifierAllValidAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class NFmiDataModifierAllValidAvg : public NFmiDataModifierAvg
{
 public:
  ~NFmiDataModifierAllValidAvg() override;
  NFmiDataModifierAllValidAvg();
  NFmiDataModifierAllValidAvg(const NFmiDataModifierAllValidAvg&) = default;
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

 private:
  NFmiDataModifierAllValidAvg& operator=(const NFmiDataModifierAllValidAvg& theOther) = delete;

};  // class NFmiDataModifierAllValidAvg

// ======================================================================
