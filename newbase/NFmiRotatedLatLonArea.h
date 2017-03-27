// ======================================================================
/*!
 * \file NFmiRotatedLatLonArea.h
 * \brief Interface of class NFmiRotatedLatLonArea
 */
// ======================================================================

#pragma once

#include "NFmiLatLonArea.h"

//! Undocumented

class _FMI_DLL NFmiRotatedLatLonArea : public NFmiLatLonArea
{
 public:
  ~NFmiRotatedLatLonArea() override;
  NFmiRotatedLatLonArea();
  NFmiRotatedLatLonArea(const NFmiRotatedLatLonArea &theRotatedLatLonArea);
  NFmiRotatedLatLonArea(const NFmiPoint &theBottomLeftLatLon,
                        const NFmiPoint &theTopRightLatLon,
                        const NFmiPoint &theSouthernPole =
                            NFmiPoint(0., -30.),  // 25.4.02/EL: lon = 0, lat = 30 South
                        const NFmiPoint &theTopLeftXY = NFmiPoint(0., 0.),
                        const NFmiPoint &theBottomRightXY = NFmiPoint(1., 1.),
                        bool initiallyRotated = false,
                        bool usePacificView = false);

  NFmiArea *Clone() const override;
  const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const override;
  virtual bool IsInside(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint ToRotLatLon(const NFmiPoint &theLatLonPoint) const;
  virtual const NFmiPoint ToRegLatLon(const NFmiPoint &theRotLatLonPoint) const;
  const NFmiPoint &SouthernPole() const { return itsSouthernPole; }
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  unsigned long ClassId() const override;
  const char *ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  using NFmiLatLonArea::operator==;
  virtual bool operator==(const NFmiRotatedLatLonArea &theArea) const;
  bool operator==(const NFmiArea &theArea) const override;

  std::ostream &Write(std::ostream &file) const override;
  std::istream &Read(std::istream &file) override;

  const NFmiAngle TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                   double theLatitudeEpsilon = 0.001) const override;

 private:
  NFmiPoint itsSouthernPole;

};  // class NFmiRotatedLatLonArea

//! Undocumented, should be removed
typedef NFmiRotatedLatLonArea *PNFmiRotatedLatLonArea;

//! ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiRotatedLatLonArea::~NFmiRotatedLatLonArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiRotatedLatLonArea::ClassId() const { return kNFmiRotatedLatLonArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiRotatedLatLonArea::ClassName() const { return "NFmiRotatedLatLonArea"; }

// ======================================================================
