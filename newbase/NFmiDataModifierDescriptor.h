// ======================================================================
/*!
 * \file NFmiDataModifierDescriptor.h
 * \brief Interface of class NFmiDataModifierDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"

class NFmiParamDataModifierList;
class NFmiParamDataModifier;
class NFmiDataIdent;
class NFmiLevel;

//! Undocumented
class _FMI_DLL NFmiDataModifierDescriptor : public NFmiDataDescriptor
{
 public:
  ~NFmiDataModifierDescriptor(void) override;
  NFmiDataModifierDescriptor(void);

  NFmiParamDataModifier* VarianceModifier(const NFmiDataIdent& theParam, const NFmiLevel* theLevel);
  NFmiParamDataModifierList* GetVarianceModifiers(void);
  void SetVarianceModifiers(NFmiParamDataModifierList* theList);

  // seuraavat on pakko kirjoittaa, koska ne on määritelty pure virtualiksi emossa
  unsigned long Index(void) const override;
  unsigned long Size(void) const override;
  bool IsActive(void) const override;
  bool SetActivity(bool theActivityState) override;
  bool NextActive(void) override;
  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 protected:
  NFmiParamDataModifierList* itsSystematicModifiers;  // Omistaa listan
  NFmiParamDataModifierList* itsVarianceModifiers;    // Omistaa listan

 private:
  NFmiDataModifierDescriptor(const NFmiDataModifierDescriptor& theDesc);
  NFmiDataModifierDescriptor& operator=(const NFmiDataModifierDescriptor& theDesc);

};  // class NFmiDataModifierDescriptor

// ======================================================================
