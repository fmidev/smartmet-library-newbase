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
  ~NFmiStereographicArea(void) override;

  NFmiStereographicArea(void);

  NFmiStereographicArea(const NFmiStereographicArea& theStereographicArea);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        const NFmiPoint& theTopRightLatLon,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.,
                        bool usePacificView = false);

  NFmiStereographicArea(double theRadialRangeInMeters,
                        const NFmiPoint& theCenterLatLon,
                        const NFmiPoint& theTopLeftXY,
                        const NFmiPoint& theBottomRightXY);

  NFmiStereographicArea(const NFmiPoint& theBottomLeftLatLon,
                        const double theWidthInMeters,
                        const double theHeightInMeters,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.);

  NFmiStereographicArea(const double theRadialRange,
                        const double theCentralLongitude = 0.,
                        const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                        const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                        const double theCentralLatitude = 90.,
                        const double theTrueLatitude = 60.);

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* Clone(void) const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  const NFmiRect WorldRect(void) const override;

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const override;

  NFmiStereographicArea& operator=(const NFmiStereographicArea& theArea);

  bool operator==(const NFmiStereographicArea& theArea) const;
  bool operator!=(const NFmiStereographicArea& theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;
  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId(void) const override;
  const char* ClassName(void) const override;
  const std::string AreaStr(void) const override;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 protected:
  double K(const double delta) const override;
  double CalcDelta(const double xyDistance) const override;
  double DistanceFromPerspectivePointToCenterOfEarth(void) const override;

 private:
};  // class NFmiStereographicArea

//! Undocumented, should be removed
typedef NFmiStereographicArea* PNFmiStereographicArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiStereographicArea::~NFmiStereographicArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiStereographicArea::WorldRect(void) const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiStereographicArea::ClassId(void) const { return kNFmiStereographicArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiStereographicArea::ClassName(void) const { return "kNFmiStereographicArea"; }

// ======================================================================
