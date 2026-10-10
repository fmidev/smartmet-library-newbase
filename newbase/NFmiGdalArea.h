// ======================================================================
/*!
 * \file NFmiGdalArea.h
 * \brief Interface of class NFmiGdalArea
 */
// ======================================================================

#ifndef DISABLED_GDAL

#pragma once

#include "NFmiArea.h"
#include <memory>

class OGRSpatialReference;

namespace Fmi
{
class CoordinateTransformation;
}

class NFmiGdalArea : public NFmiArea
{
 public:
  ~NFmiGdalArea() override;

  NFmiGdalArea();

  NFmiGdalArea(const NFmiGdalArea& theGdalArea);

  NFmiGdalArea(const std::string& theDatum,
               const std::string& theDescription,
               const NFmiPoint& theBottomLeftLatLon,
               const NFmiPoint& theTopRightLatLon,
               const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
               const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
               bool usePacificView = false);

  NFmiGdalArea(const std::string& theDatum,
               const OGRSpatialReference& theCRS,
               const NFmiPoint& theBottomLeftLatLon,
               const NFmiPoint& theTopRightLatLon,
               const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
               const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
               bool usePacificView = false);

  NFmiGdalArea(const std::string& theDatum,
               const OGRSpatialReference& theCRS,
               double theXmin,
               double theYmin,
               double theXmax,
               double theYmax,
               const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
               const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
               bool usePacificView = false);

  NFmiArea* Clone() const override;

  NFmiGdalArea& operator=(const NFmiGdalArea& theArea);

  bool operator==(const NFmiGdalArea& theArea) const;
  bool operator!=(const NFmiGdalArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string& Datum() const;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint WorldXYToXY(const NFmiPoint& theWorldXYPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiRect WorldRect() const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  double WorldXYHeight() const override;
  double WorldXYWidth() const override;

  std::size_t HashValue() const;

 private:
  void init();

  std::string itsDatum;
  std::string itsDescription;
  std::string itsWKT;

  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  NFmiRect itsWorldRect;

  std::shared_ptr<Fmi::CoordinateTransformation> itsLatLonToWorldXYTransformation;
  std::shared_ptr<Fmi::CoordinateTransformation> itsWorldXYToLatLonTransformation;

};  // class NFmiGdalArea

#endif  // DISABLED_GDAL

// ======================================================================
