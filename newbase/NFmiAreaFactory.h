// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiAreaFactory
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"
#include <string>

#include <boost/shared_ptr.hpp>

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

}  // namespace NFmiAreaFactory

// ======================================================================
