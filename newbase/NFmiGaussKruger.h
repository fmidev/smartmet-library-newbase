// ======================================================================
/*!
 * \file NFmiGaussKruger.h
 * \brief Interface of class NFmiGaussKruger
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"

//! Ellipsoidal Transverse Mercator (Gauss-Kruger) projection math.
/*!
 * A small, self-contained projection primitive with no dependency on any
 * NFmiArea subclass. Given an ellipsoid (semi-major axis and inverse
 * flattening) and the projection parameters (central meridian, scale factor
 * at the central meridian, false easting/northing), it maps geodetic
 * coordinates to projected easting/northing and back using the Krueger
 * series expansion.
 *
 * The default parameters describe ETRS-TM35FIN (EPSG:3067) on the WGS84
 * ellipsoid. This class carries no datum shift (+towgs84); it is a pure map
 * projection on the given ellipsoid.
 */
class NFmiGaussKruger
{
 public:
  // WGS84 ellipsoid and ETRS-TM35FIN (EPSG:3067) projection parameters
  static constexpr double kWGS84_A = 6378137.0;
  static constexpr double kWGS84_InvF = 298.257223563;
  static constexpr double kTM35FIN_CentralMeridian = 27.0;
  static constexpr double kTM35FIN_ScaleFactor = 0.9996;
  static constexpr double kTM35FIN_FalseEasting = 500000.0;
  static constexpr double kTM35FIN_FalseNorthing = 0.0;

  NFmiGaussKruger(double theEllipsoidA = kWGS84_A,
                  double theEllipsoidInvF = kWGS84_InvF,
                  double theCentralMeridian = kTM35FIN_CentralMeridian,
                  double theScaleFactor = kTM35FIN_ScaleFactor,
                  double theFalseEasting = kTM35FIN_FalseEasting,
                  double theFalseNorthing = kTM35FIN_FalseNorthing);

  //! Geodetic (lon,lat in degrees) -> projected (easting,northing in meters)
  NFmiPoint ToProjected(const NFmiPoint& theLonLat) const;

  //! Projected (easting,northing in meters) -> geodetic (lon,lat in degrees)
  NFmiPoint ToLatLon(const NFmiPoint& theEastingNorthing) const;

  double EllipsoidA() const { return itsA; }
  double EllipsoidInvF() const { return itsInvF; }
  double CentralMeridian() const { return itsCentralMeridian; }
  double ScaleFactor() const { return itsScaleFactor; }
  double FalseEasting() const { return itsFalseEasting; }
  double FalseNorthing() const { return itsFalseNorthing; }

 private:
  // Projection parameters
  double itsA;
  double itsInvF;
  double itsCentralMeridian;
  double itsScaleFactor;
  double itsFalseEasting;
  double itsFalseNorthing;

  // Derived ellipsoid/series coefficients (computed in the constructor)
  double itsE;      // first eccentricity
  double itsA1;     // rectifying radius
  double itsH1[4];  // forward Krueger series
  double itsH2[4];  // inverse Krueger series
};

// ======================================================================
