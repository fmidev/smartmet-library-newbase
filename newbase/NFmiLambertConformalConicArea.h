#pragma once

#include "NFmiArea.h"

//! Undocumened
class NFmiLambertConformalConicArea : public NFmiArea
{
 public:
  ~NFmiLambertConformalConicArea() override = default;
  NFmiLambertConformalConicArea() = default;
  NFmiLambertConformalConicArea(const NFmiLambertConformalConicArea& theLambertConformalConicArea) =
      default;

  NFmiLambertConformalConicArea(const NFmiPoint& theBottomLeftLatLon,
                                const NFmiPoint& theTopRightLatLon,
                                double theCentralLongitude,
                                double theCentralLatitude,
                                double theTrueLatitude1,
                                double theTrueLatitude2,
                                double theRadius = kRearth,
                                bool usePacificView = false,
                                const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
                                const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint WorldXYToXY(const NFmiPoint& theWorldXYPoint) const override;

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* Clone() const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  const NFmiRect WorldRect() const override { return itsWorldRect; }

  using NFmiArea::CreateNewArea;
  NFmiArea* CreateNewArea(const NFmiRect& theRect) const override;

  NFmiLambertConformalConicArea& operator=(const NFmiLambertConformalConicArea& theArea) = default;

  bool operator==(const NFmiLambertConformalConicArea& theArea) const;
  bool operator!=(const NFmiLambertConformalConicArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId() const override { return kNFmiLambertConformalConicArea; }

  const char* ClassName() const override { return "kNFmiLambertConformalConicArea"; }
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  std::size_t HashValue() const;

 private:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsCentralLongitude = 0.0;
  double itsCentralLatitude = 0.0;
  double itsTrueLatitude1 = 0.0;
  double itsTrueLatitude2 = 0.0;
  double itsRadius = kRearth;
  NFmiRect itsWorldRect;

  // Unique derived coefficients per projection:
  double itsXScaleFactor = 0;
  double itsYScaleFactor = 0;
  double itsN = 0;
  double itsF = 0;
  double itsRho0 = 0;

};  // class NFmiLambertConformalConicArea
