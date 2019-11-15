// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiAreaTools
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

class NFmiArea;
class NFmiPoint;

namespace NFmiAreaTools
{
void LatLonBoundingBox(const NFmiArea& theArea,
                       double& theMinLon,
                       double& theMinLat,
                       double& theMaxLon,
                       double& theMaxLat);

NFmiArea* CreateLegacyLatLonArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight);
NFmiArea* CreateLegacyRotatedLatLonArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        const NFmiPoint& theSouthPole);
NFmiArea* CreateLegacyStereographicArea(const NFmiPoint& theBottomLeft,
                                        const NFmiPoint& theTopRight,
                                        double theCentralLongitude,
                                        double theCentralLatitude,
                                        double theTrueLatitude);
NFmiArea* CreateLegacyEquiDistArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude);
NFmiArea* CreateLegacyMercatorArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight);
NFmiArea* CreateLegacyLambertEqualArea(const NFmiPoint& theBottomLeft,
                                       const NFmiPoint& theTopRight,
                                       double theCentralLongitude,
                                       double theCentralLatitude);
NFmiArea* CreateLegacyLambertConformalConicArea(const NFmiPoint& theBottomLeft,
                                                const NFmiPoint& theTopRight,
                                                double theCentralLongitude,
                                                double theCentralLatitude,
                                                double theTrueLatitude1,
                                                double theTrueLatitude2,
                                                double theRadius = kRearth);
NFmiArea* CreateLegacyGnomonicArea(const NFmiPoint& theBottomLeft,
                                   const NFmiPoint& theTopRight,
                                   double theCentralLongitude,
                                   double theCentralLatitude);
NFmiArea* CreateLegacyYKJArea(const NFmiPoint& theBottomLeft, const NFmiPoint& theTopRight);

}  // namespace NFmiAreaTools

// ======================================================================
