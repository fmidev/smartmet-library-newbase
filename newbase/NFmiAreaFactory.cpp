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

namespace
{
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

double check_latitude(double theLatitude)
{
  if (theLatitude >= -90 && theLatitude <= 90) return theLatitude;
  string msg = "Latitude value";
  msg += NFmiStringTools::Convert(theLatitude);
  msg += " is not in the required range [-90,90]";
  throw runtime_error(msg);
}

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
}  // namespace

namespace NFmiAreaFactory
{
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

boost::shared_ptr<NFmiArea> Create(const std::string &theProjection)
{
  boost::shared_ptr<NFmiArea> area;

  string projection = theProjection;
  NFmiStringTools::TrimAll(
      projection);  // siivotaan mahdolliset etu ja taka white spacet pois ettei sotke parserointia

  // NFmiGdalArea's projection string starts with FMI{:|} or WGS84{:|}, and area definition at the
  // end is delimited with pipe, thus resulting pipe to be used as the separator below. Replace
  // possible pipe delimiter at the start with colon, because parsing must see the projection part
  // upto the area as one comma separated string.
  //
  // FMI:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161
  // WGS84:PROJCS["unnamed",GEOGCS[...|0.237,51.849,49.662,71.161

  bool gdalArea = false;

#ifndef DISABLED_GDAL
  if ((projection.substr(0, 4) == "FMI:") || (projection.substr(0, 6) == "WGS84:") ||
      (projection.substr(0, 4) == "FMI|") || (projection.substr(0, 6) == "WGS84|"))
  {
    if (projection.substr(0, 3) == "FMI")
      projection = "FMI:" + projection.substr(4);
    else
      projection = "WGS84:" + projection.substr(6);

    gdalArea = true;
  }
#endif

  const char *separator = ":";
  if (projection.find('|') != std::string::npos) separator = "|";

  vector<string> parts = NFmiStringTools::Split<vector<string> >(projection, separator);

  try
  {
    if (parts.size() == 1) parts.emplace_back("6,51.3,49,70.2");

    if (parts.size() < 1 || parts.size() > 3)
      throw runtime_error("must have 1-3 parts separated by ':' or '|'");

    // extracts the parts separated by ','
    list<string> pparts = NFmiStringTools::Split<list<string> >(parts[0]);

    // Extract possible units
    bool units = false;
    double unitfactor = 1.0;
    if (parts.size() == 3)
    {
      // Allow both NxM and N,M
      std::replace(parts[2].begin(), parts[2].end(), 'x', ',');

      if (boost::iends_with(parts[2], "km"))
      {
        units = true;
        unitfactor = 1000;
        parts[2] = parts[2].substr(0, parts[2].size() - 2);
      }
      else if (boost::iends_with(parts[2], "m"))
      {
        units = true;
        parts[2] = parts[2].substr(0, parts[2].size() - 1);
      }
    }

    const vector<string> aparts = NFmiStringTools::Split<vector<string> >(parts[1], "/");
    const vector<double> avec = NFmiStringTools::Split<vector<double> >(aparts[0]);

    deque<double> gvec =
        NFmiStringTools::Split<deque<double> >(parts.size() == 2 ? "0,0,1,1" : parts[2]);

    // intermediate validity checks
    if (pparts.size() < 1) throw runtime_error("projection part missing");
    if (avec.size() < 3 || avec.size() > 4)
      throw runtime_error("area specification must have 3-4 numbers");
    if (gvec.size() != 2 && gvec.size() != 4)
      throw runtime_error("grid specification must have 2 or 4 numbers");
    if (gvec.size() != 2 && units)
      throw runtime_error("grid specification must have 2 numbers when length units are used");

    string proj = pparts.front();
    pparts.pop_front();

    vector<double> pvec;

    if (!gdalArea)
      for (list<string>::const_iterator it = pparts.begin(); it != pparts.end(); ++it)
        pvec.push_back(NFmiStringTools::Convert<double>(*it));

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

    const double width = gvec[2] - gvec[0];
    const double height = gvec[3] - gvec[1];

    // More validity checks

    if (units && centered)
      throw runtime_error(
          "Cannot use a centered projection speficiation with grid size of specific length");

    if (units && (width <= 0 || height <= 0))
      throw runtime_error("Cannot use negative lengths when specifying the grid size");

    // Generate PROJ.4 string for the projection and the spatial reference used for the
    // corners, center coordinate or bottom left corner for GRIB stuff

    std::string proj4;

    // FMI legacy sphere is the default, and NFmiArea;;SpatialReferenceProxy supports it
    std::string sphere = "FMI";

    if (proj == "latlon")
    {
      if (pvec.size() != 0) throw runtime_error("latlon area does not require any parameters");

      proj4 = fmt::format("+proj=longlat +a={:.0f} +b={:.0f} +wktext +over +no_defs +towgs84=0,0,0",
                          kRearth,
                          kRearth);
    }
    else if (proj == "rotlatlon")
    {
      if (pvec.size() > 2) throw runtime_error("rotlatlon area requires max 2 parameters");
      auto spole_lon = (pvec.size() >= 2 ? pvec[1] : 0);
      auto spole_lat = (pvec.size() >= 1 ? pvec[0] : -90);

      auto npole_lat = -spole_lat;
      auto npole_lon = (npole_lat == 90 ? 90 : fmod(spole_lon - 180, 360.0));

      proj4 = fmt::format(
          "+proj=ob_tran +o_proj=longlat +o_lon_p={} +o_lat_p={} +a={:.0f} +b={:.0f} +wktext +over "
          "+towgs84=0,0,0 +no_defs",
          npole_lon,
          npole_lat,
          kRearth,
          kRearth);
      sphere = fmt::format(
          "+proj=ob_tran +o_proj=longlat +o_lon_p={} +o_lat_p={} +a={:.0f} +b={:.0f} +over "
          "+no_defs",
          npole_lon,
          npole_lat,
          kRearth,
          kRearth);
    }
    else if (proj == "invrotlatlon")
    {
      if (pvec.size() > 2) throw runtime_error("rotlatlon area requires max 2 parameters");
      auto spole_lon = (pvec.size() >= 2 ? pvec[1] : 0);
      auto spole_lat = (pvec.size() >= 1 ? pvec[0] : -90);

      auto npole_lat = -spole_lat;
      auto npole_lon = (npole_lat == 90 ? 90 : fmod(spole_lon - 180, 360.0));

      proj4 = fmt::format(
          "+proj=ob_tran +o_proj=longlat +o_lon_p={} +o_lat_p={} +a={:.0f} +b={:.0f} +wktext +over "
          "+towgs84=0,0,0 +no_defs",
          npole_lon,
          npole_lat,
          kRearth,
          kRearth);
    }
    else if (proj == "mercator")
    {
      if (pvec.size() > 0) throw runtime_error("mercator area requires no parameters");

      proj4 = fmt::format("+proj=merc +wktext +over +towgs84=0,0,0 +no_defs");
    }
    else if (proj == "stereographic")
    {
      if (pvec.size() > 3) throw runtime_error("stereographic area requires max 3 parameters");
      const double clon = (pvec.size() >= 1 ? pvec[0] : 0);
      const double clat = (pvec.size() >= 2 ? pvec[1] : 90);
      const double tlat = (pvec.size() >= 3 ? pvec[2] : 60);

      proj4 = fmt::format(
          "+proj=stere +lat_0={} +lat_ts={} +lon_0={} +k=1 +x_0=0 +y_0=0 +a={:.0f} +b={:.0f} "
          "+units=m +wktext +towgs84=0,0,0 +no_defs",
          clat,
          tlat,
          clon,
          kRearth,
          kRearth);
    }
    else if (proj == "gnomonic")
    {
      throw std::runtime_error("gnomonic was never in old style FMI projections");
    }
    else if (proj == "lambertequal")
    {
      throw std::runtime_error("lambertequal was never in old style FMI projections");
    }
    else if (proj == "ykj")
    {
      if (pvec.size() != 0) throw runtime_error("ykj area does not require any parameters");
      proj4 =
          "+proj=tmerc +lat_0=0 +lon_0=27 +k=1 +x_0=3500000 +y_0=0 +ellps=intl +units=m +wktext "
          "+towgs84=-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964 +no_defs";
      sphere = "+proj=longlat +ellps=intl +no_defs";
    }
    else if (proj == "lcc")
    {
      // lcc,centrallongitude,centrallatitude,truelatitude1,truelatitude2=truelatitude1,radius=6371220
      if (pvec.size() < 3 || pvec.size() > 5)
        throw runtime_error("lcc area requires max 3-5 parameters");
      const double clon = pvec[0];
      const double clat = pvec[1];
      const double tlat1 = pvec[2];
      const double tlat2 = (pvec.size() >= 4 ? pvec[3] : tlat1);
      const double rad = (pvec.size() >= 5 ? pvec[4] : kRearth);

      proj4 = fmt::format(
          "+proj=lcc +lat_1={} +lat_2={} +lat_0={} +lon_0={} +x_0=0 +y_0=0 +a={:.0f} +b={:.0f} "
          "+units=m +wktext +towgs84=0,0,0 +no_defs",
          tlat1,
          tlat2,
          clat,
          clon,
          rad,
          rad);
      sphere = fmt::format("+proj=longlat +a={:.0f} +b={:.0f} +no_defs", rad, rad);
    }
    else if (proj == "equidist")
    {
      if (pvec.size() > 2) throw runtime_error("equidist area requires max 2 parameters");
      const double clon = (pvec.size() >= 1 ? pvec[0] : 0);
      const double clat = (pvec.size() >= 2 ? pvec[1] : 90);

      proj4 = fmt::format(
          "+proj=aeqd +lat_0={} +lon_0={} +x_0=0 +y_0=0 +a={:.0f} +b={:.0f} +units=m +wktext "
          "+towgs84=0,0,0 +no_defs",
          clat,
          clon,
          kRearth,
          kRearth);
    }
    else
    {
      // Legacy code: Allow FMI: or WGS84: prefix to identify corner coordinate datum

      if (proj.substr(0, 4) == "FMI:")
        proj4 = proj.substr(4, std::string::npos);
      else if (proj.substr(0, 6) == "WGS84:")
      {
        sphere = "WGS84";
        proj4 = proj.substr(6, std::string::npos);
      }
    }

    std::cout << "PROJ: " << proj << "\nSPHERE: " << sphere << std::endl;

    if (centered)
    {
      double aspect = (aparts.size() == 1 ? 1 : NFmiStringTools::Convert<double>(aparts[1]));

      // 1000 is used to convert from meters to kilometers. Number 2 is for legacy reasons:
      // due to a bug originally the width meant the width from center to edge. Since the new
      // constructor handles width correctly by adding/substracting the width divided by two,
      // we must multiply by two here to get the original scale.

      double scale = 2 * 1000 * avec[2];
      area.reset(NFmiArea::CreateFromCenter(
          proj4, sphere, bottomleft, scale * width, scale * aspect * height));
    }
    else
      area.reset(NFmiArea::CreateFromCorners(proj4, sphere, bottomleft, topright));

    // Set image area

    const NFmiPoint corner1(gvec[0], gvec[1]);
    const NFmiPoint corner2(gvec[2], gvec[3]);

    area->SetXYArea(NFmiRect(corner1, corner2));

    if (!centered)
    {
      if (width < 0 && height > 0)
      {
        // calculate width to preserve aspect ratio
        double scale = area->WorldXYAspectRatio();
        double w = round(scale * height);
        NFmiRect rect(gvec[0], gvec[1], gvec[0] + w, gvec[3]);
        area->SetXYArea(rect);
      }
      else if (height < 0 && width > 0)
      {
        // calculate height to preserve aspect ratio
        double scale = area->WorldXYAspectRatio();
        double h = round(width / scale);
        NFmiRect rect(gvec[0], gvec[1], gvec[2], gvec[1] + h);
        area->SetXYArea(rect);
      }
      else if (width < 0 && height < 0)
        throw runtime_error("Width and height cannot both be negative");
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
  }
  catch (std::runtime_error &e)
  {
    throw runtime_error("Projection specification '" + theProjection + "' is invalid: " + e.what());
  }

  return area;
}

// ----------------------------------------------------------------------
/*!
 * \brief Create the desired projection from Proj4 string
 *
 * Throws if there is an error in the projection description or when the projection is not
 * supported
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
  // Assume FMI sphere
  auto sphere = fmt::format("+proj=longlat +a={:.0f} +b={:.0f} +no_defs", kRearth, kRearth);

  return_type area(
      NFmiArea::CreateFromCorners(projString, sphere, bottomLeftLatLon, topRightLatLon));
  area->SetXYArea(NFmiRect(topLeftXY, bottomRightXY));
  return area;
}

}  // namespace NFmiAreaFactory

// ======================================================================
