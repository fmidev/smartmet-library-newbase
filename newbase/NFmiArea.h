// ======================================================================
/*!
 * \file NFmiArea.h
 * \brief Interface of class NFmiArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiProj.h"
#include "NFmiRect.h"
#include "NFmiSaveBaseFactory.h"
#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>

// We must forward declare these or many dependent modules will have to handle
// GDAL includes and linkage, which is annoying with PGDG packages

class OGRCoordinateTransformation;
class OGRSpatialReference;
class NFmiCoordinateMatrix;

//! Undocumented
class _FMI_DLL NFmiArea
{
 public:
  // These are defined since implementation is hidden
  ~NFmiArea();
  NFmiArea(const NFmiArea &theArea);
  NFmiArea &operator=(const NFmiArea &theArea);

  // Needed for reading legacy classes from a file. An immediate Read() call is expected.
  NFmiArea(int theClassId);

  bool IsInside(const NFmiPoint &theLatLonPoint) const;
  bool IsInside(const NFmiArea &theArea) const;

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

  NFmiRect WorldRect() const;

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
  NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect);
  NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                          FmiDirection theFixedPoint,
                          bool fShrinkArea);

#ifdef WGS84
  // 2D matrices of coordinates
  // NFmiCoordinateMatrix WorldXYCoordinateMatrix() const;

  void ToLatLon(NFmiCoordinateMatrix &theMatrix) const;
  void ToXY(NFmiCoordinateMatrix &theMatrix) const;
  void XYToWorldXY(NFmiCoordinateMatrix &theMatrix) const;
  void WorldXYToXY(NFmiCoordinateMatrix &theMatrix) const;
  void WorldXYToLatLon(NFmiCoordinateMatrix &theMatrix) const;
  void LatLonToWorldXY(NFmiCoordinateMatrix &theMatrix) const;

  void ToNativeLatLon(NFmiCoordinateMatrix &theMatrix) const;
  void WorldXYToNativeLatLon(NFmiCoordinateMatrix &theMatrix) const;
  void NativeLatLonToWorldXY(NFmiCoordinateMatrix &theMatrix) const;
  void NativeToXY(NFmiCoordinateMatrix &theMatrix) const;
#endif

  unsigned long ClassId() const;
  const std::string &ClassName() const;
  std::string WKT() const;
  std::string PrettyWKT() const;
  std::string ProjStr() const;

  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);

  bool operator==(const NFmiArea &theArea) const;
  bool operator!=(const NFmiArea &theArea) const;

  std::size_t HashValue() const;

  // Temporary fix until the above method is fixed to be
  std::size_t HashValueKludge() const;

  // A private proxy class to avoid unnecessary *CreateFrom duplicates. This proxy
  // implicitly converts acceptable spatial reference types into an actual spatial reference.
  class SpatialReferenceProxy
  {
   public:
    ~SpatialReferenceProxy();
    SpatialReferenceProxy() = delete;
    SpatialReferenceProxy(const SpatialReferenceProxy &other) = delete;
    SpatialReferenceProxy &operator=(const SpatialReferenceProxy &other) = delete;

    SpatialReferenceProxy(OGRSpatialReference *theSR) : itsOwned(false), itsSR(theSR) {}
    SpatialReferenceProxy(const std::string &theSR);
    SpatialReferenceProxy(const char *theSR);

    const OGRSpatialReference &operator*() const { return *itsSR; }
    OGRSpatialReference *get() const { return itsSR; }

    // This is mostly for debugging
    const std::string &ProjStr() const { return itsProjStr; }

   private:
    std::string itsProjStr;  // set only if initialized from a string
    void init(const std::string &theSR);

    bool itsOwned = false;                 // should we delete on destruction?
    OGRSpatialReference *itsSR = nullptr;  // never null after construction
  };

 public:
  // Intentional API design choice since the object has internal reference counting
  OGRSpatialReference *SpatialReference();
  const OGRSpatialReference *SpatialReference() const;

  // Named constructors used to clarify intent of the parameters. Note that the proxy
  // may accept actual spatial references or strings from which to construct them.

  static NFmiArea *CreateFromBBox(const SpatialReferenceProxy &theSR,
                                  const NFmiPoint &theBottomLeftWorldXY,
                                  const NFmiPoint &theTopRightWorldXY);

  static NFmiArea *CreateFromCenter(const SpatialReferenceProxy &theSR,
                                    const SpatialReferenceProxy &theCenterSR,
                                    const NFmiPoint &theCenterLatLon,
                                    double theWidthInMeters,
                                    double theHeightInMeters);

  static NFmiArea *CreateFromCorners(const SpatialReferenceProxy &theSR,
                                     const SpatialReferenceProxy &theBBoxSR,
                                     const NFmiPoint &theBottomLeftLatLon,
                                     const NFmiPoint &theTopRightLatLon);

  static NFmiArea *CreateFromReverseCorners(const SpatialReferenceProxy &theSR,
                                            const SpatialReferenceProxy &theBBoxSR,
                                            const NFmiPoint &theTopLeftLatLon,
                                            const NFmiPoint &theBottomRightLatLon);

  static NFmiArea *CreateFromWGS84Corners(const SpatialReferenceProxy &theSR,
                                          const NFmiPoint &theBottomLeftLatLon,
                                          const NFmiPoint &theTopRightLatLon);

  static NFmiArea *CreateFromCornerAndSize(const SpatialReferenceProxy &theSR,
                                           const SpatialReferenceProxy &theCornerSR,
                                           const NFmiPoint &theBottomLeftLatLon,
                                           double theWidthInMeters,
                                           double theHeightInMeters);

  const NFmiProj &Proj() const;

  // from spherical coordinates to WGS84
  static NFmiPoint SphereToWGS84(const NFmiPoint &theWorldXY);
  static NFmiPoint WGS84ToSphere(const NFmiPoint &theLatLon);

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

inline std::ostream &operator<<(std::ostream &file, const NFmiArea &ob) { return ob.Write(file); }
inline std::istream &operator>>(std::istream &file, NFmiArea &ob) { return ob.Read(file); }
