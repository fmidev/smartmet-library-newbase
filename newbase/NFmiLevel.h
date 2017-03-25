// ======================================================================
/*!
 * \file NFmiLevel.h
 * \brief Interface of class NFmiLevel
 */
// ======================================================================

#pragma once

#include "NFmiIndividual.h"
#include "NFmiLevelType.h"

//! Undocumented

class _FMI_DLL NFmiLevel : public NFmiIndividual
{
 public:
  ~NFmiLevel() override;
  NFmiLevel();
  NFmiLevel(const NFmiLevel& theLevel);
  NFmiLevel(unsigned long theIdent, const NFmiString& theName, float theLevelValue);
  NFmiLevel(FmiLevelType theLevelType, float theLevelValue);

  bool operator==(const NFmiLevel& theLevel) const;
  bool operator<(const NFmiLevel& theLevel) const;
  NFmiLevel& operator=(const NFmiLevel& theLevel);

  unsigned long LevelTypeId() const;
  float LevelValue() const;
  void LevelValue(float theLevelValue) { itsLevelValue = theLevelValue; }
  FmiLevelType LevelType() const;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const char* ClassName() const override;

  bool IsMissing() const;

 private:
  float itsLevelValue;

};  // class NFmiLevel

// ======================================================================
