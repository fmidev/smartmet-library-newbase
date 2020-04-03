#include "NFmiCoordinateTransformation.h"
#include "NFmiGlobals.h"
#include "NFmiPoint.h"
#include "NFmiSpatialReference.h"
#include <gdal_version.h>
#include <limits>
#include <ogr_spatialref.h>

bool is_axis_swapped(const OGRSpatialReference& sr)
{
#if GDAL_MAJOR_VERSION > 1
  return (itsSR->EPSGTreatsAsLatLong() || itsSR->EPSGTreatsAsNorthingEasting());
#else
  // GDAL1 does not seem to obey EPSGA flags at all
  return false;
#endif
}

NFmiCoordinateTransformation::NFmiCoordinateTransformation(const NFmiSpatialReference& theSource,
                                                           const NFmiSpatialReference& theTarget)
    : itsTransformation(OGRCreateCoordinateTransformation(theSource.get(), theTarget.get())),
      itsInputSwapFlag(theSource.IsAxisSwapped()),
      itsOutputSwapFlag(theTarget.IsAxisSwapped())
{
}

NFmiCoordinateTransformation::NFmiCoordinateTransformation(const OGRSpatialReference& theSource,
                                                           const OGRSpatialReference& theTarget)
    : itsTransformation(OGRCreateCoordinateTransformation(&theSource, &theTarget)),
      itsInputSwapFlag(is_axis_swapped(theSource)),
      itsOutputSwapFlag(is_axis_swapped(theTarget))
{
}

bool NFmiCoordinateTransformation::Transform(double& x, double& y) const
{
  if (itsInputSwapFlag) std::swap(x, y);

  bool ok = (itsTransformation->Transform(1, &x, &y) != 0);

  if (!ok)
  {
    x = std::numeric_limits<double>::quiet_NaN();
    y = x;
    return false;
  }

  if (itsOutputSwapFlag) std::swap(x, y);
  return true;
}

bool NFmiCoordinateTransformation::Transform(std::vector<double>& x, std::vector<double>& y) const
{
  if (x.size() != y.size())
    throw std::runtime_error("X- and Y-coordinate vector sizes do not match");

  if (x.empty())
    throw std::runtime_error(
        "Cannot do coordinate transformation for empty X- and Y-coordinate vectors");

  if (itsInputSwapFlag) std::swap(x, y);

  int n = static_cast<int>(x.size());
  std::vector<int> flags(n, 0);

  bool ok = (itsTransformation->Transform(n, &x[0], &y[0], nullptr, &flags[0]) != 0);

  if (itsOutputSwapFlag) std::swap(x, y);

  for (std::size_t i = 0; i < flags.size(); i++)
  {
    if (flags[i] == 0)
    {
      x[i] = std::numeric_limits<double>::quiet_NaN();
      y[i] = x[i];
    }
  }

  return ok;
}

bool NFmiCoordinateTransformation::Transform(NFmiPoint& xy) const
{
  double x = xy.X();
  double y = xy.Y();

  if (itsInputSwapFlag) std::swap(x, y);

  bool ok = (xy.X() != kFloatMissing && xy.Y() != kFloatMissing);  // return NaN for kFloatMissing

  if (ok) ok = (itsTransformation->Transform(1, &x, &y) != 0);

  if (!ok)
  {
    x = std::numeric_limits<double>::quiet_NaN();
    y = x;
  }
  else if (itsOutputSwapFlag)
    std::swap(x, y);
  xy.Set(x, y);
  return ok;
}

const OGRSpatialReference& NFmiCoordinateTransformation::GetSourceCS() const
{
  return *itsTransformation->GetSourceCS();
}

const OGRSpatialReference& NFmiCoordinateTransformation::GetTargetCS() const
{
  return *itsTransformation->GetTargetCS();
}

const OGRCoordinateTransformation& NFmiCoordinateTransformation::operator*() const
{
  return *itsTransformation;
}

const OGRCoordinateTransformation* NFmiCoordinateTransformation::get() const
{
  return itsTransformation.get();
}
