// ======================================================================
/*!
 * \file NFmiArea.h
 * \brief Interface of class NFmiArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
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

  NFmiArea *CreateNewArea(const NFmiRect &theRect) const;
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const;
  NFmiArea *CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                          const NFmiPoint &theTopRightLatLon) const;
  //   NFmiArea * CreateNewArea(const NFmiRect & theRect);
  NFmiArea *CreateNewAreaByWorldRect(const NFmiRect &theWorldRect);
  NFmiArea *CreateNewArea(double theNewAspectRatioXperY,
                          FmiDirection theFixedPoint,
                          bool fShrinkArea);

  unsigned long ClassId() const;
  const char *ClassName() const;
  std::string AreaStr() const;
  std::string WKT() const;

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
  class SpatialReferenceProxy
  {
   public:
    SpatialReferenceProxy(const OGRSpatialReference &theSR) : itsSR(theSR) {}
    SpatialReferenceProxy(const std::string &theSR)
    {
      auto err = itsSR.SetFromUserInput(theSR.c_str());
      if (err != OGRERR_NONE)
        throw std::runtime_error("Failed to create spatial reference from '" + theSR + "'");
    }

    const OGRSpatialReference &operator*() const { return itsSR; }
    OGRSpatialReference *get() { return &itsSR; }

   private:
    OGRSpatialReference itsSR{NULL};
  };

 public:
  // Intentional API design choice since the object has internal reference counting
  OGRSpatialReference *SpatialReference() { return &itsSpatialReference; }

  // Named constructors used to clarify intent of the parameters. s

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

  static NFmiArea *CreateFromGRIBSettings(SpatialReferenceProxy theSR,
                                          SpatialReferenceProxy theCornerSR,
                                          const NFmiPoint &theBottomLeft,
                                          double theWidth,
                                          double theHeight);

 private:
  // We allow only the Create* static methods to construct projections

  NFmiArea() = default;
  void InitSpatialReference(const std::string &theProjection);
  void InitConversions();

  OGRSpatialReference itsSpatialReference{NULL};

  boost::shared_ptr<OGRCoordinateTransformation> itsToLatLonConverter;
  boost::shared_ptr<OGRCoordinateTransformation> itsToWorldXYConverter;

  NFmiRect itsWorldRect;               // bbox in native WorldXY coordinates
  NFmiRect itsXYRectArea{0, 0, 1, 1};  // mapping from bbox to XY image coordinates

  // This is only needed when reading legacy files from disk
  int itsClassId = kNFmiArea;

};  // class NFmiArea

std::ostream &operator<<(std::ostream &file, const NFmiArea &ob) { return ob.Write(file); }
std::istream &operator>>(std::istream &file, NFmiArea &ob) { return ob.Read(file); }
