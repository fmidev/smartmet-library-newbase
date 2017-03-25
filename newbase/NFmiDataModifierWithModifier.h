// ======================================================================
/*!
 * \file NFmiDataModifierWithModifier.h
 * \brief Interface of class NFmiDataModifierWithModifier
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierWithModifier : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierWithModifier() override;
  NFmiDataModifierWithModifier(NFmiQueryInfo* theQueryInfoCopy,
                               NFmiDataModifier* theDataModifier,
                               int theXRange,
                               int theYRange);

  float FloatOperation(float theValue) override;
  void Clear() override;
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult() override;

 protected:
  NFmiQueryInfo* itsQueryInfoCopy;
  NFmiDataModifier* itsModifier;  // omistaa!!! tuhotaan destruktorissa
  int itsXRange;
  int itsYRange;

 private:
  NFmiDataModifierWithModifier(const NFmiDataModifierWithModifier& theMod);
  NFmiDataModifierWithModifier& operator=(const NFmiDataModifierWithModifier& theMod);

};  // class NFmiDataModifierWithModifier

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierWithModifier::~NFmiDataModifierWithModifier() { delete itsModifier; }
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theQueryInfoCopy Undocumented
 * \param theDataModifier Undocumented
 * \param theXRange Undocumented
 * \param theYRange Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierWithModifier::NFmiDataModifierWithModifier(NFmiQueryInfo* theQueryInfoCopy,
                                                                  NFmiDataModifier* theDataModifier,
                                                                  int theXRange,
                                                                  int theYRange)
    : itsQueryInfoCopy(theQueryInfoCopy),
      itsModifier(theDataModifier),
      itsXRange(theXRange),
      itsYRange(theYRange)
{
}

// ----------------------------------------------------------------------
/*
 *
 */
// ----------------------------------------------------------------------

inline void NFmiDataModifierWithModifier::Clear() { itsModifier->Clear(); }
// ----------------------------------------------------------------------
/*
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiDataModifierWithModifier::CalculationResult()
{
  return itsModifier->CalculationResult();
}

// ======================================================================
