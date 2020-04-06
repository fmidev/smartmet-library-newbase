#include "NFmiWGS84.h"
#include <gis/SpatialReference.h>
#include <memory>

// ----------------------------------------------------------------------
/*!
 * \brief Implementation details
 */
// ----------------------------------------------------------------------

class NFmiWGS84Impl
{
 public:
  NFmiWGS84Impl() : sr("WGS84") {}

  const Fmi::SpatialReference& SpatialReference() const { return sr; }

 private:
  Fmi::SpatialReference sr;
};

const Fmi::SpatialReference& NFmiWGS84::SpatialReference()
{
  static NFmiWGS84Impl impl;  // thread safe init in C++11
  return impl.SpatialReference();
}
