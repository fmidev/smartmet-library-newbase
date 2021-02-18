// ======================================================================
/*!
 * \file NFmiGdalArea.cpp
 * \brief Implementation of class NFmiGdalArea
 */
// ======================================================================

#ifndef DISABLED_GDAL

#include "NFmiGdalArea.h"
#include "NFmiString.h"
#include "NFmiStringTools.h"
#include <boost/functional/hash.hpp>
#include <boost/math/constants/constants.hpp>
#include <gis/CoordinateTransformation.h>
#include <gis/OGR.h>
#include <gis/SpatialReference.h>
#include <cmath>
#include <iomanip>
#include <ogr_spatialref.h>

using namespace std;

// See also NFmiLatLonArea::WKT()
std::string fmiwkt =
    R"(GEOGCS["FMI_Sphere",DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]])";

// ----------------------------------------------------------------------
/*!
 * \brief Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiGdalArea::~NFmiGdalArea() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea() : NFmiArea(), itsDatum("WGS84") {}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const NFmiGdalArea &theArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 */
// ----------------------------------------------------------------------

NFmiGdalArea &NFmiGdalArea::operator=(const NFmiGdalArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const std::string &theDescription,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(theDescription),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect()
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect()
{
  itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theCRS);

  // Guess a good value for itsDescription

  const char *auth = theCRS.GetAuthorityName(nullptr);
  if (auth != nullptr)
  {
    itsDescription = std::string(auth);
    const char *code = theCRS.GetAuthorityCode(nullptr);
    if (code != nullptr)
      itsDescription += ":" + std::string(code);
  }
  else
  {
    itsDescription = Fmi::OGR::exportToWkt(theCRS);
  }

  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from bounding box
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           double theXmin,
                           double theYmin,
                           double theXmax,
                           double theYmax,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsWorldRect(NFmiPoint(theXmin, theYmin), NFmiPoint(theXmax, theYmax))
{
  itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theCRS);

  // Guess a good value for itsDescription

  const char *auth = theCRS.GetAuthorityName(nullptr);
  if (auth != nullptr)
  {
    itsDescription = std::string(auth);
    const char *code = theCRS.GetAuthorityCode(nullptr);
    if (code != nullptr)
      itsDescription += ":" + std::string(code);
  }
  else
  {
    itsDescription = Fmi::OGR::exportToWkt(theCRS);
  }

  // The needed spatial references

  OGRErr err;
  std::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
  if (itsDatum == "FMI")
    err = datum->SetFromUserInput(fmiwkt.c_str());
  else
    err = datum->SetFromUserInput(itsDatum.c_str());

  if (err != OGRERR_NONE)
    throw std::runtime_error("Failed to set datum: '" + itsDatum + "'");

  // The needed coordinate transformations

  itsWorldXYToLatLonTransformation =
      std::make_shared<Fmi::CoordinateTransformation>(*itsSpatialReference, *datum);
  itsLatLonToWorldXYTransformation =
      std::make_shared<Fmi::CoordinateTransformation>(*datum, *itsSpatialReference);

  // Bottom left and top right coordinates - needed only for geographic projections (Width()
  // calculations)
  // The same data could be extracted from the WorldXYRect too though - and these variables
  // would not be needed.

  double x1 = theXmin;
  double y1 = theYmin;
  double x2 = theXmax;
  double y2 = theYmax;
  itsWorldXYToLatLonTransformation->transform(x1, y1);
  itsWorldXYToLatLonTransformation->transform(x2, y2);
  itsBottomLeftLatLon = NFmiPoint(x1, y1);
  itsTopRightLatLon = NFmiPoint(x2, y2);

  // Initialize itsWKT
  itsWKT = Fmi::OGR::exportToWkt(*itsSpatialReference);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return class ID
 */
// ----------------------------------------------------------------------

unsigned long NFmiGdalArea::ClassId() const
{
  return kNFmiGdalArea;
}
// ----------------------------------------------------------------------
/*!
 * \brief Return class name
 */
// ----------------------------------------------------------------------

const char *NFmiGdalArea::ClassName() const
{
  return "kNFmiGdalArea";
}
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::Clone() const
{
  return new NFmiGdalArea(*this);
}
// ----------------------------------------------------------------------
/*!
 * \brief Area descriptor
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::AreaStr() const
{
  std::ostringstream out;
  out << itsDatum << ':' << itsDescription << "|" << BottomLeftLatLon().X() << ","
      << BottomLeftLatLon().Y() << "," << TopRightLatLon().X() << "," << TopRightLatLon().Y();
  return out.str();
}
// ----------------------------------------------------------------------
/*!
 * \brief Datum
 */
// ----------------------------------------------------------------------

const std::string &NFmiGdalArea::Datum() const
{
  return itsDatum;
}
// ----------------------------------------------------------------------
/*!
 * \brief Return the WKT description
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::WKT() const
{
  return itsWKT;
}
// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiGdalArea &theArea) const
{
  return (itsBottomLeftLatLon == theArea.itsBottomLeftLatLon &&
          itsTopRightLatLon == theArea.itsTopRightLatLon && itsWorldRect == theArea.itsWorldRect &&
          itsDescription == theArea.itsDescription && itsDatum == theArea.itsDatum);
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiGdalArea &theArea) const
{
  return !(*this == theArea);
}
// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiArea &theArea) const
{
  return *this == static_cast<const NFmiGdalArea &>(theArea);
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiArea &theArea) const
{
  return !(*this == theArea);
}
// ----------------------------------------------------------------------
/*!
 * \brief Write the projection definition to a file
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGdalArea::Write(std::ostream &file) const
{
  NFmiString tmp1 = itsDatum;
  NFmiString tmp2 = itsDescription;

  NFmiArea::Write(file);
  file << itsBottomLeftLatLon << itsTopRightLatLon << tmp1 << tmp2;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief Read new projection definition from the input stream
 */
// ----------------------------------------------------------------------

std::istream &NFmiGdalArea::Read(std::istream &file)
{
  NFmiString tmp1;
  NFmiString tmp2;

  NFmiArea::Read(file);
  file >> itsBottomLeftLatLon >> itsTopRightLatLon >> tmp1 >> tmp2;

  itsDatum = tmp1.CharPtr();
  itsDescription = tmp2.CharPtr();

  init();

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief XY coordinate to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
  double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

  return WorldXYToLatLon(NFmiPoint(worldx, worldy));
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to XY-coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
  NFmiPoint worldxy = LatLonToWorldXY(latlon);

  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double x = Left() + xscale * (worldxy.X() - itsWorldRect.Left());
  double y = Top() + yscale * (itsWorldRect.Bottom() - worldxy.Y());

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief XY-coordinate to World coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
  double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

  return NFmiPoint(worldx, worldy);
}

// ----------------------------------------------------------------------
/*!
 * \param theWorldXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::WorldXYToXY(const NFmiPoint &theWorldXYPoint) const
{
  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double x = xscale * (theWorldXYPoint.X() - itsWorldRect.Left()) + Left();
  double y = Top() - yscale * (theWorldXYPoint.Y() - itsWorldRect.Bottom());
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief World coordinates to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  if (!itsWorldXYToLatLonTransformation)
    throw std::runtime_error("Trying to use an uninitialized GDAL area");
  double x = theXYPoint.X();
  double y = theXYPoint.Y();
  if (!itsWorldXYToLatLonTransformation->transform(x, y))
    return NFmiPoint(kFloatMissing, kFloatMissing);
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to world coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  if (!itsLatLonToWorldXYTransformation)
    throw std::runtime_error("Trying to use an uninitialized GDAL area");
  double x = theLatLonPoint.X();
  double y = theLatLonPoint.Y();
  if (!itsLatLonToWorldXYTransformation->transform(x, y))
    return NFmiPoint(kFloatMissing, kFloatMissing);
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the world rectangle
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiGdalArea::WorldRect() const
{
  return itsWorldRect;
}
// ----------------------------------------------------------------------
/*!
 * \brief
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                const NFmiPoint &theTopRightLatLon,
                                bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fix = NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiGdalArea(itsDatum,
                            itsDescription,
                            fix.itsBottomLeftLatlon,
                            fix.itsTopRightLatlon,
                            TopLeft(),
                            BottomRight(),
                            fix.fIsPacific);
  }
  else
    return new NFmiGdalArea(itsDatum,
                            itsDescription,
                            theBottomLeftLatLon,
                            theTopRightLatLon,
                            TopLeft(),
                            BottomRight(),
                            PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the projection transformation objects
 */
// ----------------------------------------------------------------------

void NFmiGdalArea::init()
{
  // The needed spatial references

  if (!itsSpatialReference)
    itsSpatialReference = std::make_shared<Fmi::SpatialReference>(itsDescription);

  OGRErr err;
  std::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
  if (itsDatum == "FMI")
    err = datum->SetFromUserInput(fmiwkt.c_str());
  else
    err = datum->SetFromUserInput(itsDatum.c_str());

  if (err != OGRERR_NONE)
    throw std::runtime_error("Failed to set datum: '" + itsDatum + "'");

  // The needed coordinate transformations

  itsWorldXYToLatLonTransformation =
      std::make_shared<Fmi::CoordinateTransformation>(*itsSpatialReference, *datum);
  itsLatLonToWorldXYTransformation =
      std::make_shared<Fmi::CoordinateTransformation>(*datum, *itsSpatialReference);

  // The needed world XY rectangle

  itsWorldRect = NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

  // Initialize itsWKT
  itsWKT = Fmi::OGR::exportToWkt(*itsSpatialReference);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGdalArea::WorldXYWidth() const
{
  if (!itsSpatialReference->isGeographic())
    return WorldRect().Width();
  else
  {
    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlon = itsTopRightLatLon.X() - itsBottomLeftLatLon.X();
    if (dlon < 0)
      dlon += 360;
    double clat = 0.5 * (itsBottomLeftLatLon.Y() + itsTopRightLatLon.Y());
    return dlon / 360 * circumference * cos(clat * pi / 180);
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGdalArea::WorldXYHeight() const
{
  if (!itsSpatialReference->isGeographic())
    return WorldRect().Height();
  else
  {
    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlat = itsTopRightLatLon.Y() - itsBottomLeftLatLon.Y();
    return dlat / 360.0 * circumference;  // angle -> meters
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Hash value
 */
// ----------------------------------------------------------------------

std::size_t NFmiGdalArea::HashValue() const
{
  std::size_t hash = NFmiArea::HashValue();

  // some of these may be redundant:
  boost::hash_combine(hash, boost::hash_value(itsDatum));
  boost::hash_combine(hash, boost::hash_value(itsDescription));
  boost::hash_combine(hash, boost::hash_value(itsWKT));

  boost::hash_combine(hash, itsBottomLeftLatLon.HashValue());
  boost::hash_combine(hash, itsTopRightLatLon.HashValue());
  boost::hash_combine(hash, itsWorldRect.HashValue());

  return hash;
}

#endif  // DISABLED_GDAL

// ======================================================================
