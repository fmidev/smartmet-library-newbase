// ======================================================================
/*!
 * \file NFmiGaussKruger.cpp
 * \brief Implementation of class NFmiGaussKruger
 */
// ======================================================================
/*!
 * \class NFmiGaussKruger
 *
 * Ellipsoidal Transverse Mercator (Gauss-Kruger) projection using the Krueger
 * series expansion. The series coefficients are derived from the third
 * flattening n = (a-b)/(a+b) and are valid for any ellipsoid; the projection
 * parameters (central meridian, scale factor, false easting/northing) are free.
 *
 * The forward/inverse formulae follow the standard Gauss-Kruger development;
 * see e.g. Matti Ollikainen, Geodeettinen Laitos Tiedote 8 (1993). This class
 * intentionally carries no +towgs84 datum shift.
 */
// ======================================================================

#include "NFmiGaussKruger.h"

#include "NFmiGlobals.h"  // kPii
#include <macgyver/Exception.h>
#include <cmath>

namespace
{
inline double rad(double deg)
{
  return deg * kPii / 180.0;
}
inline double deg(double rad)
{
  return rad * 180.0 / kPii;
}
inline double arsinh(double x)
{
  return std::log(x + std::sqrt(1.0 + x * x));
}
inline double artanh(double x)
{
  return 0.5 * std::log((1.0 + x) / (1.0 - x));
}
}  // namespace

// ----------------------------------------------------------------------

NFmiGaussKruger::NFmiGaussKruger(double theEllipsoidA,
                                 double theEllipsoidInvF,
                                 double theCentralMeridian,
                                 double theScaleFactor,
                                 double theFalseEasting,
                                 double theFalseNorthing)
    : itsA(theEllipsoidA),
      itsInvF(theEllipsoidInvF),
      itsCentralMeridian(theCentralMeridian),
      itsScaleFactor(theScaleFactor),
      itsFalseEasting(theFalseEasting),
      itsFalseNorthing(theFalseNorthing),
      itsE(),
      itsA1(),
      itsH1(),
      itsH2()
{
  try
  {
    const double a = itsA;
    const double p = 1.0 / itsInvF;  // flattening
    const double b = (1.0 - p) * a;  // semi-minor axis
    itsE = std::sqrt((a * a - b * b) / (a * a));

    const double dn = (a - b) / (a + b);
    const double dn2 = dn * dn;
    const double dn3 = dn2 * dn;
    const double dn4 = dn3 * dn;

    itsA1 = a / (1.0 + dn) * (1.0 + dn2 / 4.0 + dn4 / 64.0);

    itsH1[0] = dn / 2.0 - 2.0 * dn2 / 3.0 + 5.0 * dn3 / 16.0 + 41.0 * dn4 / 180.0;
    itsH1[1] = 13.0 * dn2 / 48.0 - 3.0 * dn3 / 5.0 + 557.0 * dn4 / 1440.0;
    itsH1[2] = 61.0 * dn3 / 240.0 - 103.0 * dn4 / 140.0;
    itsH1[3] = 49561.0 * dn4 / 161280.0;

    itsH2[0] = dn / 2.0 - (2.0 / 3.0) * dn2 + 37.0 * dn3 / 96.0 - dn4 / 360.0;
    itsH2[1] = dn2 / 48.0 + dn3 / 15.0 - 437.0 * dn4 / 1440.0;
    itsH2[2] = 17.0 * dn3 / 480.0 - 37.0 * dn4 / 840.0;
    itsH2[3] = 4397.0 * dn4 / 161280.0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Geodetic (lon,lat) -> projected (easting,northing)
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiGaussKruger::ToProjected(const NFmiPoint& theLonLat) const
{
  try
  {
    const double lon = theLonLat.X();
    const double lat = theLonLat.Y();

    const double fii = rad(lat);
    const double d = std::sin(fii);
    const double lambda = rad(lon - itsCentralMeridian);

    const double q = std::sinh(arsinh(std::tan(fii)) - itsE * artanh(itsE * d));
    double ksi = std::atan(q / std::cos(lambda));
    double eta = artanh(std::sin(lambda) / std::sqrt(1.0 + q * q));

    const double ksi1 = 2.0 * ksi;
    const double eta1 = 2.0 * eta;

    for (int ind = 1; ind <= 4; ind++)
    {
      const double dind = ind;
      ksi += itsH1[ind - 1] * std::sin(dind * ksi1) * std::cosh(dind * eta1);
      eta += itsH1[ind - 1] * std::cos(dind * ksi1) * std::sinh(dind * eta1);
    }

    const double northing = itsFalseNorthing + itsScaleFactor * itsA1 * ksi;
    const double easting = itsFalseEasting + itsScaleFactor * itsA1 * eta;

    return NFmiPoint(easting, northing);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Projected (easting,northing) -> geodetic (lon,lat)
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiGaussKruger::ToLatLon(const NFmiPoint& theEastingNorthing) const
{
  try
  {
    const double easting = theEastingNorthing.X();
    const double northing = theEastingNorthing.Y();

    double ksi = (northing - itsFalseNorthing) / (itsScaleFactor * itsA1);
    double eta = (easting - itsFalseEasting) / (itsScaleFactor * itsA1);

    const double ksi1 = 2.0 * ksi;
    const double eta1 = 2.0 * eta;

    for (int ind = 1; ind <= 4; ind++)
    {
      const double dind = ind;
      ksi -= itsH2[ind - 1] * std::sin(dind * ksi1) * std::cosh(dind * eta1);
      eta -= itsH2[ind - 1] * std::cos(dind * ksi1) * std::sinh(dind * eta1);
    }

    const double lambda = std::atan(std::sinh(eta) / std::cos(ksi));
    const double q = arsinh(std::cos(lambda) * std::tan(ksi));

    // Fixed-point iteration for the geodetic latitude. The initial seed only
    // affects the iteration count; convergence uses the actual eccentricity.
    double temp = std::sin(std::atan(std::sinh(1.0040685 * q)));
    double d = temp;
    double testv = 0.0;
    do
    {
      const double q2 = artanh(itsE * temp);
      d = std::sinh(q + itsE * q2);
      d = d / std::sqrt(1.0 + d * d);
      testv = std::fabs(d - temp);
      temp = d;
    } while (testv >= 1.0E-11);

    const double lat = deg(std::asin(d));
    const double lon = itsCentralMeridian + deg(lambda);

    return NFmiPoint(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
