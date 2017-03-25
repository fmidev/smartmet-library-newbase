// ======================================================================
/*!
 * \file NFmiParamDataModifier.h
 * \brief Interface of class NFmiParamDataModifier
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataIdent;
class NFmiLevel;
class NFmiDataModifierList;

//! Tämä modifier on tehty parametri ja level kohtaiseksi. Match-metodilla kysytään onko haluttu
//! modifier.

class _FMI_DLL NFmiParamDataModifier : public NFmiDataModifier
{
 public:
  ~NFmiParamDataModifier() override;
  // NFmiDataModifier(const NFmiDataModifier & theModier); // compiler generated
  NFmiParamDataModifier(NFmiDataIdent* theParam,
                        NFmiLevel* theLevel = 0,
                        FmiJoinOperator theJoinOperator = kFmiAdd);

  std::ostream& WriteOperand(std::ostream& file) const override;
  bool BoolOperation(float) override;
  float FloatOperation(float theValue) override;

  using NFmiDataModifier::CalculationResult;
  using NFmiDataModifier::Calculate;

  float CalculationResult() override;
  void Calculate(float) override;

  void Clear() override;

  bool Match(const NFmiDataIdent& theParam, const NFmiLevel* theLevel);
  bool AddSubModifier(NFmiDataModifier* theModifier);
  NFmiDataModifierList* SubModifiers();
  NFmiDataIdent* Param();
  NFmiLevel* Level();

 protected:
  NFmiDataIdent* itsParam;           // Omistaa/tuhoaa
  NFmiLevel* itsLevel;               // Omistaa/tuhoaa
  NFmiDataModifierList* itsSubList;  // tässä voi olla mitä tahansa modifiereita esim. max tai min
                                     // jne. // Omistaa/tuhoaa

 private:
  NFmiParamDataModifier(const NFmiParamDataModifier& theMod);
  NFmiParamDataModifier& operator=(const NFmiParamDataModifier& theMod);

};  // class NFmiParamDataModifier

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataIdent* NFmiParamDataModifier::Param() { return itsParam; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel* NFmiParamDataModifier::Level() { return itsLevel; }

// ======================================================================
