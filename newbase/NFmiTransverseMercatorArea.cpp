// ======================================================================
/*!
 * \file NFmiTransverseMercatorArea.cpp
 * \brief Implementation of class NFmiTransverseMercatorArea
 */
// ======================================================================
/*!
 * \class NFmiTransverseMercatorArea
 *
 * Native ellipsoidal Transverse Mercator NFmiArea. The projection math is
 * delegated to NFmiGaussKruger; this class only maps between geodetic, world
 * (projected meters) and local (relative XY) coordinates and manages the
 * spatial reference. Defaults describe ETRS-TM35FIN (EPSG:3067) on the WGS84
 * ellipsoid. No dependency on NFmiKKJArea / NFmiYKJArea, and no +towgs84.
 */
// ======================================================================

#include "NFmiTransverseMercatorArea.h"

#include <fmt/format.h>
#include <macgyver/Exception.h>
#include <macgyver/Hash.h>
#include <limits>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Void constructor (used by the serialization factory before Read)
 */
// ----------------------------------------------------------------------

NFmiTransverseMercatorArea::NFmiTransverseMercatorArea()
    : NFmiArea(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsCentralMeridian(NFmiGaussKruger::kTM35FIN_CentralMeridian),
      itsScaleFactor(NFmiGaussKruger::kTM35FIN_ScaleFactor),
      itsFalseEasting(NFmiGaussKruger::kTM35FIN_FalseEasting),
      itsFalseNorthing(NFmiGaussKruger::kTM35FIN_FalseNorthing),
      itsEllipsoidA(NFmiGaussKruger::kWGS84_A),
      itsEllipsoidInvF(NFmiGaussKruger::kWGS84_InvF),
      itsProjection()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor from geodetic corners and projection parameters
 */
// ----------------------------------------------------------------------

NFmiTransverseMercatorArea::NFmiTransverseMercatorArea(const NFmiPoint& theBottomLeftLatLon,
                                                       const NFmiPoint& theTopRightLatLon,
                                                       double theCentralMeridian,
                                                       double theScaleFactor,
                                                       double theFalseEasting,
                                                       double theFalseNorthing,
                                                       double theEllipsoidA,
                                                       double theEllipsoidInvF,
                                                       const NFmiPoint& theTopLeftXY,
                                                       const NFmiPoint& theBottomRightXY,
                                                       bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsXScaleFactor(),
      itsYScaleFactor(),
      itsWorldRect(),
      itsCentralMeridian(theCentralMeridian),
      itsScaleFactor(theScaleFactor),
      itsFalseEasting(theFalseEasting),
      itsFalseNorthing(theFalseNorthing),
      itsEllipsoidA(theEllipsoidA),
      itsEllipsoidInvF(theEllipsoidInvF),
      itsProjection()
{
  try
  {
    Init();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the projection, world rectangle, scale factors and SRS
 */
// ----------------------------------------------------------------------

void NFmiTransverseMercatorArea::Init(bool fKeepWorldRect)
{
  try
  {
    itsProjection = NFmiGaussKruger(itsEllipsoidA,
                                    itsEllipsoidInvF,
                                    itsCentralMeridian,
                                    itsScaleFactor,
                                    itsFalseEasting,
                                    itsFalseNorthing);

    if (!fKeepWorldRect)
      itsWorldRect =
          NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

    itsXScaleFactor = Width() / itsWorldRect.Width();
    itsYScaleFactor = Height() / itsWorldRect.Height();

    NFmiArea::Init(fKeepWorldRect);

    const char* fmt =
        "+proj=tmerc +lat_0=0 +lon_0={} +k={} +x_0={} +y_0={} +a={} +rf={} +units=m +no_defs "
        "+type=crs";
    itsProjStr = fmt::format(fmt::runtime(fmt),
                             itsCentralMeridian,
                             itsScaleFactor,
                             itsFalseEasting,
                             itsFalseNorthing,
                             itsEllipsoidA,
                             itsEllipsoidInvF);
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
// Geodetic <-> world (projected meters): delegated to NFmiGaussKruger
// ----------------------------------------------------------------------

const NFmiPoint NFmiTransverseMercatorArea::LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const
{
  try
  {
    return itsProjection.ToProjected(theLatLonPoint);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiTransverseMercatorArea::WorldXYToLatLon(const NFmiPoint& theXYPoint) const
{
  try
  {
    return itsProjection.ToLatLon(theXYPoint);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
// World (projected meters) <-> local (relative XY)
// ----------------------------------------------------------------------

const NFmiPoint NFmiTransverseMercatorArea::XYToWorldXY(const NFmiPoint& theXYPoint) const
{
  try
  {
    double worldX = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
    double worldY = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;
    return NFmiPoint(worldX, worldY);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiTransverseMercatorArea::WorldXYToXY(const NFmiPoint& theWorldXYPoint) const
{
  try
  {
    double x = itsXScaleFactor * (theWorldXYPoint.X() - itsWorldRect.Left()) + Left();
    double y = Top() - itsYScaleFactor * (theWorldXYPoint.Y() - itsWorldRect.Bottom());
    return NFmiPoint(x, y);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
// Geodetic <-> local (relative XY)
// ----------------------------------------------------------------------

const NFmiPoint NFmiTransverseMercatorArea::ToXY(const NFmiPoint& theLatLonPoint) const
{
  try
  {
    NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
    NFmiPoint xyWorld(LatLonToWorldXY(latlon));

    double xLocal = Left() + itsXScaleFactor * (xyWorld.X() - itsWorldRect.Left());
    double yLocal = Top() + itsYScaleFactor * (itsWorldRect.Bottom() - xyWorld.Y());
    return NFmiPoint(xLocal, yLocal);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

const NFmiPoint NFmiTransverseMercatorArea::ToLatLon(const NFmiPoint& theXYPoint) const
{
  try
  {
    return WorldXYToLatLon(XYToWorldXY(theXYPoint));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

NFmiArea* NFmiTransverseMercatorArea::NewArea(const NFmiPoint& theBottomLeftLatLon,
                                              const NFmiPoint& theTopRightLatLon,
                                              bool allowPacificFix) const
{
  try
  {
    if (allowPacificFix)
    {
      PacificPointFixerData fixedPointData =
          NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
      return new NFmiTransverseMercatorArea(fixedPointData.itsBottomLeftLatlon,
                                            fixedPointData.itsTopRightLatlon,
                                            itsCentralMeridian,
                                            itsScaleFactor,
                                            itsFalseEasting,
                                            itsFalseNorthing,
                                            itsEllipsoidA,
                                            itsEllipsoidInvF,
                                            TopLeft(),
                                            BottomRight(),
                                            fixedPointData.fIsPacific);
    }

    return new NFmiTransverseMercatorArea(theBottomLeftLatLon,
                                          theTopRightLatLon,
                                          itsCentralMeridian,
                                          itsScaleFactor,
                                          itsFalseEasting,
                                          itsFalseNorthing,
                                          itsEllipsoidA,
                                          itsEllipsoidInvF,
                                          TopLeft(),
                                          BottomRight(),
                                          PacificView());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

NFmiArea* NFmiTransverseMercatorArea::Clone() const
{
  try
  {
    return new NFmiTransverseMercatorArea(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

const std::string NFmiTransverseMercatorArea::AreaStr() const
{
  try
  {
    std::ostringstream out;
    out << "tmerc," << itsCentralMeridian << ',' << itsScaleFactor << ',' << itsFalseEasting << ','
        << itsFalseNorthing << ',' << itsEllipsoidA << ',' << itsEllipsoidInvF << ':'
        << itsBottomLeftLatLon.X() << ',' << itsBottomLeftLatLon.Y() << ',' << itsTopRightLatLon.X()
        << ',' << itsTopRightLatLon.Y();
    return out.str();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

const std::string NFmiTransverseMercatorArea::WKT() const
{
  try
  {
    return SpatialReference().WKT();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

bool NFmiTransverseMercatorArea::operator==(const NFmiTransverseMercatorArea& theArea) const
{
  try
  {
    return itsBottomLeftLatLon == theArea.itsBottomLeftLatLon &&
           itsTopRightLatLon == theArea.itsTopRightLatLon &&
           itsXScaleFactor == theArea.itsXScaleFactor &&
           itsYScaleFactor == theArea.itsYScaleFactor && itsWorldRect == theArea.itsWorldRect &&
           itsCentralMeridian == theArea.itsCentralMeridian &&
           itsScaleFactor == theArea.itsScaleFactor && itsFalseEasting == theArea.itsFalseEasting &&
           itsFalseNorthing == theArea.itsFalseNorthing && itsEllipsoidA == theArea.itsEllipsoidA &&
           itsEllipsoidInvF == theArea.itsEllipsoidInvF;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTransverseMercatorArea::operator!=(const NFmiTransverseMercatorArea& theArea) const
{
  return !(*this == theArea);
}

bool NFmiTransverseMercatorArea::operator==(const NFmiArea& theArea) const
{
  try
  {
    return *this == static_cast<const NFmiTransverseMercatorArea&>(theArea);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiTransverseMercatorArea::operator!=(const NFmiArea& theArea) const
{
  return !(*this == theArea);
}

// ----------------------------------------------------------------------

std::ostream& NFmiTransverseMercatorArea::Write(std::ostream& file) const
{
  try
  {
    // Write with full double precision so the area round-trips losslessly even
    // on a stream that has not had its precision raised by the caller.
    const auto oldprecision = file.precision(std::numeric_limits<double>::max_digits10);

    NFmiArea::Write(file);
    file << itsBottomLeftLatLon;
    file << itsTopRightLatLon;
    file << itsCentralMeridian << " " << itsScaleFactor << " " << itsFalseEasting << " "
         << itsFalseNorthing << " " << itsEllipsoidA << " " << itsEllipsoidInvF << "\n";
    file << itsXScaleFactor << " " << itsYScaleFactor << "\n";
    file << itsWorldRect;

    file.precision(oldprecision);
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

std::istream& NFmiTransverseMercatorArea::Read(std::istream& file)
{
  try
  {
    NFmiArea::Read(file);
    file >> itsBottomLeftLatLon;
    file >> itsTopRightLatLon;
    PacificView(NFmiArea::IsPacificView(itsBottomLeftLatLon, itsTopRightLatLon));

    file >> itsCentralMeridian >> itsScaleFactor >> itsFalseEasting >> itsFalseNorthing >>
        itsEllipsoidA >> itsEllipsoidInvF;

    file >> itsXScaleFactor >> itsYScaleFactor;
    file >> itsWorldRect;

    Init(true);

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------

std::size_t NFmiTransverseMercatorArea::HashValue() const
{
  try
  {
    std::size_t hash = NFmiArea::HashValue();
    Fmi::hash_combine(hash, itsBottomLeftLatLon.HashValue());
    Fmi::hash_combine(hash, itsTopRightLatLon.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsXScaleFactor));
    Fmi::hash_combine(hash, Fmi::hash_value(itsYScaleFactor));
    Fmi::hash_combine(hash, itsWorldRect.HashValue());
    Fmi::hash_combine(hash, Fmi::hash_value(itsCentralMeridian));
    Fmi::hash_combine(hash, Fmi::hash_value(itsScaleFactor));
    Fmi::hash_combine(hash, Fmi::hash_value(itsFalseEasting));
    Fmi::hash_combine(hash, Fmi::hash_value(itsFalseNorthing));
    Fmi::hash_combine(hash, Fmi::hash_value(itsEllipsoidA));
    Fmi::hash_combine(hash, Fmi::hash_value(itsEllipsoidInvF));
    return hash;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
