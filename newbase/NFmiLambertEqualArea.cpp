// ======================================================================
/*!
 * \file NFmiLambertEqualArea.cpp
 * \brief Implementation of class NFmiLambertEqualArea
 */
// ======================================================================
/*!
 * \class NFmiLambertEqualArea
 *
 */
// ======================================================================

#include "NFmiLambertEqualArea.h"
#include "NFmiStringTools.h"
#include <fmt/format.h>
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea::NFmiLambertEqualArea() = default;
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theLambertEqualArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea::NFmiLambertEqualArea(const NFmiLambertEqualArea &theLambertEqualArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCenterLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea::NFmiLambertEqualArea(const NFmiPoint &theBottomLeftLatLon,
                                           const NFmiPoint &theTopRightLatLon,
                                           const double theCentralLongitude,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY,
                                           const double theCenterLatitude,
                                           const double theTrueLatitude,
                                           bool usePacificView)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theTopRightLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCenterLatitude,
                        theTrueLatitude,
                        usePacificView)

{
  // Note: This projection was never used at FMI. Also, PROJ manual does not mention true latitude
  // for this projection, but I added +lat_ts nevertheless - Mika

  Init();
}

/*!
 * Constructor
 *
 * \param theCenterLatLon Undocumented
 * \param theRadialRangeInMeters Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
NFmiLambertEqualArea::NFmiLambertEqualArea(double theRadialRangeInMeters,
                                           const NFmiPoint &theCenterLatLon,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY)
    : NFmiAzimuthalArea(theRadialRangeInMeters, theCenterLatLon, theTopLeftXY, theBottomRightXY)
{
  // Muodostaa projektioalueen rajaamalla "world-xy"-tasossa 'theRadialRangeInMeters'-säteiselle
  // ympyrälle
  // "bounding-boxin", jonka keskipisteenä on maantiet. piste 'theCenterLatLon'

  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);
  NFmiPoint centerWorldXY = LatLonToWorldXY(NFmiPoint(theCenterLatLon.X(), theCenterLatLon.Y()));
  itsBottomLeftWorldXY = NFmiPoint(centerWorldXY.X() - theRadialRangeInMeters,
                                   centerWorldXY.Y() - theRadialRangeInMeters);
  itsWorldRect = NFmiRect(
      itsBottomLeftWorldXY,
      itsBottomLeftWorldXY + NFmiPoint(2 * theRadialRangeInMeters, 2 * theRadialRangeInMeters));

  Init(true);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theWidthInMeters Undocumented
 * \param theHeightInMeters Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea::NFmiLambertEqualArea(const NFmiPoint &theBottomLeftLatLon,
                                           const double theWidthInMeters,
                                           const double theHeightInMeters,
                                           const double theCentralLongitude,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY,
                                           const double theCentralLatitude,
                                           const double theTrueLatitude)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

  itsBottomLeftWorldXY = LatLonToWorldXY(theBottomLeftLatLon);
  itsWorldRect = NFmiRect(itsBottomLeftWorldXY,
                          itsBottomLeftWorldXY + NFmiPoint(theWidthInMeters, theHeightInMeters));

  Init(true);
  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla tässä,
  // mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theRadialRange Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea::NFmiLambertEqualArea(const double theRadialRange,
                                           const double theCentralLongitude,
                                           const NFmiPoint &theTopLeftXY,
                                           const NFmiPoint &theBottomRightXY,
                                           const double theCentralLatitude,
                                           const double theTrueLatitude)
    : NFmiAzimuthalArea(theRadialRange,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  // Purpose: to create a square bounding the circle of radius theRadialRange

  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

  itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange, -itsRadialRange),
                          NFmiPoint(itsRadialRange, itsRadialRange));
  NFmiAzimuthalArea::Init(true);

  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla
  // tässä, mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLambertEqualArea::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
  {
    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
  }
  NFmiAzimuthalArea::Init();
  const char *fmt = "+proj=laea +lat_0={} +lon_0={} +lat_ts={} +R={} +units=m +no_defs +type=crs";
  itsProjStr = fmt::format(fmt,
                           itsCentralLatitude.Value(),
                           itsCentralLongitude.Value(),
                           itsTrueLatitude.Value(),
                           kRearth);
  itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
}

// ----------------------------------------------------------------------
/*!
 * \param delta Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLambertEqualArea::K(const double delta) const
{
  //  delta = sinlat0*sinlat + coslat0*coslat*cos(dlon);

  /*
double D;

D = DistanceFromPerspectivePointToCenterOfEarth();
if((D + (kRearth*delta)) != 0)
  return kRearth*(D + kRearth)/(D + (kRearth*delta));
else
  return kFloatMissing;
  */
  // double	  itsCentralLongitude;
  // NFmiAngle itsCentralLatitude;

  //	return sqrt(2./(1. + sin(lat0)*sin(lat) + cos(lat0)*cos(lat)*cos(lon-lon0));

  if (delta <= -1.0)
    return kFloatMissing;

  return kRearth * sqrt(2. / (1. + delta));
}

// ----------------------------------------------------------------------
/*!
 * \param xyDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLambertEqualArea::CalcDelta(const double xyDistance) const
{
  // Calculates the delta angle for LambertEqual projection.
  // See details in ref. [2] p. 13.

  return 2.0 * asin(FmiMax(-1.0, FmiMin(1.0, xyDistance / (2 * kRearth))));
  // 11.5.98/EL: delta is always computed for tangential plane only --> itsTrueLatitude.Sin() == 1
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLambertEqualArea::DistanceFromPerspectivePointToCenterOfEarth() const
{
  // Distance (in world-coordinate meters) for LambertEqual projection.
  // See details in ref. [2] p. 13.

  return kRearth;
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertEqualArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                        const NFmiPoint &theTopRightLatLon,
                                        bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiLambertEqualArea(fixedPointData.itsBottomLeftLatlon,
                                    fixedPointData.itsTopRightLatlon,
                                    itsCentralLongitude.Value(),
                                    TopLeft(),
                                    BottomRight(),
                                    itsCentralLatitude.Value(),
                                    itsTrueLatitude.Value(),
                                    fixedPointData.fIsPacific);
  }
  else
    return new NFmiLambertEqualArea(theBottomLeftLatLon,
                                    theTopRightLatLon,
                                    itsCentralLongitude.Value(),
                                    TopLeft(),
                                    BottomRight(),
                                    itsCentralLatitude.Value(),
                                    itsTrueLatitude.Value(),
                                    PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertEqualArea::Clone() const
{
  return new NFmiLambertEqualArea(*this);
}
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \todo Since NFmiAzimuthalArea::operator= is unknown, must protect
 *       from self assignment
 */
// ----------------------------------------------------------------------

NFmiLambertEqualArea &NFmiLambertEqualArea::operator=(const NFmiLambertEqualArea &theArea) =
    default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertEqualArea::operator==(const NFmiLambertEqualArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(static_cast<const NFmiAzimuthalArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertEqualArea::operator!=(const NFmiLambertEqualArea &theArea) const
{
  return !(*this == theArea);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertEqualArea::operator==(const NFmiArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertEqualArea::operator!=(const NFmiArea &theArea) const
{
  return !(*this == theArea);
}
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiLambertEqualArea::Write(std::ostream &file) const
{
  NFmiAzimuthalArea::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLambertEqualArea::Read(std::istream &file)
{
  NFmiAzimuthalArea::Read(file);
  return file;
}

NFmiArea *NFmiLambertEqualArea::CreateNewArea(const NFmiRect &theRect) const
{
  NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
  NFmiPoint topRight(ToLatLon(theRect.TopRight()));
  NFmiArea *area = new NFmiLambertEqualArea(
      bottomLeft, topRight, itsCentralLongitude.Value(), TopLeft(), BottomRight());
  return area;
}

const std::string NFmiLambertEqualArea::AreaStr() const
{
  std::ostringstream out;
  out << "lambertequal," << CentralLongitude() << ',' << CentralLatitude() << ','
      << itsTrueLatitude.Value() << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y()
      << ',' << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------

const std::string NFmiLambertEqualArea::WKT() const
{
  if (itsCentralLatitude.Value() != 90)
  {
    const char *fmt = R"(PROJCS["FMI_LambertEqual",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Lambert_Azimuthal_Equal_Area"],)"
                      R"(PARAMETER["latitude_of_origin",{}],)"
                      R"(PARAMETER["central_meridian",{}],)"
                      R"(UNIT["Metre",1.0]])";
    return fmt::format(fmt, kRearth, itsCentralLatitude.Value(), itsCentralLongitude.Value());
  }
  else
  {
    const char *fmt = R"(PROJCS["FMI_LambertEqual",)"
                      R"(GEOGCS["FMI_Sphere",)"
                      R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",{:.0f},0]],)"
                      R"(PRIMEM["Greenwich",0],)"
                      R"(UNIT["Degree",0.0174532925199433]],)"
                      R"(PROJECTION["Lambert_Azimuthal_Equal_Area"],)"
                      R"(PARAMETER["latitude_of_origin",{}],)"
                      R"(PARAMETER["central_meridian",{}],)"
                      R"(UNIT["Metre",1.0]])";
    return fmt::format(fmt, kRearth, itsTrueLatitude.Value(), itsCentralLongitude.Value());
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiLambertEqualArea::HashValue() const
{
  std::size_t hash = NFmiAzimuthalArea::HashValue();
  // no private members
  return hash;
}
// ======================================================================
