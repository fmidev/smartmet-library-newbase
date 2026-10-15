// ======================================================================
/*!
 * \file NFmiDataModifierAllValidSum.h
 * \brief Interface of class NFmiDataModifierAllValidSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierSum.h"

class NFmiDataModifierAllValidSum : public NFmiDataModifierSum
{
 public:
  ~NFmiDataModifierAllValidSum() override;
  NFmiDataModifierAllValidSum();
  NFmiDataModifierAllValidSum(const NFmiDataModifierAllValidSum&) = default;

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;

 protected:
  long itsCounter = 0;

 private:
  NFmiDataModifierAllValidSum& operator=(const NFmiDataModifierAllValidSum& theOther);

};  // class NFmiDataModifierAllValidSum

// ======================================================================
