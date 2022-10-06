// ======================================================================
/*!
 * \file NFmiSaveBaseFactory.cpp
 * \brief Implementation of a factory for newbase objects
 */
// ======================================================================

#include "NFmiSaveBaseFactory.h"

#include "NFmiGrid.h"
#include "NFmiQueryData.h"
#include "NFmiStationBag.h"
#include "NFmiVersion.h"

#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <macgyver/Exception.h>

// ----------------------------------------------------------------------
/*!
 * Returns a newbase object based on the given unique numeric ID.
 * The returned object is a void pointer, which makes this a rather
 * strange factory, but it will have to do until a complete redesign.
 *
 * \param classId The ID of the object to be created
 * \return Pointer to the new object
 * \todo Should return an boost::shared_ptr
 */
// ----------------------------------------------------------------------

void *CreateSaveBase(unsigned int classId)
{
  try
  {
    switch (classId)
    {
      case kNFmiGrid:
        return static_cast<void *>(new NFmiGrid);

      case kNFmiProjArea:
      case kNFmiLatLonArea:
      case kNFmiRotatedLatLonArea:
      case kNFmiStereographicArea:
      case kNFmiYKJArea:
      case kNFmiEquiDistArea:
      case kNFmiLambertConformalConicArea:
#ifdef UNIX
#ifndef DISABLED_GDAL
      case kNFmiGdalArea:
#endif
#endif
        return static_cast<void *>(new NFmiArea(classId));

      case kNFmiQueryData:
        return static_cast<void *>(new NFmiQueryData);
      case kNFmiQueryInfo:
        return static_cast<void *>(new NFmiQueryInfo);

      case kNFmiLocationBag:
        return static_cast<void *>(new NFmiLocationBag);
      case kNFmiStationBag:
        return static_cast<void *>(new NFmiStationBag);

      default:
        throw Fmi::Exception(
            BCP,
            "Newbase: unable to create unknown class " + boost::lexical_cast<std::string>(classId));
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
