#include "NFmiCoordinateTransformation.h"
#include "NFmiGlobals.h"
#include "NFmiPoint.h"
#include "NFmiSpatialReference.h"
#include <gdal_version.h>
#include <ogr_spatialref.h>

bool must_swap_xy(const OGRSpatialReference& theSR)
{
#if GDAL_MAJOR_VERSION > 1
  return (theSR.EPSGTreatsAsLatLong() || theSR.EPSGTreatsAsNorthingEasting());
#else
  // GDAL1 does not seem to obey EPSGA flags at all
  return false;
#endif
}

NFmiCoordinateTransformation::NFmiCoordinateTransformation(const OGRSpatialReference& theSource,
                                                           const OGRSpatialReference theTarget)
    : itsTransformation(OGRCreateCoordinateTransformation(&theSource, &theTarget)),
      itsInputSwapFlag(must_swap_xy(theSource)),
      itsOutputSwapFlag(must_swap_xy(theTarget))
{
}

NFmiCoordinateTransformation::NFmiCoordinateTransformation(const NFmiSpatialReference& theSource,
                                                           const NFmiSpatialReference& theTarget)
    : itsTransformation(OGRCreateCoordinateTransformation(theSource.get(), theTarget.get())),
      itsInputSwapFlag(must_swap_xy(theSource)),
      itsOutputSwapFlag(must_swap_xy(theTarget))
{
}

bool NFmiCoordinateTransformation::Transform(double& x, double& y) const
{
  if (itsInputSwapFlag) std::swap(x, y);

  bool ok = (itsTransformation->Transform(1, &x, &y) == 1);

  if (itsOutputSwapFlag) std::swap(x, y);
  return ok;
}

bool NFmiCoordinateTransformation::Transform(std::vector<double>& x, std::vector<double>& y) const
{
  if (x.size() != y.size())
    throw std::runtime_error("X- and Y-coordinate vector sizes do not match");

  if (x.empty()) return true;

  if (itsInputSwapFlag) std::swap(x, y);

  int n = static_cast<int>(x.size());

  bool ok = (itsTransformation->Transform(n, &x[0], &y[0]) == n);

  if (itsOutputSwapFlag) std::swap(x, y);
  return ok;
}

bool NFmiCoordinateTransformation::Transform(NFmiPoint& xy) const
{
  if (xy.X() == kFloatMissing || xy.Y() == kFloatMissing) return false;

  double x = xy.X();
  double y = xy.Y();

  if (itsInputSwapFlag) std::swap(x, y);

  bool ok = (itsTransformation->Transform(1, &x, &y) == 1);

  if (itsOutputSwapFlag) std::swap(x, y);

  xy.Set(x, y);

  return ok;
}
