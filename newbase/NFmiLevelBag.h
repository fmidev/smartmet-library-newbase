// ======================================================================
/*!
 * \file NFmiLevelBag.h
 * \brief Interface of class NFmiLevelBag
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiLevel.h"
#include "NFmiSize.h"
#include "NFmiString.h"

class NFmiLevel;

//! Undocumented
class NFmiLevelBag : public NFmiSize
{
 public:
  ~NFmiLevelBag() override;
  NFmiLevelBag();
  NFmiLevelBag(const NFmiLevelBag &theLevelBag);
  NFmiLevelBag(NFmiLevel *theLevelArray, unsigned long numOfLevels);

  NFmiLevelBag(FmiLevelType theLevelType, float theMinValue, float theMaxValue, float theStep);

  NFmiLevelBag &operator=(const NFmiLevelBag &theLevelBag);
  bool operator==(const NFmiLevelBag &theLevelBag) const;

  bool IsStep();
  NFmiLevel *LevelMinValue();
  NFmiLevel *LevelMaxValue();

  NFmiLevel *Level() const;
  NFmiLevel *Level(unsigned long theIndex) const;
  bool Level(const NFmiLevel &theLevel);
  bool AddLevel(const NFmiLevel &theLevel);

  virtual const NFmiLevelBag Combine(const NFmiLevelBag &theBag) const;
  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const char* ClassName() const override;

 private:
  NFmiLevel *itsLevels;
  float itsStep;

};  // class NFmiLevelBag

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiLevelBag
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiLevelBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiLevelBag
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiLevelBag &ob)
{
  return ob.Read(file);
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiLevelBag::IsStep()
{
  return true;
  // return (itsStep >=0 ? true : false);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::LevelMinValue()
{
  // codechecker_false_positive [bugprone-pointer-arithmetic-on-polymorphic-object] the array
  // elements are exactly of the pointed-to type, there are no derived objects in it
  return &itsLevels[0];
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::LevelMaxValue()
{
  // codechecker_false_positive [bugprone-pointer-arithmetic-on-polymorphic-object] the array
  // elements are exactly of the pointed-to type, there are no derived objects in it
  return &itsLevels[itsSize - 1];
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::Level() const
{
  //  return &(itsLevels[itsIndex]);
  if (itsIndex >= 0 && itsIndex < static_cast<long>(GetSize()))
    // codechecker_false_positive [bugprone-pointer-arithmetic-on-polymorphic-object] the array
    // elements are exactly of the pointed-to type, there are no derived objects in it
    return &(itsLevels[itsIndex]);
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiLevel *NFmiLevelBag::Level(unsigned long theIndex) const
{
  if (theIndex < GetSize())
    // codechecker_false_positive [bugprone-pointer-arithmetic-on-polymorphic-object] the array
    // elements are exactly of the pointed-to type, there are no derived objects in it
    return &(itsLevels[theIndex]);
  else
    return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiLevelBag::ClassName() const
{
  return "NFmiLevelBag";
}

// ======================================================================
