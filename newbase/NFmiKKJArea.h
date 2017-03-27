// ======================================================================
/*!
 * \file NFmiKKJArea.h
 * \brief Interface of class NFmiKKJArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiKKJArea : public NFmiArea
{
 public:
  ~NFmiKKJArea() override;
  NFmiKKJArea();
  NFmiKKJArea(const NFmiKKJArea& theKKJArea);

  NFmiKKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY,
              bool usePacificView = false);

  NFmiKKJArea(double theRadialRangeInMeters,
              const NFmiPoint& theCenterLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY);

  NFmiKKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopLeftXY,
              const NFmiPoint& theBottomRightXY);

  NFmiKKJArea(const NFmiPoint& theTopLeftXY, const NFmiPoint& theBottomRightXY);

  void Init(bool fKeepWorldRect = false) override;
  const NFmiRect WorldRect() const override;

  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  virtual const NFmiPoint ToWorldXY(const NFmiPoint& theLatLonPoint) const;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;

  NFmiKKJArea& operator=(const NFmiKKJArea& theArea);

  virtual bool operator==(const NFmiKKJArea& theArea) const;
  virtual bool operator!=(const NFmiKKJArea& theArea) const;

  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  void SetupGaussKruger();
  void SetupAnyEllipsoid(double theA, double theP);

 protected:
  NFmiPoint itsTopRightLatLon;
  NFmiPoint itsBottomLeftLatLon;

  virtual int ZoneNumberByLongitude(double lon) const = 0;
  virtual int ZoneNumberByEasting(double easting) const = 0;

  double Rad(double x) const;
  double Deg(double x) const;
  double Arsinh(double x) const;
  double Artanh(double x) const;

  double a, p, b, e, dn, dn2, dn3, dn4, a1, h1[4], h2[4];

  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;

 private:
};  // class NFmiKKJArea

//! Undocumented, should be removed
typedef NFmiKKJArea* PNFmiKKJArea;

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiKKJArea::~NFmiKKJArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiKKJArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiKKJArea::ClassId() const { return kNFmiKKJArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiKKJArea::ClassName() const { return "NFmiKKJArea"; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Rad(double x) const { return x * kPii / 180.0; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Deg(double x) const { return x * 180.0 / kPii; }
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Arsinh(double x) const
{
  using namespace std;
  return log(x + sqrt(1.0 + x * x));
}

// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiKKJArea::Artanh(double x) const
{
  using namespace std;
  return 0.5 * log((1.0 + x) / (1.0 - x));
}

// ======================================================================
