// ======================================================================
/*!
 * \file NFmiVoid.h
 * \brief Interface of class NFmiVoid
 */
// ======================================================================
/*!
 * \class NFmiVoid
 *
 * A helper class for NFmiDataPool. This really should be documented better,
 * especially since the member variables are not encapsulated.
 *
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"

//! Undocumented
class NFmiVoid
{
 public:
  ~NFmiVoid();
  NFmiVoid();
  NFmiVoid& operator=(const NFmiVoid& theVoid);

  void* itsVoid = nullptr;
  unsigned long itsNumber = 0;
  unsigned short itsUnitSize = 0;
  unsigned long itsSize = 0;

 private:
  NFmiVoid(const NFmiVoid& theVoid) = delete;

};  // class NFmiVoid

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special.
 */
// ----------------------------------------------------------------------

inline NFmiVoid::~NFmiVoid() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor does nothing special, meaning all member variables
 * will be zeroed.
 */
// ----------------------------------------------------------------------

inline NFmiVoid::NFmiVoid() = default;
// ----------------------------------------------------------------------
/*!
 * Assignment operator performs a bitwise copy.
 *
 * \param theVoid The object to copy
 * \return This
 */
// ----------------------------------------------------------------------

inline NFmiVoid& NFmiVoid::operator=(const NFmiVoid& theVoid) = default;

// ======================================================================
