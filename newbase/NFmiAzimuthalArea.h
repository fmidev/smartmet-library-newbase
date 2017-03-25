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
  ~NFmiAzimuthalArea(void) override;

  NFmiAzimuthalArea(void);

  NFmiAzimuthalArea(const NFmiAzimuthalArea& theAzimuthalArea);

  NFmiAzimuthalArea(double theRadialRangeInMeters,
                    const NFmiPoint& theCenterLatLon,
                    const NFmiPoint& theTopLeftXY,
                    const NFmiPoint& theBottomRightXY,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const NFmiPoint& theBottomLeftLatLon,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  NFmiAzimuthalArea(const double theRadialRange,
                    const double theCentralLongitude = 0.,
                    const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
                    const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
                    const double theCenterLatitude = 90.,
                    const double theTrueLatitude = 60.,
                    bool usePacificView = false);

  void Init(bool fKeepWorldRect = false) override;

  const NFmiRect WorldRect(void) const override;

  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;

  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  virtual const NFmiPoint ToLatLon(double theAzimuth, double theRadius) const;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  virtual const NFmiPoint LatLonToWorldXY(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint ToXY(double theAzimuth, double theRadius) const;

  double CentralLongitude(void) const;
  double Orientation(void) const;

  double CentralLatitude(void) const;
  double TrueLatitude(void) const;
  double XScaleFactor(void) const;
  double YScaleFactor(void) const;

  virtual const NFmiPoint RadialXYPoint(double theAzimuth, double theRadius) const;
  virtual const NFmiPoint CurrentCenter(void) const;

  NFmiAzimuthalArea& operator=(const NFmiAzimuthalArea& theArea);

  virtual bool operator==(const NFmiAzimuthalArea& theArea) const;
  virtual bool operator!=(const NFmiAzimuthalArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const char* ClassName(void) const override;
  const std::string AreaStr(void) const override = 0;

 protected:
  NFmiPoint itsTopRightLatLon;
  NFmiPoint itsBottomLeftLatLon;

  NFmiPoint itsBottomLeftWorldXY;

  virtual double K(const double delta) const = 0;
  virtual double CalcDelta(const double xyDistance) const = 0;
  virtual double DistanceFromPerspectivePointToCenterOfEarth(void) const = 0;

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

inline NFmiAzimuthalArea::~NFmiAzimuthalArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiAzimuthalArea::WorldRect(void) const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLongitude(void) const { return itsCentralLongitude; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::Orientation(void) const { return itsCentralLongitude; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::CentralLatitude(void) const { return itsCentralLatitude.Value(); }
// ----------------------------------------------------------------------
/*!
 * \return Projection true latitude
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::TrueLatitude(void) const { return itsTrueLatitude.Value(); }
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::XScaleFactor(void) const { return itsXScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * Tarvitaan kun määritellään hilaväliä metreissä
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiAzimuthalArea::YScaleFactor(void) const { return itsYScaleFactor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiAzimuthalArea::ClassName(void) const { return "NFmiAzimuthalArea"; }

// ======================================================================
