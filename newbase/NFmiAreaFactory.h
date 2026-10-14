// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiAreaFactory
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"
#include <string>

#include <memory>

class NFmiArea;

namespace NFmiAreaFactory
{
using return_type = std::shared_ptr<NFmiArea>;

bool DoPossiblePacificFix(NFmiPoint &bottomLeftLatlon,
                          NFmiPoint &topRightLatlon,
                          bool &pacificView);

return_type Create(const std::string &theProjection);

return_type CreateProj(const std::string& projString,
                       const NFmiPoint& bottomLeftLatLon,
                       const NFmiPoint& topRightLatLon,
                       const NFmiPoint& topLeftXY = NFmiPoint(0.0F, 0.0F),
                       const NFmiPoint& bottomRightXY = NFmiPoint(1.0F, 1.0F));

}  // namespace NFmiAreaFactory

// ======================================================================
