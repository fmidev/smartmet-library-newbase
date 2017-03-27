// ======================================================================
/*!
 * \file NFmiEquidistArea.h
 * \brief Interface of class NFmiEquidistArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumented

class _FMI_DLL NFmiEquidistArea : public NFmiAzimuthalArea
{
 public:
  ~NFmiEquidistArea() override;
  NFmiEquidistArea();
  NFmiEquidistArea(const NFmiEquidistArea& theEquidistArea);

  NFmiEquidistArea(double theRadialRangeInMeters,
                   const NFmiPoint& theCenterLatLon,
                   const NFmiPoint& theTopLeftXY,
                   const NFmiPoint& theBottomRightXY);

  NFmiEquidistArea(const NFmiPoint& theBottomLeftLatLon,
                   const NFmiPoint& theTopRightLatLon,
                   double theOrientation = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   double theCentralLatitude = 90.,  // 90 deg. = North Pole
                   bool usePacificView = false);

  NFmiEquidistArea(const NFmiPoint& theBottomLeftLatLon,
                   double theWidthInMeters,
                   double theHeightInMeters,
                   double theOrientation = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   double theCentralLatitude = 90.);  // 90 deg. = North Pole

  NFmiEquidistArea(double theRadialRange,
                   double theOrientation = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   double theCentralLatitude = 90.);  // 90 deg. = North Pole

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  NFmiEquidistArea& operator=(const NFmiEquidistArea& theArea);
  bool operator==(const NFmiEquidistArea& theArea) const;
  bool operator!=(const NFmiEquidistArea& theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  NFmiArea* Clone() const override;
  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

 protected:
  double K(double delta) const override;
  double CalcDelta(double xyDistance) const override;
  double DistanceFromPerspectivePointToCenterOfEarth() const override;

 private:
};  // class NFmiEquidistArea

//! Undocumented, should be removed
typedef NFmiEquidistArea* PNFmiEquidistArea;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiEquidistArea::~NFmiEquidistArea() {}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiEquidistArea::NFmiEquidistArea() : NFmiAzimuthalArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiEquidistArea::ClassId() const { return kNFmiEquiDistArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiEquidistArea::ClassName() const { return "NFmiEquiDistArea"; }

// ======================================================================
