#include "NFmiWGS84.h"
#include <memory>
#include <ogr_spatialref.h>

// ----------------------------------------------------------------------
/*!
 * \brief Implementation details
 */
// ----------------------------------------------------------------------

class NFmiWGS84Impl
{
 public:
  NFmiWGS84Impl() : sr(new OGRSpatialReference)
  {
    auto err = sr->SetFromUserInput("WGS84");
    if (err != OGRERR_NONE)
      throw std::runtime_error("Failed to initialize WGS84 spatial reference");
  }

  OGRSpatialReference* SpatialReference() const { return sr.get(); }

 private:
  std::unique_ptr<OGRSpatialReference> sr;
};

OGRSpatialReference* NFmiWGS84::SpatialReference()
{
  static NFmiWGS84Impl impl;  // thread safe init in C++11
  return impl.SpatialReference();
}
