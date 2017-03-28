// ======================================================================
/*!
 * \file NFmiOrthographicArea.h
 * \brief Interface of class NFmiOrthographicArea
 */
// ======================================================================

#pragma once

#define MunFmiMax(x, y) ((x) > (y) ? (x) : (y))
#define MunFmiMin(x, y) ((x) < (y) ? (x) : (y))

#include "NFmiAzimuthalArea.h"

class _FMI_DLL NFmiOrthographicArea : public NFmiAzimuthalArea
{
 public:
  ~NFmiOrthographicArea() override;

  NFmiOrthographicArea();

  NFmiOrthographicArea(const NFmiOrthographicArea &thePerspectiveArea);

  NFmiOrthographicArea(const NFmiPoint &theCentralLatLonPoint,
                       double theRadialRange = kRearth,
                       double theAzimuthAngle = 0.,
                       const NFmiPoint &theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint &theBottomRightXY = NFmiPoint(1.f, 1.f),
                       bool usePacificView = false);

  /*
          HUOM! NFmiOrthographicArea -PROJEKTIOSSA NURKKAPISTEIDEN MUUNNOS
          EI VÄLTTÄMÄTTÄ AINA OLE BIJEKTIO:
          ESIM. JOILLEKIN SUORAKULMAISEN KOORDINAATISTON XY-PISTEILLE EI LÖYDY
          VASTAAVIA MAANTIETEELLISIÄ LONLAT -PISTEITÄ (PISTEET OVAT "KARTTAPALLON" ULKOPUOLELLA)
  */
  NFmiOrthographicArea(const NFmiPoint &theBottomLeftLatLon,
                       const NFmiPoint &theTopRightLatLon,
                       double theAzimuthAngle = 0.0,
                       const NFmiPoint &theTopLeftXY = NFmiPoint(0.f, 0.f),
                       const NFmiPoint &theBottomRightXY = NFmiPoint(1.f, 1.f),
                       bool usePacificView = false);

  void Init(bool fKeepWorldRect = false) override;

  using NFmiAzimuthalArea::LatLonToWorldXY;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXY) const override;

  void ZoomFactor(double theZoomFactor);
  double ZoomFactor() const;

  virtual double GlobeRadius() const;
  virtual void GlobeRadius(double &theGlobeRadius);

  virtual double AzimuthAngle() const;
  virtual void AzimuthAngle(double &theAzimuthAngle);

  NFmiArea *Clone() const override;
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  NFmiOrthographicArea &operator=(const NFmiOrthographicArea &theArea);
  bool operator==(const NFmiOrthographicArea &theArea) const;
  bool operator!=(const NFmiOrthographicArea &theArea) const;

  using NFmiAzimuthalArea::operator==;
  using NFmiAzimuthalArea::operator!=;
  bool operator==(const NFmiArea &theArea) const override;
  bool operator!=(const NFmiArea &theArea) const override;

  const char *ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  std::ostream &Write(std::ostream &file) const override;
  std::istream &Read(std::istream &file) override;

 protected:
  double K(const double /* delta */) const override { return 1.; };
  double CalcDelta(const double /* xyDistance */) const override { return 0.; };
  double DistanceFromPerspectivePointToCenterOfEarth() const override { return 0.; };
  double itsAzimuthAngle;
  double itsLat0;
  double itsLon0;
  double itsSinLat0;
  double itsCosLat0;
  double itsVisibilityTerm;
  double itsZoomFactor;
  double itsGlobeRadius;
  NFmiPoint itsCurrentLatlonPoint;

 private:
};  // class NFmiOrthographicArea

//! Undocumented, should be removed
using PNFmiOrthographicArea = NFmiOrthographicArea *;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiOrthographicArea::~NFmiOrthographicArea() {}
// ----------------------------------------------------------------------

inline double NFmiOrthographicArea::GlobeRadius() const { return itsGlobeRadius; }
// ----------------------------------------------------------------------

inline const char *NFmiOrthographicArea::ClassName() const { return "kNFmiOrthographicArea"; }

// ======================================================================
