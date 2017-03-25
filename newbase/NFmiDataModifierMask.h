// ======================================================================
/*!
 * \file NFmiDataModifierMask.h
 * \brief Interface of class NFmiDataModiferMask
 */
// ======================================================================
/*!
 * \class NFmiDataModiferMask
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiGlobals.h"  // Added by ClassView

//! Undocumented
class _FMI_DLL NFmiDataModifierMask : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierMask() override;
  NFmiDataModifierMask(FmiMaskOperation theCondition = kFmiNoMaskOperation,
                       double theFirstLimit = kFloatMissing,
                       double theSecondLimit = kFloatMissing);
  bool BoolOperation(float theValue) override;

 protected:
  FmiMaskOperation itsMaskOperation;
  double its1Limit;
  double its2Limit;

};  // class NFmiDataModifierMask

// ======================================================================
