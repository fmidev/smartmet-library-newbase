// ======================================================================
/*!
 * \file NFmiPKJArea.h
 * \brief Interface of class NFmiPKJArea
 */
// ======================================================================

#pragma once

#include "NFmiKKJArea.h"

//! Undocumented
class _FMI_DLL NFmiPKJArea : public NFmiKKJArea
{
 public:
  ~NFmiPKJArea(void) override;
  NFmiPKJArea(void);
  NFmiPKJArea(const NFmiPKJArea& thePKJArea);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
              bool usePacificView = false);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              const double theWidthInMeters,
              const double theHeightInMeters,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  NFmiPKJArea(const NFmiPoint& theBottomLeftWorldXY,  // defaults to meters
              const NFmiPoint& theTopRightWorldXY,    // defaults to meters
              const bool& IsWorldXYInMeters,          // dummy argument
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f));

  void Init(bool fKeepWorldRect = false) override;
  NFmiArea* NewArea(const NFmiPoint& theBottomLeftLatLon,
                    const NFmiPoint& theTopRightLatLon,
                    bool allowPacificFix = true) const override;

  NFmiArea* Clone(void) const override;

  unsigned long ClassId(void) const override;
  const char* ClassName(void) const override;

  NFmiPKJArea& operator=(const NFmiPKJArea& theArea);

  virtual bool operator==(const NFmiPKJArea& theArea) const;
  virtual bool operator!=(const NFmiPKJArea& theArea) const;

  using NFmiKKJArea::operator==;
  using NFmiKKJArea::operator!=;
  bool operator==(const NFmiArea& theArea) const override;
  bool operator!=(const NFmiArea& theArea) const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  const std::string WKT() const override;

 protected:
  int ZoneNumberByLongitude(const double lon) const override;
  int ZoneNumberByEasting(const double easting) const override;

 private:
  NFmiPoint CornerWorldXY(const NFmiPoint& latlonPoint) const;

};  // class NFmiPKJArea

//! Undocumented, should be removed
typedef NFmiPKJArea* PNFmiPKJArea;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiPKJArea::~NFmiPKJArea(void) {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiPKJArea::ClassId(void) const { return kNFmiPKJArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiPKJArea::ClassName(void) const { return "NFmiPKJArea"; }

// ======================================================================
