// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiAreaTools
 */
// ======================================================================
/*!
 * \namespace NFmiAreaTools
 *
 * \brief Various utility functions to handle NFmiArea objects
 *
 */
// ======================================================================

#include "NFmiAreaTools.h"
#include "NFmiArea.h"
#include "NFmiPoint.h"
#include <fmt/format.h>
#include <algorithm>

#include <iostream>

// Local utility functions

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Update bounding box based on given point
 *
 * \param thePoint The point to add to the bounding box
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void update_bbox(const NFmiPoint& thePoint,
                 double& theMinLon,
                 double& theMinLat,
                 double& theMaxLon,
                 double& theMaxLat)
{
  theMinLon = std::min(theMinLon, thePoint.X());
  theMinLat = std::min(theMinLat, thePoint.Y());
  theMaxLon = std::max(theMaxLon, thePoint.X());
  theMaxLat = std::max(theMaxLat, thePoint.Y());
}

}  // namespace

namespace NFmiAreaTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Find geographic bounding box for given area
 *
 * The bounding box is found by traversing the edges of the area
 * and converting the coordinates to geographic ones for extrema
 * calculations.
 *
 * \param theArea The area
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void LatLonBoundingBox(const NFmiArea& theArea,
                       double& theMinLon,
                       double& theMinLat,
                       double& theMaxLon,
                       double& theMaxLat)
{
  // Good initial values are obtained from the corners

  theMinLon = theArea.TopLeftLatLon().X();
  theMinLat = theArea.TopLeftLatLon().Y();
  theMaxLon = theMinLon;
  theMaxLat = theMinLat;

  const unsigned int divisions = 500;

  // Go through the top edge

  const double left = theArea.Left();
  const double right = theArea.Right();
  const double bottom = theArea.Bottom();
  const double top = theArea.Top();
  const double width = right - left;
  const double height = bottom - top;

  // Go through the top edge

  unsigned int i = 0;
  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left + width * i / divisions, top);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the bottom edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left + width * i / divisions, bottom);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the left edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left, top + height * i / divisions);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the right edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(right, top + height * i / divisions);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }
}

NFmiArea* CreateLegacyLatLonArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  auto proj = fmt::format("+proj=eqc +R={:.0f} +wktext +over +no_defs +towgs84=0,0,0", kRearth);
  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyRotatedLatLonArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        const NFmiPoint& theSouthPole)
{
  auto npole_lat = -theSouthPole.Y();  // reflect the pole
  auto npole_lon = fmod(180 + theSouthPole.X(), 360);

  auto proj = fmt::format(
      "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=eqc +o_lon_p={} +o_lat_p={} "
      "+R={:.0f} +wktext +over +towgs84=0,0,0 +no_defs",
      npole_lon,
      npole_lat,
      kRearth);

  // the legacy corners are in rotated spherical latlon coordinates
  auto sphere = fmt::format(
      "+to_meter=.0174532925199433 +proj=ob_tran +o_proj=longlat +o_lon_p={} +o_lat_p={} "
      "+R={:.0f} +wktext +over +towgs84=0,0,0 +no_defs",
      npole_lon,
      npole_lat,
      kRearth);

  return NFmiArea::CreateFromCorners(proj, sphere, theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyStereographicArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        double theCentralLongitude,
                                        double theCentralLatitude,
                                        double theTrueLatitude)
{
  auto proj = fmt::format(
      "+proj=stere +lat_0={} +lat_ts={} +lon_0={} +R={:.0f} +units=m +wktext "
      "+towgs84=0,0,0 +no_defs",
      theCentralLatitude,
      theTrueLatitude,
      theCentralLongitude,
      kRearth);
  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyEquiDistArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude)
{
  auto proj = fmt::format(
      "+proj=aeqd +lat_0={} +lon_0={} +R={:.0f} +units=m +wktext +towgs84=0,0,0 +no_defs",
      theCentralLatitude,
      theCentralLongitude,
      kRearth);

  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyMercatorArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  auto proj = fmt::format("+proj=merc +R={:.0f} +units=m +wktext +towgs84=0,0,0 +no_defs", kRearth);
  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyLambertEqualArea(const NFmiPoint& theBottomLeft,
                                       const NFmiPoint& theTopRight,
                                       double theCentralLongitude,
                                       double theCentralLatitude)
{
  auto proj = fmt::format(
      "+proj=laea +lat_0={} +lon_0={} +R={:.0f} +units=m +wktext +towgs84=0,0,0 "
      "+no_defs",
      theCentralLatitude,
      theCentralLongitude,
      kRearth);
  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyLambertConformalConicArea(const NFmiPoint& theBottomLeft,
                                                const NFmiPoint& theTopRight,
                                                double theCentralLongitude,
                                                double theCentralLatitude,
                                                double theTrueLatitude1,
                                                double theTrueLatitude2,
                                                double theRadius)
{
  auto proj = fmt::format(
      "+proj=lcc +lat_0={} +lon_0={} +lat_1={} +lat_2={} +R={:.0f} +units=m +wktext "
      "+towgs84=0,0,0 +no_defs",
      theCentralLatitude,
      theCentralLongitude,
      theTrueLatitude1,
      theTrueLatitude2,
      theRadius);
  auto sphere = fmt::format("+proj=longlat +R={:.0f} +over +no_defs +towgs84=0,0,0", theRadius);
  return NFmiArea::CreateFromCorners(proj, sphere, theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyGnomonicArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude)
{
  auto proj = fmt::format(
      "+proj=gnom +lat_0={} +lon_0={} +R={:.0f} +units=m +wktext +towgs84=0,0,0 "
      "+no_defs",
      theCentralLatitude,
      theCentralLongitude,
      kRearth);
  return NFmiArea::CreateFromCorners(proj, "FMI", theBottomLeft, theTopRight);
}

NFmiArea* CreateLegacyYKJArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight)
{
  std::string proj =
      "+proj=tmerc +lat_0=0 +lon_0=27 +k=1 +x_0=3500000 +y_0=0 +ellps=intl +units=m +wktext "
      "+towgs84=-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964 +no_defs";

  std::string sphere = "+proj=latlong +datum=intl";
  return NFmiArea::CreateFromCorners(proj, sphere, theBottomLeft, theTopRight);
}

}  // namespace NFmiAreaTools
