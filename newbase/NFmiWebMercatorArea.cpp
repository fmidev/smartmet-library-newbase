// ======================================================================
/*!
 * \file NFmiWebMercatorArea.cpp
 * \brief Implementation of class NFmiWebMercatorArea
 */
// ======================================================================
/*!
 * \class NFmiWebMercatorArea
 *
 * Provides for equidistance cylindrical projection. Maps geodetic
 * coordinates (in degrees) to rectangular mercator xy coordinates
 * (in meters) and vice versa.
 *
 * Projection is based on EPSG:3857
 */
// ======================================================================

#include "NFmiWebMercatorArea.h"
#include <boost/functional/hash.hpp>
#include <fmt/format.h>
#include <limits>

using namespace std;

const double kSemiAxis = 6378137.0;

// ----------------------------------------------------------------------
/*!
 * Default constructor
 */
// ----------------------------------------------------------------------

NFmiWebMercatorArea::NFmiWebMercatorArea()
    : NFmiArea(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiWebMercatorArea::NFmiWebMercatorArea(const NFmiPoint& theBottomLeftLatLon,
                                         const NFmiPoint& theTopRightLatLon,
                                         const NFmiPoint& theTopLeftXY,
                                         const NFmiPoint& theBottomRightXY,
                                         bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect()
{
  Init();
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theLatLonArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiWebMercatorArea::NFmiWebMercatorArea(const NFmiWebMercatorArea& theLatLonArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea* NFmiWebMercatorArea::Clone() const { return new NFmiWebMercatorArea(*this); }
// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiWebMercatorArea::Init(bool fKeepWorldRect)
{
  if (itsTopRightLatLon.X() < itsBottomLeftLatLon.X()) itsTopRightLatLon += NFmiPoint(360., 0.);

  if (!fKeepWorldRect)
    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

  itsXScaleFactor = Width() / itsWorldRect.Width();
  itsYScaleFactor = Height() / itsWorldRect.Height();

  NFmiArea::Init(fKeepWorldRect);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiWebMercatorArea::LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const
{
  // Limit Y-values to prevent infinity

  double y = std::max(std::min(theLatLonPoint.Y(), 89.9999), -89.9999);

  return NFmiPoint(kSemiAxis * FmiRad(theLatLonPoint.X()),
                   kSemiAxis * log(tan(FmiRad(45. + 0.5 * y))));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiWebMercatorArea::ToXY(const NFmiPoint& theLatLonPoint) const
{
  // Transforms input geodetic coordinates (longitude,latitude) into local (relative)
  // coordinates on xy-plane.
  double xLocal, yLocal;

  // Transform input geodetic coordinates into world coordinates (meters) on xy-plane.
  NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
  NFmiPoint xyWorld(LatLonToWorldXY(latlon));

  // Finally, transform world xy-coordinates into local xy-coordinates
  xLocal = Left() + itsXScaleFactor * (xyWorld.X() - itsWorldRect.Left());
  yLocal = Top() + itsYScaleFactor * (itsWorldRect.Bottom() - xyWorld.Y());

  return NFmiPoint(xLocal, yLocal);
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiWebMercatorArea::WorldXYToLatLon(const NFmiPoint& theXYPoint) const
{
  // Computes the geodetic coordinates (in degrees) from the input (metric) world xy coordinates

  double worldY = theXYPoint.Y();
  double lon = NFmiLongitude(FmiDeg(theXYPoint.X() / kSemiAxis), PacificView()).Value();
  double lat = FmiDeg(2.0 * atan(exp(worldY / kSemiAxis)) - 0.5 * kPii);

  return NFmiPoint(lon, lat);
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiWebMercatorArea::XYToWorldXY(const NFmiPoint& theXYPoint) const
{
  return LatLonToWorldXY(ToLatLon(theXYPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiWebMercatorArea::ToLatLon(const NFmiPoint& theXYPoint) const
{
  // Transforms input local xy-coordinates into geodetic coordinates
  // (longitude,latitude) on globe.

  double xWorld, yWorld;

  // Transform local xy-coordinates into world xy-coordinates (meters).
  xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
  yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

  // Transform world xy-coordinates into geodetic coordinates.
  return WorldXYToLatLon(NFmiPoint(xWorld, yWorld));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWebMercatorArea::XScale() const { return 1. / itsXScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiWebMercatorArea::YScale() const { return 1. / itsYScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea* NFmiWebMercatorArea::NewArea(const NFmiPoint& theBottomLeftLatLon,
                                       const NFmiPoint& theTopRightLatLon,
                                       bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiWebMercatorArea(fixedPointData.itsBottomLeftLatlon,
                                   fixedPointData.itsTopRightLatlon,
                                   TopLeft(),
                                   BottomRight(),
                                   fixedPointData.fIsPacific);
  }
  else
    return new NFmiWebMercatorArea(
        theBottomLeftLatLon, theTopRightLatLon, TopLeft(), BottomRight(), PacificView());
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiWebMercatorArea::Write(std::ostream& file) const
{
  NFmiArea::Write(file);
  file << itsBottomLeftLatLon;
  file << itsTopRightLatLon;

  // Dummies to replace old removed variables
  file << "0 0\n0 0\n";

  file << itsXScaleFactor << " ";
  file << itsYScaleFactor << std::endl;
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

std::istream& NFmiWebMercatorArea::Read(std::istream& file)
{
  double dummy;

  NFmiArea::Read(file);
  file >> itsBottomLeftLatLon;
  file >> itsTopRightLatLon;
  PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));

  file >> dummy >> dummy >> dummy >> dummy;  // old removed variables

  file >> itsXScaleFactor;
  file >> itsYScaleFactor;

  itsWorldRect = NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

  Init();

  return file;
}

const std::string NFmiWebMercatorArea::AreaStr() const
{
  std::ostringstream out;
  out << "webmercator:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
      << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *
 * We shall return EPSG:3857, but may have to return the following if using the plain
 * EPSG does not work for all software
 *
 * PROJCS["WGS 84 / Pseudo-Mercator",
 *    GEOGCS["WGS 84",
 *        DATUM["WGS_1984",
 *            SPHEROID["WGS 84",6378137,298.257223563,
 *                AUTHORITY["EPSG","7030"]],
 *            AUTHORITY["EPSG","6326"]],
 *        PRIMEM["Greenwich",0,
 *            AUTHORITY["EPSG","8901"]],
 *        UNIT["degree",0.0174532925199433,
 *            AUTHORITY["EPSG","9122"]],
 *        AUTHORITY["EPSG","4326"]],
 *    PROJECTION["Mercator_1SP"],
 *    PARAMETER["central_meridian",0],
 *    PARAMETER["scale_factor",1],
 *    PARAMETER["false_easting",0],
 *    PARAMETER["false_northing",0],
 *    UNIT["metre",1,
 *        AUTHORITY["EPSG","9001"]],
 *    AXIS["X",EAST],
 *    AXIS["Y",NORTH],
 *    EXTENSION["PROJ4","+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0
 * +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"], AUTHORITY["EPSG","3857"]]
 *
 */
// ----------------------------------------------------------------------

const std::string NFmiWebMercatorArea::WKT() const { return "EPSG:3857"; }

// ----------------------------------------------------------------------
/*!
 * Equality comparison with an NFmiWebMercatorArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiWebMercatorArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiWebMercatorArea::operator==(const NFmiArea& theArea) const
{
  return *this == static_cast<const NFmiWebMercatorArea&>(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other area being compared to
 * \return True, if the areas are equivalent
 * \todo Investigate whether NFmiArea::operator== should also be called.
 */
// ----------------------------------------------------------------------

bool NFmiWebMercatorArea::operator==(const NFmiWebMercatorArea& theArea) const
{
  if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
      (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
      (itsXScaleFactor == theArea.itsXScaleFactor) &&
      (itsYScaleFactor == theArea.itsYScaleFactor) && (itsWorldRect == theArea.itsWorldRect))
    return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiWebMercatorArea::HashValue() const
{
  std::size_t hash = NFmiArea::HashValue();
  boost::hash_combine(hash, itsBottomLeftLatLon.HashValue());
  boost::hash_combine(hash, itsTopRightLatLon.HashValue());
  boost::hash_combine(hash, boost::hash_value(itsXScaleFactor));
  boost::hash_combine(hash, boost::hash_value(itsYScaleFactor));
  boost::hash_combine(hash, itsWorldRect.HashValue());
  return hash;
}

// ======================================================================
