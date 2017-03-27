// ======================================================================
/*!
 * \file NFmiAzimuthalArea.h
 * \brief Interface of class NFmiAzimuthalArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiAzimuthalArea : public NFmiArea
{
 public:
  ~NFmiAzimuthalArea() override;

  NFmiAzimuthalArea();

  NFmiAzimuthalArea(const NFmiAzimuthalArea& theAzimuthalArea);

  NFmiAzimuthalArea(double theRadialRangeInMeters,
                    const NFmiPoint& theCenterLatLon,
                    const NFmiPoint& theTopLeftXY,
                    const NFmiPoint& theBottomRightXY,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    double theCenterLatitude = 90.,
                    double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    double theCenterLatitude = 90.,
                    double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(double theRadialRange,
                    double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    double theCenterLatitude = 90.,
                    double theTrueLatitude = 60.,
                    bool usePacificView = false);

  void Init(bool fKeepWorldRect = false) override;

  const NFmiRect WorldRect() const override;

  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;

  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  virtual const NFmiPoint ToLatLon(double theAzimuth, double theRadius) const;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  virtual const NFmiPoint LatLonToWorldXY(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint ToXY(double theAzimuth, double theRadius) const;

  double CentralLongitude() const;
  double Orientation() const;

  double CentralLatitude() const;
  double TrueLatitude() const;
  double XScaleFactor() const;
  double YScaleFactor() const;

  virtual const NFmiPoint RadialXYPoint(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint CurrentCenter() const;

  NFmiAzimuthalArea& operator=(const NFmiAzimuthalArea& theArea);

  virtual bool operator==(const NFmiAzimuthalArea& theArea) const;
  virtual bool operator!=(const NFmiAzimuthalArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const char* ClassName() const override;
  const std::string AreaStr() const override = 0;

 protected:
  NFmiPoint itsTopRightLatLon;
  NFmiPoint itsBottomLeftLatLon;

  NFmiPoint itsBottomLeftWorldXY;

  virtual double K(double delta) const = 0;
  virtual double CalcDelta(double xyDistance) const = 0;
  virtual double DistanceFromPerspectivePointToCenterOfEarth() const = 0;

  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;
  double itsRadialRange;
  double itsCentralLongitude;
  NFmiAngle itsCentralLatitude;
  NFmiAngle itsTrueLatitude;
  double itsTrueLatScaleFactor;

 private:
};  // class NFmiAzimuthalArea

//! Undocumented, should be removed
typedef NFmiAzimuthalArea* PNFmiAzimuthalArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

inline NFmiAzimuthalArea::~NFmiAzimuthalArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiAzimuthalArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLongitude() const { return itsCentralLongitude; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::Orientation() const { return itsCentralLongitude; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLatitude() const { return itsCentralLatitude.Value(); }
// ----------------------------------------------------------------------
/*!
 * \return Projection true latitude
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::TrueLatitude() const { return itsTrueLatitude.Value(); }
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::XScaleFactor() const { return itsXScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::YScaleFactor() const { return itsYScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiAzimuthalArea::ClassName() const { return "NFmiAzimuthalArea"; }

// ======================================================================
