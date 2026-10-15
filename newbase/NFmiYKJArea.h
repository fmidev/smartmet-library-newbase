// ======================================================================
/*!
 * \file NFmiYKJArea.h
 * \brief Interface of class NFmiYKJArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiKKJArea.h"

//! Undocumented
class NFmiYKJArea : public NFmiKKJArea
{
 public:
  ~NFmiYKJArea() override;
  NFmiYKJArea();
  NFmiYKJArea(const NFmiYKJArea& theYKJArea);

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F),
              bool usePacificView = false);

  NFmiYKJArea(double theRadialRangeInMeters,
              const NFmiPoint& theCenterLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              double theWidthInMeters,
              double theHeightInMeters,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

  NFmiYKJArea(const NFmiPoint& theBottomLeftWorldXY,  // defaults to meters
              const NFmiPoint& theTopRightWorldXY,    // defaults to meters
              const bool& IsWorldXYInMeters,          // dummy argument
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  NFmiArea* Clone() const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  NFmiYKJArea& operator=(const NFmiYKJArea& theArea);

  virtual bool operator==(const NFmiYKJArea& theArea) const;
  virtual bool operator!=(const NFmiYKJArea& theArea) const;

  using NFmiKKJArea::operator==;
  using NFmiKKJArea::operator!=;
  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  std::size_t HashValue() const;

 protected:
  int ZoneNumberByLongitude(double lon) const override;
  int ZoneNumberByEasting(double easting) const override;

 private:
  NFmiPoint CornerWorldXY(const NFmiPoint& latlonPoint) const;

};  // class NFmiYKJArea

//! Undocumented, should be removed
using PNFmiYKJArea = NFmiYKJArea*;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiYKJArea::~NFmiYKJArea() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiYKJArea::ClassId() const
{
  return kNFmiYKJArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiYKJArea::ClassName() const
{
  return "NFmiYKJArea";
}
