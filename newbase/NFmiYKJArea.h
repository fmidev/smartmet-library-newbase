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
class _FMI_DLL NFmiYKJArea : public NFmiKKJArea
{
 public:
  ~NFmiYKJArea(void) override;
  NFmiYKJArea(void);
  NFmiYKJArea(const NFmiYKJArea& theYKJArea);

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
              bool usePacificView = false);

  NFmiYKJArea(double theRadialRangeInMeters,
              const NFmiPoint& theCenterLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  NFmiYKJArea(const NFmiPoint& theBottomLeftLatLon,
              const double theWidthInMeters,
              const double theHeightInMeters,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  NFmiYKJArea(const NFmiPoint& theBottomLeftWorldXY,  // defaults to meters
              const NFmiPoint& theTopRightWorldXY,    // defaults to meters
              const bool& IsWorldXYInMeters,          // dummy argument
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  NFmiArea* Clone(void) const override;

  unsigned long ClassId(void) const override;
  const char* ClassName(void) const override;
  const std::string AreaStr(void) const override;
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

 protected:
  int ZoneNumberByLongitude(const double lon) const override;
  int ZoneNumberByEasting(const double easting) const override;

 private:
  const NFmiPoint CornerWorldXY(const NFmiPoint& latlonPoint) const;

};  // class NFmiYKJArea

//! Undocumented, should be removed
typedef NFmiYKJArea* PNFmiYKJArea;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiYKJArea::~NFmiYKJArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiYKJArea::ClassId(void) const { return kNFmiYKJArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiYKJArea::ClassName(void) const { return "NFmiYKJArea"; }
