// ======================================================================
/*!
 * \file NFmiTransverseMercatorArea.h
 * \brief Interface of class NFmiTransverseMercatorArea
 */
// ======================================================================

#pragma once

#include "NFmiArea.h"
#include "NFmiGaussKruger.h"

//! Ellipsoidal Transverse Mercator projection area.
/*!
 * A native NFmiArea for transverse mercator projections, parameterized by
 * central meridian, scale factor, false easting/northing and ellipsoid. The
 * default parameters describe ETRS-TM35FIN (EPSG:3067) on the WGS84 ellipsoid,
 * which is how FMI treats 3067 in practice.
 *
 * This class is self-contained: the projection math lives in NFmiGaussKruger
 * and there is no dependency on the (deprecated) NFmiKKJArea / NFmiYKJArea. The
 * spatial reference is built from a clean +proj=tmerc string with explicit
 * ellipsoid parameters and no +towgs84 datum shift.
 */
class NFmiTransverseMercatorArea : public NFmiArea
{
 public:
  ~NFmiTransverseMercatorArea() override = default;
  NFmiTransverseMercatorArea();
  NFmiTransverseMercatorArea(const NFmiTransverseMercatorArea& theArea) = default;

  NFmiTransverseMercatorArea(const NFmiPoint& theBottomLeftLatLon,
                             const NFmiPoint& theTopRightLatLon,
                             double theCentralMeridian = NFmiGaussKruger::kTM35FIN_CentralMeridian,
                             double theScaleFactor = NFmiGaussKruger::kTM35FIN_ScaleFactor,
                             double theFalseEasting = NFmiGaussKruger::kTM35FIN_FalseEasting,
                             double theFalseNorthing = NFmiGaussKruger::kTM35FIN_FalseNorthing,
                             double theEllipsoidA = NFmiGaussKruger::kWGS84_A,
                             double theEllipsoidInvF = NFmiGaussKruger::kWGS84_InvF,
                             const NFmiPoint& theTopLeftXY = NFmiPoint(0., 0.),
                             const NFmiPoint& theBottomRightXY = NFmiPoint(1., 1.),
                             bool usePacificView = false);

  void Init(bool fKeepWorldRect = false) override;

  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint WorldXYToXY(const NFmiPoint& theWorldXYPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;

  const NFmiRect WorldRect() const override { return itsWorldRect; }

  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  NFmiArea* Clone() const override;

  unsigned long ClassId() const override { return kNFmiTransverseMercatorArea; }
  const char* ClassName() const override { return "NFmiTransverseMercatorArea"; }
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  NFmiTransverseMercatorArea& operator=(const NFmiTransverseMercatorArea& theArea) = default;

  virtual bool operator==(const NFmiTransverseMercatorArea& theArea) const;
  virtual bool operator!=(const NFmiTransverseMercatorArea& theArea) const;
  using NFmiArea::operator==;
  using NFmiArea::operator!=;
  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  std::size_t HashValue() const;

  double CentralMeridian() const { return itsCentralMeridian; }
  double ScaleFactor() const { return itsScaleFactor; }
  double FalseEasting() const { return itsFalseEasting; }
  double FalseNorthing() const { return itsFalseNorthing; }
  double EllipsoidA() const { return itsEllipsoidA; }
  double EllipsoidInvF() const { return itsEllipsoidInvF; }

 private:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;

  // Projection parameters
  double itsCentralMeridian;
  double itsScaleFactor;  // k0 at the central meridian
  double itsFalseEasting;
  double itsFalseNorthing;
  double itsEllipsoidA;
  double itsEllipsoidInvF;

  // Projection math, rebuilt in Init() from the parameters above
  NFmiGaussKruger itsProjection;

};  // class NFmiTransverseMercatorArea

// ======================================================================
