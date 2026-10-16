// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiAreaFactory
 */
// ======================================================================
/*!
 * \namespace NFmiAreaFactory
 *
 * \brief Contains tools for creation NFmiArea objects
 *
 * The NFmiAreaFactory contains functions for creating NFmiArea
 * objects.
 *
 * The Create function takes as input a string describing the
 * desired projection, parses the description and returns
 * the desired area.
 *
 * The generic form for the string description is
 * \code
 * projection:area:grid
 * \endcode
 *
 * The possible values for the \c projection part are
 * \code
 * latlon
 * ykj
 * pkj
 * mercator
 * webmercator
 * rotlatlon,polelatitude=-90,polelongitude=0
 * invrotlatlon,polelatitude=-90,polelongitude=0
 * orthographic,azimuth=0
 * stereographic,centrallongitude=0,centrallatitude=90,truelatitude=60
 * lambertequal,centrallongitude=10,centrallatitude=0,truelatitude=52
 * lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
 * gnomonic,centrallongitude=0,centrallatitude=90,truelatitude=60
 * equidist,centrallongitude=0,centrallatitude=90
 * \endcode
 * The default values for optional parameters are as indicated above.
 * The \c invrotlatlon projection is a convenience id for creating
 * the \c rotlatlon projection based on rotated coordinates.
 *
 * The possible values for the \c area part are
 * \code
 * x1,y1,x2,y2
 * cx,cy,scale
 * cx,cy,scale/aspect
 * \endcode
 * where the first form requires the bottom left and top right
 * longitude and latitude. The latter form requires the center
 * longitude and latitude plus a scale factor.
 *
 * An optional aspect ratio may be used to compress or elongate the
 * Y-axis, which is useful for example for Meteosat images. The aspect
 * ratio is used only when the image center coordinate is defined, otherwise
 * the ratio is meaningless.
 *
 * If the projection is \c invrotlatlon, the latitude and longitude values
 * are measured in \c rotlatlon coordinates.
 *
 * The possible values for the \c grid part are
 * \code
 * x1,y1,x2,y2
 * y1,y2
 * xsize,ysize<units>,  where units may be 'm' or 'km'
 * \endcode
 * Comma may be replaced by "x".
 *
 * Also, the grid part may be omitted entirely, which is assumed
 * to imply values 0,0,1,1. The units version may not be used
 * if the center coordinate & scale are used to define the area.
 *
 * If also the area part is omitted, it is given the default
 * value of "6,51.3,49,70.2" which covers the entire Skandinavia
 * and coincides with the current editor area. This is mainly useful
 * for programs which do not care about XY-coordinates, but only
 * of world coordinates.
 *
 * For example, the following defines the stereographic projection
 * used by the forecasters:
 * \code
 * stereographic,20,90,60:6,51.3,49,70.2
 * \endcode
 * The projection is thus polar stereographic with central longitude 20,
 * true latitude 60, with bottom left corner at (5,51.3) and top right
 * corner at 49,70.2.
 *
 *
 * The CreateProj function takes a string containing a Proj4 projection
 * definition and the corner points of the desired area, and returns
 * the corresponding NFmiArea - object. If the projection described in
 * the Proj4-string is not supported, a runtime_error is thrown.
 *
 * The CreateProj function supports the following FMI-projections:
 * \code
 * latlon
 * ykj
 * mercator
 * orthographic
 * stereographic
 * lambertequal
 * gnomonic
 * equidist
 * \endcode
 *
 */
// ======================================================================

#include "NFmiAreaFactory.h"
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiGnomonicArea.h"
#include "NFmiLambertConformalConicArea.h"
#include "NFmiLambertEqualArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiMercatorArea.h"
#include "NFmiOrthographicArea.h"
#include "NFmiPKJArea.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiStringTools.h"
#include "NFmiTransverseMercatorArea.h"
#include "NFmiWebMercatorArea.h"
#include "NFmiYKJArea.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace
{
double check_longitude(double theLongitude, bool usePacificView)
{
  try
  {
    std::string rangeStr;
    if (usePacificView)
    {
      if (theLongitude >= 0 && theLongitude <= 360)
        return theLongitude;
      rangeStr = "[0,360]";
    }
    else
    {
      if (theLongitude >= -180 && theLongitude <= 180)
        return theLongitude;
      rangeStr = "[-180,180]";
    }
    string msg = "Longitude value";
    msg += NFmiStringTools::Convert(theLongitude);
    msg += " is not in the required range " + rangeStr;
    throw Fmi::Exception(BCP, msg);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double check_latitude(double theLatitude)
{
  try
  {
    if (theLatitude >= -90 && theLatitude <= 90)
      return theLatitude;
    string msg = "Latitude value";
    msg += NFmiStringTools::Convert(theLatitude);
    msg += " is not in the required range [-90,90]";
    throw Fmi::Exception(BCP, msg);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double degrees_from_projparam(const string &inParam)
{
  try
  {
    // Trims the parameter containing proj degrees and returns double.
    size_t length = inParam.size();
    try
    {
      // North
      if (boost::iends_with(inParam, "n"))
      {
        return boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // South
      if (boost::iends_with(inParam, "s"))
      {
        return -boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // East
      if (boost::iends_with(inParam, "e"))
      {
        return boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      // West
      else if (boost::iends_with(inParam, "w"))
      {
        return -boost::lexical_cast<double>(inParam.substr(0, length - 1));
      }
      else
      {
        return boost::lexical_cast<double>(inParam);
      }
    }
    catch (boost::bad_lexical_cast &e)
    {
      string errStr;
      errStr += "Bad cast to double in parameter: ";
      errStr += e.what();
      throw Fmi::Exception(BCP, errStr);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Map a proj-string's ellipsoid to (semi-major axis, inverse flattening). Recognizes the
// common named ellipsoids and explicit +a with +rf / +f / +b; defaults to WGS84, which is
// how FMI treats transverse mercator data (see NFmiTransverseMercatorArea).
std::pair<double, double> ellipsoid_from_projparams(const map<string, string> &projParams)
{
  try
  {
    auto it = projParams.find("ellps");
    if (it != projParams.end())
    {
      const string &e = it->second;
      if (e == "WGS84")
        return {6378137.0, 298.257223563};
      if (e == "GRS80")
        return {6378137.0, 298.257222101};
      if (e == "intl")
        return {6378388.0, 297.0};
    }

    auto ita = projParams.find("a");
    if (ita != projParams.end())
    {
      auto a = NFmiStringTools::Convert<double>(ita->second);
      auto itrf = projParams.find("rf");
      if (itrf != projParams.end())
        return {a, NFmiStringTools::Convert<double>(itrf->second)};
      auto itf = projParams.find("f");
      if (itf != projParams.end())
      {
        auto f = NFmiStringTools::Convert<double>(itf->second);
        if (f != 0.0)
          return {a, 1.0 / f};
      }
      auto itb = projParams.find("b");
      if (itb != projParams.end())
      {
        auto b = NFmiStringTools::Convert<double>(itb->second);
        if (a != b)
          return {a, a / (a - b)};
      }
    }

    return {6378137.0, 298.257223563};  // WGS84
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace NFmiAreaFactory
{
bool DoPossiblePacificFix(NFmiPoint &bottomLeftLatlon, NFmiPoint &topRightLatlon, bool &pacificView)
{
  try
  {
    if (pacificView)
    {
      // Fix top right longitude to Pacific view if it seems possible
      if (topRightLatlon.X() >= -180 && topRightLatlon.X() < 0)
      {
        topRightLatlon.X(topRightLatlon.X() + 360);
        return true;
      }
      if (bottomLeftLatlon.X() < topRightLatlon.X() && bottomLeftLatlon.X() >= 180 &&
          topRightLatlon.X() > 180)
      {  // molemmat nurkkapisteet ovat pacific-alueella (180 - 360), tehdään niistä molemmista
         // atlantisia koordinaatteja
        // koska SmartMet toimii paremmin tälläisissa tilanteissa, jos kyseinen muutos tehdään. En
        // tiedä tarkemmin miksi, koska
        // debug-versio SmartMetista näytti toimivan täysin oikein joka tapauksessa oli data miten
        // hyvänsä.
        bottomLeftLatlon.X(bottomLeftLatlon.X() - 360);
        topRightLatlon.X(topRightLatlon.X() - 360);
        pacificView = false;
        return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Normalize a possible GDAL area prefix FMI{:|} or WGS84{:|} to use a colon
 *
 * NFmiGdalArea's projection string starts with FMI{:|} or WGS84{:|}, and area definition at the
 * end is delimited with pipe, thus resulting pipe to be used as the separator below. Replace
 * possible pipe delimiter at the start with colon, because parsing must see the projection part
 * upto the area as one comma separated string.
 *
 * FMI:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161
 * WGS84:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161
 *
 * \return True if the projection is a GDAL area
 */
// ----------------------------------------------------------------------

bool normalize_gdal_prefix(string &projection)
{
#ifndef DISABLED_GDAL
  if ((projection.substr(0, 4) == "FMI:") || (projection.substr(0, 6) == "WGS84:") ||
      (projection.substr(0, 4) == "FMI|") || (projection.substr(0, 6) == "WGS84|"))
  {
    if (projection.substr(0, 3) == "FMI")
      projection = "FMI:" + projection.substr(4);
    else
      projection = "WGS84:" + projection.substr(6);

    return true;
  }
#endif
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract possible length units from the grid specification
 */
// ----------------------------------------------------------------------

void extract_grid_units(string &theGridSpec, bool &units, double &unitfactor)
{
  // Allow both NxM and N,M
  std::replace(theGridSpec.begin(), theGridSpec.end(), 'x', ',');

  if (boost::iends_with(theGridSpec, "km"))
  {
    units = true;
    unitfactor = 1000;
    theGridSpec = theGridSpec.substr(0, theGridSpec.size() - 2);
  }
  else if (boost::iends_with(theGridSpec, "m"))
  {
    units = true;
    theGridSpec = theGridSpec.substr(0, theGridSpec.size() - 1);
  }
}

// Projection parameter i, or the default value if it is not given
double param_or(const vector<double> &pvec, std::size_t i, double theDefault)
{
  return (pvec.size() > i ? pvec[i] : theDefault);
}

// Throw if any projection parameters are given
void require_no_params(const vector<double> &pvec, const char *theMessage)
{
  if (!pvec.empty())
    throw Fmi::Exception(BCP, theMessage);
}

// Throw if more than the given number of projection parameters are given
void require_max_params(const vector<double> &pvec, std::size_t theMax, const char *theMessage)
{
  if (pvec.size() > theMax)
    throw Fmi::Exception(BCP, theMessage);
}

// The corner points and the grid corners of the area
struct AreaCorners
{
  NFmiPoint bottomleft;
  NFmiPoint topright;
  NFmiPoint corner1;
  NFmiPoint corner2;
  bool usePacificView;
};

// ----------------------------------------------------------------------
/*!
 * \brief Create a rotated latlon area, or an inverse rotated one where the corners are
 *        given in rotated coordinates
 */
// ----------------------------------------------------------------------

return_type create_rotated_latlon_area(const string &proj,
                                       const vector<double> &pvec,
                                       const AreaCorners &c)
{
  if (proj == "rotlatlon")
    require_max_params(pvec, 2, "rotlatlon area requires max 2 parameters");
  else
    require_max_params(pvec, 2, "invrotlatlon area requires max 2 parameters");

  const double pole_lon = check_longitude(param_or(pvec, 1, 0), c.usePacificView);
  const double pole_lat = check_latitude(param_or(pvec, 0, -90));

  if (proj == "rotlatlon")
    return std::make_shared<NFmiRotatedLatLonArea>(c.bottomleft,
                                                   c.topright,
                                                   NFmiPoint(pole_lon, pole_lat),
                                                   c.corner1,
                                                   c.corner2,
                                                   c.usePacificView);

  // Invert the coordinates. Note that only the pole really matters in
  // the following constructor

  NFmiRotatedLatLonArea rot(c.bottomleft, c.topright, NFmiPoint(pole_lon, pole_lat));
  NFmiPoint bl = rot.ToRegLatLon(c.bottomleft);
  NFmiPoint tr = rot.ToRegLatLon(c.topright);

  return std::make_shared<NFmiRotatedLatLonArea>(
      bl, tr, NFmiPoint(pole_lon, pole_lat), c.corner1, c.corner2, c.usePacificView);
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a lambert conformal conic area
 *
 * lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
 */
// ----------------------------------------------------------------------

return_type create_lcc_area(const vector<double> &pvec, const AreaCorners &c)
{
  if (pvec.size() < 3 || pvec.size() > 5)
    throw Fmi::Exception(BCP, "lcc area requires max 3-5 parameters");
  const double clon = check_longitude(pvec[0], c.usePacificView);
  const double clat = check_latitude(pvec[1]);
  const double tlat1 = check_latitude(pvec[2]);
  const double tlat2 = check_latitude(param_or(pvec, 3, tlat1));
  const double rad = param_or(pvec, 4, kRearth);
  return std::make_shared<NFmiLambertConformalConicArea>(
      c.bottomleft, c.topright, clon, clat, tlat1, tlat2, rad, c.usePacificView);
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a transverse mercator area
 *
 * Legacy form written by NFmiTransverseMercatorArea::AreaStr():
 * tmerc,lon_0,k_0,x_0,y_0,a,1/f:bbox. Omitted parameters default to
 * TM35FIN (EPSG:3067) on the WGS84 ellipsoid.
 */
// ----------------------------------------------------------------------

return_type create_tmerc_legacy_area(const vector<double> &pvec, const AreaCorners &c)
{
  require_max_params(pvec, 6, "tmerc area requires max 6 parameters");
  const double lon0 = check_longitude(param_or(pvec, 0, NFmiGaussKruger::kTM35FIN_CentralMeridian),
                                      c.usePacificView);
  const double k0 = param_or(pvec, 1, NFmiGaussKruger::kTM35FIN_ScaleFactor);
  const double x0 = param_or(pvec, 2, NFmiGaussKruger::kTM35FIN_FalseEasting);
  const double y0 = param_or(pvec, 3, NFmiGaussKruger::kTM35FIN_FalseNorthing);
  const double a = param_or(pvec, 4, NFmiGaussKruger::kWGS84_A);
  const double invf = param_or(pvec, 5, NFmiGaussKruger::kWGS84_InvF);
  // Area strings may come from requests (e.g. download plugin projection=), reject
  // values which would produce an invalid PROJ definition or inf/NaN coordinates
  if (!std::isfinite(k0) || !std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(a) ||
      !std::isfinite(invf) || k0 <= 0 || a <= 0 || invf <= 1)
    throw Fmi::Exception(BCP, "Invalid tmerc area parameters")
        .addParameter("k_0", std::to_string(k0))
        .addParameter("a", std::to_string(a))
        .addParameter("1/f", std::to_string(invf));
  return std::make_shared<NFmiTransverseMercatorArea>(c.bottomleft,
                                                      c.topright,
                                                      lon0,
                                                      k0,
                                                      x0,
                                                      y0,
                                                      a,
                                                      invf,
                                                      c.corner1,
                                                      c.corner2,
                                                      c.usePacificView);
}

// ----------------------------------------------------------------------
/*!
 * \brief Create an area for an unknown legacy name, which must then be a GDAL projection
 */
// ----------------------------------------------------------------------

return_type create_other_area(string proj, const AreaCorners &c, const string &theProjection)
{
#ifdef UNIX
#ifndef DISABLED_GDAL
  // Allow FMI: or WGS84: prefix to identify datum, default is WGS84
  std::string datum = "WGS84";
  if (proj.substr(0, 4) == "FMI:")
  {
    datum = "FMI";
    proj = proj.substr(4, std::string::npos);
  }
  else if (proj.substr(0, 6) == "WGS84:")
  {
    proj = proj.substr(6, std::string::npos);
  }

  return std::make_shared<NFmiGdalArea>(
      datum, proj, c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
#else
  throw Fmi::Exception(BCP, "Unknown projection (GDAL not linked in): " + theProjection);
#endif  // DISABLED_GDAL
#else
  throw Fmi::Exception(BCP, "Unknown projection: " + theProjection);
#endif  // UNIX
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the area for the given projection name and parameters
 */
// ----------------------------------------------------------------------

return_type create_named_area(const string &proj,
                              const vector<double> &pvec,
                              const AreaCorners &c,
                              const string &theProjection)
{
  if (proj == "latlon")
  {
    require_no_params(pvec, "latlon area does not require any parameters");
    return std::make_shared<NFmiLatLonArea>(
        c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "ykj")
  {
    require_no_params(pvec, "ykj area does not require any parameters");
    return std::make_shared<NFmiYKJArea>(
        c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "pkj")
  {
    require_no_params(pvec, "pkj area does not require any parameters");
    return std::make_shared<NFmiPKJArea>(
        c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "mercator")
  {
    require_no_params(pvec, "mercator area requires no parameters");
    return std::make_shared<NFmiMercatorArea>(
        c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "webmercator")
  {
    require_no_params(pvec, "webmercator area requires no parameters");
    return std::make_shared<NFmiWebMercatorArea>(
        c.bottomleft, c.topright, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "rotlatlon" || proj == "invrotlatlon")
    return create_rotated_latlon_area(proj, pvec, c);
  if (proj == "orthographic")
  {
    require_max_params(pvec, 1, "orthographi area requires max 1 parameter");
    const double azimuth = check_longitude(param_or(pvec, 0, 0), c.usePacificView);
    return std::make_shared<NFmiOrthographicArea>(
        c.bottomleft, c.topright, azimuth, c.corner1, c.corner2, c.usePacificView);
  }
  if (proj == "stereographic")
  {
    require_max_params(pvec, 3, "stereographic area requires max 3 parameters");
    const double clon = check_longitude(param_or(pvec, 0, 0), c.usePacificView);
    const double clat = check_latitude(param_or(pvec, 1, 90));
    const double tlat = check_latitude(param_or(pvec, 2, 60));
    return std::make_shared<NFmiStereographicArea>(
        c.bottomleft, c.topright, clon, c.corner1, c.corner2, clat, tlat, c.usePacificView);
  }
  if (proj == "lambertequal")
  {
    require_max_params(pvec, 3, "lambertequal area requires max 3 parameters");
    const double clon = check_longitude(param_or(pvec, 0, 0), c.usePacificView);
    const double clat = check_latitude(param_or(pvec, 1, 90));
    const double tlat = check_latitude(param_or(pvec, 2, 60));
    return std::make_shared<NFmiLambertEqualArea>(
        c.bottomleft, c.topright, clon, c.corner1, c.corner2, clat, tlat, c.usePacificView);
  }
  if (proj == "lcc")
    return create_lcc_area(pvec, c);
  if (proj == "gnomonic")
  {
    require_max_params(pvec, 3, "gnomonic area requires max 3 parameters");
    const double clon = check_longitude(param_or(pvec, 0, 0), c.usePacificView);
    const double clat = check_latitude(param_or(pvec, 1, 90));
    const double tlat = check_latitude(param_or(pvec, 2, 60));
    return std::make_shared<NFmiGnomonicArea>(
        c.bottomleft, c.topright, clon, c.corner1, c.corner2, clat, tlat, c.usePacificView);
  }
  if (proj == "equidist")
  {
    require_max_params(pvec, 2, "equidist area requires max 2 parameters");
    const double clon = check_longitude(param_or(pvec, 0, 0), c.usePacificView);
    const double clat = check_latitude(param_or(pvec, 1, 90));
    return std::make_shared<NFmiEquidistArea>(
        c.bottomleft, c.topright, clon, c.corner1, c.corner2, clat, c.usePacificView);
  }
  if (proj == "tmerc")
    return create_tmerc_legacy_area(pvec, c);

  return create_other_area(proj, c, theProjection);
}

// ----------------------------------------------------------------------
/*!
 * \brief Recalculate the corners if the center coordinate was given instead of the corners,
 *        or the grid size was given in length units or with a negative dimension
 */
// ----------------------------------------------------------------------

void adjust_area_grid(return_type &area,
                      const string &proj,
                      const NFmiPoint &center,
                      bool centered,
                      double scale,
                      double aspect,
                      const deque<double> &gvec,
                      bool units,
                      double unitfactor)
{
  const double width = gvec[2] - gvec[0];
  const double height = gvec[3] - gvec[1];

  if (units && (width <= 0 || height <= 0))
    throw Fmi::Exception(BCP, "Cannot use negative lengths when specifying the grid size");

  if (centered)
  {
    if (width <= 0 || height <= 0)
      throw Fmi::Exception(BCP,
                           "Width and height must be positive when center coordinate is given");

    const NFmiPoint c = area->LatLonToWorldXY(center);
    const NFmiPoint bl(c.X() - scale * 1000 * width, c.Y() - scale * 1000 * aspect * height);
    const NFmiPoint tr(c.X() + scale * 1000 * width, c.Y() + scale * 1000 * aspect * height);
    NFmiPoint BL = area->WorldXYToLatLon(bl);
    NFmiPoint TR = area->WorldXYToLatLon(tr);

    if (proj == "invrotlatlon")
    {
      BL = (static_cast<NFmiRotatedLatLonArea *>(area.get()))->ToRegLatLon(BL);
      TR = (static_cast<NFmiRotatedLatLonArea *>(area.get()))->ToRegLatLon(TR);
    }

    area.reset(area->NewArea(BL, TR));
  }
  else if (width < 0 && height > 0)
  {
    // calculate width to preserve aspect ratio
    double ratio = area->WorldXYAspectRatio();
    double w = round(ratio * height);
    NFmiRect rect(gvec[0], gvec[1], gvec[0] + w, gvec[3]);
    area->SetXYArea(rect);
  }
  else if (height < 0 && width > 0)
  {
    // calculate height to preserve aspect ratio
    double ratio = area->WorldXYAspectRatio();
    double h = round(width / ratio);
    NFmiRect rect(gvec[0], gvec[1], gvec[2], gvec[1] + h);
    area->SetXYArea(rect);
  }
  else if (width < 0 && height < 0)
    throw Fmi::Exception(BCP, "Width and height cannot both be negative");
  else if (units)
  {
    double w = area->WorldXYWidth();
    double h = area->WorldXYHeight();
    auto xsize = static_cast<int>(round(w / (unitfactor * width)));
    auto ysize = static_cast<int>(round(h / (unitfactor * height)));
    NFmiRect rect(0, 0, xsize, ysize);
    area->SetXYArea(rect);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Validate the parsed parts of a projection specification
 */
// ----------------------------------------------------------------------

void validate_area_spec(const list<string> &pparts,
                        const vector<double> &avec,
                        const deque<double> &gvec,
                        bool units)
{
  if (pparts.empty())
    throw Fmi::Exception(BCP, "projection part missing");
  if (avec.size() < 3 || avec.size() > 4)
    throw Fmi::Exception(BCP, "area specification must have 3-4 numbers");
  if (gvec.size() != 2 && gvec.size() != 4)
    throw Fmi::Exception(BCP, "grid specification must have 2 or 4 numbers");
  if (gvec.size() != 2 && units)
    throw Fmi::Exception(BCP, "grid specification must have 2 numbers when length units are used");
}
}  // namespace

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection
 *
 * Throws if there is an error in the projection description.
 *
 * \param theProjection String description of the projection
 * \return The created projection
 */
// ----------------------------------------------------------------------

std::shared_ptr<NFmiArea> Create(const std::string &theProjection)
{
  try
  {
    std::shared_ptr<NFmiArea> area;

    string projection = theProjection;
    NFmiStringTools::TrimAll(projection);  // siivotaan mahdolliset etu ja taka white spacet pois
                                           // ettei sotke parserointia

    bool gdalArea = normalize_gdal_prefix(projection);

    const char *separator = ":";
    if (projection.find('|') != std::string::npos)
      separator = "|";

    auto parts = NFmiStringTools::Split<vector<string> >(projection, separator);

    try
    {
      if (parts.size() == 1)
        parts.emplace_back("6,51.3,49,70.2");

      if (parts.empty() || parts.size() > 3)
        throw Fmi::Exception(BCP, "must have 1-3 parts separated by ':' or '|'");

      // extracts the parts separated by ','
      auto pparts = NFmiStringTools::Split<list<string> >(parts[0]);

      // Extract possible units
      bool units = false;
      double unitfactor = 1.0;
      if (parts.size() == 3)
        extract_grid_units(parts[2], units, unitfactor);

      const auto aparts = NFmiStringTools::Split<vector<string> >(parts[1], "/");
      if (aparts.empty())
        throw Fmi::Exception(BCP, "area specification must have 3-4 numbers");
      const auto avec = NFmiStringTools::Split<vector<double> >(aparts[0]);
      double aspect = (aparts.size() == 1 ? 1 : NFmiStringTools::Convert<double>(aparts[1]));

      auto gvec = NFmiStringTools::Split<deque<double> >(parts.size() == 2 ? "0,0,1,1" : parts[2]);

      // intermediate validity checks
      validate_area_spec(pparts, avec, gvec, units);

      string proj = gdalArea ? parts[0] : pparts.front();
      pparts.pop_front();

      vector<double> pvec;

      if (!gdalArea)
        for (auto& ppart : pparts)
          pvec.push_back(NFmiStringTools::Convert<double>(ppart));

      // fixate gvec to size 4

      if (gvec.size() == 2)
      {
        gvec.push_front(0);
        gvec.push_front(0);
      }

      // extract the numeric parts

      const bool centered = (avec.size() == 3);

      NFmiPoint bottomleft = NFmiPoint(avec[0], avec[1]);
      NFmiPoint topright = (centered ? bottomleft : NFmiPoint(avec[2], avec[3]));

      bool usePacificView = NFmiArea::IsPacificView(bottomleft, topright);
      DoPossiblePacificFix(bottomleft, topright, usePacificView);

      check_longitude(bottomleft.X(), usePacificView);
      check_latitude(bottomleft.Y());
      check_longitude(topright.X(), usePacificView);
      check_latitude(topright.Y());

      // More validity checks

      if (units && centered)
        throw Fmi::Exception(
            BCP, "Cannot use a centered projection speficiation with grid size of specific length");

      const AreaCorners corners{bottomleft,
                                topright,
                                NFmiPoint(gvec[0], gvec[1]),
                                NFmiPoint(gvec[2], gvec[3]),
                                usePacificView};

      area = create_named_area(proj, pvec, corners, theProjection);

      // recalculate corners if center coordinate was given
      // instead of corners (bottomleft = center then)

      adjust_area_grid(area,
                       proj,
                       bottomleft,
                       centered,
                       avec[2],
                       aspect,
                       gvec,
                       units,
                       unitfactor);
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP,
                                  "Projection specification '" + theProjection + "' is invalid");
    }

    return area;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
using ProjParams = map<string, string>;

// ----------------------------------------------------------------------
/*!
 * \brief Parse a Proj4 string into a key-value map
 */
// ----------------------------------------------------------------------

ProjParams parse_proj_params(const std::string &projString)
{
  // Map to hold the parameters
  ProjParams projParams;

  // Tokenize the input string
  vector<string> tokens;
  vector<string> splitToken;
  boost::split(tokens, projString, boost::is_any_of(" \t\n"), boost::token_compress_on);

  // Build token map, while validating the proj-string
  for (auto &token : tokens)
  {
    if (token.empty())
      continue;
    // Split token into key->value pairs
    boost::split(splitToken, token, boost::is_any_of("="));

    if (!boost::starts_with(splitToken[0], "+"))
    {
      // Proj parameter keys start with a "+"

      throw Fmi::Exception(BCP, "Proj4 parameters must start with a '+'");
    }

    if (splitToken.size() < 2)
    {
      // Only key but no value, use empty string as value
      projParams.insert(make_pair(splitToken[0].erase(0, 1), string("")));
    }
    else
      projParams.insert(make_pair(splitToken[0].erase(0, 1), splitToken[1]));
  }
  return projParams;
}

// ----------------------------------------------------------------------
/*!
 * \brief Value of a mandatory angle parameter in degrees, throws the given message if missing
 */
// ----------------------------------------------------------------------

double required_degrees(const ProjParams &projParams,
                        set<string> &usedParams,
                        const string &theName,
                        const string &theErrorMessage)
{
  auto map_it = projParams.find(theName);
  if (map_it == projParams.end())
    throw Fmi::Exception(BCP, theErrorMessage);
  double value = degrees_from_projparam(map_it->second);
  usedParams.insert(map_it->first);
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the value of an optional numeric parameter if it is given
 */
// ----------------------------------------------------------------------

void optional_double(const ProjParams &projParams,
                     set<string> &usedParams,
                     const string &theName,
                     double &theValue)
{
  auto map_it = projParams.find(theName);
  if (map_it != projParams.end())
  {
    theValue = NFmiStringTools::Convert<double>(map_it->second);
    usedParams.insert(theName);
  }
}

// Transverse mercator parameters
struct TMercParams
{
  double lon_0 = 0;
  double lat_0 = 0;
  double k0 = 1.0;
  double false_easting = 0;
  double false_northing = 0;
};

// ----------------------------------------------------------------------
/*!
 * \brief Parse UTM projection parameters
 */
// ----------------------------------------------------------------------

TMercParams utm_params(const ProjParams &projParams, set<string> &usedParams)
{
  TMercParams params;
  auto map_it = projParams.find("zone");
  if (map_it == projParams.end())
    throw Fmi::Exception(BCP, "zone must be specified for utm projection");
  auto zone = NFmiStringTools::Convert<double>(map_it->second);
  usedParams.insert("zone");

  params.lon_0 = 6.0 * zone - 183.0;  // UTM zone central meridian
  params.k0 = 0.9996;
  params.false_easting = 500000.0;
  if (projParams.find("south") != projParams.end())
  {
    params.false_northing = 10000000.0;
    usedParams.insert("south");
  }
  return params;
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse transverse mercator projection parameters
 */
// ----------------------------------------------------------------------

TMercParams tmerc_params(const ProjParams &projParams, set<string> &usedParams)
{
  TMercParams params;
  auto map_it = projParams.find("lon_0");
  if (map_it == projParams.end())
    throw Fmi::Exception(BCP, "Central meridian 'lon_0' must be specified for tmerc projection");
  params.lon_0 = degrees_from_projparam(map_it->second);
  usedParams.insert("lon_0");

  map_it = projParams.find("lat_0");
  if (map_it != projParams.end())
  {
    params.lat_0 = degrees_from_projparam(map_it->second);
    usedParams.insert("lat_0");
  }

  map_it = projParams.find("k");
  if (map_it == projParams.end())
    map_it = projParams.find("k_0");
  if (map_it != projParams.end())
  {
    params.k0 = NFmiStringTools::Convert<double>(map_it->second);
    usedParams.insert(map_it->first);
  }

  optional_double(projParams, usedParams, "x_0", params.false_easting);
  optional_double(projParams, usedParams, "y_0", params.false_northing);
  return params;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a transverse mercator area from tmerc or utm parameters
 *
 * Generic ellipsoidal transverse mercator. This detects both YKJ (ellps=intl, k=1,
 * x_0=3500000, lon_0=27) and generic WGS84-based transverse mercator such as
 * ETRS-TM35FIN / EPSG:3067 (and UTM, which PROJ also emits for 3067), building a native
 * NFmiTransverseMercatorArea in every case. No dependency on the deprecated
 * NFmiYKJArea / NFmiKKJArea, and no reliance on a +towgs84 datum shift.
 */
// ----------------------------------------------------------------------

return_type create_tmerc_area(const string &projId,
                              const ProjParams &projParams,
                              set<string> &usedParams,
                              const NFmiPoint &bottomLeftLatLon,
                              const NFmiPoint &topRightLatLon,
                              const NFmiPoint &topLeftXY,
                              const NFmiPoint &bottomRightXY)
{
  TMercParams params =
      (projId == "utm" ? utm_params(projParams, usedParams) : tmerc_params(projParams, usedParams));

  // The transverse mercator development used here measures northing from the equator;
  // a latitude of origin other than zero is not supported.
  if (params.lat_0 != 0.0)
    throw Fmi::Exception(BCP, "Only lat_0=0 is supported for transverse mercator projections");

  const auto ellipsoid = ellipsoid_from_projparams(projParams);
  usedParams.insert("ellps");

  return return_type(new NFmiTransverseMercatorArea(bottomLeftLatLon,
                                                    topRightLatLon,
                                                    params.lon_0,
                                                    params.k0,
                                                    params.false_easting,
                                                    params.false_northing,
                                                    ellipsoid.first,
                                                    ellipsoid.second,
                                                    topLeftXY,
                                                    bottomRightXY));
}
}  // namespace

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection from Proj4 string
 *
 * Throws if there is an error in the projection description or when the projection is not supported
 *
 * \param projString String description of the projection
 * \return The created projection
 *
 * The following projections are not supported by the Proj4 factory: NFmiPKJArea,
 * NFmiRotatedLatLonArea, NFmiKKJArea
 */
// ----------------------------------------------------------------------

return_type CreateProj(const std::string &projString,
                       const NFmiPoint &bottomLeftLatLon,
                       const NFmiPoint &topRightLatLon,
                       const NFmiPoint &topLeftXY,
                       const NFmiPoint &bottomRightXY)
{
  try
  {
    // Map to hold the parameters
    ProjParams projParams = parse_proj_params(projString);

    // Map to hold parameters that are actually used in parsing, this can be used to print
    // parameters that are ignored
    set<string> usedParams;

    //	print_parameter_map(projParams);
    // Now parameter map is ready for processing
    auto map_it = projParams.find("proj");
    if (map_it == projParams.end())
    {
      throw Fmi::Exception(BCP, "No projection specified");
    }

    usedParams.insert(map_it->first);
    string projId = map_it->second;

    if (projId == "stere")
    {
      // Stereographic projection, find mandatory parameters

      // Central longitude
      double centralLon = required_degrees(
          projParams,
          usedParams,
          "lon_0",
          "Central meridian 'lon_0' must be specified for stereographic projection");

      // True scale latitude
      double truescaleLat = required_degrees(
          projParams,
          usedParams,
          "lat_ts",
          "True scale latitude 'lat_ts' must be specified for stereographic projection");

      // Center latitude
      double centerLat = required_degrees(
          projParams,
          usedParams,
          "lat_0",
          "Center latitude 'lat_0' must be specified for stereographic projection");

      return return_type(new NFmiStereographicArea(bottomLeftLatLon,
                                                   topRightLatLon,
                                                   centralLon,
                                                   topLeftXY,
                                                   bottomRightXY,
                                                   centerLat,
                                                   truescaleLat));
    }

    if (projId == "aeqd")
    {
      // Azimuthal equidistant projection

      double centerLat = required_degrees(
          projParams,
          usedParams,
          "lat_0",
          "Center latitude 'lat_0' must be specified for azimuthal equidistant projection");

      double centerLon = required_degrees(
          projParams,
          usedParams,
          "lon_0",
          "Center latitude 'lat_0' must be specified for azimuthal equidistant projection");

      return return_type(new NFmiEquidistArea(
          bottomLeftLatLon, topRightLatLon, centerLon, topLeftXY, bottomRightXY, centerLat));
    }

    if ((projId == "latlon") || (projId == "lonlat"))
    {
      // Latlon area

      return return_type(
          new NFmiLatLonArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
    }

    if (projId == "ortho")
    {
      // Orthographic projection
      return return_type(
          new NFmiOrthographicArea(bottomLeftLatLon, topRightLatLon, 0, topLeftXY, bottomRightXY));
    }

    if (projId == "gnom")
    {
      // Gnomonic projection
      double centralLon =
          required_degrees(projParams,
                           usedParams,
                           "lon_0",
                           "Central meridian 'lon_0' must be specified for gnomonic projection");

      // True scale latitude
      double truescaleLat = required_degrees(
          projParams,
          usedParams,
          "lat_ts",
          "True scale latitude 'lat_ts' must be specified for gnomonic projection");

      // Center latitude
      double centerLat =
          required_degrees(projParams,
                           usedParams,
                           "lat_0",
                           "Center latitude 'lat_0' must be specified for gnomonic projection");

      return return_type(new NFmiGnomonicArea(bottomLeftLatLon,
                                              topRightLatLon,
                                              centralLon,
                                              topLeftXY,
                                              bottomRightXY,
                                              centerLat,
                                              truescaleLat));
    }

    if (projId == "merc")
    {
      // Mercator projection
      return return_type(
          new NFmiMercatorArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
    }

    if (projId == "webmerc")
    {
      // WebMercator, newbase supports only +datum=WGS84
      map_it = projParams.find("datum");
      if (map_it == projParams.end() || map_it->second == "WGS84")
        return return_type(
            new NFmiWebMercatorArea(bottomLeftLatLon, topRightLatLon, topLeftXY, bottomRightXY));
      throw Fmi::Exception(BCP, "Datum " + map_it->second + " not supported for WebMercator");
    }

    if (projId == "tmerc" || projId == "utm")
      return create_tmerc_area(projId,
                               projParams,
                               usedParams,
                               bottomLeftLatLon,
                               topRightLatLon,
                               topLeftXY,
                               bottomRightXY);

    string errStr = "Unsupported projection: ";
    errStr += projId;
    throw Fmi::Exception(BCP, errStr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace NFmiAreaFactory

// ======================================================================
