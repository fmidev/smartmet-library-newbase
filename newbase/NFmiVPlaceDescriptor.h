// ======================================================================
/*!
 * \file NFmiVPlaceDescriptor.h
 * \brief Interface of class NFmiVPlaceDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiLevelBag.h"

//! Undocumented
class _FMI_DLL NFmiVPlaceDescriptor : public NFmiDataDescriptor
{
 public:
  ~NFmiVPlaceDescriptor() override;
  NFmiVPlaceDescriptor();
  NFmiVPlaceDescriptor(const NFmiVPlaceDescriptor &theVPlaceDescriptor);
  NFmiVPlaceDescriptor(NFmiLevelBag &theLevelBag);

  NFmiVPlaceDescriptor &operator=(const NFmiVPlaceDescriptor &theVPlaceDescriptor);
  bool operator==(const NFmiVPlaceDescriptor &theVPlaceDescriptor) const;

  void Reset();
  bool Next();
  bool Previous();

  NFmiLevel *Level() const;
  NFmiLevel *Level(unsigned long theIndex) const;
  NFmiLevel *LevelMinValue() const;
  NFmiLevel *LevelMaxValue() const;
  bool Level(const NFmiLevel &theLevel);

  const NFmiLevelBag *Levels() const;

  bool IsStep() const;   // 'askel' määritelty? false jos ei.
  bool IsLevel() const;  // LevelBag määritelty?

  void LevelBag(NFmiLevelBag &theLevelBag);
  const NFmiLevelBag &LevelBag() const { return *itsLevelBag; }
  void Destroy();

  unsigned long Index() const override;
  bool Index(unsigned long theIndex);
  unsigned long Size() const override;

  bool NextActive() override;
  bool IsActive() const override;
  bool SetActivity(bool theActivityState) override;

  std::ostream &Write(std::ostream &file) const override;
  std::istream &Read(std::istream &file) override;

  virtual const char *ClassName() const;

 private:
  NFmiLevelBag *itsLevelBag;
  bool *itsActivity;

};  // class NFmiVPlaceDescriptor

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLevelBag *NFmiVPlaceDescriptor::Levels() const { return itsLevelBag; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiVPlaceDescriptor::Reset()
{
  if (itsLevelBag != nullptr) itsLevelBag->Reset();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiVPlaceDescriptor::IsStep() const { return itsLevelBag->IsStep(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiVPlaceDescriptor::IsActive() const { return (itsActivity[Index()]); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiVPlaceDescriptor::ClassName() const { return "NFmiVPlaceDescriptor"; }
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiVPlaceDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiVPlaceDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiVPlaceDescriptor
 *
 * \param file The input stream to read form
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiVPlaceDescriptor &ob)
{
  return ob.Read(file);
}

// ======================================================================
