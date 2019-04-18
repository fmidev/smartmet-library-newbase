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
#include <string>

#ifdef UNIX

#include <boost/shared_ptr.hpp>
#include <gdal/ogr_geometry.h>

class OGRCoordinateTransformation;

#endif

//! Undocumented
class _FMI_DLL NFmiArea
{
 public:
  ~NFmiArea() = default;
  NFmiArea(const NFmiArea &theArea) = default;
  NFmiArea &operator=(const NFmiArea &theArea) = default;

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

#ifndef WGS84
  void Init(bool fKeepWorldRect = false);
#endif

  NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const;
  NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const;
  NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const;
  NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const;
  NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const;

#ifdef WGS84
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon, const NFmiPoint &theTopRightLatLon) const;
#else
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const;
#endif
  NFmiArea *CreateNewArea(const NFmiRect &theRect) const;
  NFmiArea *CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                          const NFmiPoint &theTopRightLatLon) const;
  NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect);
  NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                          FmiDirection theFixedPoint,
                          bool fShrinkArea);

  unsigned long ClassId() const;
  const char *ClassName() const;
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

#ifndef WGS84
 protected:
  int Sign(double theValue) const;
  double FixLongitude(double theLongitude) const;
#endif

 private:
  // A private proxy class to avoid unnecessary *CreateFrom duplicates. This proxy
  // implicitly converts acceptable spatial reference types into an actual spatial reference.
  class SpatialReferenceProxy
  {
   public:
    SpatialReferenceProxy(const OGRSpatialReference &theSR) : itsSR(theSR) {}
    SpatialReferenceProxy(const std::string &theSR);
    SpatialReferenceProxy(const char *theSR);

    const OGRSpatialReference &operator*() const { return itsSR; }
    OGRSpatialReference *get() { return &itsSR; }

   private:
    OGRSpatialReference itsSR{NULL};
  };

 public:
  // Intentional API design choice since the object has internal reference counting
  OGRSpatialReference *SpatialReference() { return &itsSpatialReference; }
  const OGRSpatialReference *SpatialReference() const { return &itsSpatialReference; }

  // Named constructors used to clarify intent of the parameters. Note that the proxy
  // may accept actual spatial references or strings from which to construct them.

  static NFmiArea *CreateFromBBox(SpatialReferenceProxy theSR,
                                  const NFmiPoint &theBottomLeft,
                                  const NFmiPoint &theTopRight);

  static NFmiArea *CreateFromCenter(SpatialReferenceProxy theSR,
                                    SpatialReferenceProxy theCenterSR,
                                    const NFmiPoint &theCenter,
                                    double theWidth,
                                    double theHeight);

  static NFmiArea *CreateFromCorners(SpatialReferenceProxy theSR,
                                     SpatialReferenceProxy theBBoxSR,
                                     const NFmiPoint &theBottomLeft,
                                     const NFmiPoint &theTopRight);

  static NFmiArea *CreateFromWGS84Corners(SpatialReferenceProxy theSR,
                                          const NFmiPoint &theBottomLeft,
                                          const NFmiPoint &theTopRight);

  static NFmiArea *CreateFromCornerAndSize(SpatialReferenceProxy theSR,
                                           SpatialReferenceProxy theCornerSR,
                                           const NFmiPoint &theBottomLeft,
                                           double theWidth,
                                           double theHeight);

  const NFmiProj &Proj() const { return itsProj; }

  // from spherical coordinates to WGS84
  static NFmiPoint SphereToWGS84(const NFmiPoint &theLatLon);
  static NFmiPoint WGS84ToSphere(const NFmiPoint &theLatLon);

  double XScale() const;
  double YScale() const;

 private:
  // We allow only the Create* static methods to construct projections

  NFmiArea() = default;
  void InitSpatialReference(const std::string &theProjection);
  void InitProj();
  void InitRectConversions();

  OGRSpatialReference itsSpatialReference{NULL};

  boost::shared_ptr<OGRCoordinateTransformation> itsToLatLonConverter;
  boost::shared_ptr<OGRCoordinateTransformation> itsToWorldXYConverter;

  NFmiRect itsWorldRect;           // bbox in native WorldXY coordinates
  NFmiRect itsXYRect{0, 0, 1, 1};  // mapping from bbox to XY image coordinates

  // This is only needed when reading legacy files from disk
  int itsClassId = kNFmiArea;

  // For writing legacy projections back to disk

  NFmiPoint TopLeftCorner() const;
  NFmiPoint BottomRightCorner() const;
  boost::optional<NFmiPoint> itsTopLeftCorner;
  boost::optional<NFmiPoint> itsBottomRightCorner;

  // For speeding up coordinate conversions and to aid legacy parts of Write()
  double itsXScaleFactor;
  double itsYScaleFactor;

  // For providing PROJ.4 parameter information

  NFmiProj itsProj;

};  // class NFmiArea

inline std::ostream &operator<<(std::ostream &file, const NFmiArea &ob) { return ob.Write(file); }
inline std::istream &operator>>(std::istream &file, NFmiArea &ob) { return ob.Read(file); }
