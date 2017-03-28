// ======================================================================
/*!
 * \file NFmiLambertEqualArea.h
 * \brief Interface of class NFmiLambertEqualArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumened
class _FMI_DLL NFmiLambertEqualArea : public NFmiAzimuthalArea
{
 public:
  ~NFmiLambertEqualArea() override;

  NFmiLambertEqualArea();

  NFmiLambertEqualArea(const NFmiLambertEqualArea& theLambertEqualArea);

  NFmiLambertEqualArea(const NFmiPoint& theBottomLeftLatLon,
                       const NFmiPoint& theTopRightLatLon,
                       double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       double theCentralLatitude = 52.,
                       double theTrueLatitude = 90.,
                       bool usePacificView = false);

  NFmiLambertEqualArea(double theRadialRangeInMeters,
                       const NFmiPoint& theCenterLatLon,
                       const NFmiPoint& theTopLeftXY,
                       const NFmiPoint& theBottomRightXY);

  NFmiLambertEqualArea(const NFmiPoint& theBottomLeftLatLon,
                       double theWidthInMeters,
                       double theHeightInMeters,
                       double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       double theCentralLatitude = 52.,
                       double theTrueLatitude = 90.);

  NFmiLambertEqualArea(double theRadialRange,
                       double theCentralLongitude = 10.,
                       const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                       double theCentralLatitude = 52.,
                       double theTrueLatitude = 90.);

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* Clone() const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  const NFmiRect WorldRect() const override;

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const override;

  NFmiLambertEqualArea& operator=(const NFmiLambertEqualArea& theArea);

  bool operator==(const NFmiLambertEqualArea& theArea) const;
  bool operator!=(const NFmiLambertEqualArea& theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;
  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 protected:
  double K(double delta) const override;
  double CalcDelta(double xyDistance) const override;
  double DistanceFromPerspectivePointToCenterOfEarth() const override;

 private:
};  // class NFmiLambertEqualArea

//! Undocumented, should be removed
using PNFmiLambertEqualArea = NFmiLambertEqualArea*;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiLambertEqualArea::~NFmiLambertEqualArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiLambertEqualArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiLambertEqualArea::ClassId() const { return kNFmiLambertEqualArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiLambertEqualArea::ClassName() const { return "kNFmiLambertEqualArea"; }

// ======================================================================
