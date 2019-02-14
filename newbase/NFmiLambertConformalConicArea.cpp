#include "NFmiLambertConformalConicArea.h"
#include "NFmiStringTools.h"
#include <boost/functional/hash.hpp>
#include <fmt/format.h>
#include <cmath>

using namespace std;

// For some reason Boost.Constants does not define quart_pi so we'll just use our own definitions
namespace
{
const double pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
const double half_pi = pi / 2;
const double quart_pi = pi / 4;
}  // namespace

// ----------------------------------------------------------------------
/*!
 * Constructor
 */
// ----------------------------------------------------------------------

NFmiLambertConformalConicArea::NFmiLambertConformalConicArea(const NFmiPoint &theBottomLeftLatLon,
                                                             const NFmiPoint &theTopRightLatLon,
                                                             double theCentralLongitude,
                                                             double theCentralLatitude,
                                                             double theTrueLatitude1,
                                                             double theTrueLatitude2,
                                                             double theRadius,
                                                             bool usePacificView,
                                                             const NFmiPoint &theTopLeftXY,
                                                             const NFmiPoint &theBottomRightXY)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsCentralLongitude(theCentralLongitude),
      itsCentralLatitude(theCentralLatitude),
      itsTrueLatitude1(theTrueLatitude1),
      itsTrueLatitude2(theTrueLatitude2),
      itsRadius(theRadius)
{
  Init();
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLambertConformalConicArea::Init(bool fKeepWorldRect)
{
  const double lat1 = FmiRad(itsTrueLatitude1);
  const double lat2 = FmiRad(itsTrueLatitude2);
  const double lat0 = FmiRad(itsCentralLatitude);

  if (itsTrueLatitude1 != itsTrueLatitude2)
    itsN = log(cos(lat1) / cos(lat2)) / log(tan(quart_pi + lat2 / 2) / tan(quart_pi + lat1 / 2));
  else
    itsN = sin(lat1);

  itsF = cos(lat1) * pow(tan(quart_pi + lat1 / 2), itsN) / itsN;
  itsRho0 = itsRadius * itsF / pow(tan(quart_pi + lat0 / 2), itsN);

  if (!fKeepWorldRect)
    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

  itsXScaleFactor = Width() / itsWorldRect.Width();
  itsYScaleFactor = Height() / itsWorldRect.Height();
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertConformalConicArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                                 const NFmiPoint &theTopRightLatLon,
                                                 bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiLambertConformalConicArea(fixedPointData.itsBottomLeftLatlon,
                                             fixedPointData.itsTopRightLatlon,
                                             itsCentralLongitude,
                                             itsCentralLatitude,
                                             itsTrueLatitude1,
                                             itsTrueLatitude2,
                                             itsRadius,
                                             fixedPointData.fIsPacific,
                                             TopLeft(),
                                             BottomRight());
  }

  return new NFmiLambertConformalConicArea(theBottomLeftLatLon,
                                           theTopRightLatLon,
                                           itsCentralLongitude,
                                           itsCentralLatitude,
                                           itsTrueLatitude1,
                                           itsTrueLatitude2,
                                           itsRadius,
                                           PacificView(),
                                           TopLeft(),
                                           BottomRight());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertConformalConicArea::Clone() const
{
  return new NFmiLambertConformalConicArea(*this);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertConformalConicArea::operator==(const NFmiLambertConformalConicArea &theArea) const
{
  if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
      (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
      (itsCentralLongitude == theArea.itsCentralLongitude) &&
      (itsCentralLatitude == theArea.itsCentralLatitude) &&
      (itsTrueLatitude1 == theArea.itsTrueLatitude1) &&
      (itsTrueLatitude2 == theArea.itsTrueLatitude2) && (itsRadius == theArea.itsRadius) &&
      (itsWorldRect == theArea.itsWorldRect))
    return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertConformalConicArea::operator!=(const NFmiLambertConformalConicArea &theArea) const
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

bool NFmiLambertConformalConicArea::operator==(const NFmiArea &theArea) const
{
  return *this == static_cast<const NFmiLambertConformalConicArea &>(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertConformalConicArea::operator!=(const NFmiArea &theArea) const
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

std::ostream &NFmiLambertConformalConicArea::Write(std::ostream &file) const
{
  NFmiArea::Write(file);

  file << itsBottomLeftLatLon << itsTopRightLatLon << endl
       << itsCentralLongitude << ' ' << itsCentralLatitude << endl
       << itsTrueLatitude1 << ' ' << itsTrueLatitude2 << endl
       << itsRadius << endl;

  int oldPrec = file.precision();
  file.precision(15);
  file << itsWorldRect << endl;

  file.precision(oldPrec);

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

std::istream &NFmiLambertConformalConicArea::Read(std::istream &file)
{
  NFmiArea::Read(file);

  file >> itsBottomLeftLatLon >> itsTopRightLatLon;
  PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));
  file >> itsCentralLongitude >> itsCentralLatitude >> itsTrueLatitude1 >> itsTrueLatitude2 >>
      itsRadius >> itsWorldRect;
  Init();

  return file;
}

NFmiArea *NFmiLambertConformalConicArea::CreateNewArea(const NFmiRect &theRect) const
{
  NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
  NFmiPoint topRight(ToLatLon(theRect.TopRight()));
  NFmiArea *area = new NFmiLambertConformalConicArea(bottomLeft,
                                                     topRight,
                                                     itsCentralLongitude,
                                                     itsCentralLatitude,
                                                     itsTrueLatitude1,
                                                     itsTrueLatitude2,
                                                     itsRadius,
                                                     false,
                                                     TopLeft(),
                                                     BottomRight());
  return area;
}

const std::string NFmiLambertConformalConicArea::AreaStr() const
{
  std::ostringstream out;
  out << "lcc," << itsCentralLongitude << ',' << itsCentralLatitude << ',' << itsTrueLatitude1;
  if (itsTrueLatitude2 != itsTrueLatitude2 || itsRadius != kRearth)
    out << ',' << itsTrueLatitude2 << ',' << itsRadius;
  out << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
      << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------

const std::string NFmiLambertConformalConicArea::WKT() const
{
  const char *fmt = R"(PROJCS["FMI_LambertConic",)"
                    R"(GEOGCS["Unknown",)"
                    R"(DATUM["Unknown",SPHEROID["Sphere",{:.0f},0]],)"
                    R"(PRIMEM["Greenwich",0],)"
                    R"(UNIT["Degree",0.0174532925199433]],)"
                    R"(PROJECTION["Lambert_Conformal_Conic_2SP"],)"
                    R"(PARAMETER["latitude_of_origin",{}],)"
                    R"(PARAMETER["central_meridian",{}],)"
                    R"(PARAMETER["standard_parallel_1",{}],)"
                    R"(PARAMETER["standard_parallel_2",{}],)"
                    R"(UNIT["Metre",1.0]])";
  return fmt::format(
      fmt, itsRadius, itsCentralLatitude, itsCentralLongitude, itsTrueLatitude1, itsTrueLatitude2);
}

const NFmiPoint NFmiLambertConformalConicArea::LatLonToWorldXY(
    const NFmiPoint &theLatLonPoint) const
{
  const double lat = FmiRad(theLatLonPoint.Y());

  const double theta = itsN * FmiRad(theLatLonPoint.X() - itsCentralLongitude);
  const double rho = itsRadius * itsF * pow(tan(quart_pi + lat / 2), -itsN);

  const double x = rho * sin(theta);
  const double y = itsRho0 - rho * cos(theta);

  return NFmiPoint(x, y);
}

const NFmiPoint NFmiLambertConformalConicArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  double lambda = 0;
  double phi = 0;

  double x = theXYPoint.X();
  double y = itsRho0 - theXYPoint.Y();

  double rho = std::hypot(x, y);

  if (rho == 0)
  {
    // Either pole
    phi = (itsN > 0 ? half_pi : -half_pi);
  }
  else
  {
    if (itsN < 0)
    {
      rho = -rho;
      x = -x;
      y = -y;
    }
    phi = 2 * atan(pow(itsRadius * itsF / rho, 1 / itsN)) - half_pi;
    lambda = atan2(x, y) / itsN + itsCentralLongitude;
  }

  return NFmiPoint(FmiDeg(lambda), FmiDeg(phi));
}

const NFmiPoint NFmiLambertConformalConicArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  double xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
  double yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

  return NFmiPoint(xWorld, yWorld);
}

const NFmiPoint NFmiLambertConformalConicArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  double xWorld, yWorld;

  // Transform local xy-coordinates into world xy-coordinates (meters).

  xWorld = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
  yWorld = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

  // Transform world xy-coordinates into geodetic coordinates.

  return WorldXYToLatLon(NFmiPoint(xWorld, yWorld));
}

const NFmiPoint NFmiLambertConformalConicArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  double xLocal, yLocal;

  // Transform input geodetic coordinates into world coordinates (meters) on xy-plane.
  NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
  NFmiPoint xyWorld(LatLonToWorldXY(latlon));

  if (xyWorld == NFmiPoint::gMissingLatlon)
  {
    return xyWorld;
  }

  // Finally, transform world xy-coordinates into local xy-coordinates
  xLocal = Left() + itsXScaleFactor * (xyWorld.X() - itsWorldRect.Left());
  yLocal = Top() + itsYScaleFactor * (itsWorldRect.Bottom() - xyWorld.Y());

  return NFmiPoint(xLocal, yLocal);
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiLambertConformalConicArea::HashValue() const
{
  std::size_t hash = NFmiArea::HashValue();
  boost::hash_combine(hash, itsBottomLeftLatLon.HashValue());
  boost::hash_combine(hash, itsTopRightLatLon.HashValue());
  boost::hash_combine(hash, boost::hash_value(itsCentralLongitude));
  boost::hash_combine(hash, boost::hash_value(itsCentralLatitude));
  boost::hash_combine(hash, boost::hash_value(itsTrueLatitude1));
  boost::hash_combine(hash, boost::hash_value(itsTrueLatitude2));
  boost::hash_combine(hash, boost::hash_value(itsRadius));
  return hash;
}
// ======================================================================
