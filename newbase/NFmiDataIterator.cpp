// ======================================================================
/*!
 * \file NFmiDataIterator.cpp
 * \brief Implementation of class NFmiDataIterator
 */
// ======================================================================
/*
 * \class NFmiDataIterator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataIterator.h"
#include "NFmiAreaMask.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataIterator::~NFmiDataIterator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIterator::NFmiDataIterator(NFmiQueryInfo* theData)
    : itsData(theData), itsMask(nullptr), fMaskInUse(false)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIterator::DoForEach(NFmiDataModifier* /* theDataModifier */) {}
// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataIterator::CheckIfMaskIsUsed()
{
  if ((itsMask != nullptr) && itsMask->IsEnabled())
    fMaskInUse = true;
  else
    fMaskInUse = false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataIterator::IsMasked(const NFmiPoint& theLatLonPoint)
{
  if (fMaskInUse && (itsMask != nullptr)) return itsMask->IsMasked(theLatLonPoint);

  return false;
}

// ======================================================================
