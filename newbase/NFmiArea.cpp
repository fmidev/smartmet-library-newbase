#include <iostream>
// ======================================================================
/*!
 * \file NFmiArea.cpp
 * \brief Implementation of class NFmiArea
 */
// ======================================================================
/*!
 * \class NFmiArea
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include "NFmiString.h"
#include "NFmiVersion.h"
#include <boost/functional/hash.hpp>
#include <fmt/printf.h>

std::unique_ptr<OGRSpatialReference> make_sr(const std::string &theSR)
{
  std::unique_ptr<OGRSpatialReference> sr(new OGRSpatialReference);
  auto err = sr->SetFromUserInput(theSR.c_str());
  if (err != OGRERR_NONE)
    throw std::runtime_error("Failed to create spatial reference from '" + theSR + "'");
  return sr;
}

NFmiArea::SpatialReferenceProxy::SpatialReferenceProxy(const char *theSR)
{
  int err = 0;

  std::string sr = theSR;

  if (sr == "FMI")
  {
    // support for legacy FMI sphere from 'FMI' like GDAL supports 'WGS84'
    auto sphere = fmt::format(
        "+proj=longlat +a={:.0f} +b={:.0f} +over +no_defs +towgs84=0,0,0", kRearth, kRearth);
    err = itsSR.SetFromUserInput(sphere.c_str());
  }
  else
    err = itsSR.SetFromUserInput(theSR);

  if (err != OGRERR_NONE)
    throw std::runtime_error("Failed to create spatial reference from '" + sr + "'");
}

NFmiArea::SpatialReferenceProxy::SpatialReferenceProxy(const std::string &theSR)
{
  int err = 0;

  if (theSR == "FMI")
  {
    // support for legacy FMI sphere from 'FMI' like GDAL supports 'WGS84'
    auto sphere = fmt::format(
        "+proj=longlat +a={:.0f} +b={:.0f} +over +no_defs +towgs84=0,0,0", kRearth, kRearth);
    err = itsSR.SetFromUserInput(sphere.c_str());
  }
  else
    err = itsSR.SetFromUserInput(theSR.c_str());

  if (err != OGRERR_NONE)
    throw std::runtime_error("Failed to create spatial reference from '" + theSR + "'");
}

NFmiArea::NFmiArea(int theClassId) : itsClassId(theClassId) {}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopLeft() const { return itsXYRect.TopLeft(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomRight() const { return itsXYRect.BottomRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopRight() const { return itsXYRect.TopRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomLeft() const { return itsXYRect.BottomLeft(); }

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiArea::IsInside(const NFmiPoint &theLatLonPoint) const
{
  NFmiPoint xyPoint = ToXY(theLatLonPoint);
  if (xyPoint == NFmiPoint::gMissingLatlon)
  {
    return false;
  }
  return itsXYRect.IsInside(xyPoint);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopLeftLatLon() const { return ToLatLon(TopLeft()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopRightLatLon() const { return ToLatLon(TopRight()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomLeftLatLon() const { return ToLatLon(BottomLeft()); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomRightLatLon() const { return ToLatLon(BottomRight()); }
// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiArea::IsInside(const NFmiArea &theArea) const
{
  return IsInside(theArea.TopLeftLatLon()) && IsInside(theArea.TopRightLatLon()) &&
         IsInside(theArea.BottomLeftLatLon()) && IsInside(theArea.BottomRightLatLon());
}

// ----------------------------------------------------------------------
/*!
 * \param newPlace Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::Place(const NFmiPoint &newPlace) { itsXYRect.Place(newPlace); }
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::Size(const NFmiPoint &newSize) { itsXYRect.Size(newSize); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Top() const { return itsXYRect.Top(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Bottom() const { return itsXYRect.Bottom(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Left() const { return itsXYRect.Left(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Right() const { return itsXYRect.Right(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Height() const { return itsXYRect.Height(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Width() const { return itsXYRect.Width(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect &NFmiArea::XYArea() const { return itsXYRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::Clone() const { return new NFmiArea(*this); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiArea::ClassId() const { return itsClassId; }

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const char *NFmiArea::ClassName() const
{
  // Legacy FMI sphere projections

  if (itsClassId == kNFmiArea) return "kNFmiArea";

  if (Proj().GetDouble("a") == kRearth && Proj().GetDouble("b") == kRearth)
  {
    if (Proj().GetString("proj") == std::string("eqc")) return "kNFmiLatLonArea";

    if (Proj().GetString("proj") == std::string("ob_tran") &&
        Proj().GetString("o_proj") == std::string("eqc") &&
        Proj().GetString("towgs84") == std::string("0,0,0"))
    {
      auto plat = Proj().GetDouble("o_lat_p");
      auto plon = Proj().GetDouble("o_lon_p");
      if (plat && plon) return "kNFmiRotatedLatLonArea";
    }

    if (Proj().GetString("proj") == std::string("merc")) return "kNFmiMercatorArea";

    if (Proj().GetString("proj") == std::string("stere")) return "kNFmiStereographicArea";

    if (Proj().GetString("proj") == std::string("aeqd")) return "kNFmiEquiDistArea";

    if (Proj().GetString("proj") == std::string("lcc")) return "kNFmiLambertConformalConicArea";

    // Possible FMI sphere but discared for being unknown
  }

  else if (Proj().GetString("ellps") == std::string("intl") &&
           Proj().GetString("proj") == std::string("tmerc") &&
           Proj().GetDouble("x_0") == 3500000.0 && Proj().GetDouble("lat_0") == 0.0 &&
           Proj().GetDouble("lon_0") == 27.0 &&
           Proj().GetString("towgs84") ==
               std::string("-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964"))
    return "kNFmiYKJArea";

  return "kNFmiProjArea";
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiArea::operator==(const NFmiArea &theArea) const
{
  return itsWorldRect == theArea.itsWorldRect;
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiArea::operator!=(const NFmiArea &theArea) const
{
  return itsWorldRect != theArea.itsWorldRect;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

#ifndef WGS84
int NFmiArea::Sign(double theValue) const { return theValue < 0 ? -1 : 1; }
#endif

// ----------------------------------------------------------------------
/*!
 * \param newArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::SetXYArea(const NFmiRect &newArea)
{
  Place(newArea.TopLeft());
  Size(newArea.Size());
#ifndef WGS84
  Init();
#endif
}

// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

NFmiRect NFmiArea::XYArea(const NFmiArea *theArea) const
{
#ifdef WGS84
  // TODO: Why would you need this??? This won't be correct
  NFmiPoint topLeft(ToXY(theArea->ToLatLon(theArea->TopLeft())));
  NFmiPoint bottomRight(ToXY(theArea->ToLatLon(theArea->BottomRight())));
  NFmiRect rect(topLeft, bottomRight);
  return rect;
#else
  if (PacificView() && theArea->PacificView() == false)
  {
    NFmiPoint topLeftLatlon = theArea->ToLatLon(theArea->TopLeft());
    ::FixPacificLongitude(topLeftLatlon);
    NFmiPoint bottomRightLatlon = theArea->ToLatLon(theArea->BottomRight());
    ::FixPacificLongitude(bottomRightLatlon);

    NFmiPoint topLeft(ToXY(topLeftLatlon));
    NFmiPoint bottomRight(ToXY(bottomRightLatlon));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
  else if (PacificView() == false && theArea->PacificView())
  {
    std::unique_ptr<NFmiArea> pacificAreaFromThis(DoForcePacificFix());

    NFmiPoint topLeft(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->TopLeft())));
    NFmiPoint bottomRight(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->BottomRight())));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
  else
  {
    NFmiPoint topLeft(ToXY(theArea->ToLatLon(theArea->TopLeft())));
    NFmiPoint bottomRight(ToXY(theArea->ToLatLon(theArea->BottomRight())));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
#endif
}

// ----------------------------------------------------------------------
/*!
 * \brief Helper method for Write() to keep original corners intact
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopLeftCorner() const
{
  if (itsTopLeftCorner) return *itsTopLeftCorner;

  // Calculate bottom left corner in FMI latlon coordinates

  SpatialReferenceProxy sr("FMI");
  auto mysr = itsSpatialReference;

  std::unique_ptr<OGRCoordinateTransformation> transformation(
      OGRCreateCoordinateTransformation(&mysr, sr.get()));

  if (transformation == nullptr)
    throw std::runtime_error(
        "Failed to create requested coordinate transformation from bbox spatial reference");

  double x1 = itsWorldRect.Left();
  double y1 = itsWorldRect.Top();

  if (transformation->Transform(1, &x1, &y1) == 0)
    throw std::runtime_error("Failed to convert corner coordinate to native latlon");

  return NFmiPoint(x1, y1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Helper method for Write() to keep original corners intact
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomRightCorner() const
{
  if (itsBottomRightCorner) return *itsBottomRightCorner;

  // Calculate top right corner in FMI latlon coordinates

  SpatialReferenceProxy sr("FMI");
  auto mysr = itsSpatialReference;

  std::unique_ptr<OGRCoordinateTransformation> transformation(
      OGRCreateCoordinateTransformation(&mysr, sr.get()));

  if (transformation == nullptr)
    throw std::runtime_error(
        "Failed to create requested coordinate transformation from bbox spatial reference");

  double x1 = itsWorldRect.Right();
  double y1 = itsWorldRect.Bottom();

  if (transformation->Transform(1, &x1, &y1) == 0)
    throw std::runtime_error("Failed to convert corner coordinate to native latlon");

  return NFmiPoint(x1, y1);
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream &NFmiArea::Write(std::ostream &file) const
{
  // Legacy FMI sphere projections

  if (itsClassId == kNFmiArea)
  {
    file << kNFmiArea << " kNFmiArea\n";
    return file;
  }

#if 0
  std::cout << "Class id = " << itsClassId << "\n";
  Proj().Dump(std::cout);
#endif

  // TODO: No idea why latlon and rotlatlon yscalefactor has to be negated to get correct output.
  // Perhaps original calculation is reversed too?

  if (Proj().GetDouble("a") == kRearth && Proj().GetDouble("b") == kRearth)
  {
    if (Proj().GetString("proj") == std::string("eqc"))
    {
      file << kNFmiLatLonArea << " kNFmiLatLonArea\n"
           << itsXYRect << itsWorldRect.TopLeft() << itsWorldRect.BottomRight() << "0 0\n0 0\n"
           << itsXScaleFactor << ' ' << -itsYScaleFactor << '\n';
      return file;
    }

    if (Proj().GetString("proj") == std::string("ob_tran") &&
        Proj().GetString("o_proj") == std::string("eqc") &&
        Proj().GetString("towgs84") == std::string("0,0,0"))
    {
      auto plat = Proj().GetDouble("o_lat_p");
      auto plon = Proj().GetDouble("o_lon_p");
      if (plon && plat)
      {
        // Note: the world rect print order is correct, top left then bottom right
        NFmiPoint southpole(*plon, -(*plat));
        file << kNFmiRotatedLatLonArea << " kNFmiRotatedLatLonArea\n"
             << itsXYRect << itsWorldRect.TopLeft() << itsWorldRect.BottomRight() << "0 0\n0 0\n"
             << itsXScaleFactor << ' ' << -itsYScaleFactor << '\n'
             << southpole;
        return file;
      }
    }

    if (Proj().GetString("proj") == std::string("merc"))
    {
      file << kNFmiMercatorArea << " kNFmiMercatorArea\n"
           << TopLeftCorner() << BottomRightCorner() << "0 0\n0 0\n"
           << itsXScaleFactor << ' ' << itsYScaleFactor << '\n';
      return file;
    }

    if (Proj().GetString("proj") == std::string("stere"))
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");
      auto tlat = Proj().GetDouble("lat_ts");

      if (clon && clat && tlat)
      {
        file << kNFmiStereographicArea << " kNFmiStereographicArea\n"
             << itsXYRect << TopLeftCorner() << BottomRightCorner() << *clon << '\n'
             << *clat << '\n'
             << *tlat << '\n';

        if (FmiInfoVersion >= 5) file << "0 0 0\n";

        int oldPrec = file.precision();
        file.precision(15);
        file << itsWorldRect << ' ';
        file.precision(oldPrec);

        return file;
      }
    }

    if (Proj().GetString("proj") == std::string("aeqd"))
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");

      if (clon && clat)
      {
        // legacy tlat = 90
        file << kNFmiEquiDistArea << " kNFmiEquiDistArea\n"
             << itsXYRect << TopLeftCorner() << BottomRightCorner() << *clon << '\n'
             << *clat << "\n90\n";

        if (FmiInfoVersion >= 5) file << "0 0 0\n";

        int oldPrec = file.precision();
        file.precision(15);
        file << itsWorldRect << ' ';
        file.precision(oldPrec);

        return file;
      }
    }

    if (Proj().GetString("proj") == std::string("lcc"))
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");
      auto lat1 = Proj().GetDouble("lat_1");
      auto lat2 = Proj().GetDouble("lat_2");
      if (clon && clat && lat1 && lat2)
      {
        file << kNFmiLambertConformalConicArea << " kNFmiLambertConformalConicArea\n"
             << itsXYRect << TopLeftCorner() << BottomRightCorner() << *clon << ' ' << *clat << ' '
             << *lat1 << ' ' << *lat2 << ' ' << kRearth << '\n';

        int oldPrec = file.precision();
        file.precision(15);
        file << itsWorldRect << ' ';
        file.precision(oldPrec);

        return file;
      }
    }

    // Possible FMI sphere but discared for being unknown
  }

  else if (Proj().GetString("ellps") == std::string("intl") &&
           Proj().GetString("proj") == std::string("tmerc") &&
           Proj().GetDouble("x_0") == 3500000.0 && Proj().GetDouble("lat_0") == 0.0 &&
           Proj().GetDouble("lon_0") == 27.0 &&
           Proj().GetString("towgs84") ==
               std::string("-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964"))
  {
    file << kNFmiYKJArea << " kNFmiYKJArea\n"
         << itsXYRect << TopLeftCorner() << BottomRightCorner() << "0 0\n0 0\n"
         << itsXScaleFactor << ' ' << itsYScaleFactor << '\n'
         << itsWorldRect;
    return file;
  }

  // modern GDAL/PROJ.4 projections
  NFmiString txt = ProjStr();
  file << kNFmiProjArea << " kNFmiProjArea\n" << itsXYRect << txt << itsWorldRect;
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::istream &NFmiArea::Read(std::istream &file)
{
  NFmiString txt;
  NFmiPoint bottomleft;
  NFmiPoint topright;
  double dummy;

  file >> itsXYRect;

  // FMI legacy sphere
  std::string sphere = "FMI";

  switch (itsClassId)
  {
    case kNFmiArea:
    {
      // empty class
      return file;
    }
    case kNFmiProjArea:
    {
      // Generic PROJ area
      file >> txt >> itsWorldRect;
      InitSpatialReference(txt.CharPtr());
      break;
    }
    case kNFmiLatLonArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;
      auto proj = fmt::format(
          "+proj=eqc +a={:.0f} +b={:.0f} +wktext +over +no_defs +towgs84=0,0,0", kRearth, kRearth);
      *this = *NFmiArea::CreateFromBBox(proj, bottomleft, topright);
      break;
    }
    case kNFmiRotatedLatLonArea:
    {
      NFmiPoint southpole;
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >>
          southpole;

      auto npole_lat = -southpole.Y();  // reflect the pole
      auto npole_lon = southpole.X();   // either this or 360-southpole.X() is correct // TODO

      auto proj = fmt::format(
          "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=eqc +o_lon_p={} +o_lat_p={} "
          "+a={:.0f} +b={:.0f} +wktext +over +towgs84=0,0,0 +no_defs",
          npole_lon,
          npole_lat,
          kRearth,
          kRearth);

      *this = *NFmiArea::CreateFromBBox(proj, bottomleft, topright);
      break;
    }
    case kNFmiStereographicArea:
    {
      double clon, clat, truelat;
      file >> bottomleft >> topright >> clon >> clat >> truelat >> dummy >> dummy >> dummy >>
          itsWorldRect;

      auto proj = fmt::format(
          "+proj=stere +lat_0={} +lat_ts={} +lon_0={} +a={:.0f} +b={:.0f} +units=m +wktext "
          "+towgs84=0,0,0 +no_defs",
          clat,
          truelat,
          clon,
          kRearth,
          kRearth);

      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiEquiDistArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          itsWorldRect;
      auto proj = fmt::format(
          "+proj=aeqd +lat_0={} +lon_0={} +a={:.0f} +b={:.0f} +units=m +wktext +towgs84=0,0,0 "
          "+no_defs",
          clat,
          clon,
          kRearth,
          kRearth);
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiMercatorArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy;
      auto proj =
          fmt::format("+proj=merc +a={:.0f} +b={:.0f} +units=m +wktext +towgs84=0,0,0 +no_defs",
                      kRearth,
                      kRearth);
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiLambertEqualArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          itsWorldRect;
      auto proj = fmt::format(
          "+proj=laea +lat_0={} +lon_0={} +a={:.0f} +b={:.0f} +units=m +wktext +towgs84=0,0,0 "
          "+no_defs",
          clat,
          clon,
          kRearth,
          kRearth);
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiLambertConformalConicArea:
    {
      double clon, clat, tlat1, tlat2, radius;
      file >> bottomleft >> topright >> clon >> clat >> tlat1 >> tlat2 >> radius >> itsWorldRect;
      auto proj = fmt::format(
          "+proj=lcc +lat_0={} +lon_0={} +lat_1={} +lat_2={} +a={:.0f} +b={:.0f} +units=m +wktext "
          "+towgs84=0,0,0 "
          "+no_defs",
          clat,
          clon,
          tlat1,
          tlat2,
          radius,
          radius);
      auto sphere = fmt::format(
          "+proj=longlat +a={:.0f} +b={:.0f} +over +no_defs +towgs84=0,0,0", radius, radius);
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiGnomonicArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          itsWorldRect;
      auto proj = fmt::format(
          "+proj=gnom +lat_0={} +lon_0={} +a={:.0f} +b={:.0f} +units=m +wktext +towgs84=0,0,0 "
          "+no_defs",
          clat,
          clon,
          kRearth,
          kRearth);
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiYKJArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >>
          itsWorldRect;
      std::string proj =
          "+proj=tmerc +lat_0=0 +lon_0=27 +k=1 +x_0=3500000 +y_0=0 +ellps=intl +units=m +wktext "
          "+towgs84=-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964 +no_defs";
      // For legacy reasons corners are in YKJ ellipsoid coordinates
      *this = *NFmiArea::CreateFromCorners(proj, sphere, bottomleft, topright);
      break;
    }
    case kNFmiGdalArea:
      throw std::runtime_error("GDAL areas are no longer supported");
    case kNFmiPKJArea:
      throw std::runtime_error("PKJ projection is no longer supported in Finland");
    case kNFmiKKJArea:
      throw std::runtime_error("KKJ projection is no longer supported in Finland");
    default:
      // kFmiPolSetArea, perhaps some other legacy classes too
      throw std::runtime_error("Projection number " + std::to_string(itsClassId) +
                               " is no longer supported");
  }

  itsProj = NFmiProj(ProjStr());
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYSize() const { return WorldRect().Size(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYPlace() const { return WorldRect().Place(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYWidth() const { return WorldRect().Width(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYHeight() const { return WorldRect().Height(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYAspectRatio() const { return WorldXYWidth() / WorldXYHeight(); }

// ----------------------------------------------------------------------
/*!
 * Measured from the input point, returns the azimuth angle between
 * the true geodetic north direction and the "up" Y-axis of the map
 * area rectangle. Azimuth angle runs 0..360 degrees clockwise, with
 * north zero degrees.
 *
 * \param theLatLonPoint Undocumented
 * \param theLatitudeEpsilon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiAngle NFmiArea::TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                     double theLatitudeEpsilon) const
{
  using namespace std;

  NFmiPoint xyWorldPoint = LatLonToWorldXY(theLatLonPoint);
  NFmiPoint latLonIncr =
      NFmiPoint(0., theLatitudeEpsilon);  // Arbitrary small latitude increment in degrees

  // Move up toward geo-north along the meridian of the input point
  NFmiPoint xyDistanceAlongMeridian = LatLonToWorldXY(theLatLonPoint + latLonIncr) - xyWorldPoint;

  // Get the angle between 'xyDistanceAlongMeridian.X()' and map "up" direction Y-axis
  if (xyDistanceAlongMeridian.Y() == 0.)
    return xyDistanceAlongMeridian.X() > 0.
               ? NFmiAngle(90.)
               : NFmiAngle(
                     270.);  // Azimuth is exactly east 90 degrees or west 270 degrees, respectively

  return NFmiAngle(FmiDeg(atan2(xyDistanceAlongMeridian.X(), xyDistanceAlongMeridian.Y())));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return center latlon coordinate
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::CenterLatLon() const
{
  NFmiPoint bl = BottomLeft();
  NFmiPoint tr = TopRight();

  NFmiPoint center(0.5 * (bl.X() + tr.X()), 0.5 * (bl.Y() + tr.Y()));

  return ToLatLon(center);
}

#ifndef WGS84
bool NFmiArea::IsPacificLongitude(double theLongitude)
{
  if (theLongitude > 180 && theLongitude <= 360)
    return true;
  else
    return false;
}
#endif

#ifndef WGS84
void NFmiArea::CheckForPacificView()
{
  fPacificView = NFmiArea::IsPacificView(BottomLeftLatLon(), TopRightLatLon());
}
#endif

// 1. Tarkistaa kattaako annetut pisteet Atlantic vai Pacific alueen
// 2. Jos Pacific, korjaa longitudet pacific:eiksi
// 3. Palauta originaali/korjatut pisteet pair:issa

#ifndef WGS84
PacificPointFixerData NFmiArea::PacificPointFixer(const NFmiPoint &theBottomLeftLatlon,
                                                  const NFmiPoint &theTopRightLatlon)
{
  bool usePacificView = NFmiArea::IsPacificView(theBottomLeftLatlon, theTopRightLatlon);
  if (usePacificView)
  {
    NFmiPoint bottomLeftLatLon = theBottomLeftLatlon;
    NFmiPoint topRightLatLon = theTopRightLatlon;
    NFmiAreaFactory::DoPossiblePacificFix(bottomLeftLatLon, topRightLatLon, usePacificView);
    return PacificPointFixerData(bottomLeftLatLon, topRightLatLon, usePacificView);
  }
  else
    return PacificPointFixerData(theBottomLeftLatlon, theTopRightLatlon, usePacificView);
}
#endif

#ifndef WGS84
bool NFmiArea::IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon)
{
  // Obvious case
  if (bottomleftLatlon.X() >= 0 && toprightLatlon.X() < 0) return true;
  // 0...360 coordinate system is used
  if (IsPacificLongitude(bottomleftLatlon.X()) || IsPacificLongitude(toprightLatlon.X()))
    return true;
  return false;
}
#endif

#ifndef WGS84
double NFmiArea::FixLongitude(double theLon) const
{
  if (!fPacificView)
  {
    if (theLon > 180.00000001)  // Tämä ei voi olla tasan 180, koska SmartMet maailman rajaviivat
                                // shapessa (maps\\shapes\\ne_10m_admin_0_countries) ja niiden
                                // piirto imagine-kirjastolla menee jossain kohdissa sekaisin
      // reunoilla, koska siellä on käytetty vähän yli 180-pituuspiirin
      // meneviä arvoja Tyynenmeren 180 asteen pituuspiirin reunoilla
      return theLon - 360;
    else
      return theLon;
  }
  else if (theLon < 0)
    return theLon + 360;
  else
    return theLon;
}
#endif

#ifndef WGS84
NFmiArea *NFmiArea::DoPossiblePacificFix() const
{
  // On olemassa pari erikoistapausta, mitkä halutaan eri areoissa korjata, että alueet toimisivat
  // paremmin newbase:ssa.
  if (fPacificView)
  {
    bool usedPacificViewState = fPacificView;
    NFmiPoint bottomleft = BottomLeftLatLon();
    NFmiPoint topright = TopRightLatLon();
    bool createNewArea =
        NFmiAreaFactory::DoPossiblePacificFix(bottomleft, topright, usedPacificViewState);

    if (createNewArea)
    {
      NFmiArea *newArea = NewArea(bottomleft, topright);
      if (newArea)
      {
        newArea->PacificView(usedPacificViewState);
        return newArea;
      }
    }
  }
  return nullptr;
}
#endif

#ifndef WGS84
NFmiArea *NFmiArea::DoForcePacificFix() const
{
  // Joskus on pakko muuttaa atlantic-area pacific tyyppiseksi vaikka väkisin
  if (!fPacificView)
  {
    NFmiPoint bottomleftLatlon = BottomLeftLatLon();
    ::FixPacificLongitude(bottomleftLatlon);
    NFmiPoint toprightLatlon = TopRightLatLon();
    ::FixPacificLongitude(toprightLatlon);

    NFmiArea *newArea = NewArea(bottomleftLatlon, toprightLatlon, false);
    if (newArea)
    {
      newArea->PacificView(true);
      return newArea;
    }
  }
  return nullptr;
}
#endif

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the base NFmiArea components
 */
// ----------------------------------------------------------------------
std::size_t NFmiArea::HashValue() const
{
  std::size_t hash = itsWorldRect.HashValue();
#ifndef WGS84
  boost::hash_combine(hash, boost::hash_value(fPacificView));
#endif
  return hash;
}

// ----------------------------------------------------------------------
/*!
 * \brief Temporary fix until HashValue is made virtual (breaks ABI, delayed for now)
 */
// ----------------------------------------------------------------------

std::size_t NFmiArea::HashValueKludge() const
{
#ifdef WGS84
  return HashValue();
#else

#ifdef UNIX
#ifndef DISABLED_GDAL
  if (const auto *a = dynamic_cast<const NFmiGdalArea *>(this)) return a->HashValue();
#endif
#endif

  // azimuthal is the base class
  if (const auto *a = dynamic_cast<const NFmiEquidistArea *>(this)) return a->HashValue();
  if (const auto *a = dynamic_cast<const NFmiStereographicArea *>(this)) return a->HashValue();
  if (const auto *a = dynamic_cast<const NFmiAzimuthalArea *>(this)) return a->HashValue();

  // kkj is the base class
  if (const auto *a = dynamic_cast<const NFmiYKJArea *>(this)) return a->HashValue();
  if (const auto *a = dynamic_cast<const NFmiKKJArea *>(this)) return a->HashValue();

  // latlon is the base class
  if (const auto *a = dynamic_cast<const NFmiRotatedLatLonArea *>(this)) return a->HashValue();
  if (const auto *a = dynamic_cast<const NFmiLatLonArea *>(this)) return a->HashValue();

  return HashValue();
#endif
}

#ifdef WGS84

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from WGS84 to native metric/geographic coordinates
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::LatLonToWorldXY(const NFmiPoint &theWgs84) const
{
  if (!itsToWorldXYConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  double x = theWgs84.X();
  double y = theWgs84.Y();

  if (itsToWorldXYConverter->Transform(1, &x, &y) == 0) return NFmiPoint::gMissingLatlon;

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from native metric/geographic coordinates to WGS84
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYToLatLon(const NFmiPoint &theWorldXY) const
{
  if (!itsToLatLonConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  double x = theWorldXY.X();
  double y = theWorldXY.Y();

  if (itsToLatLonConverter->Transform(1, &x, &y) == 0) return NFmiPoint::gMissingLatlon;

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from image XY to WGS84
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  // Transform local xy-coordinates into world xy-coordinates (meters).

  auto x = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
  auto y = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;

  // Transform world xy-coordinates into WGS84

  return WorldXYToLatLon(NFmiPoint(x, y));
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from WGS84 to image XY
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  auto worldxy = LatLonToWorldXY(theLatLonPoint);
  if (worldxy == NFmiPoint::gMissingLatlon) return worldxy;

  // Transform world xy-coordinates into local xy-coordinates
  auto x = Left() + itsXScaleFactor * (worldxy.X() - itsWorldRect.Left());
  auto y = Top() + itsYScaleFactor * (itsWorldRect.Bottom() - worldxy.Y());
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from image XY to projection XY
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  auto x = itsWorldRect.Left() + (theXYPoint.X() - Left()) / itsXScaleFactor;
  auto y = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / itsYScaleFactor;
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the spatial reference
 */
// ----------------------------------------------------------------------

void NFmiArea::InitSpatialReference(const std::string &theProjection)
{
  auto err = itsSpatialReference.SetFromUserInput(theProjection.c_str());
  if (err != OGRERR_NONE) throw std::runtime_error("Failed to create spatial reference from WKT");

  InitProj();
}

void NFmiArea::InitProj()
{
  std::unique_ptr<OGRSpatialReference> wgs84(new OGRSpatialReference);
  auto err = wgs84->SetFromUserInput("WGS84");
  if (err != OGRERR_NONE) throw std::runtime_error("Failed to create spatial reference for WGS84");

  itsToWorldXYConverter.reset(OGRCreateCoordinateTransformation(wgs84.get(), &itsSpatialReference));

  if (itsToWorldXYConverter == nullptr)
    throw std::runtime_error("Failed to create coordinate transformation from WGS84");

  itsToLatLonConverter.reset(OGRCreateCoordinateTransformation(&itsSpatialReference, wgs84.get()));

  if (itsToLatLonConverter == nullptr)
    throw std::runtime_error("Failed to create coordinate transformation to WGS84");

  // Init PROJ.4 settings
  itsProj = NFmiProj(ProjStr());

  // Switch writer to ProjArea if we were not reading a legacy projection
  if (itsClassId == kNFmiArea) itsClassId = kNFmiProjArea;
}

void NFmiArea::InitRectConversions()
{
  itsXScaleFactor = itsXYRect.Width() / itsWorldRect.Width();
  itsYScaleFactor = itsXYRect.Height() / itsWorldRect.Height();
}

NFmiArea *NFmiArea::CreateFromBBox(SpatialReferenceProxy theSR,
                                   const NFmiPoint &theBottomLeftWorldXY,
                                   const NFmiPoint &theTopRightWorldXY)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();
    area->itsWorldRect = NFmiRect(theBottomLeftWorldXY.X(),
                                  theTopRightWorldXY.Y(),
                                  theTopRightWorldXY.X(),
                                  theBottomLeftWorldXY.Y());
    area->InitRectConversions();
    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromCorners(SpatialReferenceProxy theSR,
                                      SpatialReferenceProxy theBBoxSR,
                                      const NFmiPoint &theBottomLeftLatLon,
                                      const NFmiPoint &theTopRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();

    std::unique_ptr<OGRCoordinateTransformation> transformation(
        OGRCreateCoordinateTransformation(theBBoxSR.get(), area->SpatialReference()));

    if (transformation == nullptr)
      throw std::runtime_error(
          "Failed to create requested coordinate transformation from bbox spatial reference");

    double x1 = theBottomLeftLatLon.X();
    double y1 = theBottomLeftLatLon.Y();
    double x2 = theTopRightLatLon.X();
    double y2 = theTopRightLatLon.Y();

    if (transformation->Transform(1, &x1, &y1) == 0 || transformation->Transform(1, &x2, &y2) == 0)
      throw std::runtime_error("Failed to initialize projection from BBOX corner coordinates");

    area->itsWorldRect = NFmiRect(x1, y2, x2, y1);
    area->InitRectConversions();

    // save legacy corner coordinates for Write()
    area->itsTopLeftCorner = theBottomLeftLatLon;
    area->itsBottomRightCorner = theTopRightLatLon;

    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromReverseCorners(SpatialReferenceProxy theSR,
                                             SpatialReferenceProxy theBBoxSR,
                                             const NFmiPoint &theTopLeftLatLon,
                                             const NFmiPoint &theBottomRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();

    std::unique_ptr<OGRCoordinateTransformation> transformation(
        OGRCreateCoordinateTransformation(theBBoxSR.get(), area->SpatialReference()));

    if (transformation == nullptr)
      throw std::runtime_error(
          "Failed to create requested coordinate transformation from bbox spatial reference");

    double x1 = theTopLeftLatLon.X();
    double y1 = theTopLeftLatLon.Y();
    double x2 = theBottomRightLatLon.X();
    double y2 = theBottomRightLatLon.Y();

    if (transformation->Transform(1, &x1, &y1) == 0 || transformation->Transform(1, &x2, &y2) == 0)
      throw std::runtime_error("Failed to initialize projection from BBOX corner coordinates");

    area->itsWorldRect = NFmiRect(x1, y1, x2, y2);
    area->InitRectConversions();

    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromWGS84Corners(SpatialReferenceProxy theSR,
                                           const NFmiPoint &theBottomLeftLatLon,
                                           const NFmiPoint &theTopRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();

    double x1 = theBottomLeftLatLon.X();
    double y1 = theBottomLeftLatLon.Y();
    double x2 = theTopRightLatLon.X();
    double y2 = theTopRightLatLon.Y();

    if (area->itsToWorldXYConverter->Transform(1, &x1, &y1) == 0 ||
        area->itsToWorldXYConverter->Transform(1, &x2, &y2) == 0)
      throw std::runtime_error("Failed to initialize projection from WGS84 corner coordinates");

    area->itsWorldRect = NFmiRect(x1, y2, x2, y1);
    area->InitRectConversions();
    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromCornerAndSize(SpatialReferenceProxy theSR,
                                            SpatialReferenceProxy theCornerSR,
                                            const NFmiPoint &theBottomLeftLatLon,
                                            double theWidth,
                                            double theHeight)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();

    std::unique_ptr<OGRCoordinateTransformation> transformation(
        OGRCreateCoordinateTransformation(theCornerSR.get(), theSR.get()));

    if (transformation == nullptr)
      throw std::runtime_error(
          "Failed to create requested coordinate transformation from center coordinate spatial "
          "reference");

    double x = theBottomLeftLatLon.X();
    double y = theBottomLeftLatLon.Y();

    if (transformation->Transform(1, &x, &y) == 0)
      throw std::runtime_error(
          "Failed to initialize projection from bottom left corner coordinates");

    area->itsWorldRect = NFmiRect(x, y + theHeight, x + theWidth, y);
    area->InitRectConversions();
    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromCenter(SpatialReferenceProxy theSR,
                                     SpatialReferenceProxy theCenterSR,
                                     const NFmiPoint &theCenterLatLon,
                                     double theWidth,
                                     double theHeight)
{
  auto area = new NFmiArea;
  try
  {
    area->itsSpatialReference = *theSR;
    area->InitProj();

    std::unique_ptr<OGRCoordinateTransformation> transformation(
        OGRCreateCoordinateTransformation(theCenterSR.get(), theSR.get()));

    if (transformation == nullptr)
      throw std::runtime_error(
          "Failed to create requested coordinate transformation from center coordinate spatial "
          "reference");

    double x = theCenterLatLon.X();
    double y = theCenterLatLon.Y();

    if (transformation->Transform(1, &x, &y) == 0)
      throw std::runtime_error("Failed to initialize projection from center coordinates");

    area->itsWorldRect =
        NFmiRect(x - theWidth / 2, y + theHeight / 2, x + theWidth / 2, y - theHeight / 2);

    area->InitRectConversions();
    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

std::string NFmiArea::WKT() const
{
  char *out;
  itsSpatialReference.exportToWkt(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

std::string NFmiArea::PrettyWKT() const
{
  char *out;
  itsSpatialReference.exportToPrettyWkt(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

std::string NFmiArea::ProjStr() const
{
  char *out;
  itsSpatialReference.exportToProj4(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

NFmiRect NFmiArea::WorldRect() const { return itsWorldRect; }

#ifdef WGS84
NFmiArea *NFmiArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon) const
{
  return CreateNewArea(theBottomLeftLatLon, theTopRightLatLon);
}
#else
NFmiArea *NFmiArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                            const NFmiPoint &theTopRightLatLon,
                            bool allowPacificFix = true) const
{
  // Ignoring Pacific "fix" for now
  return CreateNewArea(theBottomLeftLatLon, theTopRightLatLon);
}
#endif

NFmiArea *NFmiArea::CreateNewArea(const NFmiRect &theRect) const
{
  // Note: We use spherical latlon coordinates for legacy reasons. Use CreateFromCorners directly
  // to use other spatial references.
  return CreateFromCorners(itsSpatialReference, "FMI", theRect.BottomLeft(), theRect.TopRight());
}

NFmiArea *NFmiArea::CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const
{
  // Note: We use spherical latlon coordinates for legacy reasons. Use CreateFromCorners directly
  // to use other spatial references.
  return CreateFromCorners(itsSpatialReference, "FMI", theBottomLeftLatLon, theTopRightLatLon);
}

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect)
{
  NFmiPoint newBottomLeftXY = theWorldRect.BottomLeft();
  NFmiPoint newTopRightXY = theWorldRect.TopRight();

  NFmiPoint newBottomLeftLatLon = WorldXYToLatLon(newBottomLeftXY);
  NFmiPoint newTopRightLatLon = WorldXYToLatLon(newTopRightXY);

  if (!IsInside(newBottomLeftLatLon) || !IsInside(newTopRightLatLon)) return nullptr;

  auto *newArea = CreateFromBBox(itsSpatialReference, newBottomLeftXY, newTopRightXY);

  if (!IsInside(*newArea)) return nullptr;

  return newArea;
}

NFmiArea *NFmiArea::CreateNewArea(double theNewAspectRatioXperY,
                                  FmiDirection theFixedPoint,
                                  bool fShrinkArea)
{
  double originalAspectRatio = WorldXYAspectRatio();

  bool keepWidth;

  if (fShrinkArea)
  {
    // The new area will be "shrunk" to completely fit inside the current area
    if ((theNewAspectRatioXperY < originalAspectRatio))
      keepWidth = false;  // Maintain height, compute width
    else
      keepWidth = true;  // Maintain width, compute height
  }
  else
  {
    // The new area will in part grow out of the current area
    if ((theNewAspectRatioXperY < originalAspectRatio))
      keepWidth = true;  // Maintain width, compute height
    else
      keepWidth = false;  // Maintain height, compute width
  }

  // Create copy of the original "world rectangle" to be freely modified
  NFmiRect newWorldRect = WorldRect();

  // REDIMENSIONING OF THE WORLD RECTANGLE
  //----------------------------------------

  if (!newWorldRect.AdjustAspectRatio(theNewAspectRatioXperY, keepWidth, theFixedPoint))
    return nullptr;

  // Create a new area with the new aspect ratio
  NFmiArea *newArea =
      CreateFromBBox(itsSpatialReference, newWorldRect.BottomLeft(), newWorldRect.TopRight());

  // Return the re-dimensioned copy of the original area
  return newArea;
}

double NFmiArea::XScale() const { return 1 / itsXScaleFactor; }
double NFmiArea::YScale() const { return 1 / itsYScaleFactor; }

NFmiPoint NFmiArea::SphereToWGS84(const NFmiPoint &theLatLon)
{
  SpatialReferenceProxy fmi("FMI");
  SpatialReferenceProxy wgs84("WGS84");

  std::unique_ptr<OGRCoordinateTransformation> transformation(
      OGRCreateCoordinateTransformation(fmi.get(), wgs84.get()));

  if (transformation == nullptr)
    throw std::runtime_error("Failed to create Sphere to WGS84 coordinate transformation");

  double x = theLatLon.X();
  double y = theLatLon.Y();

  if (transformation->Transform(1, &x, &y) == 0)
    throw std::runtime_error("Failed to project spherical coordinate to WGS84");

  return NFmiPoint(x, y);
}

NFmiPoint NFmiArea::WGS84ToSphere(const NFmiPoint &theLatLon)
{
  SpatialReferenceProxy fmi("FMI");
  SpatialReferenceProxy wgs84("WGS84");

  std::unique_ptr<OGRCoordinateTransformation> transformation(
      OGRCreateCoordinateTransformation(wgs84.get(), fmi.get()));

  if (transformation == nullptr)
    throw std::runtime_error("Failed to create WGS84 to Sphere coordinate transformation");

  double x = theLatLon.X();
  double y = theLatLon.Y();

  if (transformation->Transform(1, &x, &y) == 0)
    throw std::runtime_error("Failed to project WGS84 coordinate to sphere");

  return NFmiPoint(x, y);
}

#endif
