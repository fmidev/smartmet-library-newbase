// ======================================================================
/*!
 * \file NFmiLatLonArea.h
 * \brief Interface of NFmiLatLonArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiLatLonArea : public NFmiArea
{
 public:
  ~NFmiLatLonArea() override;
  NFmiLatLonArea();
  NFmiLatLonArea(const NFmiLatLonArea& theLatLonArea);
  NFmiLatLonArea(const NFmiPoint& theBottomLeftLatLon,
                 const NFmiPoint& theTopRightLatLon,
                 const NFmiPoint& theTopLeftXY = NFmiPoint(0., 0.),
                 const NFmiPoint& theBottomRightXY = NFmiPoint(1., 1.),
                 bool usePacificView = false);

  NFmiArea* Clone() const override;
  const NFmiPoint ToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint& theLatLonPoint) const override;
  virtual double XScale() const;
  virtual double YScale() const;
  void Init(bool fKeepWorldRect = false) override;

  const NFmiRect WorldRect() const override;
  const NFmiPoint XYToWorldXY(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint& theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  unsigned long ClassId() const override;
  const char* ClassName() const override;
  const std::string AreaStr() const override;
  const std::string WKT() const override;

  virtual bool operator==(const NFmiLatLonArea& theArea) const;
  bool operator==(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

 protected:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;

};  // class NFmiLatLonArea

//! Undocumented, should be removed
typedef NFmiLatLonArea* PNFmiLatLonArea;

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiLatLonArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiLatLonArea::WorldXYToLatLon(const NFmiPoint& theXYPoint) const
{
  return NFmiPoint(180. * theXYPoint.X() / (kPii * kRearth),
                   180.0 * theXYPoint.Y() / (kPii * kRearth));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiPoint NFmiLatLonArea::LatLonToWorldXY(const NFmiPoint& theLatLonPoint) const
{
  return NFmiPoint(kRearth * FmiRad(theLatLonPoint.X()), kRearth * FmiRad(theLatLonPoint.Y()));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiLatLonArea::ClassId() const { return kNFmiLatLonArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiLatLonArea::ClassName() const { return "NFmiLatLonArea"; }

// ======================================================================
