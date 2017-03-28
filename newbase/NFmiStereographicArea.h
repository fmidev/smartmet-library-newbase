// ======================================================================
/*!
 * \file NFmiStereographicArea.h
 * \brief Interface of class NFmiStereographicArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumened
class _FMI_DLL NFmiStereographicArea : public NFmiAzimuthalArea
{
 public:
  ~NFmiStereographicArea() override;

  NFmiStereographicArea();

  NFmiStereographicArea(const NFmiStereographicArea& theStereographicArea);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        const NFmiPoint& theTopRightLatLon,
                        double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        double theCenterLatitude = 90.,
                        double theTrueLatitude = 60.,
                        bool usePacificView = false);

  NFmiStereographicArea(double theRadialRangeInMeters,
                        const NFmiPoint& theCenterLatLon,
                        const NFmiPoint& theTopLeftXY,
                        const NFmiPoint& theBottomRightXY);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        double theWidthInMeters,
                        double theHeightInMeters,
                        double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        double theCentralLatitude = 90.,
                        double theTrueLatitude = 60.);

  NFmiStereographicArea(double theRadialRange,
                        double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        double theCentralLatitude = 90.,
                        double theTrueLatitude = 60.);

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* Clone() const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  const NFmiRect WorldRect() const override;

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const override;

  NFmiStereographicArea& operator=(const NFmiStereographicArea& theArea);

  bool operator==(const NFmiStereographicArea& theArea) const;
  bool operator!=(const NFmiStereographicArea& theArea) const;

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
};  // class NFmiStereographicArea

//! Undocumented, should be removed
using PNFmiStereographicArea = NFmiStereographicArea*;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiStereographicArea::~NFmiStereographicArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiStereographicArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiStereographicArea::ClassId() const { return kNFmiStereographicArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiStereographicArea::ClassName() const { return "kNFmiStereographicArea"; }

// ======================================================================
