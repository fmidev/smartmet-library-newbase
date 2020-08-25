#include "NFmiArea.h"

#include "NFmiAreaFactory.h"
#include "NFmiAreaTools.h"
#include "NFmiString.h"
#include "NFmiVersion.h"
#include <boost/functional/hash.hpp>
#include <fmt/format.h>
#include <gis/BilinearCoordinateTransformation.h>
#include <gis/CoordinateMatrix.h>
#include <gis/CoordinateTransformation.h>
#include <gis/OGR.h>
#include <gis/ProjInfo.h>
#include <gis/SpatialReference.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <ogr_spatialref.h>

namespace
{
std::string exportToProj(const OGRSpatialReference &theSRS)
{
  char *out;
  theSRS.exportToProj4(&out);
  std::string ret = out;
  CPLFree(out);
  return ret;
}

NFmiPoint transform(const Fmi::CoordinateTransformation &theTransformation,
                    const NFmiPoint &thePoint)
{
  double x = thePoint.X();
  double y = thePoint.Y();
  theTransformation.transform(x, y);
  return NFmiPoint(x, y);
}

std::string class_name_from_id(int id)
{
  switch (id)
  {
    case kNFmiArea:
      return "NFmiArea";
    case kNFmiProjArea:
      return "NFmiProjArea";
    case kNFmiGdalArea:
      return "kNFmiGdalArea";  // legacy "k"
    case kNFmiLambertEqualArea:
      return "NFmiLambertEqualArea";
    case kNFmiLatLonArea:
      return "NFmiLatLonArea";
    case kNFmiRotatedLatLonArea:
      return "NFmiRotatedLatLonArea";
    case kNFmiStereographicArea:
      return "kNFmiStereographicArea";  // legacy "k"
    case kNFmiPKJArea:
      return "NFmiPKJArea";
    case kNFmiYKJArea:
      return "NFmiYKJArea";
    case kNFmiEquiDistArea:
      return "NFmiEquiDistArea";
    case kNFmiGnomonicArea:
      return "kNFmiGnomonicArea";  // legacy "k"
    case kNFmiKKJArea:
      return "NFmiKKJArea";
    case kNFmiMercatorArea:
      return "NFmiMercatorArea";
    case kNFmiLambertConformalConicArea:
      return "kNFmiLambertConformalConicArea";  // legacy "k"
    default:
      throw std::runtime_error("Unknown projection class id " + std::to_string(id));
  }
}

}  // namespace

// Implementation details

struct NFmiArea::Impl
{
  ~Impl() = default;
  Impl() = default;
  Impl(const Impl &other)
      : itsSpatialReference(other.itsSpatialReference),
        itsToLatLonConverter(new Fmi::CoordinateTransformation(*other.itsToLatLonConverter)),
        itsToWorldXYConverter(new Fmi::CoordinateTransformation(*other.itsToWorldXYConverter)),
        itsToLatLonBilinearConverter(other.itsToLatLonBilinearConverter),
        itsToWorldXYBilinearConverter(other.itsToWorldXYBilinearConverter),
        itsNativeToLatLonConverter(
            new Fmi::CoordinateTransformation(*other.itsNativeToLatLonConverter)),
        itsNativeToWorldXYConverter(
            new Fmi::CoordinateTransformation(*other.itsNativeToWorldXYConverter)),
        itsWorldRect(other.itsWorldRect),
        itsXYRect(other.itsXYRect),
        itsClassId(other.itsClassId),
        itsClassName(other.itsClassName),
        itsTopLeftCorner(other.itsTopLeftCorner),
        itsBottomRightCorner(other.itsBottomRightCorner),
        itsXScaleFactor(other.itsXScaleFactor),
        itsYScaleFactor(other.itsYScaleFactor),
        itsFlopped(other.itsFlopped),
        itsLegacyWriteFlag(other.itsLegacyWriteFlag)
  {
  }

  Impl &operator=(const Impl &) = delete;

  std::shared_ptr<Fmi::SpatialReference> itsSpatialReference;

  // WGS84 conversions
  std::unique_ptr<Fmi::CoordinateTransformation> itsToLatLonConverter;
  std::unique_ptr<Fmi::CoordinateTransformation> itsToWorldXYConverter;

  // Fast bilinear interpolation coordinate conversions
  std::shared_ptr<Fmi::BilinearCoordinateTransformation> itsToLatLonBilinearConverter;
  std::shared_ptr<Fmi::BilinearCoordinateTransformation> itsToWorldXYBilinearConverter;

  // Projection geographic coordinate conversions
  std::unique_ptr<Fmi::CoordinateTransformation> itsNativeToLatLonConverter;
  std::unique_ptr<Fmi::CoordinateTransformation> itsNativeToWorldXYConverter;

  NFmiRect itsWorldRect;           // bbox in native WorldXY coordinates
  NFmiRect itsXYRect{0, 0, 1, 1};  // mapping from bbox to XY image coordinates

  // This is only needed when reading legacy files from disk
  int itsClassId = kNFmiArea;
  std::string itsClassName = "NFmiArea";

  // For writing legacy projections back to disk

  NFmiPoint TopLeftCorner() const;
  NFmiPoint BottomRightCorner() const;
  boost::optional<NFmiPoint> itsTopLeftCorner;
  boost::optional<NFmiPoint> itsBottomRightCorner;

  // For speeding up coordinate conversions and to aid legacy parts of Write()
  double itsXScaleFactor = 0;
  double itsYScaleFactor = 0;

  // Should we flop the data?
  bool itsFlopped = false;

  // Should we write legacy definitions if possible?
  bool itsLegacyWriteFlag = true;
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

  Fmi::CoordinateTransformation trans(*itsSpatialReference, "FMI");

  auto tl = itsWorldRect.TopLeft();
  return transform(trans, tl);
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

  Fmi::CoordinateTransformation transformation(*itsSpatialReference, "FMI");

  auto br = itsWorldRect.BottomRight();
  return transform(transformation, br);
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

bool NFmiArea::IsInsideWorldXY(const NFmiPoint &theWorldXYPoint) const
{
  NFmiPoint xyPoint = WorldXYToXY(theWorldXYPoint);
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
  return transform(*impl->itsNativeToLatLonConverter, theWorldXY);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert native latlon to projected coordinate
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::NativeLatLonToWorldXY(const NFmiPoint &theLatLon) const
{
  return transform(*impl->itsNativeToWorldXYConverter, theLatLon);
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
  return SpatialReference().get()->IsSame(theArea.SpatialReference().get()) &&
         impl->itsWorldRect == theArea.impl->itsWorldRect;
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object to compare against
 * \return True if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiArea::operator!=(const NFmiArea &theArea) const { return !(*this == theArea); }

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
 * \param transformation is made with spatial references from source and destination data.
 * \param coordinate is usually worldXY coordinate, but can be in any coordinate system that spatial
 * references support.
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::TransformCoordinate(const Fmi::CoordinateTransformation &transformation,
                                        const NFmiPoint &coordinate)
{
  double x = coordinate.X();
  double y = coordinate.Y();
  transformation.transform(x, y);
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * If theArea object is put over this area object, what xy-rect would that cover in this object's
 * xy-space. \param theArea is map-area that is put over this map object for calculations. This
 * method is intended to be used only if this object and theArea object are both in similar
 * projection and datum. E.g. both are in latlon projection type and both are on sphere or wgs84
 * datum.
 */
// ----------------------------------------------------------------------

NFmiRect NFmiArea::XYArea(const NFmiArea *theArea) const
{
#ifdef WGS84
  const auto &worldXyRect = theArea->WorldRect();
  // This transformation object is needed because there is at least one special case where different
  // areas have the same worldXY bounding-boxes and had to be addressed with this transformation
  // object: Case of Atlantic centric and Pacific centric world areas, where pacific centric has
  // worldXY origo moved by 180 degrees.
  Fmi::CoordinateTransformation transformation(theArea->SpatialReference(), SpatialReference());
  NFmiPoint topLeft(WorldXYToXY(TransformCoordinate(transformation, worldXyRect.TopLeft())));
  NFmiPoint bottomRight(
      WorldXYToXY(TransformCoordinate(transformation, worldXyRect.BottomRight())));
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
  // Use generic PROJ area if writing legacy definitions is disabled

  const auto id = (impl->itsLegacyWriteFlag ? impl->itsClassId : kNFmiProjArea);

  switch (id)
  {
    case kNFmiArea:
    {
      file << kNFmiArea << " NFmiArea\n";
      return file;
    }
    case kNFmiProjArea:
    {
      NFmiString txt = ProjStr();
      file << kNFmiProjArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << txt << impl->itsWorldRect;
      return file;
    }
    case kNFmiLatLonArea:
    {
      // Legacy scale factors for old newbase programs, new programs ignore the values:
      auto xscalefactor =
          (Right() - Left()) / (impl->BottomRightCorner().X() - impl->TopLeftCorner().X());
      auto yscalefactor =
          (Top() - Bottom()) / (impl->TopLeftCorner().Y() - impl->BottomRightCorner().Y());

      file << kNFmiLatLonArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << xscalefactor << ' ' << -yscalefactor << '\n';
      return file;
    }
    case kNFmiRotatedLatLonArea:
    {
      auto plat = ProjInfo().getDouble("o_lat_p");
      auto plon = ProjInfo().getDouble("o_lon_p");
      auto lon0 = ProjInfo().getDouble("lon_0");
      if (!plon || !plat || !lon0)
        throw std::runtime_error("Internal error in writing rotated latlon area");
      if (*plon != 0)
        throw std::runtime_error("Legacy rotated latlon with pole longitude != 0 not supported");

      // Legacy scale factors for old newbase programs, new programs ignore the values:
      auto xscalefactor =
          (Right() - Left()) / (impl->BottomRightCorner().X() - impl->TopLeftCorner().X());
      auto yscalefactor =
          (Top() - Bottom()) / (impl->TopLeftCorner().Y() - impl->BottomRightCorner().Y());

      // Note: the world rect print order is correct, top left then bottom right
      NFmiPoint southpole(*lon0, -(*plat));
      file << kNFmiRotatedLatLonArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << xscalefactor << ' ' << yscalefactor << '\n'
           << southpole;
      return file;
    }
    case kNFmiMercatorArea:
    {
      file << kNFmiMercatorArea << ' ' << impl->itsClassName << '\n'
           << impl->TopLeftCorner() << impl->BottomRightCorner() << "0 0\n0 0\n"
           << impl->itsXScaleFactor << ' ' << impl->itsYScaleFactor << '\n';
      return file;
    }
    case kNFmiStereographicArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");
      auto tlat = ProjInfo().getDouble("lat_ts");

      if (!clon || !clat || !tlat)
        throw std::runtime_error("Internal error in writing stereographic area");

      file << kNFmiStereographicArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << '\n'
           << *clat << '\n'
           << *tlat << '\n';

      if (DefaultFmiInfoVersion >= 5) file << "0 0 0\n";

      auto oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiEquiDistArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");

      if (!clon || !clat) throw std::runtime_error("Internal error writing aeqd area");

      // legacy tlat = 90
      file << kNFmiEquiDistArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << '\n'
           << *clat << "\n90\n";

      if (DefaultFmiInfoVersion >= 5) file << "0 0 0\n";

      auto oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiLambertConformalConicArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");
      auto lat1 = ProjInfo().getDouble("lat_1");
      auto lat2 = ProjInfo().getDouble("lat_2");
      if (!clon || !clat || !lat1 || !lat2)
        throw std::runtime_error("Internal error writing lcc area");

      file << kNFmiLambertConformalConicArea << ' ' << impl->itsClassName << '\n'
           << impl->itsXYRect << impl->TopLeftCorner() << impl->BottomRightCorner() << *clon << ' '
           << *clat << ' ' << *lat1 << ' ' << *lat2 << ' ' << kRearth << '\n';

      auto oldPrec = file.precision();
      file.precision(15);
      file << impl->itsWorldRect << ' ';
      file.precision(oldPrec);

      return file;
    }
    case kNFmiYKJArea:
    {
      file << kNFmiYKJArea << ' ' << impl->itsClassName << '\n'
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
               : NFmiAngle(270.);  // Azimuth is exactly east 90 degrees or west 270 degrees,
                                   // respectively

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
  if (impl->itsToWorldXYBilinearConverter)
  {
    double x = theWgs84.X();
    double y = theWgs84.Y();
    if (impl->itsToWorldXYBilinearConverter->transform(x, y)) return NFmiPoint(x, y);
  }

  if (!impl->itsToWorldXYConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  return transform(*impl->itsToWorldXYConverter, theWgs84);
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert coordinate from native metric/geographic coordinates to WGS84
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiArea::WorldXYToLatLon(const NFmiPoint &theWorldXY) const
{
  if (impl->itsToLatLonBilinearConverter)
  {
    double x = theWorldXY.X();
    double y = theWorldXY.Y();
    if (impl->itsToLatLonBilinearConverter->transform(x, y)) return NFmiPoint(x, y);
  }

  if (!impl->itsToLatLonConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  return transform(*impl->itsToLatLonConverter, theWorldXY);
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
  impl->itsSpatialReference.reset(new Fmi::SpatialReference(theProjection));
  InitRectConversions();
  InitProj();
}

void NFmiArea::InitProj()
{
  // WGS84 converters

  Fmi::SpatialReference wgs84("WGS84");

  impl->itsToWorldXYConverter.reset(
      new Fmi::CoordinateTransformation(wgs84, *impl->itsSpatialReference));

  impl->itsToLatLonConverter.reset(
      new Fmi::CoordinateTransformation(*impl->itsSpatialReference, wgs84));

  if (impl->itsToLatLonConverter == nullptr)
    throw std::runtime_error("Failed to create coordinate transformation to WGS84");

  // Init geographic coordinate conversions in native datum

  auto proj = impl->itsSpatialReference->projInfo().inverseProjStr();

  Fmi::SpatialReference latlon(proj);

  impl->itsNativeToLatLonConverter.reset(
      new Fmi::CoordinateTransformation(*impl->itsSpatialReference, latlon));

  impl->itsNativeToWorldXYConverter.reset(
      new Fmi::CoordinateTransformation(latlon, *impl->itsSpatialReference));

  // Switch writer to ProjArea if we were not reading a legacy projection
  if (impl->itsClassId == kNFmiArea) impl->itsClassId = kNFmiProjArea;

  // Switch classId to legacy mode if legacy mode can be detected

  if (impl->itsClassId == kNFmiProjArea) impl->itsClassId = DetectClassId();

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

const Fmi::SpatialReference &NFmiArea::SpatialReference() const
{
  return *impl->itsSpatialReference;
}

const Fmi::ProjInfo &NFmiArea::ProjInfo() const { return impl->itsSpatialReference->projInfo(); }

int NFmiArea::DetectClassId() const
{
  const auto proj = ProjInfo();

  auto name = proj.getString("proj");
  if (!name) throw std::runtime_error("Projection name not set, should be impossible");

  if (*name == "eqc" && proj.getString("datum") == std::string("WGS84")) return kNFmiLatLonArea;

  if (proj.getDouble("R") == kRearth ||
      (proj.getDouble("a") == kRearth && proj.getDouble("b") == kRearth))
  {
    if (*name == "merc") return kNFmiMercatorArea;
    if (*name == "stere") return kNFmiStereographicArea;
    if (*name == "aeqd") return kNFmiEquiDistArea;
    if (*name == "lcc") return kNFmiLambertConformalConicArea;
    if (*name == "ob_tran" && proj.getString("o_proj") == std::string("eqc") &&
        proj.getDouble("o_lon_p") == 0.0)
    {
      if (proj.getString("towgs84") == std::string("0,0,0") ||
          proj.getString("towgs84") == std::string("0,0,0,0,0,0,0"))
        return kNFmiRotatedLatLonArea;
    }
  }
  else if (*name == "tmerc" && proj.getString("ellps") == std::string("intl") &&
           proj.getDouble("x_0") == 3500000.0 && proj.getDouble("lat_0") == 0.0 &&
           proj.getDouble("lon_0") == 27.0 &&
           proj.getString("towgs84") ==
               std::string("-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964"))
    return kNFmiYKJArea;

  // Not a legacy projection, use PROJ.4
  return kNFmiProjArea;
}

void NFmiArea::DisableLegacyWrite() { impl->itsLegacyWriteFlag = false; }

NFmiArea *NFmiArea::CreateFromBBox(const Fmi::SpatialReference &theSR,
                                   const NFmiPoint &theBottomLeftWorldXY,
                                   const NFmiPoint &theTopRightWorldXY)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
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

NFmiArea *NFmiArea::CreateFromCorners(const Fmi::SpatialReference &theSR,
                                      const Fmi::SpatialReference &theBBoxSR,
                                      const NFmiPoint &theBottomLeftLatLon,
                                      const NFmiPoint &theTopRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
    area->InitProj();

    Fmi::CoordinateTransformation transformation(theBBoxSR, area->SpatialReference());

    auto bl = transform(transformation, theBottomLeftLatLon);
    auto tr = transform(transformation, theTopRightLatLon);

    area->impl->itsWorldRect = NFmiRect(bl, tr);
    // area->impl->itsWorldRect = NFmiRect(x1, y2, x2, y1); TODO:: ORDER?!?!?!!?!?!?
    area->impl->itsFlopped = (bl.X() > tr.X());
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

NFmiArea *NFmiArea::CreateFromReverseCorners(const Fmi::SpatialReference &theSR,
                                             const Fmi::SpatialReference &theBBoxSR,
                                             const NFmiPoint &theTopLeftLatLon,
                                             const NFmiPoint &theBottomRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
    area->InitProj();

    Fmi::CoordinateTransformation transformation(theBBoxSR, area->SpatialReference());

    auto tl = transform(transformation, theTopLeftLatLon);
    auto br = transform(transformation, theBottomRightLatLon);

    area->impl->itsWorldRect = NFmiRect(tl, br);
    // area->impl->itsWorldRect = NFmiRect(x1, y1, x2, y2); TODOO:: ORDER?!?!?!?!?
    area->impl->itsFlopped = (tl.X() > br.X());
    area->InitRectConversions();

    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromWGS84Corners(const Fmi::SpatialReference &theSR,
                                           const NFmiPoint &theBottomLeftLatLon,
                                           const NFmiPoint &theTopRightLatLon)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
    area->InitProj();

    auto bl = transform(*area->impl->itsToWorldXYConverter, theBottomLeftLatLon);
    auto tr = transform(*area->impl->itsToWorldXYConverter, theTopRightLatLon);

    area->impl->itsWorldRect = NFmiRect(bl, tr);
    // area->impl->itsWorldRect = NFmiRect(x1, y2, x2, y1); ORDER!?!?!?!?!?!?
    area->impl->itsFlopped = (bl.X() > tr.X());
    area->InitRectConversions();
    return area;
  }
  catch (...)
  {
    delete area;
    throw;
  }
}

NFmiArea *NFmiArea::CreateFromCornerAndSize(const Fmi::SpatialReference &theSR,
                                            const Fmi::SpatialReference &theCornerSR,
                                            const NFmiPoint &theBottomLeftLatLon,
                                            double theWidth,
                                            double theHeight)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
    area->InitProj();

    Fmi::CoordinateTransformation transformation(theCornerSR, theSR);

    double x = theBottomLeftLatLon.X();
    double y = theBottomLeftLatLon.Y();

    transformation.transform(x, y);

    // TODO: ???????????????
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

NFmiArea *NFmiArea::CreateFromCenter(const Fmi::SpatialReference &theSR,
                                     const Fmi::SpatialReference &theCenterSR,
                                     const NFmiPoint &theCenterLatLon,
                                     double theWidth,
                                     double theHeight)
{
  auto area = new NFmiArea;
  try
  {
    area->impl->itsSpatialReference = std::make_shared<Fmi::SpatialReference>(theSR);
    area->InitProj();

    Fmi::CoordinateTransformation transformation(theCenterSR, theSR);

    double x = theCenterLatLon.X();
    double y = theCenterLatLon.Y();

    transformation.transform(x, y);

    // TODO: ORDER?!?!?!?!?!?
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

std::string NFmiArea::WKT() const { return Fmi::OGR::exportToWkt(*impl->itsSpatialReference); }

std::string NFmiArea::PrettyWKT() const
{
  return Fmi::OGR::exportToPrettyWkt(*impl->itsSpatialReference);
}

std::string NFmiArea::ProjStr() const { return Fmi::OGR::exportToProj(*impl->itsSpatialReference); }

std::string NFmiArea::AreaFactoryStr() const
{
  if (impl->itsClassId == kNFmiProjArea) return AreaFactoryProjStr();

  auto tl = impl->TopLeftCorner();
  auto br = impl->BottomRightCorner();

  auto corners = fmt::format("{},{},{},{}", tl.X(), tl.Y(), br.X(), br.Y());

  // Note: We use : instead of | for legacy projections for backward compatibility

  switch (impl->itsClassId)
  {
    case kNFmiArea:
    case kNFmiLatLonArea:
      return "latlon:" + corners;
    case kNFmiRotatedLatLonArea:
    {
      auto plat = ProjInfo().getDouble("o_lat_p");
      auto plon = ProjInfo().getDouble("o_lon_p");
      auto lon0 = ProjInfo().getDouble("lon_0");
      if (!plon || !plat || !lon0)
        throw std::runtime_error("Internal error in writing rotated latlon area");
      if (*plon != 0)
        throw std::runtime_error("Legacy rotated latlon with pole longitude != 0 not supported");
      return fmt::format("invrotlatlon,{},{}:{}", -(*plat), *lon0, corners);
    }
    case kNFmiMercatorArea:
    {
      return "mercator:" + corners;
    }
    case kNFmiStereographicArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");
      auto tlat = ProjInfo().getDouble("lat_ts");

      if (!clon || !clat || !tlat)
        throw std::runtime_error("Internal error in writing stereographic area");

      return fmt::format("stereographic,{},{},{}:{}", *clon, *clat, *tlat, corners);
    }
    case kNFmiEquiDistArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");

      if (!clon || !clat) throw std::runtime_error("Internal error writing aeqd area");

      return fmt::format("equidist,{},{}:{}", *clon, *clat, corners);
    }
    case kNFmiLambertConformalConicArea:
    {
      auto clon = ProjInfo().getDouble("lon_0");
      auto clat = ProjInfo().getDouble("lat_0");
      auto lat1 = ProjInfo().getDouble("lat_1");
      auto lat2 = ProjInfo().getDouble("lat_2");
      if (!clon || !clat || !lat1 || !lat2)
        throw std::runtime_error("Internal error writing lcc area");

      if (*lat1 == *lat2) return fmt::format("lcc,{},{},{}:{}", *clon, *clat, *lat1, corners);
      return fmt::format("lcc,{},{},{},{}:{}", *clon, *clat, *lat1, *lat2, corners);
    }
    case kNFmiYKJArea:
    {
      return "ykj:" + corners;
    }
    default:
      return AreaFactoryProjStr();
  }
}

std::string NFmiArea::AreaFactoryProjStr() const
{
  // Newbase with wgs84 support doesn't support old style area strings anymore,
  // we build here substitute string with PROJ library's Proj-string and area's corner points.

  auto tl = impl->TopLeftCorner();
  auto br = impl->BottomRightCorner();

  // Note: Must use | instead of : for new projections due to strings like epsg:4326

  return fmt::format("{}|{},{},{},{}", ProjStr(), tl.X(), tl.Y(), br.X(), br.Y());
}

const NFmiRect &NFmiArea::WorldRect() const { return impl->itsWorldRect; }

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
  // theRect is in relative xy coordinates (not in world xy coordinates).
  auto bottomLeftXy = XYToWorldXY(theRect.BottomLeft());
  auto topRightXy = XYToWorldXY(theRect.TopRight());
  return CreateFromBBox(*impl->itsSpatialReference, bottomLeftXy, topRightXy);
}

NFmiArea *NFmiArea::CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const
{
  return CreateFromWGS84Corners(*impl->itsSpatialReference, theBottomLeftLatLon, theTopRightLatLon);
}

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect) const
{
  NFmiPoint newBottomLeftXY = theWorldRect.BottomLeft();
  NFmiPoint newTopRightXY = theWorldRect.TopRight();

  NFmiPoint newBottomLeftLatLon = WorldXYToLatLon(newBottomLeftXY);
  NFmiPoint newTopRightLatLon = WorldXYToLatLon(newTopRightXY);

  if (!IsInside(newBottomLeftLatLon) || !IsInside(newTopRightLatLon)) return nullptr;

  auto *newArea = CreateFromBBox(*impl->itsSpatialReference, newBottomLeftXY, newTopRightXY);

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
  NFmiArea *newArea = CreateFromBBox(
      *impl->itsSpatialReference, newWorldRect.BottomLeft(), newWorldRect.TopRight());

  // Return the re-dimensioned copy of the original area
  return newArea;
}

double NFmiArea::XScale() const { return 1 / impl->itsXScaleFactor; }
double NFmiArea::YScale() const { return 1 / impl->itsYScaleFactor; }

NFmiPoint NFmiArea::SphereToWGS84(const NFmiPoint &theWorldXY)
{
  Fmi::CoordinateTransformation transformation("FMI", "WGS84");
  return transform(transformation, theWorldXY);
}

NFmiPoint NFmiArea::WGS84ToSphere(const NFmiPoint &theLatLon)
{
  Fmi::CoordinateTransformation transformation("WGS84", "FMI");
  return transform(transformation, theLatLon);
}

Fmi::CoordinateMatrix NFmiArea::CoordinateMatrix(std::size_t nx, std::size_t ny, bool wrap) const
{
  auto x1 = impl->itsWorldRect.Left();
  auto x2 = impl->itsWorldRect.Right();
  const auto y1 = impl->itsWorldRect.Top();
  const auto y2 = impl->itsWorldRect.Bottom();

  if (impl->itsFlopped) std::swap(x1, x2);

  if (!wrap) return Fmi::CoordinateMatrix(nx, ny, x1, y1, x2, y2);

  // Add one more column to the right since wrapping is requested. We assume an earlier phase
  // has already checked the data is geographic and global apart from one column.

  const auto dx = (x2 - x1) / (nx - 1);
  return Fmi::CoordinateMatrix(nx + 1, ny, x1, y1, x2 + dx, y2);
}

void NFmiArea::ToLatLon(Fmi::CoordinateMatrix &theMatrix) const
{
  // Transform local xy-coordinates into world xy-coordinates (meters).
  XYToWorldXY(theMatrix);

  // Transform world xy-coordinates into WGS84
  WorldXYToLatLon(theMatrix);

  // Note: No pole fixing as in the code for a single point. Maybe not worth the
  // trouble? Or another algorithm would be better?
}

void NFmiArea::ToXY(Fmi::CoordinateMatrix &theMatrix) const
{
  LatLonToWorldXY(theMatrix);
  WorldXYToXY(theMatrix);
}

void NFmiArea::XYToWorldXY(Fmi::CoordinateMatrix &theMatrix) const
{
  const auto wleft = impl->itsWorldRect.Left();
  const auto wright = impl->itsWorldRect.Right();
  const auto wbottom = impl->itsWorldRect.Bottom();
  const auto left = Left();
  const auto top = Top();
  const auto xscale = impl->itsXScaleFactor;
  const auto yscale = impl->itsYScaleFactor;
  const auto flopped = impl->itsFlopped;

  // Note: We do not assume x/y are constants for rows/columns even though they most likely are
  double x;
  for (std::size_t j = 0; j < theMatrix.height(); j++)
    for (std::size_t i = 0; i < theMatrix.width(); i++)
    {
      if (!flopped)
        x = wleft + (theMatrix.x(i, j) - left) / xscale;
      else
        x = wright - (theMatrix.y(i, j) - left) / xscale;
      auto y = wbottom - (theMatrix.y(i, j) - top) / yscale;
      theMatrix.set(i, j, x, y);
    }
}

void NFmiArea::WorldXYToXY(Fmi::CoordinateMatrix &theMatrix) const
{
  const auto wleft = impl->itsWorldRect.Left();
  const auto wright = impl->itsWorldRect.Right();
  const auto wbottom = impl->itsWorldRect.Bottom();
  const auto left = Left();
  const auto top = Top();
  const auto xscale = impl->itsXScaleFactor;
  const auto yscale = impl->itsYScaleFactor;
  const auto flopped = impl->itsFlopped;
  // Note: We do not assume x/y are constants for rows/columns even though they most likely are
  double x;
  for (std::size_t j = 0; j < theMatrix.height(); j++)
    for (std::size_t i = 0; i < theMatrix.width(); i++)
    {
      if (!flopped)
        x = left + xscale * (theMatrix.x(i, j) - wleft);
      else
        x = left - xscale * (theMatrix.x(i, j) - wright);
      auto y = top + yscale * (wbottom - theMatrix.y(i, j));
      theMatrix.set(i, j, x, y);
    }
}

void NFmiArea::WorldXYToLatLon(Fmi::CoordinateMatrix &theMatrix) const
{
  if (!impl->itsToLatLonConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  theMatrix.transform(*impl->itsToLatLonConverter);
}

void NFmiArea::LatLonToWorldXY(Fmi::CoordinateMatrix &theMatrix) const
{
  if (!impl->itsToWorldXYConverter)
    throw std::runtime_error("Spatial reference not set for WGS84 conversions");

  theMatrix.transform(*impl->itsToWorldXYConverter);
}

void NFmiArea::ToNativeLatLon(Fmi::CoordinateMatrix &theMatrix) const
{
  XYToWorldXY(theMatrix);
  WorldXYToNativeLatLon(theMatrix);
}

void NFmiArea::WorldXYToNativeLatLon(Fmi::CoordinateMatrix &theMatrix) const
{
  if (!impl->itsNativeToLatLonConverter)
    throw std::runtime_error("Spatial reference not set for native latlon conversions");

  theMatrix.transform(*impl->itsNativeToLatLonConverter);
}

void NFmiArea::NativeLatLonToWorldXY(Fmi::CoordinateMatrix &theMatrix) const
{
  if (!impl->itsNativeToWorldXYConverter)
    throw std::runtime_error("Spatial reference not set for native latlon conversions");

  theMatrix.transform(*impl->itsNativeToWorldXYConverter);
}

void NFmiArea::NativeToXY(Fmi::CoordinateMatrix &theMatrix) const
{
  NativeLatLonToWorldXY(theMatrix);
  WorldXYToXY(theMatrix);
}

void NFmiArea::SetGridSize(std::size_t theWidth, std::size_t theHeight)
{
  if (theWidth < 2 || theHeight < 2)
    throw std::runtime_error("NFmiArea SetGridSize arguments must be at least 2");

  // Establish WorldXY to LatLon bilinear conversion
  double x1 = impl->itsWorldRect.Left();
  double y1 = impl->itsWorldRect.Top();  // upside down
  double x2 = impl->itsWorldRect.Right();
  double y2 = impl->itsWorldRect.Bottom();

  impl->itsToLatLonBilinearConverter.reset(new Fmi::BilinearCoordinateTransformation(
      *impl->itsToLatLonConverter, theWidth, theHeight, x1, y1, x2, y2));

  // Establish LatLon to WorldXY bilinear conversion by looking for the grid bbox

  // TODO: Create a LatLonBBox cache to optimize this step

  const auto big_value = 1e6;

  double lon1 = +big_value;
  double lat1 = +big_value;
  double lon2 = -big_value;
  double lat2 = -big_value;

  const auto &matrix = impl->itsToLatLonBilinearConverter->coordinateMatrix();

  for (std::size_t j = 0; j < theHeight; ++j)
    for (std::size_t i = 0; i < theWidth; ++i)
    {
      double lon = matrix.x(i, j);
      double lat = matrix.y(i, j);
      if (std::isfinite(lon) && std::isfinite(lat))
      {
        if (lon < lon1)
          lon1 = lon;
        else if (lon > lon2)
          lon2 = lon;
        if (lat < lat1)
          lat1 = lat;
        else if (lat > lat2)
          lat2 = lat;
      }
    }

  if (lon1 != +big_value && lat1 != +big_value)
  {
    // lon2,lat2 must be valid too
    impl->itsToWorldXYBilinearConverter.reset(new Fmi::BilinearCoordinateTransformation(
        *impl->itsToWorldXYConverter, theWidth, theHeight, lon1, lat1, lon2, lat2));
  }
}

#endif
