#include "NFmiLambertConformalConicArea.h"
#include "NFmiStringTools.h"
#include <boost/functional/hash.hpp>
#include <fmt/format.h>
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Constructor
 */
// ----------------------------------------------------------------------

NFmiLambertConformalConic::NFmiLambertConformalConic(const NFmiPoint &theBottomLeftLatLon,
                                                     const NFmiPoint &theTopRightLatLon,
                                                     double theCentralLongitude,
                                                     double theTrueLatitude1,
                                                     double theTrueLatitude2,
                                                     double theRadius,
                                                     const NFmiPoint &theTopLeftXY,
                                                     const NFmiPoint &theBottomRightXY,
                                                     bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsCentralLatitude(theCentralLongitude),
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

void NFmiLambertConformalConic::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
  {
    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertConformalConic::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                             const NFmiPoint &theTopRightLatLon,
                                             bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiLambertConformalConic(fixedPointData.itsBottomLeftLatlon,
                                         fixedPointData.itsTopRightLatlon,
                                         itsCentralLongitude,
                                         itsTrueLatitude1,
                                         itsTrueLatitude2,
                                         itsRadius,
                                         TopLeft(),
                                         BottomRight(),
                                         fixedPointData.fIsPacific);
  }

  return new NFmiLambertConformalConic(theBottomLeftLatLon,
                                       theTopRightLatLon,
                                       itsCentralLongitude,
                                       itsTrueLatitude1,
                                       itsTrueLatitude2,
                                       itsRadius,
                                       TopLeft(),
                                       BottomRight(),
                                       PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLambertConformalConic::Clone() const { return new NFmiLambertConformalConic(*this); }

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiLambertConformalConic::operator==(const NFmiLambertConformalConic &theArea) const
{
  if ((itsBottomLeftLatLon == theArea.itsBottomLeftLatLon) &&
      (itsTopRightLatLon == theArea.itsTopRightLatLon) &&
      (itsCentralLongitude == theArea.itsCentralLongitude) &&
      (itsTrueLatitude1 == theArea.itsTrueLatitude1) &&
      (itsTrueLatitude2 == theArea.itsTrueLatitude2) && (itsRadius = theArea.itsRadius) &&
      (itsBottomLeftWorldXY == theArea.itsBottomLeftWorldXY) &&
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

bool NFmiLambertConformalConic::operator!=(const NFmiLambertConformalConic &theArea) const
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

bool NFmiLambertConformalConic::operator==(const NFmiArea &theArea) const
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

bool NFmiLambertConformalConic::operator!=(const NFmiArea &theArea) const
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

std::ostream &NFmiLambertConformalConic::Write(std::ostream &file) const
{
  NFmiArea::Write(file);

  file << itsBottomLeftLatLon << itsTopRightLatLon << itsCentralLongitude << endl
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

std::istream &NFmiLambertConformalConic::Read(std::istream &file)
{
  NFmiArea::Read(file);

  file >> itsBottomLeftLatLon >> itsTopRightLatLon;
  PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));
  file >> itsCentralLongitude >> itsTrueLatitude1 >> itsTrueLatitude2 >> itsRadius >> itsWorldRect;
  Init();

  return file;
}

NFmiArea *NFmiLambertConformalConic::CreateNewArea(const NFmiRect &theRect) const
{
  NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
  NFmiPoint topRight(ToLatLon(theRect.TopRight()));
  NFmiArea *area = new NFmiLambertConformalConic(bottomLeft,
                                                 topRight,
                                                 itsCentralLongitude,
                                                 itsTrueLatitude1,
                                                 itsTrueLatitude2,
                                                 itsRadius,
                                                 TopLeft(),
                                                 BottomRight());
  return area;
}

const std::string NFmiLambertConformalConic::AreaStr() const
{
  std::ostringstream out;
  out << "lcc," << itsCentralLongitude << ',' << itsTrueLatitude1;
  if (itsTrueLatitude2 != itsTrueLatitude2 || itsRadius != kRearth)
    out << ',' << itsTrueLatitude2 << ',' << itsRadius;
  out << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
      << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------

const std::string NFmiLambertConformalConic::WKT() const { return "shit"; }

const NFmiPoint NFmiLambertConformalConicArea::LatLonToWorldXY(
    const NFmiPoint &theLatLonPoint) const
{
}

const NFmiPoint NFmiLambertConformalConicArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const {}

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

std::size_t NFmiLambertConformalConic::HashValue() const
{
  std::size_t hash = NFmiArea::HashValue();
  boost::hash_combine(hash, itsBottomLeftLatLon.HashValue());
  boost::hash_combine(hash, itsTopRightLatLon.HashValue());
  boost::hash_combine(hash, boost::hash_value(itsCentralLongitude));
  boost::hash_combine(hash, boost::hash_value(itsTrueLatitude1));
  boost::hash_combine(hash, boost::hash_value(itsTrueLatitude2));
  boost::hash_combine(hash, boost::hash_value(itsRadius));
  return hash;
}
// ======================================================================
