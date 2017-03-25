// ======================================================================
/*!
 * \file NFmiStationBag.h
 * \brief Interface of class NFmiStationBag
 */
// ======================================================================

#pragma once

#include "NFmiLocationBag.h"
#include "NFmiStation.h"

// ÄLÄ KÄYTÄ TÄTÄ LUOKKAA UUSIIN JUTTUIHIN, VAAN KÄYTÄ NFmiLocationBag:ia

class _FMI_DLL NFmiStationBag : public NFmiLocationBag
{
 public:
  NFmiStationBag();
  NFmiStationBag(unsigned long *theStationArray, unsigned long theNumberOfStations);
  NFmiStationBag(NFmiStation *theStationArray, unsigned long theNumberOfStations);
  NFmiStationBag(NFmiLocation *theLocationArray,
                 unsigned long theNumberOfStations,
                 NFmiIndividual *theStationArray);
  NFmiStationBag(const NFmiStationBag &theBag);

  ~NFmiStationBag() override { Destroy(); }
  long CurrentStation() const;
  bool SetCurrent(long theStation);

  bool Current(const NFmiStation &theStation);
  const NFmiStation Current() const;

  bool AddStation(const NFmiStation &theStation);

  const NFmiLocationBag Combine(const NFmiLocationBag &theBag) override;
  bool AddLocation(const NFmiLocation &theLocation, bool theChecking = true) override;
  bool Location(const NFmiLocation &theLocation) override;

  using NFmiLocationBag::Location;
  const NFmiLocation *Location() const override;

  NFmiLocationBag *Clone() const override;
  void Destroy() override;

  unsigned long ClassId() const override { return kNFmiStationBag; }
  std::ostream &Write(std::ostream &file) const override;
  std::istream &Read(std::istream &file) override;

  NFmiStationBag &operator=(const NFmiStationBag &theStationBag);

};  // class NFmiStationBag

// ----------------------------------------------------------------------
/*!
 * Output operator for NFmiStationBag class
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiStationBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for NFmiStationBag class
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiStationBag &ob) { return ob.Read(file); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug One must check itsIndex is valid (not -1).
 */
// ----------------------------------------------------------------------

inline const NFmiStation NFmiStationBag::Current() const
{
  return *static_cast<NFmiStation *>(itsLocations[itsIndex]);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiStationBag::CurrentStation() const { return Current().GetIdent(); }
// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiStationBag::Current(const NFmiStation &theStation)
{
  return NFmiLocationBag::Location(theStation);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiLocation *NFmiStationBag::Location() const
{
  return (static_cast<NFmiStation *>(itsLocations[itsIndex]));
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiStationBag::Location(const NFmiLocation &theLocation)
{
  return NFmiLocationBag::Location(theLocation);
}

// ======================================================================
