// ======================================================================
/*!
 * \file NFmiPKJArea.h
 * \brief Interface of class NFmiPKJArea
 */
// ======================================================================

#pragma once

#include "NFmiKKJArea.h"

//! Undocumented
class NFmiPKJArea : public NFmiKKJArea
{
 public:
  ~NFmiPKJArea() override;
  NFmiPKJArea();
  NFmiPKJArea(const NFmiPKJArea& thePKJArea);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              const NFmiPoint& theTopRightLatLon,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F),
              bool usePacificView = false);

  NFmiPKJArea(const NFmiPoint& theBottomLeftLatLon,
              const double theWidthInMeters,
              const double theHeightInMeters,
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

  NFmiPKJArea(const NFmiPoint& theBottomLeftWorldXY,  // defaults to meters
              const NFmiPoint& theTopRightWorldXY,    // defaults to meters
              const bool& IsWorldXYInMeters,          // dummy argument
              const NFmiPoint& theTopLeftXY = NFmiPoint(0.F, 0.F),
              const NFmiPoint& theBottomRightXY = NFmiPoint(1.F, 1.F));

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

  std::size_t HashValue() const;

 protected:
  int ZoneNumberByLongitude(const double lon) const override;
  int ZoneNumberByEasting(const double easting) const override;

 private:
  NFmiPoint CornerWorldXY(const NFmiPoint& latlonPoint) const;

};  // class NFmiPKJArea

//! Undocumented, should be removed
using PNFmiPKJArea = NFmiPKJArea*;

// ----------------------------------------------------------------------
/*!
 * The destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiPKJArea::~NFmiPKJArea() = default;
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiPKJArea::ClassId() const
{
  return kNFmiPKJArea;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiPKJArea::ClassName() const
{
  return "NFmiPKJArea";
}

// ======================================================================
