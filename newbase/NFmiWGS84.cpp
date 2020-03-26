#include "NFmiWGS84.h"
#include "NFmiSpatialReference.h"
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

  const NFmiSpatialReference& SpatialReference() const { return sr; }

 private:
  NFmiSpatialReference sr;
};

const NFmiSpatialReference& NFmiWGS84::SpatialReference()
{
  static NFmiWGS84Impl impl;  // thread safe init in C++11
  return impl.SpatialReference();
}
