// ======================================================================
/*!
 * \file NFmiDataModifierAllValidSum.h
 * \brief Interface of class NFmiDataModifierAllValidSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierSum.h"

class _FMI_DLL NFmiDataModifierAllValidSum : public NFmiDataModifierSum
{
 public:
  ~NFmiDataModifierAllValidSum() override;
  NFmiDataModifierAllValidSum();

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;

 protected:
  long itsCounter;

 private:
  NFmiDataModifierAllValidSum& operator=(const NFmiDataModifierAllValidSum& theOther);

};  // class NFmiDataModifierAllValidSum

// ======================================================================
