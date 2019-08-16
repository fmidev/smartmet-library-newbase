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

#include "NFmiArea.h"
#include "NFmiStringTools.h"
#ifndef WGS84
#include "NFmiEquidistArea.h"
#include "NFmiGdalArea.h"
#include "NFmiLambertConformalConicArea.h"
#include "NFmiLatLonArea.h"
#include "NFmiRotatedLatLonArea.h"
#include "NFmiStereographicArea.h"
#include "NFmiYKJArea.h"
#endif
#include <boost/algorithm/string.hpp>
#include <fmt/printf.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using boost::algorithm::starts_with;

namespace
{
#ifndef WGS84
double check_longitude(double theLongitude, bool usePacificView)
{
  std::string rangeStr;
  if (usePacificView)
  {
    if (theLongitude >= 0 && theLongitude <= 360) return theLongitude;
    rangeStr = "[0,360]";
  }
  else
  {
    if (theLongitude >= -180 && theLongitude <= 180) return theLongitude;
    rangeStr = "[-180,180]";
  }
  string msg = "Longitude value";
  msg += NFmiStringTools::Convert(theLongitude);
  msg += " is not in the required range " + rangeStr;
  throw runtime_error(msg);
}
#endif

#ifndef WGS84
double check_latitude(double theLatitude)
{
  if (theLatitude >= -90 && theLatitude <= 90) return theLatitude;
  string msg = "Latitude value";
  msg += NFmiStringTools::Convert(theLatitude);
  msg += " is not in the required range [-90,90]";
  throw runtime_error(msg);
}
#endif

#ifndef WGS84
double degrees_from_projparam(const string &inParam)
{
  // Trims the parameter containing proj degrees and returns double.
  size_t length = inParam.size();
  try
  {
    // North
    if (boost::iends_with(inParam, "n"))
    {
      return std::stod(inParam.substr(0, length - 1));
    }
    // South
    else if (boost::iends_with(inParam, "s"))
    {
      return -std::stod(inParam.substr(0, length - 1));
    }
    // East
    else if (boost::iends_with(inParam, "e"))
    {
      return std::stod(inParam.substr(0, length - 1));
    }
    // West
    else if (boost::iends_with(inParam, "w"))
    {
      return -std::stod(inParam.substr(0, length - 1));
    }
    else
    {
      return std::stod(inParam);
    }
  }
  catch (std::exception &e)
  {
    string errStr;
    errStr += "Bad cast to double in parameter: ";
    errStr += e.what();
    throw runtime_error(errStr);
  }
}
#endif

}  // namespace

namespace NFmiAreaFactory
{
#ifndef WGS84
bool DoPossiblePacificFix(NFmiPoint &bottomLeftLatlon, NFmiPoint &topRightLatlon, bool &pacificView)
{
  if (pacificView)
  {
    // Fix top right longitude to Pacific view if it seems possible
    if (topRightLatlon.X() >= -180 && topRightLatlon.X() < 0)
    {
      topRightLatlon.X(topRightLatlon.X() + 360);
      return true;
    }
    else if (bottomLeftLatlon.X() < topRightLatlon.X() && bottomLeftLatlon.X() >= 180 &&
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
#endif

struct ProjStrings
{
  std::string proj4{};
  std::string sphere{};
};

ProjStrings parse_projection(const std::string &theProjection)
{
  if (theProjection.empty())
    throw std::runtime_error("Cannot construct projection from an empty string");

  ProjStrings result;

  auto projection = theProjection;

  // Initial guess for the spatial reference of the bbox coordinates
  if (starts_with(projection, "FMI:"))
  {
    result.sphere = "FMI";
    projection = projection.substr(4);
  }
  else if (starts_with(projection, "WGS84:"))
  {
    result.sphere = "WGS84";
    projection = projection.substr(6);
  }
  else
    result.sphere = "FMI";  // default

  std::vector<std::string> words;
  boost::algorithm::split(words, projection, boost::is_any_of(","));

  // Extract projection name
  auto name = words[0];

  // Extract projection parameters
  std::vector<double> params;
  for (std::size_t i = 1; i < words.size(); i++)
    params.push_back(std::stod(words[i]));

  if (name == "latlon")
  {
    // for lgeacy reasons "latlon" means "eqc" instead of PROJ.4 "latlon"
    if (params.size() != 0) throw runtime_error("latlon area does not require any parameters");

    result.proj4 =
        fmt::format("+proj=eqc +R={:.0f} +wktext +over +no_defs +towgs84=0,0,0", kRearth);
  }
  else if (name == "rotlatlon")
  {
    if (params.size() > 2) throw runtime_error("rotlatlon area requires max 2 parameters");
    auto spole_lon = (params.size() >= 2 ? params[1] : 0);
    auto spole_lat = (params.size() >= 1 ? params[0] : -90);

    auto npole_lat = -spole_lat;
    auto npole_lon = spole_lon;

    result.proj4 = fmt::format(
        "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=eqc +o_lon_p={} +o_lat_p={} +R={:.0f} "
        "+wktext +over "
        "+towgs84=0,0,0 +no_defs",
        npole_lon,
        npole_lat,
        kRearth);
  }
  else if (name == "invrotlatlon")
  {
    if (params.size() > 2) throw runtime_error("invrotlatlon area requires max 2 parameters");
    auto spole_lon = (params.size() >= 2 ? params[1] : 0);
    auto spole_lat = (params.size() >= 1 ? params[0] : -90);

    auto npole_lat = -spole_lat;
    auto npole_lon = spole_lon;

    result.proj4 = fmt::format(
        "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=eqc +o_lon_p={} +o_lat_p={} +R={:.0f} "
        "+wktext +over "
        "+towgs84=0,0,0 +no_defs",
        npole_lon,
        npole_lat,
        kRearth);
    // the legacy corners are in rotated spherical latlon coordinates
    result.sphere = fmt::format(
        "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=latlong +o_lon_p={} +o_lat_p={} "
        "+R={:.0f} +wktext +over +towgs84=0,0,0 +no_defs",
        npole_lon,
        npole_lat,
        kRearth);
  }
  else if (name == "mercator")
  {
    if (params.size() > 0) throw runtime_error("mercator area requires no parameters");

    result.proj4 = fmt::format("+proj=merc +wktext +over +towgs84=0,0,0 +no_defs");
  }
  else if (name == "stereographic")
  {
    if (params.size() > 3) throw runtime_error("stereographic area requires max 3 parameters");
    const double clon = (params.size() >= 1 ? params[0] : 0);
    const double clat = (params.size() >= 2 ? params[1] : 90);
    const double tlat = (params.size() >= 3 ? params[2] : 60);

    result.proj4 = fmt::format(
        "+proj=stere +lat_0={} +lat_ts={} +lon_0={} +R={:.0f} "
        "+units=m +wktext +towgs84=0,0,0 +no_defs",
        clat,
        tlat,
        clon,
        kRearth);
  }
  else if (name == "gnomonic")
  {
    throw std::runtime_error("gnomonic was never in old style FMI projections");
  }
  else if (name == "lambertequal")
  {
    throw std::runtime_error("lambertequal was never in old style FMI projections");
  }
  else if (name == "ykj")
  {
    if (params.size() != 0) throw runtime_error("ykj area does not require any parameters");
    result.proj4 =
        "+proj=tmerc +lat_0=0 +lon_0=27 +k=1 +x_0=3500000 +y_0=0 +ellps=intl +units=m +wktext "
        "+towgs84=-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964 +no_defs";
    result.sphere = "+proj=longlat +ellps=intl +no_defs";
  }
  else if (name == "lcc")
  {
    // lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
    if (params.size() < 3 || params.size() > 5)
      throw runtime_error("lcc area requires max 3-5 parameters");
    const double clon = params[0];
    const double clat = params[1];
    const double tlat1 = params[2];
    const double tlat2 = (params.size() >= 4 ? params[3] : tlat1);
    const double rad = (params.size() >= 5 ? params[4] : kRearth);

    result.proj4 = fmt::format(
        "+proj=lcc +lat_1={} +lat_2={} +lat_0={} +lon_0={} +x_0=0 +y_0=0 +R={:.0f} "
        "+units=m +wktext +towgs84=0,0,0 +no_defs",
        tlat1,
        tlat2,
        clat,
        clon,
        rad);
    result.sphere = fmt::format("+proj=longlat +R={:.0f} +no_defs +towgs84=0,0,0", rad);
  }
  else if (name == "equidist")
  {
    if (params.size() > 2) throw runtime_error("equidist area requires max 2 parameters");
    const double clon = (params.size() >= 1 ? params[0] : 0);
    const double clat = (params.size() >= 2 ? params[1] : 90);

    result.proj4 = fmt::format(
        "+proj=aeqd +lat_0={} +lon_0={} +x_0=0 +y_0=0 +R={:.0f} +units=m +wktext "
        "+towgs84=0,0,0 +no_defs",
        clat,
        clon,
        kRearth);
  }
  else
  {
    // Assume WKT, PROJ.4 or other string
    result.proj4 = projection;
    if (result.sphere.empty()) result.sphere = "FMI";
  }

  return result;
}

std::string preprocess_projection(const std::string &theProjection)
{
  auto projection = boost::algorithm::trim_copy(theProjection);

  // NFmiGdalArea's projection string starts with FMI{:|} or WGS84{:|}, and area definition at the
  // end is delimited with pipe, thus resulting pipe to be used as the separator below. Replace
  // possible pipe delimiter at the start with colon, because parsing must see the projection part
  // upto the area as one comma separated string.
  //
  // FMI:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161
  // WGS84:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161

  if (starts_with(projection, "FMI:") || starts_with(projection, "WGS84:") ||
      starts_with(projection, "FMI|") || starts_with(projection, "WGS84|"))
  {
    if (projection.substr(0, 3) == "FMI")
      projection[3] = ':';
    else
      projection[5] = ':';
  }

  return projection;
}

std::vector<std::string> split_components(const std::string &theProjection)
{
  auto separator = (theProjection.find('|') != std::string::npos ? "|" : ":");

  std::vector<std::string> words;
  boost::algorithm::split(words, theProjection, boost::is_any_of(separator));

  if (words.size() < 1 || words.size() > 3)
    throw runtime_error("must have 1-3 projection components separated by ':' or '|'");

  return words;
}

struct Bounds
{
  boost::optional<NFmiPoint> bottomleft;
  boost::optional<NFmiPoint> topright;

  boost::optional<NFmiPoint> center;
  double scale = 1;
  double aspect = 1;
};

Bounds parse_bounds(const std::string &theBounds)
{
  if (theBounds.empty()) throw std::runtime_error("Projection bbox/center settings missing");

  Bounds bounds;

  std::vector<std::string> words;
  boost::algorithm::split(words, theBounds, boost::is_any_of("/"));

  if (words.size() < 1 || words.size() > 2)
    throw std::runtime_error("Invalid projection bbox/center setting: " + theBounds);

  if (words.size() == 2) bounds.aspect = std::stod(words[1]);

  // extract x1,y1,x2,y2 or center,width,height
  boost::algorithm::split(words, words[0], boost::is_any_of(","));
  std::vector<double> numbers;
  for (const auto &word : words)
    numbers.push_back(std::stod(word));

  if (numbers.size() == 4)
  {
    bounds.bottomleft = NFmiPoint(numbers[0], numbers[1]);
    bounds.topright = NFmiPoint(numbers[2], numbers[3]);
  }
  else if (numbers.size() == 3)
  {
    bounds.center = NFmiPoint(numbers[0], numbers[1]);
    bounds.scale = numbers[2];
  }
  else
    throw std::runtime_error("Invalid number of elements in bbox/center definition: " + theBounds);

  return bounds;
}

struct Grid
{
  boost::optional<double> units;
  double x1 = 0;
  double y1 = 0;
  double x2 = 0;
  double y2 = 0;

  double width() const { return x2 - x1; }
  double height() const { return y2 - y1; }
};

Grid parse_grid(std::string str)
{
  Grid grid;

  if (boost::iends_with(str, "km"))
  {
    grid.units = 1000;
    str = str.substr(0, str.size() - 2);
  }
  else if (boost::iends_with(str, "m"))
  {
    grid.units = 1;
    str = str.substr(0, str.size() - 1);
  }

  std::vector<std::string> words;
  boost::algorithm::split(words, str, boost::is_any_of(",x"));

  // intermediate validity checks
  if (words.size() != 2 && words.size() != 4)
    throw std::runtime_error("grid specification must have 2 or 4 numbers");
  if (words.size() != 2 && grid.units)
    throw std::runtime_error("grid specification must have 2 numbers when length units are used");

  std::vector<double> numbers;
  for (const auto &word : words)
    numbers.push_back(std::stod(word));

  if (numbers.size() == 2)
  {
    grid.x2 = numbers[0];
    grid.y2 = numbers[1];
  }
  else
  {
    grid.x1 = numbers[0];
    grid.y1 = numbers[1];
    grid.x2 = numbers[2];
    grid.y2 = numbers[3];
  }

  if (grid.units && (grid.width() <= 0 || grid.height() <= 0))
    throw runtime_error("Cannot use negative lengths when specifying the grid size");

  return grid;
}

void set_grid(NFmiArea &theArea, const Grid &theGrid, const Bounds &theBounds)
{
  const NFmiPoint corner1(theGrid.x1, theGrid.y1);
  const NFmiPoint corner2(theGrid.x2, theGrid.y2);

  if (theBounds.center)
    theArea.SetXYArea(NFmiRect(corner1, corner2));
  else if (theGrid.width() < 0 && theGrid.height() > 0)
  {
    // calculate width to preserve aspect ratio
    double scale = theArea.WorldXYAspectRatio();
    double w = round(scale * theGrid.height());
    NFmiRect rect(theGrid.x1, theGrid.y1, theGrid.x1 + w, theGrid.y2);
    theArea.SetXYArea(rect);
  }
  else if (theGrid.height() < 0 && theGrid.width() > 0)
  {
    // calculate height to preserve aspect ratio
    double scale = theArea.WorldXYAspectRatio();
    double h = round(theGrid.width() / scale);
    NFmiRect rect(theGrid.x1, theGrid.y1, theGrid.x2, theGrid.y1 + h);
    theArea.SetXYArea(rect);
  }
  else if (theGrid.width() < 0 && theGrid.height() < 0)
    throw runtime_error("Width and height cannot both be negative");
  else if (!theGrid.units)
    theArea.SetXYArea(NFmiRect(corner1, corner2));
  else
  {
    double w = theArea.WorldXYWidth();
    double h = theArea.WorldXYHeight();

    if (theArea.SpatialReference()->IsGeographic())
    {
      // Haven't figured out how to calculate this. Use "eqc" projection instead.
      throw std::runtime_error("Cropping metric areas from geographic data is not supported");
    }
    else
    {
      auto xsize = static_cast<int>(round(w / (*theGrid.units * theGrid.width())));
      auto ysize = static_cast<int>(round(h / (*theGrid.units * theGrid.height())));
      NFmiRect rect(0, 0, xsize, ysize);
      theArea.SetXYArea(rect);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection
 *
 * Throws if there is an error in the projection description.
 *
 * \param theProjection String description of the projection projection[:area[:grid]]
 * \return The created projection
 */
// ----------------------------------------------------------------------

boost::shared_ptr<NFmiArea> Create(const std::string &theProjection)
{
  try
  {
    string projection = preprocess_projection(theProjection);

    // Extract projection:area:grid parts

    const auto parts = split_components(theProjection);

    const std::string projpart = parts[0];
    const std::string boxpart = (parts.size() > 1 ? parts[1] : "6,51.3,49,70.2");
    const std::string gridpart = (parts.size() > 2 ? parts[2] : "0,1,0,1");

    const ProjStrings projstrings = parse_projection(projpart);
    const Grid grid = parse_grid(gridpart);
    const Bounds bounds = parse_bounds(boxpart);

    // More validity checks

    if (grid.units && bounds.center)
      throw runtime_error(
          "Cannot use a centered projection speficiation with grid size of specific length");

    // Generate PROJ.4 string for the projection and the spatial reference used for the
    // corners, center coordinate or bottom left corner for GRIB stuff

    boost::shared_ptr<NFmiArea> area;

    if (!bounds.center)
      area.reset(NFmiArea::CreateFromCorners(
          projstrings.proj4, projstrings.sphere, *bounds.bottomleft, *bounds.topright));
    else
      // 1000 is used to convert from meters to kilometers. Number 2 is for legacy reasons:
      // due to a bug originally the width meant the width from center to edge. Since the new
      // constructor handles width correctly by adding/substracting the width divided by two,
      // we must multiply by two here to get the original scale.

      area.reset(
          NFmiArea::CreateFromCenter(projstrings.proj4,
                                     projstrings.sphere,
                                     *bounds.center,
                                     2 * 1000 * bounds.scale * grid.width(),
                                     2 * 1000 * bounds.scale * bounds.aspect * grid.height()));

    // Set image area
    set_grid(*area, grid, bounds);

    return area;
  }
  catch (std::runtime_error &e)
  {
    throw runtime_error("Projection specification '" + theProjection + "' is invalid: " + e.what());
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection from corners, default XY bounds
 */
// ----------------------------------------------------------------------

boost::shared_ptr<NFmiArea> CreateFromCorners(const std::string &theProjection,
                                              const NFmiPoint &theBottomLeftLatLon,
                                              const NFmiPoint &theTopRightLatLon)
{
  try
  {
    string projection = preprocess_projection(theProjection);

    const ProjStrings projstrings = parse_projection(projection);

    // Generate PROJ.4 string for the projection and the spatial reference used for the
    // corners, center coordinate or bottom left corner for GRIB stuff

    boost::shared_ptr<NFmiArea> area{NFmiArea::CreateFromCorners(
        projstrings.proj4, projstrings.sphere, theBottomLeftLatLon, theTopRightLatLon)};

    return area;
  }
  catch (std::runtime_error &e)
  {
    throw runtime_error("Projection specification '" + theProjection + "' is invalid: " + e.what());
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection from center and size, default XY bounds
 */
// ----------------------------------------------------------------------

boost::shared_ptr<NFmiArea> CreateFromCenter(const std::string &theProjection,
                                             const NFmiPoint &theCenterLatLon,
                                             double theWidthInMeters,
                                             double theHeightInMeters)
{
  try
  {
    string projection = preprocess_projection(theProjection);

    const ProjStrings projstrings = parse_projection(projection);

    boost::shared_ptr<NFmiArea> area{NFmiArea::CreateFromCenter(projstrings.proj4,
                                                                projstrings.sphere,
                                                                theCenterLatLon,
                                                                theWidthInMeters,
                                                                theHeightInMeters)};

    return area;
  }
  catch (std::runtime_error &e)
  {
    throw runtime_error("Projection specification '" + theProjection + "' is invalid: " + e.what());
  }
}

}  // namespace NFmiAreaFactory

// ======================================================================
