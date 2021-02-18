// ======================================================================
/*!
 * \file NFmiLatLonArea.cpp
 * \brief Implementation of class NFmiLatLonArea
 */
// ======================================================================
/*!
 * \class NFmiLatLonArea
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiLatLonArea.h"
#include "NFmiAreaFactory.h"
#include <boost/functional/hash.hpp>
#include <fmt/format.h>

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::~NFmiLatLonArea() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiLatLonArea::NFmiLatLonArea()
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

NFmiLatLonArea::NFmiLatLonArea(const NFmiPoint &theBottomLeftLatLon,
                               const NFmiPoint &theTopRightLatLon,
                               const NFmiPoint &theTopLeftXY,
                               const NFmiPoint &theBottomRightXY,
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

NFmiLatLonArea::NFmiLatLonArea(const NFmiLatLonArea &theLatLonArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLatLonArea::Clone() const
{
  return new NFmiLatLonArea(*this);
}
// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiLatLonArea::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon),
                 LatLonToWorldXY(itsTopRightLatLon));  // 28.8.2001/Marko&Esa Lisätty laskuihin.

  itsXScaleFactor = (Right() - Left()) / (itsTopRightLatLon.X() - itsBottomLeftLatLon.X());
  itsYScaleFactor = (Top() - Bottom()) / (itsTopRightLatLon.Y() - itsBottomLeftLatLon.Y());

  NFmiArea::Init(fKeepWorldRect);

  const char *fmt = "+proj=eqc +R={} +wktext +no_defs +type=crs";
  itsProjStr = fmt::format(fmt, kRearth);
  itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsProjStr);
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  NFmiLongitude lon(itsBottomLeftLatLon.X() + (theXYPoint.X() - Left()) / itsXScaleFactor,
                    PacificView());
  NFmiLatitude lat(itsBottomLeftLatLon.Y() + (theXYPoint.Y() - Bottom()) / itsYScaleFactor);
  return NFmiPoint(lon.Value(), lat.Value());
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  double usedLongitude = FixLongitude(theLatLonPoint.X());
  double X = Left() + (usedLongitude - itsBottomLeftLatLon.X()) * itsXScaleFactor;
  double Y = Top() + (theLatLonPoint.Y() - itsTopRightLatLon.Y()) *
                         itsYScaleFactor;  // Tässä on edelleen virhe
  return NFmiPoint(X, Y);
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  return LatLonToWorldXY(ToLatLon(theXYPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiLatLonArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
{
  // TODO: Fix slow implementation
  return ToXY(WorldXYToLatLon(theWorldXYPoint));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLatLonArea::XScale() const
{
  return 1. / itsXScaleFactor;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiLatLonArea::YScale() const
{
  return 1. / itsYScaleFactor;
}
// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiLatLonArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon,
                                  bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiLatLonArea(fixedPointData.itsBottomLeftLatlon,
                              fixedPointData.itsTopRightLatlon,
                              TopLeft(),
                              BottomRight(),
                              fixedPointData.fIsPacific);
  }
  else
    return new NFmiLatLonArea(
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

std::ostream &NFmiLatLonArea::Write(std::ostream &file) const
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
 * Read new object contents from the given stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiLatLonArea::Read(std::istream &file)
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

  // 28.8.2001/Marko&Esa Tätä kutsuttaessa kaikki dataosat päivittyvät,
  // jolloin voidaan esim. editoida tiedostoissa olevia areoiden projektio
  // parametreja siten (tätä ei voitu tehdä ennen koska skaalauskertoimet eivät päivittyneet)

  return file;
}

const std::string NFmiLatLonArea::AreaStr() const
{
  std::ostringstream out;
  out << "latlon:" << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ','
      << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  UNIT["Metre",1.0]
 */
// ----------------------------------------------------------------------

const std::string NFmiLatLonArea::WKT() const
{
  const char *fmt =
      R"(PROJCS["unknown",GEOGCS["FMI_Sphere",DATUM["FMI_2007",SPHEROID["FMI_Sphere",{},0]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],PROJECTION["Equirectangular"],PARAMETER["standard_parallel_1",0],PARAMETER["central_meridian",0],PARAMETER["false_easting",0],PARAMETER["false_northing",0],UNIT["metre",1, AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]])";

  return fmt::format(fmt, kRearth);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison with an area derived from NFmiLatLonArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiLatLonArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiLatLonArea::operator==(const NFmiArea &theArea) const
{
  return *this == static_cast<const NFmiLatLonArea &>(theArea);
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

bool NFmiLatLonArea::operator==(const NFmiLatLonArea &theArea) const
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

std::size_t NFmiLatLonArea::HashValue() const
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
