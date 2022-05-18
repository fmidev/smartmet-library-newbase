// ======================================================================
/*!
 * \file NFmiArea.h
 * \brief Interface of class NFmiArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiDef.h"
#include "NFmiRect.h"
#include <boost/shared_ptr.hpp>
#include <gis/SpatialReference.h>
#include <memory>
#include <string>

// Note: We could forward declare Fmi::SpatialReference too, but then all code using this or
// NFmiAreaTools would have to include Fmi::SpatialReference.h to enable implicit conversion of
// strings such as "WGS84" into actual spatial references. Having the include here is a smaller
// nuisance.

namespace Fmi
{
class CoordinateMatrix;
class ProjInfo;
class CoordinateTransformation;
}  // namespace Fmi

//! Undocumented
class NFmiArea
{
 public:
  // These are defined since implementation is hidden
  ~NFmiArea();
  NFmiArea(const NFmiArea &theArea);
  NFmiArea &operator=(const NFmiArea &theArea);

  // Needed for reading legacy classes from a file. An immediate Read() call is expected.
  NFmiArea(int theClassId);

  // Set the grid size used for calculating the bilinear interpolation matrices
  void SetGridSize(std::size_t theWidth, std::size_t theHeight);

  bool IsInside(const NFmiPoint &theLatLonPoint) const;
  bool IsInside(const NFmiArea &theArea) const;
  bool IsInsideWorldXY(const NFmiPoint &theWorldXYPoint) const;

  NFmiPoint LatLon(unsigned long i, unsigned long j) const;

  NFmiPoint TopLeftLatLon() const;
  NFmiPoint TopRightLatLon() const;
  NFmiPoint BottomLeftLatLon() const;
  NFmiPoint BottomRightLatLon() const;
  NFmiPoint CenterLatLon() const;

  NFmiPoint TopLeft() const;
  NFmiPoint BottomRight() const;
  NFmiPoint TopRight() const;
  NFmiPoint BottomLeft() const;

  void Place(const NFmiPoint &newPlace);
  void Size(const NFmiPoint &newSize);

  double Top() const;
  double Bottom() const;
  double Left() const;
  double Right() const;
  double Height() const;
  double Width() const;

  const NFmiRect &XYArea() const;
  void SetXYArea(const NFmiRect &newArea);
  NFmiRect XYArea(const NFmiArea *theArea) const;

  const NFmiRect &WorldRect() const;

  NFmiPoint WorldXYPlace() const;
  NFmiPoint WorldXYSize() const;
  double WorldXYWidth() const;
  double WorldXYHeight() const;
  double WorldXYAspectRatio() const;

  NFmiAngle TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                             double theLatitudeEpsilon = 0.001) const;

  NFmiArea *Clone() const;

  NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const;
  NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const;
  NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const;
  NFmiPoint WorldXYToXY(const NFmiPoint &theWorldXY) const;
  NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const;
  NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const;

#ifdef WGS84
  NFmiPoint ToNativeLatLon(const NFmiPoint &theXY) const;
  NFmiPoint WorldXYToNativeLatLon(const NFmiPoint &theWorldXY) const;
  NFmiPoint NativeLatLonToWorldXY(const NFmiPoint &theLatLon) const;
  NFmiPoint NativeToXY(const NFmiPoint &theLatLon) const;
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon, const NFmiPoint &theTopRightLatLon) const;
#endif

  NFmiArea *CreateNewArea(const NFmiRect &theRect) const;
  NFmiArea *CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                          const NFmiPoint &theTopRightLatLon) const;
  NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect) const;
  NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                          FmiDirection theFixedPoint,
                          bool fShrinkArea);

#ifdef WGS84
  Fmi::CoordinateMatrix CoordinateMatrix(std::size_t nx, std::size_t ny, bool wrap) const;
  void ToLatLon(Fmi::CoordinateMatrix &theMatrix) const;
  void ToXY(Fmi::CoordinateMatrix &theMatrix) const;
  void XYToWorldXY(Fmi::CoordinateMatrix &theMatrix) const;
  void WorldXYToXY(Fmi::CoordinateMatrix &theMatrix) const;
  void WorldXYToLatLon(Fmi::CoordinateMatrix &theMatrix) const;
  void LatLonToWorldXY(Fmi::CoordinateMatrix &theMatrix) const;

  void ToNativeLatLon(Fmi::CoordinateMatrix &theMatrix) const;
  void WorldXYToNativeLatLon(Fmi::CoordinateMatrix &theMatrix) const;
  void NativeLatLonToWorldXY(Fmi::CoordinateMatrix &theMatrix) const;
  void NativeToXY(Fmi::CoordinateMatrix &theMatrix) const;

  static bool BilinearInterpolationEnabled();
  static void EnableBilinearInterpolation();
  static void DisableBilinearInterpolation();

#endif

  unsigned long ClassId() const;
  const std::string &ClassName() const;
  std::string WKT() const;
  std::string PrettyWKT() const;
  std::string ProjStr() const;
  std::string AreaFactoryStr() const;
  std::string AreaFactoryProjStr() const;

  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);

  bool operator==(const NFmiArea &theArea) const;
  bool operator!=(const NFmiArea &theArea) const;

  // Some legacy pacific functions, that just return false or do nothing.
  // Remove these and their usage, if proven that they are no longer needed with new wgs84 systems.
  bool PacificView_legacy(void) const { return false; }
  void PacificView_legacy(bool) {}
  bool PacificView(void) const { return false; }

  static bool IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon)
  {
    // Obvious case
    if (bottomleftLatlon.X() >= 0 && toprightLatlon.X() < 0)
      return true;
    // 0...360 coordinate system is used
    if (IsPacificLongitude(bottomleftLatlon.X()) || IsPacificLongitude(toprightLatlon.X()))
      return true;
    return false;
  }

  static bool IsPacificLongitude(double theLongitude)
  {
    if (theLongitude > 180 && theLongitude <= 360)
      return true;
    else
      return false;
  }

  std::size_t HashValue() const;

  // Temporary fix until the above method is fixed to be
  std::size_t HashValueKludge() const;

 public:
  const Fmi::SpatialReference &SpatialReference() const;

  // Named constructors used to clarify intent of the parameters. Note that the proxy
  // may accept actual spatial references or strings from which to construct them.

  static NFmiArea *CreateFromBBox(const Fmi::SpatialReference &theSR,
                                  const NFmiPoint &theBottomLeftWorldXY,
                                  const NFmiPoint &theTopRightWorldXY);

  static NFmiArea *CreateFromCenter(const Fmi::SpatialReference &theSR,
                                    const Fmi::SpatialReference &theCenterSR,
                                    const NFmiPoint &theCenterLatLon,
                                    double theWidthInMeters,
                                    double theHeightInMeters);

  static NFmiArea *CreateFromCorners(const Fmi::SpatialReference &theSR,
                                     const Fmi::SpatialReference &theBBoxSR,
                                     const NFmiPoint &theBottomLeftLatLon,
                                     const NFmiPoint &theTopRightLatLon);

  static NFmiArea *CreateFromReverseCorners(const Fmi::SpatialReference &theSR,
                                            const Fmi::SpatialReference &theBBoxSR,
                                            const NFmiPoint &theTopLeftLatLon,
                                            const NFmiPoint &theBottomRightLatLon);

  static NFmiArea *CreateFromWGS84Corners(const Fmi::SpatialReference &theSR,
                                          const NFmiPoint &theBottomLeftLatLon,
                                          const NFmiPoint &theTopRightLatLon);

  static NFmiArea *CreateFromCornerAndSize(const Fmi::SpatialReference &theSR,
                                           const Fmi::SpatialReference &theCornerSR,
                                           const NFmiPoint &theBottomLeftLatLon,
                                           double theWidthInMeters,
                                           double theHeightInMeters);

  const Fmi::ProjInfo &ProjInfo() const;
  int DetectClassId() const;

  void DisableLegacyWrite();

  // from spherical coordinates to WGS84
  static NFmiPoint SphereToWGS84(const NFmiPoint &theWorldXY);
  static NFmiPoint WGS84ToSphere(const NFmiPoint &theLatLon);
  static NFmiPoint TransformCoordinate(const Fmi::CoordinateTransformation &transformation,
                                       const NFmiPoint &coordinate);

  double XScale() const;
  double YScale() const;

 private:
  // We allow only the Create* static methods to construct projections
  NFmiArea();

  void InitSpatialReference(const std::string &theProjection);
  void InitProj();
  void InitRectConversions();
  void CheckRectConversions(double theXScaleFactor, double theYScaleFactor);

  struct Impl;
  std::unique_ptr<Impl> impl;

#ifndef WGS84
 protected:
  int Sign(double theValue) const;
  double FixLongitude(double theLongitude) const;

 public:
  void Init(bool fKeepWorldRect = false);
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const;
#endif
};  // class NFmiArea

inline std::ostream &operator<<(std::ostream &file, const NFmiArea &ob)
{
  return ob.Write(file);
}
inline std::istream &operator>>(std::istream &file, NFmiArea &ob)
{
  return ob.Read(file);
}
