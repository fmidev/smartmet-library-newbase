// ======================================================================
/*!
 * \file NFmiDataModifierBoolean.h
 * \brief Interface of class NFmiDataModifierBoolean
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

//! Undocumented
enum FmiModifierBoolOperations
{
  kFmiModifierValueLessThan,
  kFmiModifierValueGreaterThan,
  kFmiModifierValueLessOrEqualThan,
  kFmiModifierValueGreaterOrEqualThan,
  kFmiModifierValueIsEqual
};

//! Undocumented
class _FMI_DLL NFmiDataModifierBoolean : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierBoolean(void) override;
  NFmiDataModifierBoolean(FmiModifierBoolOperations theCondition,
                          NFmiDataModifier* theFirstValue,
                          NFmiDataModifier* theSecondValue);

  virtual bool BooleanValue(void);

  std::ostream& WriteOperator(std::ostream& file) const override;
  std::ostream& WriteExpressionBody(std::ostream& file) override;

  operator bool();

 protected:
  NFmiDataModifier* itsSecondValue;
  NFmiDataModifier* itsFirstValue;
  FmiModifierBoolOperations itsCondition;
  bool itsBooleanValue;

 private:
  NFmiDataModifierBoolean(const NFmiDataModifierBoolean& theMod);
  NFmiDataModifierBoolean& operator=(const NFmiDataModifierBoolean& theMod);

};  // class NFmiDataModifierBoolean

// ======================================================================
