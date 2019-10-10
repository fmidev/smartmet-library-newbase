#include <boost/stacktrace.hpp>

#include <iomanip>
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
#include "NFmiAreaTools.h"
#include "NFmiString.h"
#include "NFmiVersion.h"

#include <boost/functional/hash.hpp>
#include <fmt/format.h>

namespace
{
// Known datums : those listed in PROJ.4 pj_datums.c

std::map<std::string, std::string> known_datums = {
    {"FMI", "+R=6371220 +towgs84=0,0,0"},
    {"WGS84", "+a=6378137 +rf=298.257223563 +towgs84=0,0,0"},
    {"GGRS87", "+a=6378137 +rf=298.257222101 +towgs84=-199.87,74.79,246.62"},
    {"NAD83", "+a=6378137 +rf=298.257222101 +towgs84=0,0,0"},
    {"NAD27", "+a=6378206.4 +b=6356583.8 +nadgrids=@conus,@alaska,@ntv2_0.gsb,@ntv1_can.dat"},
    {"potsdam", "+a=6377397.155 +rf=299.1528128 +nadgrids=@BETA2007.gsb"},
    {"carthage", "+a=6378249.2 +rf=293.4660212936269 +towgs84=-263.0,6.0,431.0"},
    {"hermannskogel",
     "+a=6377397.155 +rf=299.1528128 +towgs84=577.326,90.129,463.919,5.137,1.474,5.297,2.4232"},
    {"ire65",
     "+a=6377340.189 +b=6356034.446 +towgs84=482.530,-130.596,564.557,-1.042,-0.214,-0.631,8.15"},
    {"nzgd49", "+a=6378388 +rf=297. +towgs84=59.47,-5.04,187.44,0.47,-0.1,1.024,-4.5993"},
    {"OSGB36",
     "+a=6377563.396 +b=6356256.910 "
     "+towgs84=446.448,-125.157,542.060,0.1502,0.2470,0.8421,-20.4894"}};

// Known reference ellipsoids : those listed in PROJ.4 pj_ellps.c

std::map<std::string, std::string> known_ellipsoids = {
    {"MERIT", "+a=6378137 +rf=298.257"},
    {"SGS85", "+a=6378136 +rf=298.257"},
    {"GRS80", "+a=6378137 +rf=298.257222101"},
    {"IAU76", "+a=6378140 +rf=298.257"},
    {"airy", "+a=6377563.396 +b=6356256.910"},
    {"APL4.9", "+a=6378137.0. +rf=298.25"},
    {"NWL9D", "+a=6378145.0. +rf=298.25"},
    {"mod_airy", "+a=6377340.189 +b=6356034.446"},
    {"andrae", "+a=6377104.43 +rf=300.0"},
    {"aust_SA", "+a=6378160 +rf=298.25"},
    {"GRS67", "+a=6378160 +rf=298.2471674270"},
    {"bessel", "+a=6377397.155 +rf=299.1528128"},
    {"bess_nam", "+a=6377483.865 +rf=299.1528128"},
    {"clrk66", "+a=6378206.4 +b=6356583.8"},
    {"clrk80", "+a=6378249.145 +rf=293.4663"},
    {"clrk80ign", "+a=6378249.2 +rf=293.4660212936269"},
    {"CPM", "+a=6375738.7 +rf=334.29"},
    {"delmbr", "+a=6376428. +rf=311.5"},
    {"engelis", "+a=6378136.05 +rf=298.2566"},
    {"evrst30", "+a=6377276.345 +rf=300.8017"},
    {"evrst48", "+a=6377304.063 +rf=300.8017"},
    {"evrst56", "+a=6377301.243 +rf=300.8017"},
    {"evrst69", "+a=6377295.664 +rf=300.8017"},
    {"evrstSS", "+a=6377298.556 +rf=300.8017"},
    {"fschr60", "+a=6378166. +rf=298.3"},
    {"fschr60m", "+a=6378155. +rf=298.3"},
    {"fschr68", "+a=6378150. +rf=298.3"},
    {"helmert", "+a=6378200. +rf=298.3"},
    {"hough", "+a=6378270 +rf=297."},
    {"intl", "+a=6378388 +rf=297."},
    {"krass", "+a=6378245 +rf=298.3"},
    {"kaula", "+a=6378163. +rf=298.24"},
    {"lerch", "+a=6378139. +rf=298.257"},
    {"mprts", "+a=6397300. +rf=191."},
    {"new_intl", "+a=6378157.5 +b=6356772.2"},
    {"plessis", "+a=6376523 +b=6355863"},
    {"SEasia", "+a=6378155 +b=6356773.3205"},
    {"walbeck", "+a=6376896 +b=6355834.8467"},
    {"WGS60", "+a=6378165 +rf=298.3"},
    {"WGS66", "+a=6378145 +rf=298.25"},
    {"WGS72", "+a=6378135 +rf=298.26"},
    {"WGS84", "+a=6378137 +rf=298.257223563"},
    {"sphere", "+a=6370997 +b=6370997"}};

// Utility function for creating spatial references

std::unique_ptr<OGRSpatialReference> make_sr(std::string theDesc)
{
  if (theDesc.empty())
    throw std::runtime_error("Cannot create spatial reference from empty string");

  // Substitute for known datums/ellipsoids

  auto desc = theDesc;

  auto pos = known_datums.find(desc);
  if (pos != known_datums.end())
    desc = std::string("+proj=longlat ") + pos->second;
  else
  {
    pos = known_ellipsoids.find(desc);
    if (pos != known_ellipsoids.end()) desc = std::string("+proj=longlat ") + pos->second;
  }

  std::unique_ptr<OGRSpatialReference> sr(new OGRSpatialReference);
  auto err = sr->SetFromUserInput(desc.c_str());

  if (err == OGRERR_NONE) return sr;

  if (theDesc == desc)
    throw std::runtime_error("Failed to create spatial reference from '" + theDesc + "'");
  throw std::runtime_error("Failed to create spatial reference from '" + theDesc + "' ('" + desc +
                           "')");
}

std::string class_name_from_id(int id)
{
  switch (id)
  {
    case kNFmiArea:
      return "kNFmiArea";
    case kNFmiProjArea:
      return "kNFmiProjArea";
    case kNFmiGdalArea:
      return "kNFmiGdalArea";
    case kNFmiLambertEqualArea:
      return "kNFmiLambertEqualArea";
    case kNFmiLatLonArea:
      return "kNFmiLatLonArea";
    case kNFmiRotatedLatLonArea:
      return "kNFmiRotatedLatLonArea";
    case kNFmiStereographicArea:
      return "kNFmiStereographicArea";
    case kNFmiPKJArea:
      return "kNFmiPKJArea";
    case kNFmiYKJArea:
      return "kNFmiYKJArea";
    case kNFmiEquiDistArea:
      return "kNFmiEquiDistArea";
    case kNFmiGnomonicArea:
      return "kNFmiGnomonicArea";
    case kNFmiKKJArea:
      return "kNFmiKKJArea";
    case kNFmiMercatorArea:
      return "kNFmiMercatorArea";
    case kNFmiLambertConformalConicArea:
      return "kNFmiLambertConformalConicArea";
    default:
      throw std::runtime_error("Unknown projection class id " + std::to_string(id));
  }
}

}  // namespace

NFmiArea::SpatialReferenceProxy::SpatialReferenceProxy(const char *theSR) { init(theSR); }

NFmiArea::SpatialReferenceProxy::SpatialReferenceProxy(const std::string &theSR) { init(theSR); }

void NFmiArea::SpatialReferenceProxy::init(const std::string &theSR)
{
  itsProjStr = theSR;
  itsSR = *make_sr(theSR).release();
}

// Implementation details

struct NFmiArea::Impl
{
  OGRSpatialReference itsSpatialReference{NULL};

  // WGS84 conversions
  boost::shared_ptr<OGRCoordinateTransformation> itsToLatLonConverter;
  boost::shared_ptr<OGRCoordinateTransformation> itsToWorldXYConverter;

  // Projection geographic coordinate conversions
  boost::shared_ptr<OGRCoordinateTransformation> itsNativeToLatLonConverter;
  boost::shared_ptr<OGRCoordinateTransformation> itsNativeToWorldXYConverter;

  NFmiRect itsWorldRect;           // bbox in native WorldXY coordinates
  NFmiRect itsXYRect{0, 0, 1, 1};  // mapping from bbox to XY image coordinates

  // This is only needed when reading legacy files from disk
  int itsClassId = kNFmiArea;
  std::string itsClassName = "kNFmiArea";

  // For writing legacy projections back to disk

  NFmiPoint TopLeftCorner() const;
  NFmiPoint BottomRightCorner() const;
  boost::optional<NFmiPoint> itsTopLeftCorner;
  boost::optional<NFmiPoint> itsBottomRightCorner;

  // For speeding up coordinate conversions and to aid legacy parts of Write()
  double itsXScaleFactor = 0;
  double itsYScaleFactor = 0;

  // For providing PROJ.4 parameter information

  NFmiProj itsProj;

  // Should we flop the data?
  bool itsFlopped = false;
};

// Must be after Impl definition above
NFmiArea::~NFmiArea() = default;

// Needed due to Impl
NFmiArea::NFmiArea(const NFmiArea &other) : impl(new Impl(*other.impl)) {}

// Needed due to Impl
NFmiArea &NFmiArea::operator=(const NFmiArea &other)
{
  if (this != &other) impl.reset(new Impl(*other.impl));
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Helper method for Write() to keep original corners intact
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::Impl::TopLeftCorner() const
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

NFmiPoint NFmiArea::Impl::BottomRightCorner() const
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

NFmiArea::NFmiArea() : impl(new NFmiArea::Impl) {}

NFmiArea::NFmiArea(int theClassId) : impl(new NFmiArea::Impl) { impl->itsClassId = theClassId; }

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopLeft() const { return impl->itsXYRect.TopLeft(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomRight() const { return impl->itsXYRect.BottomRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TopRight() const { return impl->itsXYRect.TopRight(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::BottomLeft() const { return impl->itsXYRect.BottomLeft(); }

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
  return impl->itsXYRect.IsInside(xyPoint);
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
 * \brief Convert projected coordinate to native latlon
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYToNativeLatLon(const NFmiPoint &theWorldXY) const
{
  double x = theWorldXY.X();
  double y = theWorldXY.Y();

  if (impl->itsNativeToLatLonConverter->Transform(1, &x, &y) == 0)
    throw std::runtime_error("Failed to convert coordinate to native latlon");

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert native latlon to projected coordinate
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::NativeLatLonToWorldXY(const NFmiPoint &theLatLon) const
{
  double x = theLatLon.X();
  double y = theLatLon.Y();

  if (impl->itsNativeToWorldXYConverter->Transform(1, &x, &y) == 0)
    throw std::runtime_error("Failed to convert coordinate from native latlon");

  return NFmiPoint(x, y);
}

NFmiPoint NFmiArea::ToNativeLatLon(const NFmiPoint &theXY) const
{
  return WorldXYToNativeLatLon(XYToWorldXY(theXY));
}

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

void NFmiArea::Place(const NFmiPoint &newPlace)
{
  impl->itsXYRect.Place(newPlace);
  InitRectConversions();
}
// ----------------------------------------------------------------------
/*!
 * \param newSize Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::Size(const NFmiPoint &newSize)
{
  impl->itsXYRect.Size(newSize);
  InitRectConversions();
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Top() const { return impl->itsXYRect.Top(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Bottom() const { return impl->itsXYRect.Bottom(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Left() const { return impl->itsXYRect.Left(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Right() const { return impl->itsXYRect.Right(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Height() const { return impl->itsXYRect.Height(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::Width() const { return impl->itsXYRect.Width(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect &NFmiArea::XYArea() const { return impl->itsXYRect; }
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

unsigned long NFmiArea::ClassId() const { return impl->itsClassId; }

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const std::string &NFmiArea::ClassName() const { return impl->itsClassName; }

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
  return impl->itsWorldRect == theArea.impl->itsWorldRect;
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
  return impl->itsWorldRect != theArea.impl->itsWorldRect;
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
  InitRectConversions();
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
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream &NFmiArea::Write(std::ostream &file) const
{
  switch (impl->itsClassId)
  {
    case kNFmiArea:
    {
      file << kNFmiArea << " kNFmiArea\n";
      return file;
    }
    case kNFmiProjArea:
    {
      NFmiString txt = ProjStr();
      file << kNFmiProjArea << " kNFmiProjArea\n" << impl->itsXYRect << txt << impl->itsWorldRect;
      return file;
    }
    case kNFmiLatLonArea:
    {
      // WGS84: Not sure if the factors are readable by legacy programs
      file << kNFmiLatLonArea << " kNFmiLatLonArea\n"
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << impl->itsXScaleFactor << ' ' << -impl->itsYScaleFactor << '\n';
      return file;
    }
    case kNFmiRotatedLatLonArea:
    {
      auto plat = Proj().GetDouble("o_lat_p");
      auto plon = Proj().GetDouble("o_lon_p");
      if (!plon || !plat) throw std::runtime_error("Internal error in writing rotated latlon area");

      // Note: the world rect print order is correct, top left then bottom right
      NFmiPoint southpole(180 + *plon, -(*plat));
      file << kNFmiRotatedLatLonArea << " kNFmiRotatedLatLonArea\n"
           << impl->itsXYRect << impl->itsWorldRect.TopLeft() << impl->itsWorldRect.BottomRight()
           << "0 0\n0 0\n"
           << impl->itsXScaleFactor << ' ' << -impl->itsYScaleFactor << '\n'
           << southpole;
      return file;
    }
    case kNFmiMercatorArea:
    {
      file << kNFmiMercatorArea << " kNFmiMercatorArea\n"
           << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << impl->itsXScaleFactor << ' ' << impl->itsYScaleFactor << '\n';
      return file;
    }
    case kNFmiStereographicArea:
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");
      auto tlat = Proj().GetDouble("lat_ts");

      if (!clon || !clat || !tlat)
        throw std::runtime_error("Internal error in writing stereographic area");

      file << kNFmiStereographicArea << " kNFmiStereographicArea\n"
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << '\n'
           << *clat << '\n'
           << *tlat << '\n';

      if (DefaultFmiInfoVersion >= 5) file << "0 0 0\n";

      int oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiEquiDistArea:
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");

      if (!clon || !clat) throw std::runtime_error("Internal error writing aeqd area");

      // legacy tlat = 90
      file << kNFmiEquiDistArea << " kNFmiEquiDistArea\n"
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << '\n'
           << *clat << "\n90\n";

      if (DefaultFmiInfoVersion >= 5) file << "0 0 0\n";

      int oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiLambertConformalConicArea:
    {
      auto clon = Proj().GetDouble("lon_0");
      auto clat = Proj().GetDouble("lat_0");
      auto lat1 = Proj().GetDouble("lat_1");
      auto lat2 = Proj().GetDouble("lat_2");
      if (!clon || !clat || !lat1 || !lat2)
        throw std::runtime_error("Internal error writing lcc area");

      file << kNFmiLambertConformalConicArea << " kNFmiLambertConformalConicArea\n"
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << ' '
           << *clat << ' ' << *lat1 << ' ' << *lat2 << ' ' << kRearth << '\n';

      int oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiYKJArea:
    {
      file << kNFmiYKJArea << " kNFmiYKJArea\n"
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << impl->itsXScaleFactor << ' ' << impl->itsYScaleFactor << '\n'
           << impl->itsWorldRect;
      return file;
    }

    default:
    {
      throw std::runtime_error("Internal error writing unknown area");
    }
  }
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
  double xscalefactor = 0, yscalefactor = 0;

  file >> impl->itsXYRect;

  switch (impl->itsClassId)
  {
    case kNFmiArea:
    {
      // empty class
      return file;
    }
    case kNFmiProjArea:
    {
      // Generic PROJ area
      file >> txt >> impl->itsWorldRect;
      InitSpatialReference(txt.CharPtr());
      break;
    }
    case kNFmiLatLonArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> xscalefactor >>
          yscalefactor;
      *this = *NFmiAreaTools::CreateLegacyLatLonArea(bottomleft, topright);
      CheckRectConversions(xscalefactor, yscalefactor);
      break;
    }
    case kNFmiRotatedLatLonArea:
    {
      NFmiPoint southpole;
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> xscalefactor >>
          yscalefactor >> southpole;
      *this = *NFmiAreaTools::CreateLegacyRotatedLatLonArea(bottomleft, topright, southpole);
      CheckRectConversions(xscalefactor, yscalefactor);
      break;
    }
    case kNFmiStereographicArea:
    {
      double clon, clat, truelat;
      file >> bottomleft >> topright >> clon >> clat >> truelat >> dummy >> dummy >> dummy >>
          impl->itsWorldRect;

      *this =
          *NFmiAreaTools::CreateLegacyStereographicArea(bottomleft, topright, clon, clat, truelat);
      break;
    }
    case kNFmiEquiDistArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          impl->itsWorldRect;
      *this = *NFmiAreaTools::CreateLegacyEquiDistArea(bottomleft, topright, clon, clat);
      break;
    }
    case kNFmiMercatorArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> xscalefactor >>
          yscalefactor;
      *this = *NFmiAreaTools::CreateLegacyMercatorArea(bottomleft, topright);
      CheckRectConversions(xscalefactor, yscalefactor);
      break;
    }
    case kNFmiLambertEqualArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          impl->itsWorldRect;
      *this = *NFmiAreaTools::CreateLegacyLambertEqualArea(bottomleft, topright, clon, clat);
      break;
    }
    case kNFmiLambertConformalConicArea:
    {
      double clon, clat, tlat1, tlat2, radius;
      file >> bottomleft >> topright >> clon >> clat >> tlat1 >> tlat2 >> radius >>
          impl->itsWorldRect;
      *this = *NFmiAreaTools::CreateLegacyLambertConformalConicArea(
          bottomleft, topright, clon, clat, tlat1, tlat2);
      break;
    }
    case kNFmiGnomonicArea:
    {
      double clon, clat;
      file >> bottomleft >> topright >> clon >> clat >> dummy >> dummy >> dummy >> dummy >>
          impl->itsWorldRect;
      *this = *NFmiAreaTools::CreateLegacyGnomonicArea(bottomleft, topright, clon, clat);
      break;
    }
    case kNFmiYKJArea:
    {
      file >> bottomleft >> topright >> dummy >> dummy >> dummy >> dummy >> xscalefactor >>
          yscalefactor >> impl->itsWorldRect;
      *this = *NFmiAreaTools::CreateLegacyYKJArea(bottomleft, topright);
      CheckRectConversions(xscalefactor, yscalefactor);
      break;
    }
    case kNFmiGdalArea:
    {
      impl->itsClassId = kNFmiProjArea;
      NFmiString datum, proj;
      file >> bottomleft >> topright >> datum >> proj;
      *this = *NFmiArea::CreateFromCorners(proj.CharPtr(), datum.CharPtr(), bottomleft, topright);
      break;
    }
    case kNFmiPKJArea:
      throw std::runtime_error("PKJ projection is no longer supported in Finland");
    case kNFmiKKJArea:
      throw std::runtime_error("KKJ projection is no longer supported in Finland");
    default:
      // kFmiPolSetArea, perhaps some other legacy classes too
      throw std::runtime_error("Projection number " + std::to_string(impl->itsClassId) +
                               " is no longer supported");
  }

  impl->itsProj = NFmiProj(ProjStr());
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
  std::size_t hash = impl->itsWorldRect.HashValue();
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
  if (!impl->itsToWorldXYConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  double x = theWgs84.X();
  double y = theWgs84.Y();

  if (impl->itsToWorldXYConverter->Transform(1, &x, &y) == 0) return NFmiPoint::gMissingLatlon;

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from native metric/geographic coordinates to WGS84
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYToLatLon(const NFmiPoint &theWorldXY) const
{
  if (!impl->itsToLatLonConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  double x = theWorldXY.X();
  double y = theWorldXY.Y();

  if (impl->itsToLatLonConverter->Transform(1, &x, &y) == 0) return NFmiPoint::gMissingLatlon;

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

  auto xy = XYToWorldXY(theXYPoint);

  // Transform world xy-coordinates into WGS84

  auto res = WorldXYToLatLon(xy);

  // Return result if not a pole
  const double eps = 0.0001;
  if (res.X() != 0 || (res.Y() < (90 - eps) && res.Y() > (-90 + eps))) return res;

  // GDAL/PROJ.4 may set longitude to zero for poles. We attempt to fix this based on the
  // X-coordinate at the equator.

  double x;
  if (!impl->itsFlopped)
    x = impl->itsWorldRect.Left() + (theXYPoint.X() - Left()) / impl->itsXScaleFactor;
  else
    x = impl->itsWorldRect.Right() - (theXYPoint.X() - Left()) / impl->itsXScaleFactor;

  auto polex = WorldXYToLatLon(NFmiPoint(x, 0));

  // Return adjusted longitude

  return NFmiPoint(polex.X(), res.Y());
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
  return WorldXYToXY(worldxy);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from native latlon to image XY
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::NativeToXY(const NFmiPoint &theLatLonPoint) const
{
  auto worldxy = NativeLatLonToWorldXY(theLatLonPoint);
  if (worldxy == NFmiPoint::gMissingLatlon) return worldxy;
  return WorldXYToXY(worldxy);
}

// ----------------------------------------------------------------------
/*!
 * \brief Transform World XY coordinate to image XY
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYToXY(const NFmiPoint &theWorldXY) const
{
  double x;
  if (!impl->itsFlopped)
    x = Left() + impl->itsXScaleFactor * (theWorldXY.X() - impl->itsWorldRect.Left());
  else
    x = Left() - impl->itsXScaleFactor * (theWorldXY.X() - impl->itsWorldRect.Right());

  auto y = Top() + impl->itsYScaleFactor * (impl->itsWorldRect.Bottom() - theWorldXY.Y());
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from image XY to projection XY
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  double x;
  if (!impl->itsFlopped)
    x = impl->itsWorldRect.Left() + (theXYPoint.X() - Left()) / impl->itsXScaleFactor;
  else
    x = impl->itsWorldRect.Right() - (theXYPoint.X() - Left()) / impl->itsXScaleFactor;
  const auto y = impl->itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / impl->itsYScaleFactor;
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the spatial reference
 */
// ----------------------------------------------------------------------

void NFmiArea::InitSpatialReference(const std::string &theProjection)
{
  impl->itsSpatialReference = *make_sr(theProjection).release();
  InitRectConversions();
  InitProj();
}

void NFmiArea::InitProj()
{
  // WGS84 converters

  auto wgs84 = make_sr("WGS84");

  impl->itsToWorldXYConverter.reset(
      OGRCreateCoordinateTransformation(wgs84.get(), &impl->itsSpatialReference));

  if (impl->itsToWorldXYConverter == nullptr)
    throw std::runtime_error("Failed to create coordinate transformation from WGS84");

  impl->itsToLatLonConverter.reset(
      OGRCreateCoordinateTransformation(&impl->itsSpatialReference, wgs84.get()));

  if (impl->itsToLatLonConverter == nullptr)
    throw std::runtime_error("Failed to create coordinate transformation to WGS84");

  // Init PROJ.4 settings
  impl->itsProj = NFmiProj(ProjStr());

  // Init geographic coordinate conversions in native datum

  auto proj = impl->itsProj.InverseProjStr();

  auto latlon = make_sr(proj);

  impl->itsNativeToLatLonConverter.reset(
      OGRCreateCoordinateTransformation(&impl->itsSpatialReference, latlon.get()));

  if (impl->itsNativeToLatLonConverter == nullptr)
    throw std::runtime_error("Failed to create requested coordinate transformation to " + proj);

  impl->itsNativeToWorldXYConverter.reset(
      OGRCreateCoordinateTransformation(latlon.get(), &impl->itsSpatialReference));

  if (impl->itsNativeToWorldXYConverter == nullptr)
    throw std::runtime_error("Failed to create requested coordinate transformation from " + proj);

  // Switch writer to ProjArea if we were not reading a legacy projection
  if (impl->itsClassId == kNFmiArea) impl->itsClassId = kNFmiProjArea;

  // Switch classId to legacy mode if legacy mode can be detected

  if (impl->itsClassId == kNFmiProjArea) impl->itsClassId = Proj().DetectClassId();

  impl->itsClassName = class_name_from_id(impl->itsClassId);
}

void NFmiArea::InitRectConversions()
{
  impl->itsXScaleFactor = impl->itsXYRect.Width() / impl->itsWorldRect.Width();
  impl->itsYScaleFactor = impl->itsXYRect.Height() / impl->itsWorldRect.Height();
}

// Verify with scale factors read from the file, perhaps we need to flip and/or flop
void NFmiArea::CheckRectConversions(double theXScaleFactor, double theYScaleFactor)
{
#if 0  
  if (theXScaleFactor * impl->itsXScaleFactor < 0)
  {
    impl->itsXScaleFactor = -impl->itsXScaleFactor;
    std::cerr << "Switched! " << impl->itsXScaleFactor << std::endl;
  }
#endif
}

OGRSpatialReference *NFmiArea::SpatialReference() { return &impl->itsSpatialReference; }
const OGRSpatialReference *NFmiArea::SpatialReference() const { return &impl->itsSpatialReference; }

const NFmiProj &NFmiArea::Proj() const { return impl->itsProj; }

NFmiArea *NFmiArea::CreateFromBBox(SpatialReferenceProxy theSR,
                                   const NFmiPoint &theBottomLeftWorldXY,
                                   const NFmiPoint &theTopRightWorldXY)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = *theSR;
    area->InitProj();
    area->impl->itsWorldRect = NFmiRect(theBottomLeftWorldXY.X(),
                                        theTopRightWorldXY.Y(),
                                        theTopRightWorldXY.X(),
                                        theBottomLeftWorldXY.Y());
    area->impl->itsFlopped = (theBottomLeftWorldXY.X() > theTopRightWorldXY.X());
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
    area->impl->itsSpatialReference = *theSR;
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

    area->impl->itsWorldRect = NFmiRect(x1, y2, x2, y1);
    area->impl->itsFlopped = (x1 > x2);
    area->InitRectConversions();

    // save legacy corner coordinates for Write()
    area->impl->itsTopLeftCorner = theBottomLeftLatLon;
    area->impl->itsBottomRightCorner = theTopRightLatLon;

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
    area->impl->itsSpatialReference = *theSR;
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
      throw std::runtime_error(
          "Failed to initialize projection from BBOX reverse corner coordinates");

    area->impl->itsWorldRect = NFmiRect(x1, y1, x2, y2);
    area->impl->itsFlopped = (x1 > x2);
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
    area->impl->itsSpatialReference = *theSR;
    area->InitProj();

    double x1 = theBottomLeftLatLon.X();
    double y1 = theBottomLeftLatLon.Y();
    double x2 = theTopRightLatLon.X();
    double y2 = theTopRightLatLon.Y();

    if (area->impl->itsToWorldXYConverter->Transform(1, &x1, &y1) == 0 ||
        area->impl->itsToWorldXYConverter->Transform(1, &x2, &y2) == 0)
      throw std::runtime_error("Failed to initialize projection from WGS84 corner coordinates");

    area->impl->itsWorldRect = NFmiRect(x1, y2, x2, y1);
    area->impl->itsFlopped = (x1 > x2);
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
    area->impl->itsSpatialReference = *theSR;
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

    area->impl->itsWorldRect = NFmiRect(x, y + theHeight, x + theWidth, y);
    area->impl->itsFlopped = (theWidth < 0);
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
    area->impl->itsSpatialReference = *theSR;
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

    area->impl->itsWorldRect =
        NFmiRect(x - theWidth / 2, y + theHeight / 2, x + theWidth / 2, y - theHeight / 2);
    area->impl->itsFlopped = (theWidth < 0);

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
  impl->itsSpatialReference.exportToWkt(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

std::string NFmiArea::PrettyWKT() const
{
  char *out;
  impl->itsSpatialReference.exportToPrettyWkt(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

std::string NFmiArea::ProjStr() const
{
  char *out;
  impl->itsSpatialReference.exportToProj4(&out);
  std::string ret = out;
  OGRFree(out);
  return ret;
}

NFmiRect NFmiArea::WorldRect() const { return impl->itsWorldRect; }

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
  return CreateFromCorners(
      impl->itsSpatialReference, "FMI", theRect.BottomLeft(), theRect.TopRight());
}

NFmiArea *NFmiArea::CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const
{
  // Note: We use spherical latlon coordinates for legacy reasons. Use CreateFromCorners directly
  // to use other spatial references.
  return CreateFromCorners(
      impl->itsSpatialReference, "FMI", theBottomLeftLatLon, theTopRightLatLon);
}

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect)
{
  NFmiPoint newBottomLeftXY = theWorldRect.BottomLeft();
  NFmiPoint newTopRightXY = theWorldRect.TopRight();

  NFmiPoint newBottomLeftLatLon = WorldXYToLatLon(newBottomLeftXY);
  NFmiPoint newTopRightLatLon = WorldXYToLatLon(newTopRightXY);

  if (!IsInside(newBottomLeftLatLon) || !IsInside(newTopRightLatLon)) return nullptr;

  auto *newArea = CreateFromBBox(impl->itsSpatialReference, newBottomLeftXY, newTopRightXY);

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
      CreateFromBBox(impl->itsSpatialReference, newWorldRect.BottomLeft(), newWorldRect.TopRight());

  // Return the re-dimensioned copy of the original area
  return newArea;
}

double NFmiArea::XScale() const { return 1 / impl->itsXScaleFactor; }
double NFmiArea::YScale() const { return 1 / impl->itsYScaleFactor; }

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
