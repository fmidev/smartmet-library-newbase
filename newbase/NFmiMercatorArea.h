// ======================================================================
/*!
 * \file NFmiMercatorArea.h
 * \brief Interface of class NFmiMercatorArea
 */
// ======================================================================

#pragma once

#include "NFmiAngle.h"
#include "NFmiArea.h"

//! Undocumented
class _FMI_DLL NFmiMercatorArea : public NFmiArea
{
 public:
  ~NFmiMercatorArea(void) override;
  NFmiMercatorArea(void);
  NFmiMercatorArea(const NFmiMercatorArea &theLatLonArea);
  NFmiMercatorArea(const NFmiPoint &theBottomLeftLatLon,
                   const NFmiPoint &theTopRightLatLon,
                   const NFmiPoint &theTopLeftXY = NFmiPoint(0., 0.),
                   const NFmiPoint &theBottomRightXY = NFmiPoint(1., 1.),
                   bool usePacificView = false);

  NFmiArea *Clone(void) const override;
  const NFmiPoint ToLatLon(const NFmiPoint &theXYPoint) const override;
  const NFmiPoint ToXY(const NFmiPoint &theLatLonPoint) const override;
  virtual double XScale(void) const;
  virtual double YScale(void) const;
  void Init(bool fKeepWorldRect = false) override;

  const NFmiRect WorldRect(void) const override;

  const NFmiPoint XYToWorldXY(const NFmiPoint &theXYPoint) const override;
  const NFmiPoint WorldXYToLatLon(const NFmiPoint &theXYPoint) const override;
  const NFmiPoint LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const override;
  NFmiArea *NewArea(const NFmiPoint &theBottomLeftLatLon,
                    const NFmiPoint &theTopRightLatLon,
                    bool allowPacificFix = true) const override;
  unsigned long ClassId(void) const override;
  const char *ClassName(void) const override;
  const std::string AreaStr(void) const override;
  const std::string WKT() const override;
  virtual bool operator==(const NFmiMercatorArea &theArea) const;
  bool operator==(const NFmiArea &theArea) const override;
  std::ostream &Write(std::ostream &file) const override;
  std::istream &Read(std::istream &file) override;

 protected:
  NFmiPoint itsBottomLeftLatLon;
  NFmiPoint itsTopRightLatLon;
  double itsXScaleFactor;
  double itsYScaleFactor;
  NFmiRect itsWorldRect;
};  // class NFmiMercatorArea

//! Undocumented, should be removed
typedef NFmiMercatorArea *PNFmiMercatorArea;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiMercatorArea::~NFmiMercatorArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiRect NFmiMercatorArea::WorldRect(void) const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMercatorArea::ClassId(void) const { return kNFmiMercatorArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiMercatorArea::ClassName(void) const { return "NFmiMercatorArea"; }

// ======================================================================
