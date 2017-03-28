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
  ~NFmiPKJArea() override;
  NFmiPKJArea();
  NFmiPKJArea(const NFmiPKJArea& thePKJArea);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.f, 0.f),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.f, 1.f),
              bool usePacificView = false);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              double theWidthInMeters,
              double theHeightInMeters,
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

  NFmiArea* Clone() const override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;

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
  int ZoneNumberByLongitude(double lon) const override;
  int ZoneNumberByEasting(double easting) const override;

 private:
  NFmiPoint CornerWorldXY(const NFmiPoint& theLatLonPoint) const;

};  // class NFmiPKJArea

//! Undocumented, should be removed
using PNFmiPKJArea = NFmiPKJArea*;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiPKJArea::~NFmiPKJArea() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiPKJArea::ClassId() const { return kNFmiPKJArea; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiPKJArea::ClassName() const { return "NFmiPKJArea"; }

// ======================================================================
