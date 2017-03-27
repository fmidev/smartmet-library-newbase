// ======================================================================
/*!
 * \file NFmiDataModifierList.h
 * \brief Interface of class NFmiDataModifierList
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiPtrList.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierList : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierList() override;
  // NFmiDataModifier(const NFmiDataModifier & theModier); // compiler generates
  NFmiDataModifierList(NFmiCombinedParam* thePotentialCombinedParam = 0);

  bool Add(NFmiDataModifier* theModifier);
  void Clear() override;

  bool IsCombinedParam();

  bool BoolOperation(float /*theValue*/) override;
  float FloatOperation(float theValue) override;

  using NFmiDataModifier::CalculationResult;
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::WriteExpressionBody;
  float CalculationResult() override;
  void Calculate(float NotInUse) override;
  std::ostream& WriteExpressionBody(std::ostream& file) override;

 public:
  double FloatValue() override;
  bool Reset();
  bool Next();
  NFmiDataModifier* Current();
  bool Remove(bool fDeleteData = false);
  void Clear(bool fDeleteData = false);
  unsigned long NumberOfItems();

 protected:
  double itsReturnValue;
  NFmiPtrList<NFmiDataModifier> itsList;
  NFmiPtrList<NFmiDataModifier>::Iterator itsIter;

};  // class NFmiDataModifierList

// ----------------------------------------------------------------------
/*!
 * return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiDataModifierList::BoolOperation(float /*theValue*/) { return false; }
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::FloatOperation(float theValue) { return theValue; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierList::CalculationResult()
{
  return static_cast<float>(itsReturnValue);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierList::Clear() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiDataModifierList::NumberOfItems() { return itsList.NumberOfItems(); }

// ======================================================================
