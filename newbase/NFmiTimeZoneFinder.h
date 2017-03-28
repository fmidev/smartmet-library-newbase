// ======================================================================
/*!
 * \file NFmiTimeZoneFinder.h
 * \brief Interface of class NFmiTimeZoneFinder
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <boost/shared_ptr.hpp>
#include <string>

class NFmiPoint;
class NFmiTimeZoneFinderPimple;

//! Undocumented
class _FMI_DLL NFmiTimeZoneFinder
{
 public:
  ~NFmiTimeZoneFinder();
  NFmiTimeZoneFinder();
  NFmiTimeZoneFinder(const NFmiTimeZoneFinder& theFinder);
  NFmiTimeZoneFinder& operator=(const NFmiTimeZoneFinder& theFinder);

  bool ReadFile(const std::string& theFileName);
  float Find(const NFmiPoint& theLatLon) const;

  bool Empty() const;
  void Clear();

#ifndef NDEBUG
  void Check(const NFmiPoint& theLatLon) const;
#endif

 private:
  boost::shared_ptr<NFmiTimeZoneFinderPimple> itsPimple;

};  // class NFmiTimeZoneFinder

// ======================================================================
