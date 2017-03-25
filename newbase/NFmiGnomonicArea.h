// ======================================================================
/*!
 * \file NFmiGnomonicArea.h
 * \brief Interface of class NFmiGnomonicArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiAzimuthalArea.h"

//! Undocumented
class _FMI_DLL NFmiGnomonicArea : public NFmiAzimuthalArea
{
 public:
  ~NFmiGnomonicArea() override;

  NFmiGnomonicArea();

  NFmiGnomonicArea(const NFmiGnomonicArea& theGnomonicArea);

  NFmiGnomonicArea(const NFmiPoint& theBottomLeftLatLon,
                   const NFmiPoint& theTopRightLatLon,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.,  // see ref [2]
                   bool usePacificView = false);

  NFmiGnomonicArea(const NFmiPoint& theBottomLeftLatLon,
                   const double theWidthInMeters,
                   const double theHeightInMeters,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.);  // see ref [2]

  NFmiGnomonicArea(const double theRadialRange,
                   const double theCentralLongitude = 0.,
                   const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                   const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                   const double theCentralLatitude = 90.,
                   const double theTrueLatitude = 60.);

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  NFmiArea* Clone() const override;

  NFmiGnomonicArea& operator=(const NFmiGnomonicArea& theArea);

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;

  bool operator==(const NFmiGnomonicArea& theArea) const;
  bool operator!=(const NFmiGnomonicArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 protected:
  double K(const double delta) const override;
  double CalcDelta(const double xyDistance) const override;
  double DistanceFromPerspectivePointToCenterOfEarth() const override;

 private:
};  // class NFmiGnomonicArea

//! Undocumented, should be removed
typedef NFmiGnomonicArea* PNFmiGnomonicArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiGnomonicArea::~NFmiGnomonicArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiGnomonicArea::ClassId() const { return kNFmiGnomonicArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiGnomonicArea::ClassName() const { return "kNFmiGnomonicArea"; }

// ======================================================================
