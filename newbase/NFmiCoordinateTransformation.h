#pragma once

#include "NFmiDef.h"
#include <memory>
#include <vector>

class OGRSpatialReference;
class OGRCoordinateTransformation;
class NFmiSpatialReference;
class NFmiPoint;

class _FMI_DLL NFmiCoordinateTransformation
{
 public:
  ~NFmiCoordinateTransformation() = default;
  NFmiCoordinateTransformation() = delete;

  // Directly equivalent construction
  NFmiCoordinateTransformation(const OGRSpatialReference& theSource,
                               const OGRSpatialReference theTarget);

  // Implicit construction from strings, etc
  NFmiCoordinateTransformation(const NFmiSpatialReference& theSource,
                               const NFmiSpatialReference& theTarget);

  bool Transform(double& x, double& y) const;
  bool Transform(std::vector<double>& x, std::vector<double>& y) const;
  bool Transform(NFmiPoint& xy) const;

 private:
  std::unique_ptr<OGRCoordinateTransformation> itsTransformation;

  bool itsInputSwapFlag = false;   // swap xy before calling GDAL?
  bool itsOutputSwapFlag = false;  // swap xy after calling GDAL?

};  // class NFmiCoordinateTransformation
