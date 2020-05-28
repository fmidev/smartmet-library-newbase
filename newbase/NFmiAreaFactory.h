// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiAreaFactory
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"

#include <boost/shared_ptr.hpp>

#include <string>

class NFmiArea;

namespace NFmiAreaFactory
{
typedef boost::shared_ptr<NFmiArea> return_type;

#ifndef WGS84
bool DoPossiblePacificFix(NFmiPoint &bottomLeftLatlon,
                          NFmiPoint &topRightLatlon,
                          bool &pacificView);
#endif

return_type Create(const std::string &theProjection);

return_type CreateFromCorners(const std::string &theProjection,
                              const NFmiPoint &theBottomLeftLatLon,
                              const NFmiPoint &theTopRightLatLon);

return_type CreateFromCenter(const std::string &theProjection,
                             const NFmiPoint &theCenterLatLon,
                             double theWidthInMeters,
                             double theHeightInMeters);

}  // namespace NFmiAreaFactory

// ======================================================================
