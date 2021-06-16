// ======================================================================
/*!
 * \file NFmiFastQueryInfo.cpp
 * \brief Implementation of class NFmiFastQueryInfo
 */
// ======================================================================
/*!
 * \class NFmiFastQueryInfo
 *
 * Undocumented
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4512)  // poistaa MSVC++2008 kääntäjän ikävän näköisen boost:ista tulevan
                                 // varoituksen
#endif

#include "NFmiFastQueryInfo.h"
#include "NFmiCombinedParam.h"
#include "NFmiDataModifier.h"
#include "NFmiGrid.h"
#include "NFmiInterpolation.h"
#include "NFmiMetMath.h"
#include "NFmiQueryData.h"
#include "NFmiRawData.h"
#include "NFmiTotalWind.h"
#include "NFmiWeatherAndCloudiness.h"
#include <gis/CoordinateMatrix.h>
#include <gis/CoordinateTransformation.h>
#include <gis/SpatialReference.h>
#include <macgyver/Exception.h>
#include <stdexcept>

// ----------------------------------------------------------------------
/*!
 * \brief Utility peek functions for getting grid cell values
 */
// ----------------------------------------------------------------------

namespace
{
void PeekCellValues(NFmiFastQueryInfo &theInfo,
                    long dx,
                    long dy,
                    float &bottomLeftValue,
                    float &bottomRightValue,
                    float &topLeftValue,
                    float &topRightValue)
{
  try
  {
    bottomLeftValue = theInfo.PeekLocationValue(dx, dy);
    bottomRightValue = theInfo.PeekLocationValue(dx + 1, dy);
    topLeftValue = theInfo.PeekLocationValue(dx, dy + 1);
    topRightValue = theInfo.PeekLocationValue(dx + 1, dy + 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void PeekCellValues(NFmiFastQueryInfo &theInfo,
                    long dx,
                    long dy,
                    const NFmiMetTime &theTime,
                    float &bottomLeftValue,
                    float &bottomRightValue,
                    float &topLeftValue,
                    float &topRightValue)
{
  try
  {
    bottomLeftValue = theInfo.PeekLocationValue(dx, dy, theTime);
    bottomRightValue = theInfo.PeekLocationValue(dx + 1, dy, theTime);
    topLeftValue = theInfo.PeekLocationValue(dx, dy + 1, theTime);
    topRightValue = theInfo.PeekLocationValue(dx + 1, dy + 1, theTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo::NFmiFastQueryInfo(double theInfoVersion)
    : NFmiQueryInfo(theInfoVersion),
      itsParamIndex(),
      itsLocationIndex(),
      itsLevelIndex(),
      itsTimeIndex(),
      itsParamSize(),
      itsLocationSize(),
      itsLevelSize(),
      itsTimeSize()
      //  , itsDataArraySize()
      // , itsDataArray()
      ,
      itsTemporaryGridData(nullptr),
      itsLocLevTimSize(),
      itsLevTimSize(),
      fUseSubParam(false),
      fHeightValueAvailable(false),
      fHeightLevelDataAvailable(false),
      itsHeightLevelDataHeights(),
      fHeightParamIsRising(false),
      itsHeightParamIndex(static_cast<unsigned long>(-1)),
      fPressureValueAvailable(false),
      fPressureLevelDataAvailable(false),
      fPressureParamIsRising(false),
      itsPressureParamIndex(static_cast<unsigned long>(-1)),
      itsOldParamIndex(static_cast<unsigned long>(-1)),
      fOldFSubParamUsed(false),
      itsPressureLevelDataPressures(),
      itsLongitudeParamIndex(static_cast<unsigned long>(-1)),
      itsLatitudeParamIndex(static_cast<unsigned long>(-1)),
      itsFastStationIdSeekList(),
      itsDataType(NFmiInfoData::kNoDataType),
      itsDummyTime(1900, 1, 1, 1, 1, 1)
{
  try
  {
    InitFastInfo();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 * \param theInfoVersion
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo::NFmiFastQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                                     const NFmiTimeDescriptor &theTimeDescriptor,
                                     const NFmiHPlaceDescriptor &theHPlaceDescriptor,
                                     const NFmiVPlaceDescriptor &theVPlaceDescriptor,
                                     double theInfoVersion)
    : NFmiQueryInfo(theParamDescriptor,
                    theTimeDescriptor,
                    theHPlaceDescriptor,
                    theVPlaceDescriptor,
                    theInfoVersion),
      itsParamIndex(),
      itsLocationIndex(),
      itsLevelIndex(),
      itsTimeIndex(),
      itsParamSize(),
      itsLocationSize(),
      itsLevelSize(),
      itsTimeSize()
      //  , itsDataArraySize()
      //  , itsDataArray()
      ,
      itsTemporaryGridData(nullptr),
      itsLocLevTimSize(),
      itsLevTimSize(),
      fUseSubParam(false),
      fHeightValueAvailable(false),
      fHeightLevelDataAvailable(false),
      itsHeightLevelDataHeights(),
      fHeightParamIsRising(false),
      itsHeightParamIndex(static_cast<unsigned long>(-1)),
      fPressureValueAvailable(false),
      fPressureLevelDataAvailable(false),
      fPressureParamIsRising(false),
      itsPressureParamIndex(static_cast<unsigned long>(-1)),
      itsOldParamIndex(static_cast<unsigned long>(-1)),
      fOldFSubParamUsed(false),
      itsPressureLevelDataPressures(),
      itsLongitudeParamIndex(static_cast<unsigned long>(-1)),
      itsLatitudeParamIndex(static_cast<unsigned long>(-1)),
      itsFastStationIdSeekList(),
      itsDataType(NFmiInfoData::kNoDataType),
      itsDummyTime(1900, 1, 1, 1, 1, 1)
{
  try
  {
    InitFastInfo();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param data Undocumented
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo::NFmiFastQueryInfo(NFmiQueryData *data,
                                     NFmiParamDescriptor *theParamDescriptor,
                                     NFmiTimeDescriptor *theTimeDescriptor,
                                     NFmiHPlaceDescriptor *theHPlaceDescriptor,
                                     NFmiVPlaceDescriptor *theVPlaceDescriptor)
    : NFmiQueryInfo(
          data, theParamDescriptor, theTimeDescriptor, theHPlaceDescriptor, theVPlaceDescriptor),
      itsParamIndex(),
      itsLocationIndex(),
      itsLevelIndex(),
      itsTimeIndex(),
      itsParamSize(),
      itsLocationSize(),
      itsLevelSize(),
      itsTimeSize()
      //  , itsDataArraySize()
      //  , itsDataArray()
      ,
      itsTemporaryGridData(nullptr),
      itsLocLevTimSize(),
      itsLevTimSize(),
      fUseSubParam(false),
      fHeightValueAvailable(false),
      fHeightLevelDataAvailable(false),
      itsHeightLevelDataHeights(),
      fHeightParamIsRising(false),
      itsHeightParamIndex(static_cast<unsigned long>(-1)),
      fPressureValueAvailable(false),
      fPressureLevelDataAvailable(false),
      fPressureParamIsRising(false),
      itsPressureParamIndex(static_cast<unsigned long>(-1)),
      itsOldParamIndex(static_cast<unsigned long>(-1)),
      fOldFSubParamUsed(false),
      itsPressureLevelDataPressures(),
      itsLongitudeParamIndex(static_cast<unsigned long>(-1)),
      itsLatitudeParamIndex(static_cast<unsigned long>(-1)),
      itsFastStationIdSeekList(),
      itsDataType(NFmiInfoData::kNoDataType),
      itsDummyTime(1900, 1, 1, 1, 1, 1)
{
  try
  {
    InitFastInfo();
    // tehdään First, niin Info-iteraattori on heti käytettävissä (ja mm. levelit
    // osoittaa 1. leveliin, mikä unohtuu helposti käyttäjiltä)
    First();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfo The object being copied
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo::NFmiFastQueryInfo(const NFmiQueryInfo &theInfo)
    : NFmiQueryInfo(theInfo),
      itsParamIndex(),
      itsLocationIndex(),
      itsLevelIndex(),
      itsTimeIndex(),
      itsParamSize(),
      itsLocationSize(),
      itsLevelSize(),
      itsTimeSize()
      //  , itsDataArraySize()
      //  , itsDataArray()
      ,
      itsTemporaryGridData(nullptr),
      itsLocLevTimSize(),
      itsLevTimSize(),
      fUseSubParam(false),
      fHeightValueAvailable(false),
      fHeightLevelDataAvailable(false),
      itsHeightLevelDataHeights(),
      fHeightParamIsRising(false),
      itsHeightParamIndex(static_cast<unsigned long>(-1)),
      fPressureValueAvailable(false),
      fPressureLevelDataAvailable(false),
      fPressureParamIsRising(false),
      itsPressureParamIndex(static_cast<unsigned long>(-1)),
      itsOldParamIndex(static_cast<unsigned long>(-1)),
      fOldFSubParamUsed(false),
      itsPressureLevelDataPressures(),
      itsLongitudeParamIndex(static_cast<unsigned long>(-1)),
      itsLatitudeParamIndex(static_cast<unsigned long>(-1)),
      itsFastStationIdSeekList(),
      itsDataType(NFmiInfoData::kNoDataType),
      itsDummyTime(1900, 1, 1, 1, 1, 1)
{
  try
  {
    InitFastInfo();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theInfo The object being copied
 */

// ----------------------------------------------------------------------
NFmiFastQueryInfo::NFmiFastQueryInfo(const NFmiFastQueryInfo &theInfo)
    : NFmiQueryInfo(theInfo),
      itsParamIndex(theInfo.itsParamIndex),
      itsLocationIndex(theInfo.itsLocationIndex),
      itsLevelIndex(theInfo.itsLevelIndex),
      itsTimeIndex(theInfo.itsTimeIndex),
      itsParamSize(theInfo.itsParamSize),
      itsLocationSize(theInfo.itsLocationSize),
      itsLevelSize(theInfo.itsLevelSize),
      itsTimeSize(theInfo.itsTimeSize)
      //  , itsDataArraySize(theInfo.itsDataArraySize)
      //  , itsDataArray(theInfo.itsDataArray)
      ,
      itsTemporaryGridData(
          theInfo.itsTemporaryGridData ? new NFmiGrid(*theInfo.itsTemporaryGridData) : nullptr),
      itsLocLevTimSize(theInfo.itsLocLevTimSize),
      itsLevTimSize(theInfo.itsLevTimSize),
      fUseSubParam(theInfo.fUseSubParam),
      fHeightValueAvailable(theInfo.fHeightValueAvailable),
      fHeightLevelDataAvailable(theInfo.fHeightLevelDataAvailable),
      itsHeightLevelDataHeights(theInfo.itsHeightLevelDataHeights),
      fHeightParamIsRising(theInfo.fHeightParamIsRising),
      itsHeightParamIndex(theInfo.itsHeightParamIndex),
      fPressureValueAvailable(theInfo.fPressureValueAvailable),
      fPressureLevelDataAvailable(theInfo.fPressureLevelDataAvailable),
      fPressureParamIsRising(theInfo.fPressureParamIsRising),
      itsPressureParamIndex(theInfo.itsPressureParamIndex),
      itsOldParamIndex(theInfo.itsOldParamIndex),
      fOldFSubParamUsed(theInfo.fOldFSubParamUsed),
      itsPressureLevelDataPressures(theInfo.itsPressureLevelDataPressures),
      itsLongitudeParamIndex(theInfo.itsLongitudeParamIndex),
      itsLatitudeParamIndex(theInfo.itsLatitudeParamIndex),
      itsFastStationIdSeekList(theInfo.itsFastStationIdSeekList),
      itsDataType(theInfo.itsDataType),
      itsDummyTime(theInfo.itsDummyTime)
{
  // ei kutsuta InitFastInfo:a!!!!!
}

// ----------------------------------------------------------------------
/*!
 * Destructora
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo::~NFmiFastQueryInfo()
{
  try
  {
    delete itsTemporaryGridData;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiFastQueryInfo::Reset()
{
  try
  {
    fUseSubParam = false;

    ResetParam();
    ResetLocation();
    ResetLevel();
    ResetTime();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return true, jos jokaisen descriptorin First() onnistuu, muutoin false
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::First()
{
  try
  {
    fUseSubParam = false;
    bool firstOK = true;
    firstOK &= FirstParam();
    firstOK &= FirstLocation();
    firstOK &= FirstLevel();
    firstOK &= FirstTime();

    return firstOK;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 * \todo Make subparameters work
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NextParam(bool fIgnoreSubParam)
{
  try
  {
    if (fIgnoreSubParam)
    {
      fUseSubParam = false;
      itsParamIndex++;
      if (itsParamIndex < itsParamSize)
        return true;
      else
      {
        itsParamIndex = itsParamSize;
        return false;
      }
    }

    if (itsParamIndex == static_cast<unsigned long>(-1))
      itsParamDescriptor->Reset();

    bool status = itsParamDescriptor->Next(fIgnoreSubParam);
    itsParamIndex = NFmiQueryInfo::ParamIndex();
    fUseSubParam = itsParamDescriptor->IsSubParamUsed();

    if (fUseSubParam)
    {
      NFmiDataIdent &param =
          itsParamDescriptor->Param(true);  // huom! tässä pitää pyytää yliparametria (->true)
      ChangeCombinedParamParser(param);
    }

    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NextLevel()
{
  try
  {
    itsLevelIndex++;
    if (itsLevelIndex < itsLevelSize)
      return true;

    itsLevelIndex = itsLevelSize;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 * \todo Make subparameters work
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::PreviousParam(bool fIgnoreSubParam)
{
  try
  {
    if (fIgnoreSubParam)
    {
      fUseSubParam = false;
      if (itsParamIndex > 0 &&
          itsParamIndex != static_cast<unsigned long>(-1))  // != -1 pakko tarkistaa, koska se on
                                                            // todellisuudessa yli 4 miljardia
                                                            // ulong:ina
      {
        itsParamIndex--;
        return true;
      }
      else
      {
        itsParamIndex =
            static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
        return false;
      }
    }

    NFmiDataIdent temp(Param());  // tämä asettaa alla olevan queryinfon oikeaan osoitteeseen
    NFmiQueryInfo::Param(temp);
    bool status = NFmiQueryInfo::PreviousParam(fIgnoreSubParam);
    itsParamIndex = NFmiQueryInfo::ParamIndex();
    fUseSubParam = itsParamDescriptor->IsSubParamUsed();
    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::PreviousLocation()
{
  try
  {
    if (itsLocationIndex > 0 &&
        itsLocationIndex !=
            static_cast<unsigned long>(-1))  // != -1 pakko tarkistaa, koska se on todellisuudessa
                                             // yli 4 miljardia ulong:ina
    {
      itsLocationIndex--;
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::PreviousLevel()
{
  try
  {
    if (itsLevelIndex > 0 && itsLevelIndex != static_cast<unsigned long>(
                                                  -1))  // != -1 pakko tarkistaa, koska se on
                                                        // todellisuudessa yli 4 miljardia ulong:ina
    {
      itsLevelIndex--;
      return true;
    }

    itsLevelIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::PreviousTime()
{
  try
  {
    if (itsTimeIndex > 0 && itsTimeIndex != static_cast<unsigned long>(
                                                -1))  // != -1 pakko tarkistaa, koska se on
                                                      // todellisuudessa yli 4 miljardia ulong:ina
    {
      itsTimeIndex--;
      return true;
    }

    itsTimeIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Param(const NFmiParam &theParam)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Param(theParam))
    {
      itsParamIndex = NFmiQueryInfo::ParamIndex();
      fUseSubParam = itsParamDescriptor->IsSubParamUsed();
      return true;
    }

    itsParamIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    fUseSubParam = false;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Param(const NFmiParam &theParam, const NFmiParam &theSubParam)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    // Metodin käyttö: kts. NFmiQueryInfo::Param(NFmiParam&, NFmiParam&)
    if (NFmiQueryInfo::Param(theParam, theSubParam))
    {
      itsParamIndex = NFmiQueryInfo::ParamIndex();
      fUseSubParam = itsParamDescriptor->IsSubParamUsed();
      return true;
    }

    itsParamIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    fUseSubParam = false;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Param(const NFmiDataIdent &theDataIdent)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Param(theDataIdent))
    {
      itsParamIndex = NFmiQueryInfo::ParamIndex();
      fUseSubParam = itsParamDescriptor->IsSubParamUsed();
      return true;
    }

    itsParamIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    fUseSubParam = false;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Location(const NFmiLocation &theLocation)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Location(theLocation))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Location(const unsigned long &theIdent)
{
  try
  {
    if (itsFastStationIdSeekList.empty() == false)
    {  // jos on rakennettu fast-seeker, käytetään sitä
      StationIdSeekContainer::iterator it = itsFastStationIdSeekList.find(theIdent);
      if (it != itsFastStationIdSeekList.end())
      {
        itsLocationIndex = (*it).second;
        return true;
      }
    }
    else if (NFmiQueryInfo::Location(theIdent))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex = static_cast<unsigned long>(
        -1);  // resetoidaan, kun yritetään mennä rajan yli, tai asemaa ei löydy
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Location(const NFmiString &theLocationName)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Location(theLocationName))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLatPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Location(theLonLatPoint, theGridPoint))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NearestLocation(const NFmiLocation &theLocation, double theMaxDistance)
{
  try
  {
    if (NFmiQueryInfo::NearestLocation(theLocation, theMaxDistance))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }
    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::NearestLocation(const NFmiLocation &theLocation,
                                        const NFmiArea *theArea,
                                        double theMaxDistance)
{
  try
  {
    if (NFmiQueryInfo::NearestLocation(theLocation, theArea, theMaxDistance))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NearestPoint(const NFmiPoint &theLatLonPoint)
{
  try
  {
    if (NFmiQueryInfo::NearestPoint(theLatLonPoint))
    {
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      return true;
    }

    itsLocationIndex =
        static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLevelValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Level(const NFmiLevel &theLevelValue)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Level(theLevelValue))
    {
      itsLevelIndex = NFmiQueryInfo::LevelIndex();
      return true;
    }

    itsLevelIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Time(const NFmiMetTime &theTime)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::Time(theTime))
    {
      itsTimeIndex = NFmiQueryInfo::TimeIndex();
      return true;
    }

    itsTimeIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theDirection Undocumented
 * \param theTimeRangeInMinutes Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::TimeToNearestStep(const NFmiMetTime &theTime,
                                          FmiDirection theDirection,
                                          long theTimeRangeInMinutes)
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (NFmiQueryInfo::TimeToNearestStep(theTime, theDirection, theTimeRangeInMinutes))
    {
      itsTimeIndex = NFmiQueryInfo::TimeIndex();
      return true;
    }

    itsTimeIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiFastQueryInfo::Param() const
{
  try
  {
    return itsParamDescriptor->Param(itsParamIndex, !fUseSubParam);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiProducer *NFmiFastQueryInfo::Producer()
{
  try
  {
    if (itsParamSize == 0)
      throw Fmi::Exception(
          BCP, "NFmiFastQueryInfo::Producer: Ei ole parametrejä, miltä voisi pyytää tuottajaa.");
    else if (itsParamIndex >= itsParamSize)
      return itsParamDescriptor->Param(0, !fUseSubParam)
          .GetProducer();  // palautetaan reset-tilassa 1. parametrin tuottaja

    return itsParamDescriptor->Param(itsParamIndex, !fUseSubParam).GetProducer();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Tarvitsin funktion joka varmasti palauttaa tuottajan riippumatta siitä onko
 * Info reset tilassa parametrien suhteen vai ei.
 * \return Palauttaa ensimmäisen parametrin tuottajan, tai dummy tuottajan
 * jos ei ole ollenkaan parametreja.
 */
// ----------------------------------------------------------------------
const NFmiProducer &NFmiFastQueryInfo::FirstParamProducer()
{
  try
  {
    static NFmiProducer dummy;
    if (SizeParams() <= 0)
      return dummy;

    return *(itsParamDescriptor->Param(0, true).GetProducer());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
//   MUISTA OTTAA HUOMIOON GRIDIN PALAUTUS
//   (temporary location muuttuja luokkaan???)

const NFmiLocation *NFmiFastQueryInfo::Location() const
{
  try
  {
    static NFmiLocation dummy(kFloatMissing, kFloatMissing);
    const NFmiLocation *loc = itsHPlaceDescriptor->LocationWithIndex(itsLocationIndex);
    if (loc)
      return loc;

    dummy = NFmiLocation(LatLon());
    return &dummy;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiFastQueryInfo::WorldXY(unsigned long index) const
{
  try
  {
    if (index >= itsLocationSize)
      return NFmiPoint::gMissingLatlon;

    return itsHPlaceDescriptor->WorldXY(index);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa annetun indeksin koordinaatit. Koordinaatticache sijaitsee
 * QueryData-luokan cachessa
 *
 * \return Annetun indeksin koordinaatit
 */
// ----------------------------------------------------------------------

const NFmiPoint &NFmiFastQueryInfo::LatLon(unsigned long index) const
{
  try
  {
    static const NFmiPoint dummy(kFloatMissing, kFloatMissing);

    if (index >= itsLocationSize)
      return dummy;

    return (*itsRefQueryData->LatLonCache())[index];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLevel *NFmiFastQueryInfo::Level() const
{
  try
  {
    return itsVPlaceDescriptor->Level(itsLevelIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

FmiLevelType NFmiFastQueryInfo::LevelType() const
{
  try
  {
    if (itsLevelIndex < SizeLevels())
      return itsVPlaceDescriptor->Level(itsLevelIndex)->LevelType();

    return itsVPlaceDescriptor->Level(0)
        ->LevelType();  // Jos levelIndex ei ollut minkään levelin kohdalla,
    // 1. level type pitäisi aina löytyä ja sen pitäisi olla sama kuin kaikki muutkin
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// HUOM!! VAARA, jos pyydetään aikaa, kun aika ei ole timeindeksin rajoissa,
// palautuu tilapäismuuttujan referenssi ja se ei ole välttämättä käyttökelpoinen
// (tällöin funktiota on tosin käytetty väärin!)

const NFmiMetTime &NFmiFastQueryInfo::Time() const
{
  try
  {
    if (itsTimeDescriptor->Time(itsTimeIndex))
      return NFmiQueryInfo::Time();

    return itsDummyTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// HUOM!! VAARA, jos pyydetään aikaa, kun aika ei ole timeindeksin rajoissa,
// palautuu tilapäismuuttujan referenssi ja se ei ole välttämättä käyttökelpoinen
// (tällöin funktiota on tosin käytetty väärin!)

const NFmiMetTime &NFmiFastQueryInfo::ValidTime() const
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehdä nopeamman!!!!
    if (itsTimeDescriptor->Time(itsTimeIndex))
      return NFmiQueryInfo::ValidTime();

    return itsDummyTime;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// HUOM!! VAARA, jos pyydetään aikaa, kun aika ei ole timeindeksin rajoissa,
// palautuu tilapäismuuttujan referenssi ja se ei ole välttämättä käyttökelpoinen
// (tällöin funktiota on tosin käytetty väärin!)

const NFmiMetTime &NFmiFastQueryInfo::OriginTime() const
{
  try
  {
    // HUOM!!! tästä voisi varmaan tehd<E4> nopeamman!!!!
    // HUOM!!! en ole varma oikeasta toiminnasta, laitan palauttamaan suoraan origin timea
    return itsTimeDescriptor->OriginTime();  // tämän toiminta epävarma ainakin jos käytetään origin
                                             // timebagia validtimebagin sijasta!!!!
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiFastQueryInfo::PeekLocationValue(int theXOffset, int theYOffset) const
{
  try
  {
    unsigned long theHPlaceIndex = PeekLocationIndex(theXOffset, theYOffset);
    size_t idx = Index(itsParamIndex, theHPlaceIndex, itsLevelIndex, itsTimeIndex);
    return NFmiQueryInfo::PeekValue(idx);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// peek xy aika interpolaation kera
float NFmiFastQueryInfo::PeekLocationValue(int theXOffset,
                                           int theYOffset,
                                           const NFmiMetTime &theTime)
{
  try
  {
    unsigned long oldindex = itsLocationIndex;
    const NFmiPoint latlon(this->PeekLocationLatLon(theXOffset, theYOffset));
    float value = this->InterpolatedValue(latlon, theTime);
    itsLocationIndex = oldindex;
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fUseExisting Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiGrid *NFmiFastQueryInfo::GridData(bool fUseExisting)
{
  try
  {
    if (!Grid())
      return nullptr;

    if (fUseExisting)
      return itsTemporaryGridData;

    delete itsTemporaryGridData;
    NFmiDataPool datapool;
    itsTemporaryGridData = new NFmiGrid(*Grid());
    auto *floatarray = new float[Grid()->Size()];

    unsigned long arrayCount = 0;
    ResetLocation();
    while (NextLocation())
    {
      floatarray[arrayCount++] = NFmiQueryInfo::FloatValue();
    }
    datapool.Init(Grid()->Size(), floatarray);
    itsTemporaryGridData->Init(&datapool);
    delete[] floatarray;
    return itsTemporaryGridData;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theInfo The object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiFastQueryInfo &NFmiFastQueryInfo::operator=(const NFmiFastQueryInfo &theInfo)
{
  try
  {
    NFmiQueryInfo::operator=(theInfo);

    itsParamIndex = theInfo.itsParamIndex;
    itsLocationIndex = theInfo.itsLocationIndex;
    itsLevelIndex = theInfo.itsLevelIndex;
    itsTimeIndex = theInfo.itsTimeIndex;

    itsParamSize = theInfo.itsParamSize;
    itsLocationSize = theInfo.itsLocationSize;
    itsLevelSize = theInfo.itsLevelSize;
    itsTimeSize = theInfo.itsTimeSize;

    //  itsDataArraySize = theInfo.itsDataArraySize;
    //  itsDataArray = theInfo.itsDataArray;

    itsTemporaryGridData =
        theInfo.itsTemporaryGridData ? new NFmiGrid(*theInfo.itsTemporaryGridData) : nullptr;

    itsLocLevTimSize = theInfo.itsLocLevTimSize;
    itsLevTimSize = theInfo.itsLevTimSize;
    fUseSubParam = theInfo.fUseSubParam;
    fHeightValueAvailable = theInfo.fHeightValueAvailable;
    fHeightLevelDataAvailable = theInfo.fHeightLevelDataAvailable;
    itsHeightLevelDataHeights = theInfo.itsHeightLevelDataHeights;
    fHeightParamIsRising = theInfo.fHeightParamIsRising;
    itsHeightParamIndex = theInfo.itsHeightParamIndex;
    fPressureValueAvailable = theInfo.fPressureValueAvailable;
    fPressureLevelDataAvailable = theInfo.fPressureLevelDataAvailable;
    fPressureParamIsRising = theInfo.fPressureParamIsRising;
    itsPressureParamIndex = theInfo.itsPressureParamIndex;
    itsOldParamIndex = theInfo.itsOldParamIndex;
    fOldFSubParamUsed = theInfo.fOldFSubParamUsed;
    itsPressureLevelDataPressures = theInfo.itsPressureLevelDataPressures;
    itsLongitudeParamIndex = theInfo.itsLongitudeParamIndex;
    itsLatitudeParamIndex = theInfo.itsLatitudeParamIndex;
    itsDataType = theInfo.itsDataType;
    itsDummyTime = theInfo.itsDummyTime;

    return *this;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Hakee currentista pisteestä, parametrista ja ajasta arvoja
// niin kauan että löytyy kaksi "non missing" arvoa.
static bool GetFirstAndSecondParamValues(NFmiFastQueryInfo *theInfo,
                                         float &theValue1,
                                         float &theValue2)
{
  try
  {
    float value = kFloatMissing;
    int valuesFound = 0;
    for (theInfo->ResetLevel(); theInfo->NextLevel();)
    {
      value = theInfo->FloatValue();
      if (value != kFloatMissing)
      {
        if (valuesFound == 0)
          theValue1 = value;
        else if (valuesFound == 1)
        {
          theValue2 = value;
          return true;
        }
        valuesFound++;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Palautetaan muutama aikaindeksi joita käydään läpi kun etsitää korkeus dataa.
// Tämä siis sen takia, että datassa ei aina ole kaikilla ajan hetkillä kuin
// puuttuvaa korkeus dataa. Tässä fiksu arvaus mitä ajan hetkiä käydää läpi, koska jos
// käy kaikki hilapisteet ja ajat läpi, on se liian raskasta.
static std::vector<unsigned long> FillTimeIndexies(NFmiFastQueryInfo *theInfo)
{
  try
  {
    int timeSize = theInfo->SizeTimes();
    std::vector<unsigned long> timeIndexies;
    if (timeSize > 0)
    {
      timeIndexies.push_back(
          timeSize /
          2);  // laitetaan ensimmäiseksi puoliväli, koska alusta ja lopusta puuttuu helposti dataa
      if (timeSize > 4)
      {
        timeIndexies.push_back(timeSize / 4);  // laitetaan sitten neljäsosan kohta, koska alusta ja
                                               // lopusta puuttuu helposti dataa
        timeIndexies.push_back(3 * timeSize / 4);  // laitetaan sitten kolmannen neljäsosan kohta,
                                                   // koska alusta ja lopusta puuttuu helposti dataa
      }
      if (timeSize > 16)
      {
        timeIndexies.push_back(0);  // laitetaan vielä alusta muutamia aikoja mukaan
        timeIndexies.push_back(1);
        timeIndexies.push_back(2);
        timeIndexies.push_back(3);
        timeIndexies.push_back(timeSize - 1);  // laitetaan vielä lopusta muutamia aikoja mukaan
        timeIndexies.push_back(timeSize - 2);
        timeIndexies.push_back(timeSize - 3);
        timeIndexies.push_back(timeSize - 4);
      }
    }
    return timeIndexies;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static int GetGridDataIndex(int xSize, int xInd, int yInd)
{
  try
  {
    return (xSize * yInd) + xInd;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Palautetaan muutama locationindeksi joita käydään läpi kun etsitää korkeus dataa.
// Tämä siis sen takia, että datassa ei aina ole kaikissa paikoissa kuin
// puuttuvaa korkeus dataa. Tässä fiksu arvaus mitä paikkoja käydää läpi, koska jos
// käy kaikki hilapisteet ja ajat läpi, on se liian raskasta.
static std::vector<unsigned long> FillLocationIndexies(NFmiFastQueryInfo *theInfo)
{
  try
  {
    if (theInfo->IsGrid())
    {  // hiladatalle palautetaan seuraavat paikkaindeksit
      int xSize = theInfo->Grid()->XNumber();
      int ySize = theInfo->Grid()->YNumber();
      std::vector<unsigned long> locationIndexies;
      if (xSize > 0 && ySize > 0)
      {
        locationIndexies.push_back(GetGridDataIndex(
            xSize, xSize / 2, ySize / 2));  // laitetaan ensimmäiseksi hilan puoliväli, koska
                                            // reunoista puuttuu helposti dataa
        locationIndexies.push_back(0);      // laitetaan 1. hilapiste
        locationIndexies.push_back(theInfo->SizeLocations() -
                                   1);  // laitetaan mukaan vielä viimeinen hilapiste
        if (xSize > 4 && ySize > 4)
        {  // laitetaan sitten 'neljännes' kehä pisteet ks. kaava kuva
           // --------
           // -*-*-*--
           // --------
           // -*---*--
           // --------
           // -*-*-*--
           // --------

          locationIndexies.push_back(GetGridDataIndex(xSize, xSize / 4, ySize / 4));
          locationIndexies.push_back(GetGridDataIndex(xSize, xSize / 4, ySize / 2));
          locationIndexies.push_back(GetGridDataIndex(xSize, xSize / 4, 3 * ySize / 4));
          locationIndexies.push_back(GetGridDataIndex(xSize, xSize / 2, ySize / 4));
          locationIndexies.push_back(GetGridDataIndex(xSize, xSize / 2, 3 * ySize / 4));
          locationIndexies.push_back(GetGridDataIndex(xSize, 3 * xSize / 4, ySize / 4));
          locationIndexies.push_back(GetGridDataIndex(xSize, 3 * xSize / 4, ySize / 2));
          locationIndexies.push_back(GetGridDataIndex(xSize, 3 * xSize / 4, 3 * ySize / 4));
        }
      }
      return locationIndexies;
    }

    // asema datalle palautetaan seuraavat paikka indeksit
    int ssize = theInfo->SizeLocations();
    std::vector<unsigned long> locationIndexies;
    if (ssize > 0)
    {
      locationIndexies.push_back(
          ssize /
          2);  // laitetaan ensimmäiseksi puoliväli, koska alusta ja lopusta puuttuu helposti dataa
      if (ssize > 4)
      {
        locationIndexies.push_back(ssize / 4);  // laitetaan sitten neljäsosan kohta, koska alusta
                                                // ja lopusta puuttuu helposti dataa
        locationIndexies.push_back(3 * ssize / 4);  // laitetaan sitten kolmannen neljäsosan kohta,
                                                    // koska alusta ja lopusta puuttuu helposti
                                                    // dataa
      }
      if (ssize > 16)
      {
        locationIndexies.push_back(0);  // laitetaan vain alusta jotain lukuja listaan
        locationIndexies.push_back(1);
        locationIndexies.push_back(2);
        locationIndexies.push_back(3);
        locationIndexies.push_back(4);
        locationIndexies.push_back(ssize - 1);  // laietaan vain lopusta jotain indeksejä mukaan
        locationIndexies.push_back(ssize - 2);
        locationIndexies.push_back(ssize - 3);
        locationIndexies.push_back(ssize - 4);
        locationIndexies.push_back(ssize - 5);
      }
    }
    return locationIndexies;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tutkii onko annetun parametrin arvot korkeus leveleiden mukaan
// nousevassa vai laskevassa järjestyksessä.
// Jos datassa on jotain puuttuvaa osiota, voi joutua tarkistamaan/käymään läpi muutamia
// aikoja/hilapisteitä.
// Asettaa lopuksi ajan ja paikan ennalleen.
static bool IsParamValuesRising(NFmiFastQueryInfo *theInfo, unsigned long theParamIndex)
{
  try
  {
    bool rising = false;
    if (theInfo->RefRawData() && theInfo->ParamIndex(theParamIndex))
    {
      unsigned long oldTimeIndex = theInfo->TimeIndex();
      unsigned long oldLocationIndex = theInfo->LocationIndex();
      std::vector<unsigned long> timeIndexies(FillTimeIndexies(theInfo));  // jos ei heti tärppää
                                                                           // (on puuttuvaa dataa
                                                                           // joissain ajoissa),
                                                                           // pitää käydä ehkä
                                                                           // useampia aikoja läpi
      std::vector<unsigned long> locIndexies(
          FillLocationIndexies(theInfo));  // jos ei heti tärppää (on puuttuvaa dataa joissain
      // pisteissä), pitää käydä ehkä useampia paikkoja läpi
      float firstValue = kFloatMissing;
      float secondValue = kFloatMissing;
      for (int j = 0; j < static_cast<int>(timeIndexies.size()); j++)
      {
        theInfo->TimeIndex(timeIndexies[j]);
        for (int i = 0; i < static_cast<int>(locIndexies.size()); i++)
        {
          theInfo->LocationIndex(locIndexies[i]);
          if (GetFirstAndSecondParamValues(theInfo, firstValue, secondValue))
          {
            rising = secondValue > firstValue;
            theInfo->TimeIndex(oldTimeIndex);
            theInfo->LocationIndex(oldLocationIndex);
            return rising;
          }
        }
      }
      theInfo->TimeIndex(oldTimeIndex);
      theInfo->LocationIndex(oldLocationIndex);
    }
    return rising;  // palautetaan 'false' arvo, vaikka ei ole tietoa, tämä on sitten oletus arvo
                    // puuttuvan datan takia.
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// tutkii onko annetun infon painepinta levelit laskevassa vai nousevassa järjestyksessä
// Laskeva = 1000, 925, 850, 700 jne.
// Nouseva = 300, 500, 700, 850 jne.
// HUOM! tämä toimii samoin painepinta datoille että height-level datoille
static bool IsLevelValuesInRisingOrder(NFmiFastQueryInfo *theInfo)
{
  try
  {
    unsigned long oldLevelIndex = theInfo->LevelIndex();
    theInfo->LevelIndex(0);
    double pressureLevel1 = theInfo->Level()->LevelValue();
    theInfo->LevelIndex(1);
    double pressureLevel2 = theInfo->Level()->LevelValue();
    bool rising = pressureLevel2 > pressureLevel1;

    theInfo->LevelIndex(oldLevelIndex);
    return rising;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------
void NFmiFastQueryInfo::InitFastInfo()
{
  try
  {
    if (itsParamDescriptor && itsHPlaceDescriptor && itsVPlaceDescriptor && itsTimeDescriptor)
    {
      itsParamSize = NFmiQueryInfo::SizeParams();
      itsLocationSize = NFmiQueryInfo::SizeLocations();
      itsLevelSize = NFmiQueryInfo::SizeLevels();
      itsTimeSize = NFmiQueryInfo::SizeTimes();

      itsParamIndex = NFmiQueryInfo::ParamIndex();
      itsLocationIndex = NFmiQueryInfo::LocationIndex();
      itsLevelIndex = NFmiQueryInfo::LevelIndex();
      itsTimeIndex = NFmiQueryInfo::TimeIndex();

      if (IsGrid())
      {
        // HUOM! Nämä alustukset pitäisi tehdä NFmiQueryInfo -luokassa, mutta jos
        // qdataa käytetään vain NFmiFastQueryInfo -luokan kautta (kuten pitäisi), ei tästä tule
        // ongelmaa.
        itsGridXNumber = Grid()->XNumber();
        itsGridYNumber = Grid()->YNumber();
        fIsStrechableGlobalGrid = NFmiGrid::IsStrechableGlobalGrid(*Grid());
      }

      itsLocLevTimSize = itsLocationSize * itsLevelSize * itsTimeSize;
      itsLevTimSize = itsLevelSize * itsTimeSize;
      fUseSubParam = itsParamDescriptor->IsSubParamUsed();

      unsigned long oldParamIndex = itsParamIndex;

      // tutkitaan onko level dataa ja jos on, löytyykö korkeus parametri
      itsHeightParamIndex = static_cast<unsigned long>(-1);
      if (Param(kFmiGeomHeight))
        itsHeightParamIndex = itsParamIndex;
      else if (Param(kFmiGeopHeight))
        itsHeightParamIndex = itsParamIndex;
      fHeightValueAvailable =
          (itsLevelSize > 1) && (itsHeightParamIndex != static_cast<unsigned long>(-1));

      fHeightLevelDataAvailable = false;
      if (!fHeightValueAvailable && itsLevelSize > 1)
      {
        unsigned long oldLevelIndex = itsLevelIndex;
        LevelIndex(0);  // asetetaan varmuuden vuoksi 1. leveliin
        if (Level()->LevelType() == kFmiHeight)
        {
          fHeightLevelDataAvailable = true;
          itsHeightLevelDataHeights = CalcHeightLevelDataHeights();
        }
        LevelIndex(oldLevelIndex);
      }

      fHeightParamIsRising = false;
      if (fHeightValueAvailable)
        fHeightParamIsRising = IsParamValuesRising(this, itsHeightParamIndex);
      else if (fHeightLevelDataAvailable)
        fHeightParamIsRising = IsLevelValuesInRisingOrder(this);

      // tutkitaan onko level dataa ja jos on, löytyykö paine parametri
      itsPressureParamIndex = static_cast<unsigned long>(-1);
      if (Param(kFmiPressure))
        itsPressureParamIndex = itsParamIndex;
      fPressureValueAvailable =
          (itsLevelSize > 1) && (itsPressureParamIndex != static_cast<unsigned long>(-1));
      fPressureLevelDataAvailable = false;
      if (!fPressureValueAvailable && itsLevelSize > 1)
      {
        unsigned long oldLevelIndex = itsLevelIndex;
        LevelIndex(0);  // asetetaan varmuuden vuoksi 1. leveliin
        if (Level()->LevelType() == kFmiPressureLevel)
        {
          fPressureLevelDataAvailable = true;
          itsPressureLevelDataPressures = CalcPressureLevelDataPressures();
        }
        LevelIndex(oldLevelIndex);
      }

      fPressureParamIsRising = false;
      if (fPressureValueAvailable)
        fPressureParamIsRising = IsParamValuesRising(this, itsPressureParamIndex);
      else if (fPressureLevelDataAvailable)
        fPressureParamIsRising = IsLevelValuesInRisingOrder(this);

      Param(kFmiLongitude);
      itsLongitudeParamIndex = ParamIndex();
      Param(kFmiLatitude);
      itsLatitudeParamIndex = ParamIndex();

      itsParamIndex = oldParamIndex;

      // initialisoidaan vielä mahdollinen station-id fast-seek indeksi lista
      itsFastStationIdSeekList.clear();
      if (itsHPlaceDescriptor->IsLocation())
      {
        for (itsHPlaceDescriptor->Reset(); itsHPlaceDescriptor->Next();)
        {
          itsFastStationIdSeekList[itsHPlaceDescriptor->Location()->GetIdent()] =
              itsHPlaceDescriptor->Index();
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee paine datan painepinta level tietojen avulla. xSize kertoo matriisin x koon ja levelSize y
// koon.
// Jokaiseen columniin siis sijoitetaan vain samat paine arvot uudestaan ja uudestaan.
std::vector<float> NFmiFastQueryInfo::CalcPressureLevelDataPressures()
{
  try
  {
    std::vector<float> pressureLevels;
    for (ResetLevel(); NextLevel();)
    {
      pressureLevels.push_back(static_cast<float>(Level()->LevelValue()));
    }
    return pressureLevels;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Sama kuin CalcPressureLevelDataPressures, mutta heightlevel-datoille
std::vector<float> NFmiFastQueryInfo::CalcHeightLevelDataHeights()
{
  try
  {
    std::vector<float> heightLevels;
    for (ResetLevel(); NextLevel();)
    {
      heightLevels.push_back(static_cast<float>(Level()->LevelValue()));
    }
    return heightLevels;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::IndexFloatValue(size_t theIndex, float theValue)
{
  try
  {
    return itsRefRawData->SetValue(theIndex, theValue);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeIndexOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// theTimeIndexOffset itseasiassa aikasiirto suoraan datan timebagin resoluutiona
// esim. 1 voi tarkoittaa 1 tunnin tai 6 tunnin siirtoa eteenpäin riippuen resoluutiosta.

float NFmiFastQueryInfo::PeekTimeValue(int theTimeIndexOffset)
{
  try
  {
    unsigned long timeIndex = itsTimeIndex;
    timeIndex += theTimeIndexOffset;
    if (timeIndex < itsTimeSize)
    {
      size_t idx = Index(itsParamIndex, itsLocationIndex, itsLevelIndex, timeIndex);
      return NFmiQueryInfo::PeekValue(idx);
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theQueryInfo Undocumented
 * \param fIgnoreLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel)
{
  try
  {
    bool status = NFmiQueryInfo::SetDescriptors(theQueryInfo, fIgnoreLevel);

    itsParamIndex = NFmiQueryInfo::ParamIndex();
    itsLocationIndex = NFmiQueryInfo::LocationIndex();
    itsLevelIndex = NFmiQueryInfo::LevelIndex();
    itsTimeIndex = NFmiQueryInfo::TimeIndex();

    return status;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theOtherInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiFastQueryInfo::CreateCombinedInfo(NFmiQueryInfo *theOtherInfo)
{
  try
  {
    return new NFmiFastQueryInfo(itsParamDescriptor->Combine(theOtherInfo->ParamDescriptor()),
                                 itsTimeDescriptor->Combine(theOtherInfo->TimeDescriptor()),
                                 itsHPlaceDescriptor->Combine(theOtherInfo->HPlaceDescriptor()),
                                 *itsVPlaceDescriptor);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 *
 * Note: Modifies current location! (Marko: Minusta ei muuta, ja tämän ja emon metodin voisi muuttaa
 * const:iksi)
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiFastQueryInfo::PeekLocationLatLon(int theXOffset, int theYOffset)
{
  try
  {
    if (IsGrid())
    {
      unsigned long index = itsLocationIndex;

      int peekXIndex = (index % itsGridXNumber) + theXOffset;
      int peekYIndex = (index / itsGridXNumber) + theYOffset;
      if (peekXIndex < 0 || peekXIndex >= int(itsGridXNumber) ||  // x- ja y-indeksien pitää pysyä
                                                                  // gridin sisällä offsettien kera!
          peekYIndex < 0 || peekYIndex >= int(itsGridYNumber))
      {
        return NFmiPoint::gMissingLatlon;
      }

      unsigned long peekindex = index + theYOffset * itsGridXNumber + theXOffset;

      return LatLon(peekindex);
    }
    return NFmiPoint::gMissingLatlon;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NextActiveParam()
{
  try
  {
    return NextActiveParam(true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NextActiveParam(bool fIgnoreSubParam)
{
  try
  {
    for (; NextParam(fIgnoreSubParam);)
    {
      if (IsActiveParam())
        return true;
    }
    return false;  // toteutetaan myöhemmin
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::IsActiveParam()
{
  try
  {
    return Param().IsActive();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::ActivateParam(bool newState, bool /* fIgnoreSubParam */)
{
  try
  {
    return itsParamDescriptor->SetActivity(newState, itsParamIndex);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param newState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::ActivateParam(bool newState)
{
  try
  {
    return ActivateParam(newState, true);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin paikan suhteellisen sijainnin (esim. gridin areassa)
 *
 * \return The current relative location
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiFastQueryInfo::RelativePoint() const
{
  try
  {
    if (IsGrid())
    {
      int currentXIndex = (itsLocationIndex % itsGridXNumber);
      int currentYIndex = (itsLocationIndex / itsGridXNumber);
      double relativeX = currentXIndex / (itsGridXNumber - 1.);
      double relativeY = currentYIndex / (itsGridYNumber - 1.);
      return NFmiPoint(relativeX, relativeY);
    }
    return NFmiPoint();  // ei-grid:in tapausta ei voi laskea
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiFastQueryInfo::PeekValue(int theTimeOffset, int theXOffset, int theYOffset)
{
  try
  {
    unsigned long locationIndex = itsLocationIndex;

    if (IsGrid())
    {
      int currentXIndex = (locationIndex % itsGridXNumber) + theXOffset;
      int currentYIndex = (locationIndex / itsGridXNumber) + theYOffset;

      // voiko tämän seuraavan tarkistuksen poistaa, kun indeksi tarkistetaan kuitenkin
      // Index-metodissa??
      if (currentXIndex >= 0 &&
          currentXIndex < int(itsGridXNumber) &&  // x- ja y-indeksien pitää pysyä gridin sisällä
                                                  // offsettien kera!
          currentYIndex >= 0 && currentYIndex < int(itsGridYNumber))
        locationIndex = locationIndex + theYOffset * itsGridXNumber + theXOffset;
      else
        locationIndex = static_cast<unsigned long>(-1);
    }
    else
      locationIndex = locationIndex + theXOffset;

    size_t idx = Index(itsParamIndex, locationIndex, itsLevelIndex, itsTimeIndex + theTimeOffset);
    return NFmiQueryInfo::PeekValue(idx);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 *
 * Note: This does not set the sub parameter flag properly if you are
 *       switching between sub parameters. Switch only between regular
 *       parameters and you are safe.
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::ParamIndex(unsigned long theIndex)
{
  try
  {
    if (theIndex >= itsParamSize)
    {
      itsParamIndex = static_cast<unsigned long>(-1);
      return false;
    }
    itsParamIndex = theIndex;
    fUseSubParam = false;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::LocationIndex(unsigned long theIndex)
{
  try
  {
    if (theIndex >= itsLocationSize)
    {
      itsLocationIndex = static_cast<unsigned long>(-1);
      return false;
    }
    itsLocationIndex = theIndex;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::LevelIndex(unsigned long theIndex)
{
  try
  {
    if (theIndex >= itsLevelSize)
    {
      itsLevelIndex = static_cast<unsigned long>(-1);
      return false;
    }
    itsLevelIndex = theIndex;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::TimeIndex(unsigned long theIndex)
{
  try
  {
    if (theIndex >= itsTimeSize)
    {
      itsTimeIndex = static_cast<unsigned long>(-1);
      return false;
    }
    itsTimeIndex = theIndex;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::MoveUp(int moveBy)  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = itsLocationIndex / itsGridXNumber;
    yLoc += moveBy;
    if (yLoc < 0 || static_cast<unsigned int>(yLoc) >= itsGridYNumber)
      return false;  // ei liikuteta ulkopuolelle, eikä aseteta resettiin!!!

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::MoveDown(int moveBy)  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = itsLocationIndex / itsGridXNumber;
    yLoc -= moveBy;
    if (yLoc < 0 || static_cast<unsigned int>(yLoc) >= itsGridYNumber)
      return false;  // ei liikuteta ulkopuolelle, eikä aseteta resettiin!!!

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::MoveLeft(int moveBy)  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = itsLocationIndex / itsGridXNumber;
    xLoc -= moveBy;
    if (xLoc < 0 || static_cast<unsigned int>(xLoc) >= itsGridXNumber)
      return false;  // ei liikuteta ulkopuolelle, eikä aseteta resettiin!!!

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::MoveRight(int moveBy)  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = itsLocationIndex / itsGridXNumber;
    xLoc += moveBy;
    if (xLoc < 0 || static_cast<unsigned int>(xLoc) >= itsGridXNumber)
      return false;  // ei liikuteta ulkopuolelle, eikä aseteta resettiin!!!

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Top()  // toimii vain gridi datalle oikein!!!
{
  try
  {
    // tämä on hilan yläosa eli suurin y-indeksi
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = itsGridYNumber - 1;

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Bottom()  // toimii vain gridi datalle oikein!!!
{
  try
  {
    // tämä on hilan alaosa, eli 0 y-indeksi
    int xLoc = itsLocationIndex % itsGridXNumber;
    int yLoc = 0;

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Left()  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = 0;
    int yLoc = itsLocationIndex / itsGridXNumber;

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::Right()  // toimii vain gridi datalle oikein!!!
{
  try
  {
    int xLoc = itsGridXNumber - 1;
    int yLoc = itsLocationIndex / itsGridXNumber;

    itsLocationIndex = yLoc * itsGridXNumber + xLoc;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan XY-koordinaatit datamatriisiin
 *
 * \param theArea The projection in which to store the XY coordinates
 */
// ----------------------------------------------------------------------

Fmi::CoordinateMatrix NFmiFastQueryInfo::LocationsXY(const NFmiArea &theArea) const
{
  try
  {
    if (!IsGrid())
      return Fmi::CoordinateMatrix(0, 0);

    auto coords = LocationsWorldXY(theArea);

    auto nx = coords.width();
    auto ny = coords.height();

    for (auto j = 0ull; j < ny; j++)
    {
      for (auto i = 0ull; i < nx; i++)
      {
        coords.set(i, j, theArea.WorldXYToXY(coords(i, j)));
      }
    }

    return coords;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan WorldXY-koordinaatit datamatriisiin
 *
 * \param theMatrix The matrix in which to store the coordinates
 * \param theArea The projection in which to store the world XY coordinates
 */
// ----------------------------------------------------------------------

Fmi::CoordinateMatrix NFmiFastQueryInfo::LocationsWorldXY(const NFmiArea &theArea) const
{
  try
  {
    if (!IsGrid())
      return Fmi::CoordinateMatrix(0, 0);

    auto coords = CoordinateMatrix();

    Fmi::CoordinateTransformation transformation(Area()->SpatialReference(),
                                                 theArea.SpatialReference());
    coords.transform(transformation);
    return coords;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa hilan data-arvot halutulta croppaus alueelta annettuun datamatriisiin.
 * Croppaus alue annetaan hila indekseinä x1,y1 ja x2,y2, jotka muodostavat hilaindeksi-
 * laatikon, joka cropataan (x ja y indeksit alkavat 0:sta).
 * Jos asema dataa, ei tehdä mitään.
 *
 * \param theMatrix The matrix in which to store the values
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::CroppedValues(int x1, int y1, int x2, int y2) const
{
  try
  {
    auto nx = static_cast<int>(x2 - x1 + 1);
    auto ny = static_cast<int>(y2 - y1 + 1);

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    if (IsGrid())
    {
      long long idx = Index(itsParamIndex, 0, itsLevelIndex, itsTimeIndex);
      long long offset = Index(itsParamIndex, 1, itsLevelIndex, itsTimeIndex) - idx;
      long long totalLineOffset = offset * itsGridXNumber;  // tällä hypitään kokonaisia rivejä
      long long startLineOffset = offset * x1;              // tällä hypitään rivin alku
      long long endLineOffset = offset * (itsGridXNumber - x2 - 1);  // tällä hypitään rivin loppu

      // Mika: Must have this loop order so that the offset trick works

      idx += totalLineOffset * y1;  // hypätään ensin alusta halutulle riville
      for (int j = 0; j < ny; j++)
      {
        idx += startLineOffset;       // hypätään cropatun rivin alusta haluttuun kohtaan
        for (int i = 0; i < nx; i++)  // pientä optimointia olisi jos for loopit saisi toisin päin
        {
          values[i][j] = NFmiQueryInfo::PeekValue(idx);
          idx += offset;
        }
        idx += endLineOffset;  // lopuksi pitää hypätä vielä cropattu rivin loppu pois
      }
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static float InterpolationHelper(float theValue1, float theValue2, float theFactor1)
{
  try
  {
    if (theValue1 == kFloatMissing && theValue2 == kFloatMissing)
      return kFloatMissing;
    else if (theValue1 == kFloatMissing || theValue2 == kFloatMissing)
    {
      // Jos toinen arvoista puuttuva, palautetaan suoraan se arvo, jota lähempänä theFactor1 on,
      // oli se puuttuva tai ei Jos theFactor1 on 1, ollaan theValue1:n ajassa kiinni ja jos se on
      // 0, ollaan theValue2:n ajassa kiinni
      if (theFactor1 <= 0.5)
        return theValue2;
      else
        return theValue1;
    }

    return theFactor1 * theValue1 + (1 - theFactor1) * theValue2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Luokka jonka avulla tehdään matriisi aikainterpolaatioita hallitummin
class TimeInterpolationData
{
 public:
  bool doNearestTimeIfPossible = false;
  bool isGrid = false;
  bool hasWantedTime = false;
  unsigned long oldTimeIndex = gMissingIndex;
  bool isInsideAtAll = false;
  NFmiMetTime previousTime = NFmiMetTime::gMissingTime;
  unsigned long previousTimeIndex = gMissingIndex;
  float previousToInterpolatedTimeDifferenceInMinutes = 0;
  NFmiMetTime nextTime = NFmiMetTime::gMissingTime;
  unsigned long nextTimeIndex = gMissingIndex;
  float previousToNextTimeDifferenceInMinutes = 0;

  TimeInterpolationData(NFmiFastQueryInfo &info,
                        const NFmiMetTime &theInterpolatedTime,
                        long theTimeRangeInMinutes = kLongMissing,
                        bool doNearestTimeIfPossible = false)
  {
    try
    {
      this->doNearestTimeIfPossible = doNearestTimeIfPossible;
      oldTimeIndex = info.TimeIndex();
      isGrid = info.IsGrid();
      if (isGrid)
      {
        if (info.Time(theInterpolatedTime))
        {
          hasWantedTime = true;
          return;
        }
        else
        {
          if (this->doNearestTimeIfPossible)
          {
            if (info.TimeToNearestStep(theInterpolatedTime, kCenter, theTimeRangeInMinutes))
            {
              SetPreviousTimeValues(info, theInterpolatedTime);
            }
          }
          else
          {
            isInsideAtAll = info.IsInside(theInterpolatedTime);
            if (isInsideAtAll)
            {
              if (info.TimeToNearestStep(theInterpolatedTime, kBackward, theTimeRangeInMinutes) ||
                  this->doNearestTimeIfPossible)
              {
                SetPreviousTimeValues(info, theInterpolatedTime);
                if (info.TimeToNearestStep(theInterpolatedTime, kForward, theTimeRangeInMinutes))
                {
                  nextTime = info.Time();
                  nextTimeIndex = info.TimeIndex();
                  previousToNextTimeDifferenceInMinutes =
                      static_cast<float>(nextTime.DifferenceInMinutes(previousTime));
                }
              }
            }
          }
        }
      }
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  bool CanGetValues()
  {
    try
    {
      if (!isGrid)
        return false;
      if (hasWantedTime)
        return true;
      if (doNearestTimeIfPossible)
      {
        if (previousTimeIndex == gMissingIndex && nextTimeIndex == gMissingIndex)
          return false;
      }
      else
      {
        if (!isInsideAtAll)
          return false;
        if (previousTimeIndex == gMissingIndex || nextTimeIndex == gMissingIndex)
          return false;
      }

      return true;
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  bool HasWantedTime()
  {
    try
    {
      return hasWantedTime;
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  bool DoNearestTime()
  {
    try
    {
      if (doNearestTimeIfPossible)
      {
        if (previousTimeIndex != gMissingIndex && nextTimeIndex == gMissingIndex)
          return true;
        if (previousTimeIndex == gMissingIndex && nextTimeIndex != gMissingIndex)
          return true;
      }

      return false;
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }

  void SetPreviousTimeValues(NFmiFastQueryInfo &info, const NFmiMetTime &theInterpolatedTime)
  {
    try
    {
      previousTime = info.Time();
      previousTimeIndex = info.TimeIndex();
      previousToInterpolatedTimeDifferenceInMinutes =
          static_cast<float>(theInterpolatedTime.DifferenceInMinutes(previousTime));
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }
};

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot annettuun datamatriisiin haluttuun
 * aikaan interpoloituna.
 *
 * \param theMatrix The matrix in which to store the values
 * \param theInterpolatedTime The desired time
 */
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot annettuun datamatriisiin haluttuun
 * aikaan interpoloituna.
 *
 * \param theMatrix The matrix in which to store the values
 * \param theInterpolatedTime The desired time
 */
// ----------------------------------------------------------------------
NFmiDataMatrix<float> NFmiFastQueryInfo::Values(const NFmiMetTime &theInterpolatedTime)
{
  try
  {
    return NFmiFastQueryInfo::Values(theInterpolatedTime, kLongMissing);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::Values(const NFmiMetTime &theInterpolatedTime,
                                                long theTimeRangeInMinutes,
                                                bool doNearestTimeIfPossible)
{
  try
  {
    TimeInterpolationData timeInterpolationData(
        *this, theInterpolatedTime, theTimeRangeInMinutes, doNearestTimeIfPossible);
    if (!timeInterpolationData.CanGetValues())
    {
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return {};
    }

    // Handle exact existing time
    if (timeInterpolationData.HasWantedTime())
    {
      auto values = Values();
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    if (timeInterpolationData.DoNearestTime())
    {
      if (timeInterpolationData.previousTimeIndex != gMissingIndex)
        TimeIndex(timeInterpolationData.previousTimeIndex);
      else
        TimeIndex(timeInterpolationData.nextTimeIndex);
      auto values = Values();
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    // Extract leftside and rightside data values

    TimeIndex(timeInterpolationData.previousTimeIndex);
    auto values1 = Values();

    TimeIndex(timeInterpolationData.nextTimeIndex);
    auto values2 = Values();

    auto diff1 = timeInterpolationData.previousToInterpolatedTimeDifferenceInMinutes;
    auto diff2 = timeInterpolationData.previousToNextTimeDifferenceInMinutes;

    float factor = 1 - diff1 / diff2;

    // Then interpolate the data. We must be careful to obey the interpolation
    // rules of both discrete data and combined parameters

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    unsigned long param = Param().GetParam()->GetIdent();

    const auto nx = values1.NX();
    const auto ny = values2.NY();

    if (param == kFmiTotalWindMS)
    {
      NFmiTotalWind resultWind(itsInfoVersion);
      for (std::size_t j = 0; j < ny; j++)
        for (std::size_t i = 0; i < nx; i++)
        {
          NFmiTotalWind tempWind1(values1[i][j], kFmiPackedWind, itsInfoVersion);
          NFmiTotalWind tempWind2(values2[i][j], kFmiPackedWind, itsInfoVersion);
          resultWind.SetToWeightedMean(
              &tempWind1, factor, &tempWind2, 1 - factor, &tempWind1, 0, &tempWind1, 0);
          values1[i][j] = resultWind.TransformedFloatValue();
        }
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values1;
    }

    if (param == kFmiWeatherAndCloudiness)
    {
      NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);
      for (std::size_t j = 0; j < ny; j++)
        for (std::size_t i = 0; i < nx; i++)
        {
          NFmiWeatherAndCloudiness tempWeather1(
              values1[i][j], kFmiPackedWeather, kFloatMissing, itsInfoVersion);
          NFmiWeatherAndCloudiness tempWeather2(
              values2[i][j], kFmiPackedWeather, kFloatMissing, itsInfoVersion);
          resultWeather.SetToWeightedMean(
              &tempWeather1, factor, &tempWeather2, 1 - factor, &tempWeather1, 0, &tempWeather1, 0);
          values1[i][j] = resultWeather.TransformedFloatValue();
        }
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values1;
    }

    if (interp != kLinearly)
    {
      TimeIndex(timeInterpolationData.oldTimeIndex);
      if (factor > 0.5)
        return values1;
      return values2;
    }

    for (std::size_t j = 0; j < ny; j++)
      for (std::size_t i = 0; i < nx; i++)
        values1[i][j] = InterpolationHelper(values1[i][j], values2[i][j], factor);

    TimeIndex(timeInterpolationData.oldTimeIndex);

    return values1;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetLevelToVec(std::vector<float> &values)
{
  try
  {
    FirstLocation();
    size_t startIndex = Index();
    size_t step = SizeTimes() * SizeLevels();
    size_t count = SizeLocations();

    if (!GetValues(startIndex, step, count, values))
    {
      // std::cout << __FUNCTION__ << '(' << startIndex << ',' << step << ',' << count << ',' <<
      // &values << ") out of bounds!" << std::endl;
      return false;
    }

    if (IsSubParamUsed())
    {
      if (itsCombinedParamParser)
      {
        std::transform(begin(values),
                       end(values),
                       begin(values),
                       [this](const float &fVal) { return SubValueFromFloat(fVal); });
      }
      else
        throw Fmi::Exception(BCP, "CombinedParamParser missing!");
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetLevelToVecPartial(
    size_t x1, size_t y1, size_t x2, size_t y2, std::vector<float> &values)
{
  try
  {
    size_t columnCount = x2 - x1 + 1;
    if (columnCount <= 0 || columnCount > itsGridXNumber)
      throw Fmi::Exception(BCP, "Degenerate X axis");
    size_t columnStep = SizeTimes() * SizeLevels();

    FirstLocation();
    size_t startIndex = Index() + (x1 + y1 * itsGridXNumber) * columnStep;

    size_t rowCount = y2 - y1 + 1;
    if (rowCount <= 0 || rowCount > itsGridYNumber)
      throw Fmi::Exception(BCP, "Degenerate Y axis");
    size_t rowStep = (itsGridXNumber - columnCount) * columnStep;

    if (!GetValuesPartial(startIndex, rowCount, rowStep, columnCount, columnStep, values))
    {
      return false;
    }

    if (IsSubParamUsed())
    {
      if (itsCombinedParamParser)
      {
        std::transform(begin(values),
                       end(values),
                       begin(values),
                       [this](const float &fVal) { return SubValueFromFloat(fVal); });
      }
      else
        throw Fmi::Exception(BCP, "CombinedParamParser missing!");
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetCube(std::vector<float> &values)
{
  try
  {
    values.resize(SizeLocations() * SizeLevels());

    // 	bool rising = HeightParamIsRising();
    //
    // 	if (rising) ResetLevel();
    // 	else LastLevel();

    FirstLevel();
    FirstLocation();
    size_t startIndex = Index();
    size_t step = SizeTimes();
    size_t count = SizeLocations() * SizeLevels();

    if (!GetValues(startIndex, step, count, values))
      return false;

    if (IsSubParamUsed())
    {
      if (itsCombinedParamParser)
      {
        std::transform(values.rbegin(),
                       values.rend(),
                       values.rbegin(),
                       [this](const float &fVal) { return SubValueFromFloat(fVal); });
      }
      else
        throw Fmi::Exception(BCP, "CombinedParamParser is null!");
    }

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetInterpolatedLevel(std::vector<float> &values, const NFmiMetTime &time)
{
  try
  {
    auto oldTime = TimeIndex();

    values.resize(SizeLocations());
    std::fill(begin(values), end(values), kFloatMissing);
    static std::vector<float> nextValues(SizeLocations(), kFloatMissing);

    NFmiTimeCache timeCache = CalcTimeCache(time);

    TimeIndex(timeCache.itsTimeIndex1);
    GetLevelToVec(values);

    TimeIndex(timeCache.itsTimeIndex2);
    GetLevelToVec(nextValues);

    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());

    std::transform(values.begin(),
                   values.end(),
                   nextValues.begin(),
                   values.begin(),
                   [&](const float &a, const float &b)
                   { return CachedTimeInterpolatedValue(a, b, timeCache, interp, parId); });

    TimeIndex(oldTime);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetInterpolatedCube(std::vector<float> &values, const NFmiMetTime &time)
{
  try
  {
    auto oldTime = TimeIndex();

    values.resize(SizeLocations() * SizeLevels());
    std::fill(begin(values), end(values), kFloatMissing);
    static std::vector<float> nextValues(SizeLocations() * SizeLevels(), kFloatMissing);

    NFmiTimeCache timeCache = CalcTimeCache(time);
    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
    bool simpleInterp = (interp == kNoneInterpolation) || (interp == kNearestPoint);
    if ((simpleInterp && timeCache.itsOffset <= 0.5) || timeCache.itsOffset < 0.001)
    {
      TimeIndex(timeCache.itsTimeIndex1);
      GetCube(values);
    }
    else if ((simpleInterp && timeCache.itsOffset > 0.5) || timeCache.itsOffset > 0.999)
    {
      TimeIndex(timeCache.itsTimeIndex2);
      GetCube(values);
    }
    else
    {
      TimeIndex(timeCache.itsTimeIndex1);
      GetCube(values);

      TimeIndex(timeCache.itsTimeIndex2);
      GetCube(nextValues);

      std::transform(values.begin(),
                     values.end(),
                     nextValues.begin(),
                     values.begin(),
                     [&](const float &a, const float &b)
                     { return CachedTimeInterpolatedValue(a, b, timeCache, interp, parId); });
    }

    TimeIndex(oldTime);

    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot annettuun datamatriisiin haluttuun
 * aikaan interpoloituna.
 *
 * \param theMatrix The matrix in which to store the values
 * \param theInterpolatedTime The desired time
 */
// ----------------------------------------------------------------------
NFmiDataMatrix<float> NFmiFastQueryInfo::CroppedValues(
    const NFmiMetTime &theInterpolatedTime, int x1, int y1, int x2, int y2)
{
  try
  {
    return CroppedValues(theInterpolatedTime, x1, y1, x2, y2, kLongMissing, false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::CroppedValues(const NFmiMetTime &theInterpolatedTime,
                                                       int x1,
                                                       int y1,
                                                       int x2,
                                                       int y2,
                                                       long theTimeRangeInMinutes,
                                                       bool doNearestTimeIfPossible)
{
  try
  {
    TimeInterpolationData timeInterpolationData(
        *this, theInterpolatedTime, theTimeRangeInMinutes, doNearestTimeIfPossible);
    if (!timeInterpolationData.CanGetValues())
    {
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return {};
    }

    // Handle exact existing time
    if (timeInterpolationData.HasWantedTime())
    {
      auto values = CroppedValues(x1, y1, x2, y2);
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    if (timeInterpolationData.DoNearestTime())
    {
      if (timeInterpolationData.previousTimeIndex != gMissingIndex)
        TimeIndex(timeInterpolationData.previousTimeIndex);
      else
        TimeIndex(timeInterpolationData.nextTimeIndex);
      auto values = CroppedValues(x1, y1, x2, y2);
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    // Extract leftside and rightside data values

    auto nx = static_cast<int>(x2 - x1 + 1);
    auto ny = static_cast<int>(y2 - y1 + 1);
    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    NFmiDataMatrix<float> values1;
    NFmiDataMatrix<float> values2;

    TimeIndex(timeInterpolationData.previousTimeIndex);
    values1 = CroppedValues(x1, y1, x2, y2);

    TimeIndex(timeInterpolationData.nextTimeIndex);
    values2 = CroppedValues(x1, y1, x2, y2);

    auto diff1 = timeInterpolationData.previousToInterpolatedTimeDifferenceInMinutes;
    auto diff2 = timeInterpolationData.previousToNextTimeDifferenceInMinutes;

    float factor = 1 - diff1 / diff2;

    // Then interpolate the data. We must be careful to obey the interpolation
    // rules of both discrete data and combined parameters

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    unsigned long param = Param().GetParam()->GetIdent();

    if (param == kFmiTotalWindMS)
    {
      NFmiTotalWind resultWind(itsInfoVersion);
      for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
        {
          NFmiTotalWind tempWind1(values1[i][j], kFmiPackedWind, itsInfoVersion);
          NFmiTotalWind tempWind2(values2[i][j], kFmiPackedWind, itsInfoVersion);
          resultWind.SetToWeightedMean(
              &tempWind1, factor, &tempWind2, 1 - factor, &tempWind1, 0, &tempWind1, 0);
          values[i][j] = resultWind.TransformedFloatValue();
        }
    }
    else if (param == kFmiWeatherAndCloudiness)
    {
      NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);
      for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
        {
          NFmiWeatherAndCloudiness tempWeather1(
              values1[i][j], kFmiPackedWeather, kFloatMissing, itsInfoVersion);
          NFmiWeatherAndCloudiness tempWeather2(
              values2[i][j], kFmiPackedWeather, kFloatMissing, itsInfoVersion);
          resultWeather.SetToWeightedMean(
              &tempWeather1, factor, &tempWeather2, 1 - factor, &tempWeather1, 0, &tempWeather1, 0);
          values[i][j] = resultWeather.TransformedFloatValue();
        }
    }
    else if (interp != kLinearly)
    {
      if (factor > 0.5)
        values = values1;
      else
        values = values2;
    }
    else
    {
      for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
          values[i][j] = InterpolationHelper(values1[i][j], values2[i][j], factor);
    }

    TimeIndex(timeInterpolationData.oldTimeIndex);
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot annettuun datamatriisiin. Lasketaan tulokset
 * käyttäen haluttua funktiota-datamodifieria (esim. min, max, sum). Laskuissa
 * käytetään annettua aikahaarukkaa (aika +- annetut aikaoffsetit).
 *
 * \param theMatrix The matrix in which to store the values
 * \param theFunction The function to apply to the data
 * \param theTime The desired time
 * \param theBackwardOffsetInMinutes The backward offset from theTime
 * \param theForwardOffsetInMinutes The forward offset from theTime
 * \todo Muuta toiminto käyttämään TimeDescriptoria, ei toimi vaihtuva aikaresoluutioiselle
 * datalle!!!!!!
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::Values(NFmiDataModifier *theFunction,
                                                const NFmiMetTime &theTime,
                                                int theBackwardOffsetInMinutes,
                                                int theForwardOffsetInMinutes)
{
  try
  {
    if (!(IsGrid() && theFunction))
      return {};

    int nx = itsGridXNumber;
    int ny = itsGridYNumber;
    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    // **** Rakennetaan laskuja varten timebagi ****
    NFmiTimeBag validTimes(ValidTimes());
    NFmiMetTime tmpTime(
        theTime);  // aikaa pitää pystyä muuttamaan, joten tehdään siitä väliaikainen muuttuja
    tmpTime.ChangeByMinutes(-theBackwardOffsetInMinutes);
    if (!validTimes.IsInside(tmpTime))
      return values;  // offsetaika meni timebagin ohi, voidaan lopettaa
    // Mika: Tästä tulee jostain syystä overflow varoitus
    if (!validTimes.FindNearestTime(tmpTime))
      return values;  // laskettavan aikajakson alkuaikaa ei löytynyt, turha jatkaa
    else if (validTimes.CurrentTime() <
             tmpTime)  // löytynyt aika oli pienempi kuin raja, kasvatetaan esaatua aikaa yhdellä
      if (!validTimes.Next())
        return values;  // jos Next epäonnistui, mentiin timebagin ulkopuolelle ja voidaan lopettaa
    NFmiMetTime startTime(validTimes.CurrentTime());

    tmpTime.ChangeByMinutes(
        theBackwardOffsetInMinutes +
        theForwardOffsetInMinutes);  // pitää liikuttaa takaisin nolla pisteeseen ja siitä eteen
    if (!validTimes.IsInside(tmpTime))
      return values;  // offsetaika meni timebagin ohi, voidaan lopettaa

    if (!validTimes.FindNearestTime(tmpTime))
      return values;  // laskettavan aikajakson loppuaikaa ei löytynyt, turha jatkaa
    else if (validTimes.CurrentTime() > tmpTime)  // löytynyt aika oli suurempi kuin raja,
      // vähennetään saatua aikaa yhdellä
      // aika-askeleella
      if (!validTimes.Previous())
        return values;  // jos Previous epäonnistui, mentiin timebagin ulkopuolelle ja voidaan
    // lopettaa
    NFmiMetTime endTime(validTimes.CurrentTime());
    NFmiTimeBag calculatedTimes(startTime, endTime, validTimes.Resolution());
    // **** Rakennetaan laskuja varten timebagi ****

    ResetLocation();  // resetoi paikan (= yksi ennen alkua)
    for (int j = 0; j < ny; j++)
      for (int i = 0; i < nx; i++)
      {
        if (NextLocation())  // juoksutetaan  paikkaa ensin!
        {
          theFunction->Clear();  // nollataan datamodifier laskujen välillä
          CalcTimeData(theFunction, &calculatedTimes);
          float tmp =
              theFunction
                  ->CalculationResult();  // otetaan arvo talteen tähän vain debuggausta varten
          values[i][j] = tmp;
        }
      }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                                float P,
                                                float H)
{
  try
  {
    NFmiDataMatrix<float> values(theLatlonMatrix.width(), theLatlonMatrix.height(), kFloatMissing);

    // Cannot interpolate non-gridded data
    if (HPlaceDescriptor().IsGrid() == false)
      return values;

    bool doNormalInterpolation = (P == kFloatMissing && H == kFloatMissing);

    for (std::size_t j = 0; j < theLatlonMatrix.height(); j++)
      for (std::size_t i = 0; i < theLatlonMatrix.width(); i++)
      {
        const auto latlon = theLatlonMatrix(i, j);
        if (doNormalInterpolation)
          values[i][j] = InterpolatedValue(latlon);
        else if (H != kFloatMissing)
          values[i][j] = HeightValue(H, latlon);
        else
          values[i][j] = PressureLevelValue(P, latlon);
      }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                                const NFmiMetTime &theTime,
                                                float P,
                                                float H)
{
  try
  {
    return Values(theLatlonMatrix, theTime, P, H, kLongMissing, false);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::Values(const Fmi::CoordinateMatrix &theLatlonMatrix,
                                                const NFmiMetTime &theTime,
                                                float P,
                                                float H,
                                                long theTimeRangeInMinutes,
                                                bool doNearestTimeIfPossible)
{
  try
  {
    TimeInterpolationData timeInterpolationData(
        *this, theTime, theTimeRangeInMinutes, doNearestTimeIfPossible);
    if (!timeInterpolationData.CanGetValues())
    {
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return {};
    }

    // Handle exact existing time
    if (timeInterpolationData.HasWantedTime())
    {
      auto values = Values(theLatlonMatrix, P, H);
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    if (timeInterpolationData.DoNearestTime())
    {
      if (timeInterpolationData.previousTimeIndex != gMissingIndex)
        TimeIndex(timeInterpolationData.previousTimeIndex);
      else
        TimeIndex(timeInterpolationData.nextTimeIndex);
      auto values = Values(theLatlonMatrix, P, H);
      TimeIndex(timeInterpolationData.oldTimeIndex);
      return values;
    }

    NFmiDataMatrix<float> values(theLatlonMatrix.width(), theLatlonMatrix.height(), kFloatMissing);

    bool doNormalInterpolation = (P == kFloatMissing && H == kFloatMissing);
    for (std::size_t j = 0; j < theLatlonMatrix.height(); j++)
    {
      for (std::size_t i = 0; i < theLatlonMatrix.width(); i++)
      {
        const NFmiPoint &latlon = theLatlonMatrix(i, j);
        if (doNormalInterpolation)
          values[i][j] =
              InterpolatedValue(latlon, theTime, static_cast<int>(theTimeRangeInMinutes));
        else if (H != kFloatMissing)
          values[i][j] =
              HeightValue(H, latlon, theTime, static_cast<unsigned long>(theTimeRangeInMinutes));
        else
          values[i][j] = PressureLevelValue(
              P, latlon, theTime, static_cast<unsigned long>(theTimeRangeInMinutes));
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Asettaa kaikki hilan data-arvot annetusta datamatriisista
 * Pitää olla samankokoinen kuin hilan! Osaa hanskata myös aliparametrit.
 *
 * \param theMatrix Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::SetValues(const NFmiDataMatrix<float> &theMatrix)
{
  try
  {
    if (IsGrid())
    {
      unsigned int nx = itsGridXNumber;
      unsigned int ny = itsGridYNumber;
      if (nx == theMatrix.NX() && ny == theMatrix.NY())
      {
        long long idx = Index(itsParamIndex, 0, itsLevelIndex, itsTimeIndex);
        long long offset = Index(itsParamIndex, 1, itsLevelIndex, itsTimeIndex) - idx;

        // Mika: Must have this loop order so that the offset trick works

        for (unsigned int j = 0; j < ny; j++)
          for (unsigned int i = 0; i < nx; i++)
          {
            PokeValue(idx, theMatrix[i][j]);
            idx += offset;
          }
        return true;
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiFastQueryInfo::Clone() const
{
  try
  {
    return new NFmiFastQueryInfo(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
// ----------------------------------------------------------------------
/*!
 * \brief Get the float value at the index, subparam or not
 *
 * \return The value
 */
// ----------------------------------------------------------------------

float NFmiFastQueryInfo::GetFloatValue(size_t theIndex) const
{
  try
  {
    float value = IndexFloatValue(theIndex);

    if (!IsSubParamUsed())
      return value;

    if (!itsCombinedParamParser)
      return kFloatMissing;

    itsCombinedParamParser->TransformFromFloatValue(value);
    return static_cast<float>(itsCombinedParamParser->SubValue(
        FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent())));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::LastParam(bool fIgnoreSubParam)
{
  try
  {
    itsParamIndex = itsParamSize - 1;
    fUseSubParam = false;
    if (fIgnoreSubParam == false)
    {
      // tämä on viritys, etsitään mahd. viimeinen aliparametreista paramDescriptorista loopissa
      for (; itsParamDescriptor->Next(false);)
        ;
      itsParamIndex = itsParamSize - 1;  // asetetaan indeksi taas viimeiseen
      fUseSubParam = itsParamDescriptor->IsSubParamUsed();

      if (fUseSubParam)
      {
        NFmiDataIdent &param =
            itsParamDescriptor->Param(true);  // huom! tässä pitää pyytää yliparametria (->true)
        ChangeCombinedParamParser(param);
      }
    }
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee logaritmisessa asteikossa interpoloidun arvon.
// Käytetään esim. logaritmisen paine asteikon kanssa.
// Palauttaa x:ää vastaavan y:n, kun x1 arvoa vastaa y1 ja x2:n arvoa vastaa y2.
// eli x-parametrit ovat logaritmisella asteikolla (eli paineet ovat x-lukuja).
// Palauttaa x:ää vastaavan luvun y, joka on siis interpoloitu logaritmisesti.
static double CalcLogInterpolatedValue(double x1, double x2, double x, double y1, double y2)
{
  try
  {
    double y = kFloatMissing;
    if (x1 != kFloatMissing && x2 != kFloatMissing && x != kFloatMissing)
    {
      if (x1 == x2)
        y = y1 != kFloatMissing ? y1 : y2;
      else if (y1 != kFloatMissing && y2 != kFloatMissing)
      {
        double w = (::log(x) - ::log(x1)) / (::log(x2) - ::log(x1));
        y = (1 - w) * y1 + w * y2;
      }
      else if (y1 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
        y = y1;
      else if (y2 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
        y = y2;
    }
    return y;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee logaritmisessa asteikossa interpoloidun arvon.
// Käytetään esim. logaritmisen paine asteikon kanssa.
// Palauttaa x:ää vastaavan y:n, kun x1 arvoa vastaa y1 ja x2:n arvoa vastaa y2.
float CalcLogModLinearInterpolatedValue(
    float x1, float x2, float x, float y1, float y2, unsigned int modulo)
{
  try
  {
    float y = kFloatMissing;
    if (x1 != kFloatMissing && x2 != kFloatMissing && x != kFloatMissing)
    {
      if (x1 == x2)
        y = y1 != kFloatMissing ? y1 : y2;
      else if (y1 != kFloatMissing && y2 != kFloatMissing)
      {
        float w = (::log(x) - ::log(x1)) / (::log(x2) - ::log(x1));
        y = static_cast<float>(NFmiInterpolation::ModLinear(w, y1, y2, modulo));
      }
      else if (y1 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
        y = y1;
      else if (y2 != kFloatMissing)  // jos toinen -arvoista puuttuu annetaan arvoksi toinen
        y = y2;
    }
    return y;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float CalcLogInterpolatedWindWectorValue(float x1, float x2, float x, float wv1, float wv2)
{
  try
  {
    float y = kFloatMissing;
    if (wv1 != kFloatMissing && wv2 != kFloatMissing)
    {
      float wd1 = (static_cast<int>(wv1) % 100) * 10.f;
      auto ws1 = static_cast<float>(static_cast<int>(wv1) / 100);
      float wd2 = (static_cast<int>(wv2) % 100) * 10.f;
      auto ws2 = static_cast<float>(static_cast<int>(wv2) / 100);

      float wdInterp = CalcLogModLinearInterpolatedValue(x1, x2, x, wd1, wd2, 360);
      auto wsInterp = static_cast<float>(CalcLogInterpolatedValue(x1, x2, x, ws1, ws2));
      if (wdInterp != kFloatMissing && wsInterp != kFloatMissing)
      {
        y = static_cast<float>(round(wsInterp) * 100 + round(wdInterp / 10.));
      }
    }
    else if (wv1 != kFloatMissing)
      y = wv1;
    else if (wv2 != kFloatMissing)
      y = wv2;
    return y;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static double GetLogaritmicFactor(double x1, double x2, double x)
{
  try
  {
    double w = (::log(x) - ::log(x1)) / (::log(x2) - ::log(x1));
    return w;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetLocationIndex(const NFmiPoint &theLatlon, double &xInd, double &yInd)
{
  try
  {
    if (Grid())
    {
      if (Grid()->Area()->IsInside(theLatlon))
      {
        NFmiPoint gpoint = Grid()->LatLonToGrid(theLatlon);
        xInd = gpoint.X();
        yInd = gpoint.Y();
        return true;
      }
    }
    xInd = -1;
    yInd = -1;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetTimeIndex(const NFmiMetTime &theTime, double &tInd)
{
  try
  {
    if (Time(theTime))
    {
      tInd = TimeIndex();
      return true;
    }
    else if (IsInside(theTime))
    {
      if (FindNearestTime(theTime, kBackward))  // pitäisi löytyä, eikä ole reunalla, koska edellä
                                                // on kokeiltu, löytyykö theTime jo suoraan
      {
        NFmiMetTime time1(Time());
        unsigned long timeIndex = TimeIndex();
        NextTime();  // tämänkin pitää löytyä
        NFmiMetTime time2(Time());

        auto totalDiff = static_cast<float>(time2.DifferenceInMinutes(time1));
        auto diff1 = static_cast<float>(theTime.DifferenceInMinutes(time1));
        if (totalDiff)
        {
          float offset = (diff1 / totalDiff);
          tInd = timeIndex + offset;
          return true;
        }
      }
    }
    tInd = -1;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::GetLevelIndex(const NFmiPoint &theLatlon,
                                      const NFmiMetTime &theTime,
                                      double pressure,
                                      double &pInd)
{
  try
  {
    const double P_epsilon = 0.001;  // joskus interpoloinnit antavat ilmeisesti pieniä heittoja,
    // jotka pitää ottaa huomioon, kun ollaan datan ala/ylä rajalla
    if (fPressureLevelDataAvailable)
    {
      float firstPressureValue = itsPressureLevelDataPressures[0];
      // huom! isUnder on true myös jos P == 1. painearvo
      bool isUnder = (firstPressureValue != kFloatMissing)
                         ? (pressure <= firstPressureValue)
                         : fPressureParamIsRising;  // jos 1. paine oli puuttuvaa, annetaan
                                                    // sellainen arvo että voidaan jatkaa etsintöjä
      if (fPressureParamIsRising ^ isUnder)
      {  // jos risingHeight ja isUnder ovat joku molemmat true tai false, voidaan jatkaa, koska
         // haluttu korkeus voi vielä löytyä
        bool isUnderNow = isUnder;
        float lastPressure = firstPressureValue;
        float pressureValue = kFloatMissing;
        bool foundWantedLevels = false;
        int i = 0;
        for (; i < static_cast<int>(itsPressureLevelDataPressures.size());
             i++)  // HUOM! tämän haun voisi optimoida, jos tarpeeksi leveleita (>20 ?) ja käyttäisi
                   // binary searchia
        {
          pressureValue = itsPressureLevelDataPressures[i];
          if (pressureValue != kFloatMissing)
            isUnderNow = (pressure - P_epsilon) <
                         pressureValue;  // pieni virhemarginaali (P_epsilon) piti laittaa
          if (isUnderNow !=
              isUnder)  // nyt mentiin kriittisen pisteen ohi, nyt voidaan tehdä laskut
          {
            foundWantedLevels = true;
            break;
          }
          lastPressure = pressureValue;
        }  // end of for loop
        if (foundWantedLevels)
        {
          double w = GetLogaritmicFactor(pressureValue, lastPressure, pressure);
          pInd = i - w;
          return true;
        }
      }
      return false;
    }
    else if (fPressureValueAvailable)
    {
      // aluksi pitää järjestellä parametrin indeksi paine datan kohdalle ja
      // ottaa currentti paramindex talteen ja oliko
      // vanha parametri aliparametri, koska se nämä asetukset pitää laittaa kohdalleen
      // sitten kun haetaan haluttua parametria oikeilta paine pinnoilta.
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;

      FirstLevel();
      float firstPressureValue = InterpolatedValue(theLatlon, theTime);
      // huom! isUnder on true myös jos P == 1. painearvo
      bool isUnder = (firstPressureValue != kFloatMissing)
                         ? (pressure <= firstPressureValue)
                         : !fPressureParamIsRising;  // jos 1. paine oli puuttuvaa, annetaan
                                                     // sellainen arvo että voidaan jatkaa etsintöjä
      if (fPressureParamIsRising ^ isUnder)
      {  // jos risingHeight ja isUnder ovat joku molemmat true tai false, voidaan jatkaa, koska
         // haluttu korkeus voi vielä löytyä
        bool isUnderNow = isUnder;
        float lastPressure = firstPressureValue;
        float pressureValue = kFloatMissing;
        int i = 0;
        bool foundWantedLevels = false;
        for (ResetLevel(); NextLevel(); i++)  // HUOM! tämän haun voisi optimoida, jos tarpeeksi
                                              // leveleita (>20 ?) ja käyttäisi binary searchia
        {
          pressureValue = InterpolatedValue(theLatlon, theTime);
          if (pressureValue != kFloatMissing)
            isUnderNow = (pressure - P_epsilon) <
                         pressureValue;  // pieni virhemarginaali (P_epsilon) piti laittaa
          if (isUnderNow !=
              isUnder)  // nyt mentiin kriittisen pisteen ohi, nyt voidaan tehdä laskut
          {
            foundWantedLevels = true;
            break;
          }
          lastPressure = pressureValue;
        }  // end of for loop

        if (!foundWantedLevels)
        {
          ParamIndex(oldParamIndex);  // laitetaan haluttu parametri taas 'päälle'
          fUseSubParam = oldFSubParamUsed;
          pInd = -1;
          return false;
        }

        // tee korkeus interpolointi nyt kun tiedetään halutut levelit
        ParamIndex(oldParamIndex);  // laitetaan haluttu parametri taas 'päälle'
        fUseSubParam = oldFSubParamUsed;
        //			if(!fPressureParamIsRising)
        //				PreviousLevel();

        double w = GetLogaritmicFactor(pressureValue, lastPressure, pressure);
        pInd = itsLevelIndex - w;
        return true;
      }
      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;
    }
    pInd = -1;
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static const double kEpsilon = 0.001;  // tälläistä virhe rajaa käytetään kun tarkistellaan ollaanko
// tarpeeksi lähellä kokonaisluku indeksejä FastPressureLevelValue-metodeissa

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::FastPressureLevelValue(double xInd, double yInd, double tInd, double pInd)
{
  try
  {
    // jos t % 1 == 0 (tarpeeksi lähellä)
    // timeindex = round(tInd)
    // return FastPressureLevelValue(double xInd, double yInd, double pInd)
    // muuten
    // timeindex = int(tInd)
    // T1 = FastPressureLevelValue(double xInd, double yInd, double pInd)
    // timeindex = int(tInd)+1
    // T2 = FastPressureLevelValue(double xInd, double yInd, double pInd)
    // T1 ja T2 interpolointi ajassa
    if (::fabs(tInd - round(tInd)) < kEpsilon)  // jos tarpeeksi lähellä yhtä aikaa
    {
      TimeIndex(static_cast<unsigned long>(round(tInd)));
      return FastPressureLevelValue(xInd, yInd, pInd);
    }
    else
    {
      TimeIndex(static_cast<unsigned long>(tInd));
      float value1 = FastPressureLevelValue(xInd, yInd, pInd);
      NextTime();  // tämänkin pitää löytyä
      float value2 = FastPressureLevelValue(xInd, yInd, pInd);

      double factor = tInd - static_cast<int>(tInd);
      float value = kFloatMissing;
      FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
      if (IsGrid() && interp == kLinearly)
      {  // lineaarinen interpolointi
        auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
        if (param == kFmiWindVectorMS)
          value = static_cast<float>(NFmiInterpolation::WindVector(factor, value1, value2));
        else if (param == kFmiWindDirection || param == kFmiWaveDirection)
          value = static_cast<float>(NFmiInterpolation::ModLinear(factor, value1, value2, 360));
        else
          value = static_cast<float>(NFmiInterpolation::Linear(factor, value1, value2));
      }
      else
      {  // muut tapaukset eli nearest interpolointi
        if (factor < 0.5)
          value = value1;
        else
          value = value2;
      }
      return value;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::FastPressureLevelValue(double xInd, double yInd, double pInd)
{
  try
  {
    // jos p % 1 == 0 (tarpeeksi lähellä)
    // levelindex = round(pInd)
    // return FastPressureLevelValue(double xInd, double yInd)
    // muuten
    // levelindex = int(pInd)
    // P1 = FastPressureLevelValue(double xInd, double yInd)
    // levelindex = int(pInd)+1
    // P2 = FastPressureLevelValue(double xInd, double yInd)
    // P1 ja P2 interpolointi logaritmisesti (oikeasti lineaarisesti, koska pInd:issä on jo
    // logaritmien painotus) paineen mukaan
    if (::fabs(pInd - round(pInd)) < kEpsilon)  // jos tarpeeksi lähellä yhtä leveliä
    {
      LevelIndex(static_cast<unsigned long>(round(pInd)));
      return FastPressureLevelValue(xInd, yInd);
    }
    else
    {
      LevelIndex(static_cast<unsigned long>(pInd));
      float value1 = FastPressureLevelValue(xInd, yInd);
      NextLevel();
      float value2 = FastPressureLevelValue(xInd, yInd);
      double factor =
          pInd - static_cast<int>(
                     pInd);  // huom! tässä factorissa on jo otettu huomioon logaritminen kerroin

      float value = kFloatMissing;
      FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
      if (IsGrid() && interp == kLinearly)
      {  // lineaarinen interpolointi
        auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
        if (param == kFmiWindVectorMS)
          value = static_cast<float>(NFmiInterpolation::WindVector(factor, value1, value2));
        else if (param == kFmiWindDirection || param == kFmiWaveDirection)
          value = static_cast<float>(NFmiInterpolation::ModLinear(factor, value1, value2, 360));
        else
          value = static_cast<float>(NFmiInterpolation::Linear(factor, value1, value2));
      }
      else
      {  // muut tapaukset eli nearest interpolointi
        if (factor < 0.5)
          value = value1;
        else
          value = value2;
      }
      return value;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::FastPressureLevelValue(double xInd, double yInd)
{
  try
  {
    // jos x % 1 == 0 ja y % 1 == 0 (tarpeeksi lähellä)
    // locationindex = CalcLocIndex(round(xInd), round(yInd))
    // return FloatValue()
    // muuten
    // locationindex = CalcLocIndex(int(xInd), int(yInd))
    // bl = FloatValue()
    // MoveRight()
    // br = FloatValue()
    // MoveUp()
    // tr = FloatValue()
    // MoveLeft()
    // tl = FloatValue()
    // interpoloi paikan suhteen
    int nearestLocIndex = GetGridDataIndex(itsGridXNumber,
                                           static_cast<unsigned long>(round(xInd)),
                                           static_cast<unsigned long>(round(yInd)));
    float value = kFloatMissing;
    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    if (interp == kLinearly && IsGrid())  // onko lineaarinen interpolointi ja hila dataa
    {
      // optimointia, jos ollaan tarpeeksi lähellä hilaa, palautetaan suoraan hilan arvo
      if (::fabs(xInd - round(xInd)) < kEpsilon && ::fabs(yInd - round(yInd)) < kEpsilon)
      {
        LocationIndex(nearestLocIndex);
        return FloatValue();
      }

      int bottomLeftLocIndex =
          GetGridDataIndex(itsGridXNumber, static_cast<int>(xInd), static_cast<int>(yInd));
      LocationIndex(bottomLeftLocIndex);
      float bottomLeftValue = FloatValue();
      MoveRight();
      float bottomRightValue = FloatValue();
      MoveUp();
      float topRightValue = FloatValue();
      MoveLeft();
      float topLeftValue = FloatValue();

      auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
      double dx = xInd - floor(xInd);
      double dy = yInd - floor(yInd);
      if (param == kFmiWindVectorMS)
        value = static_cast<float>(NFmiInterpolation::WindVector(
            dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue));
      else if (param == kFmiWindDirection || param == kFmiWaveDirection)
        value = static_cast<float>(NFmiInterpolation::ModBiLinear(
            dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue, 360));
      else
        value = static_cast<float>(NFmiInterpolation::BiLinear(
            dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue));

      return value;
    }
    LocationIndex(nearestLocIndex);  // tämä hakee nearest paikan jos löytää
    return FloatValue();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Palauttaa PressureLevelValue-metodia varten paineparametrin tai asetetun painepinnan arvon, tai
// palauttaa puuttuvan arvon jos sellaista ei löydy.
float NFmiFastQueryInfo::GetCurrentLevelPressure()
{
  try
  {
    if (!PressureDataAvailable())  // Tarkistetaan että datassa on P parametri tai painepintadataa
      return kFloatMissing;

    if (fPressureValueAvailable)
    {
      // Talletetaan originaali parametrin asetukset
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      // Asetetaan paineparametri päälle
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;

      float value = FloatValue();

      // Palautetaan originaali parametrin asetukset
      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;

      return value;
    }
    else
      return itsPressureLevelDataPressures[LevelIndex()];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::GetCurrentLevelPressure(const NFmiPoint &theLatlon)
{
  try
  {
    if (!PressureDataAvailable())  // Tarkistetaan että datassa on P parametri tai painepintadataa
      return kFloatMissing;

    if (fPressureValueAvailable)
    {
      // Talletetaan originaali parametrin asetukset
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      // Asetetaan paineparametri päälle
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;

      float value = InterpolatedValue(theLatlon);

      // Palautetaan originaali parametrin asetukset
      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;

      return value;
    }
    else
      return itsPressureLevelDataPressures[LevelIndex()];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::GetCurrentLevelPressure(const NFmiPoint &theLatlon,
                                                 const NFmiMetTime &theTime)
{
  try
  {
    if (!PressureDataAvailable())  // Tarkistetaan että datassa on P parametri tai painepintadataa
      return kFloatMissing;

    if (fPressureValueAvailable)
    {
      // Talletetaan originaali parametrin asetukset
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      // Asetetaan paineparametri päälle
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;

      float value = InterpolatedValue(theLatlon, theTime);

      // Palautetaan originaali parametrin asetukset
      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;

      return value;
    }

    return itsPressureLevelDataPressures[LevelIndex()];
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Etsii PressureLevelValue-metodia varten 1. paine arvon, mikä ei ole puuttuvaa, tai
// palauttaa puuttuvan arvon jos sellaista ei löydy.
float NFmiFastQueryInfo::FindFirstPressureValue()
{
  try
  {
    float firstPressureValue = kFloatMissing;
    FirstLevel();
    if (fPressureValueAvailable)
    {
      // aluksi pitää järjestellä parametrin indeksi paine datan kohdalle ja
      // ottaa currentti paramindex talteen ja oliko
      // vanha parametri aliparametri, koska se nämä asetukset pitää laittaa kohdalleen
      // sitten kun haetaan haluttua parametria oikeilta paine pinnoilta.
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;

      do
      {
        firstPressureValue = FloatValue();
        if (firstPressureValue != kFloatMissing)
          break;
      } while (NextLevel());

      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fPressureLevelDataAvailable)
    {  // painepinta data tapauksessa plautetaan vain 1. painpinnan arvo (se ei saisi olla
       // puuttuvaa)
      firstPressureValue = itsPressureLevelDataPressures[0];
    }
    return firstPressureValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::FindFirstHeightValue()
{
  try
  {
    float firstHeightValue = kFloatMissing;
    FirstLevel();
    if (fHeightValueAvailable)
    {
      do
      {
        firstHeightValue = FloatValue();
        if (firstHeightValue != kFloatMissing)
          break;
      } while (NextLevel());
    }
    else if (fHeightLevelDataAvailable)
    {  // painepinta data tapauksessa plautetaan vain 1. painpinnan arvo (se ei saisi olla
       // puuttuvaa)
      firstHeightValue = itsHeightLevelDataHeights[0];
    }
    return firstHeightValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::PressureLevelValue(float P)
{
  try
  {
    float value = kFloatMissing;
    if (PressureDataAvailable())
    {
      float firstPressureValue = FindFirstPressureValue();
      if (firstPressureValue == kFloatMissing)
        return kFloatMissing;  // turha jatkaa, palautetaan puuttuvaa

      // quick exit if we found an exact match
      if (firstPressureValue == P)
        return FloatValue();

      // huom! isUnder on true myös jos P == 1. painearvo
      bool isUnder = (firstPressureValue != kFloatMissing)
                         ? (P <= firstPressureValue)
                         : fPressureParamIsRising;  // jos 1. paine oli puuttuvaa, annetaan
                                                    // sellainen arvo että voidaan jatkaa etsintöjä
      if (fPressureParamIsRising ^ isUnder)
      {  // jos fPressureParamIsRising on true ja isUnder on false (tai toisin päin), voidaan
         // jatkaa, koska haluttu korkeus voi vielä löytyä
        bool isUnderNow = isUnder;
        float lastPressure = firstPressureValue;
        float pressureValue = kFloatMissing;
        int i = 0;
        bool foundWantedLevels = false;
        for (ResetLevel(); NextLevel(); i++)  // HUOM! tämän haun voisi optimoida, jos tarpeeksi
                                              // leveleita (>20 ?) ja käyttäisi binary searchia
        {
          pressureValue = GetCurrentLevelPressure();
          if (pressureValue != kFloatMissing)
            isUnderNow = P < pressureValue;
          if (isUnderNow !=
              isUnder)  // nyt mentiin kriittisen pisteen ohi, nyt voidaan tehdä laskut
          {
            foundWantedLevels = true;
            break;
          }
          lastPressure = pressureValue;
        }  // end of for loop

        if (!foundWantedLevels)
        {
          return kFloatMissing;
        }

        // tee korkeus interpolointi nyt kun tiedetään halutut levelit
        float value1 = FloatValue();

        // Quick exit for exact match
        if (pressureValue == P)
          return value1;

        PreviousLevel();
        float value2 = FloatValue();

        // Quick exit for exact match
        if (lastPressure == P)
          return value2;

        FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
        if (IsGrid() && interp == kLinearly)
        {  // lineaarinen interpolointi
          auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
          if (param == kFmiWindVectorMS)
            return static_cast<float>(
                CalcLogInterpolatedWindWectorValue(pressureValue, lastPressure, P, value1, value2));
          else if (param == kFmiWindDirection || param == kFmiWaveDirection)
          {
            float factor = 1. - (::fabs(P - lastPressure) / ::fabs(lastPressure - pressureValue));
            value = static_cast<float>(NFmiInterpolation::ModLinear(factor, value1, value2, 360));
          }
          else
            value = static_cast<float>(
                CalcLogInterpolatedValue(pressureValue, lastPressure, P, value1, value2));
        }
        else
        {  // muut tapaukset eli nearest interpolointi
          if (::fabs(P - pressureValue) < ::fabs(P - lastPressure))
            value = value1;
          else
            value = value2;
        }
      }
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::PressureLevelValue(float P, const NFmiPoint &theLatlon)
{
  try
  {
    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    if (interp != kLinearly || !IsGrid())
    {
      Location(theLatlon);  // search for nearest location (if any)
      return PressureLevelValue(P);
    }

    if (!PressureDataAvailable())  // Tarkistetaan että datassa on P parametri tai painepintadataa
      return kFloatMissing;

    if (!Location(theLatlon))  // katsotaanko, oliko annettu piste hilan sisällä
      return kFloatMissing;

    float p1 = kFloatMissing;
    float p2 = kFloatMissing;

    for (ResetLevel(); NextLevel();)
    {
      p1 = p2;
      p2 = GetCurrentLevelPressure(theLatlon);

      if (p1 != kFloatMissing && p2 != kFloatMissing && std::min(p1, p2) <= P &&
          P <= std::max(p1, p2))
      {
        float value2 = InterpolatedValue(theLatlon);
        PreviousLevel();
        float value1 = InterpolatedValue(theLatlon);

        auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
        if (param == kFmiWindVectorMS)
          return static_cast<float>(CalcLogInterpolatedWindWectorValue(p1, p2, P, value1, value2));
        else if (param != kFmiWindDirection && param != kFmiWaveDirection)
          return static_cast<float>(CalcLogInterpolatedValue(p1, p2, P, value1, value2));

        float factor = ::fabs(P - p1) / ::fabs(p2 - p1);
        return static_cast<float>(NFmiInterpolation::ModLinear(factor, value1, value2, 360));
      }
    }

    // Failed to find the straddling levels
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::PressureLevelValue(float P,
                                            const NFmiPoint &theLatlon,
                                            const NFmiMetTime &theTime,
                                            unsigned long theTimeRangeInMinutes)
{
  try
  {
    bool isTimeInside;

    if (Time(theTime))
      return PressureLevelValue(P, theLatlon);
    else if ((isTimeInside = IsInside(theTime)) && (theTimeRangeInMinutes > 0))
    {
      NFmiMetTime refTime(theTime);
      boost::posix_time::ptime rangeStartTime, rangeEndTime;
      float value1 = kFloatMissing;

      if (theTimeRangeInMinutes != kUnsignedLongMissing)
      {
        rangeStartTime = theTime.PosixTime() - boost::posix_time::minutes(theTimeRangeInMinutes);
        rangeEndTime = theTime.PosixTime() + boost::posix_time::minutes(theTimeRangeInMinutes);
      }
      else
      {
        FirstTime();
        rangeStartTime = Time().PosixTime();
        LastTime();
        rangeEndTime = Time().PosixTime();
      }

      for (; ((value1 == kFloatMissing) &&
              FindNearestTime(refTime, kBackward, theTimeRangeInMinutes) &&
              (Time().PosixTime() >= rangeStartTime));)
      {
        value1 = PressureLevelValue(P, theLatlon);
        refTime = Time().PosixTime() - boost::posix_time::seconds(1);
      }

      auto time1 = Time();
      float value2 = kFloatMissing;

      for (refTime = theTime; ((value2 == kFloatMissing) &&
                               FindNearestTime(refTime, kForward, theTimeRangeInMinutes) &&
                               (Time().PosixTime() <= rangeEndTime));)
      {
        // Using minute step forwards because seconds get zeroed when constructing NFmiMetTime

        value2 = PressureLevelValue(P, theLatlon);
        refTime = Time().PosixTime() + boost::posix_time::minutes(1);
      }

      return Interpolate(Param(), theTime, time1, Time(), value1, value2);
    }
    else if (isTimeInside)
    {
      if (FindNearestTime(theTime, kBackward))  // pitäisi löytyä, eikä ole reunalla, koska edellä
                                                // on kokeiltu, löytyykö theTime jo suoraan
      {
        NFmiMetTime time1(Time());
        float value1 = PressureLevelValue(P, theLatlon);
        NextTime();  // tämänkin pitää löytyä
        NFmiMetTime time2(Time());
        float value2 = PressureLevelValue(P, theLatlon);
        float value = Interpolate(Param(), theTime, time1, time2, value1, value2);
        return value;
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::PressureLevelValue(float P,
                                            const NFmiPoint &theLatlon,
                                            const NFmiMetTime &theTime)
{
  try
  {
    return PressureLevelValue(P, theLatlon, theTime, 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::PressureLevelValue(float P, const NFmiMetTime &theTime)
{
  try
  {
    if (Time(theTime))
      return PressureLevelValue(P);
    else if (IsInside(theTime))
    {
      if (FindNearestTime(theTime, kBackward))  // pitäisi löytyä, eikä ole reunalla, koska edellä
                                                // on kokeiltu, löytyykö theTime jo suoraan
      {
        NFmiMetTime time1(Time());
        float value1 = PressureLevelValue(P);
        NextTime();  // tämänkin pitää löytyä
        NFmiMetTime time2(Time());
        float value2 = PressureLevelValue(P);
        float value = Interpolate(Param(), theTime, time1, time2, value1, value2);
        return value;
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::HeightValue(float theHeight)
{
  try
  {
    float value = kFloatMissing;
    if (HeightDataAvailable())
    {
      // aluksi pitää järjestellä parametrin indeksi korkeus datan kohdalle ja
      // ottaa currentti paramindex talteen ja oliko
      // vanha parametri aliparametri, koska se nämä asetukset pitää laittaa kohdalleen
      // sitten kun haetaan haluttua parametria oikeilta korkeuksilta.
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsHeightParamIndex);
      fUseSubParam = false;

      float firstHeightValue = FindFirstHeightValue();
      // huom! isUnder on true myös jos P == 1. painearvo
      bool isUnder = theHeight <= firstHeightValue;
      //		if((fHeightParamIsRising && isUnder) || (!(fHeightParamIsRising ||
      // isUnder)))
      if (fHeightParamIsRising ^ isUnder)
      {  // jos risingHeight ja isUnder ovat joku molemmat true tai false, voidaan jatkaa, koska
         // haluttu korkeus voi vielä löytyä
        bool isUnderNow = isUnder;
        float lastHeight = firstHeightValue;
        int i = 0;
        for (ResetLevel(); NextLevel(); i++)  // HUOM! tämän haun voisi optimoida, jos tarpeeksi
                                              // leveleita (>20 ?) ja käyttäisi binary searchia
        {
          float heightValue = fHeightValueAvailable ? FloatValue() : itsHeightLevelDataHeights[i];
          if (heightValue != kFloatMissing)
            isUnderNow = theHeight < heightValue;
          if (isUnderNow !=
              isUnder)  // nyt mentiin kriittisen pisteen ohi, nyt voidaan tehdä laskut
          {             // tee korkeus interpolointi nyt kun tiedetään halutut levelit
            ParamIndex(oldParamIndex);  // laitetaan haluttu parametri taas 'päälle'
            fUseSubParam = oldFSubParamUsed;
            float value1 = FloatValue();
            PreviousLevel();
            float value2 = FloatValue();

            FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
            if (IsGrid() && interp == kLinearly)
            {  // lineaarinen interpolointi
              float factor = ::fabs(theHeight - lastHeight) / ::fabs(lastHeight - heightValue);
              auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
              if (param == kFmiWindVectorMS)
                value = static_cast<float>(
                    NFmiInterpolation::WindVector((1. - factor), value1, value2));
              else if (param == kFmiWindDirection || param == kFmiWaveDirection)
                value = static_cast<float>(
                    NFmiInterpolation::ModLinear((1. - factor), value1, value2, 360));
              else
                value =
                    static_cast<float>(NFmiInterpolation::Linear((1. - factor), value1, value2));
            }
            else
            {  // muut tapaukset eli nearest interpolointi
              if (::fabs(theHeight - heightValue) < ::fabs(theHeight - lastHeight))
                value = value1;
              else
                value = value2;
            }
            break;
          }
          lastHeight = heightValue;
        }  // end of for loop
      }
      ParamIndex(oldParamIndex);
      fUseSubParam = oldFSubParamUsed;
    }  // if(fHeightValueAvailable)
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::HeightValue(float theHeight, const NFmiPoint &theLatlon)
{
  try
  {
    float value = kFloatMissing;
    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    if (interp == kLinearly && IsGrid())  // onko lineaarinen interpolointi ja hila dataa
    {
      if (Location(theLatlon))  // katsotaanko, oliko annettu piste hilan sisällä
      {
        NFmiPoint gpoint = Grid()->LatLonToGrid(theLatlon);

        // optimointia, jos ollaan tarpeeksi lähellä hilaa, palautetaan suoraan hilan arvo
        if (fabs(gpoint.X() - round(gpoint.X())) < 0.00001 &&
            fabs(gpoint.Y() - round(gpoint.Y())) < 0.00001)
          return HeightValue(theHeight);

        int bottomLeftLocIndex = GetGridDataIndex(
            itsGridXNumber, static_cast<int>(gpoint.X()), static_cast<int>(gpoint.Y()));
        LocationIndex(bottomLeftLocIndex);
        float bottomLeftValue = HeightValue(theHeight);
        MoveRight();
        float bottomRightValue = HeightValue(theHeight);
        MoveUp();
        MoveLeft();
        float topLeftValue = HeightValue(theHeight);
        MoveRight();
        float topRightValue = HeightValue(theHeight);

        auto param = static_cast<FmiParameterName>(Param().GetParamIdent());
        double dx = gpoint.X() - floor(gpoint.X());
        double dy = gpoint.Y() - floor(gpoint.Y());

        if (param == kFmiWindVectorMS)
          value = static_cast<float>(NFmiInterpolation::WindVector(
              dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue));
        else if (param == kFmiWindDirection || param == kFmiWaveDirection)
          value = static_cast<float>(NFmiInterpolation::ModBiLinear(
              dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue, 360));
        else
          value = static_cast<float>(NFmiInterpolation::BiLinear(
              dx, dy, topLeftValue, topRightValue, bottomLeftValue, bottomRightValue));
      }
      return value;
    }
    Location(theLatlon);  // tämä hakee nearest paikan jos löytää
    return HeightValue(theHeight);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::HeightValue(float theHeight,
                                     const NFmiPoint &theLatlon,
                                     const NFmiMetTime &theTime,
                                     unsigned long theTimeRangeInMinutes)
{
  try
  {
    bool isTimeInside;

    if (Time(theTime))
      return HeightValue(theHeight, theLatlon);
    else if ((isTimeInside = IsInside(theTime)) && (theTimeRangeInMinutes > 0))
    {
      NFmiMetTime refTime(theTime);
      boost::posix_time::ptime rangeStartTime, rangeEndTime;
      float value1 = kFloatMissing;

      if (theTimeRangeInMinutes != kUnsignedLongMissing)
      {
        rangeStartTime = theTime.PosixTime() - boost::posix_time::minutes(theTimeRangeInMinutes);
        rangeEndTime = theTime.PosixTime() + boost::posix_time::minutes(theTimeRangeInMinutes);
      }
      else
      {
        FirstTime();
        rangeStartTime = Time().PosixTime();
        LastTime();
        rangeEndTime = Time().PosixTime();
      }

      for (; ((value1 == kFloatMissing) &&
              FindNearestTime(refTime, kBackward, theTimeRangeInMinutes) &&
              (Time().PosixTime() >= rangeStartTime));)
      {
        value1 = HeightValue(theHeight, theLatlon);
        refTime = Time().PosixTime() - boost::posix_time::seconds(1);
      }

      auto time1 = Time();
      float value2 = kFloatMissing;

      for (refTime = theTime; ((value2 == kFloatMissing) &&
                               FindNearestTime(refTime, kForward, theTimeRangeInMinutes) &&
                               (Time().PosixTime() <= rangeEndTime));)
      {
        // Using minute step forwards because seconds get zeroed when constructing NFmiMetTime

        value2 = HeightValue(theHeight, theLatlon);
        refTime = Time().PosixTime() + boost::posix_time::minutes(1);
      }

      return Interpolate(Param(), theTime, time1, Time(), value1, value2);
    }
    else if (isTimeInside)
    {
      if (FindNearestTime(theTime, kBackward))  // pitäisi löytyä, eikä ole reunalla, koska edellä
                                                // on kokeiltu, löytyykö theTime jo suoraan
      {
        NFmiMetTime time1(Time());
        float value1 = HeightValue(theHeight, theLatlon);
        NextTime();  // tämänkin pitää löytyä
        NFmiMetTime time2(Time());
        float value2 = HeightValue(theHeight, theLatlon);
        float value = Interpolate(Param(), theTime, time1, time2, value1, value2);
        return value;
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::HeightValue(float theHeight,
                                     const NFmiPoint &theLatlon,
                                     const NFmiMetTime &theTime)
{
  try
  {
    return HeightValue(theHeight, theLatlon, theTime, 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// TODO kFmiWindDirection tapaus pitää koodata käyttämään NFmiInterpolation::WindInterpolator:ia
float NFmiFastQueryInfo::HeightValue(float theHeight, const NFmiMetTime &theTime)
{
  try
  {
    if (Time(theTime))
      return HeightValue(theHeight);
    else if (IsInside(theTime))
    {
      if (FindNearestTime(theTime, kBackward))  // pitäisi löytyä, eikä ole reunalla, koska edellä
                                                // on kokeiltu, löytyykö theTime jo suoraan
      {
        NFmiMetTime time1(Time());
        float value1 = HeightValue(theHeight);
        NextTime();  // tämänkin pitää löytyä
        NFmiMetTime time2(Time());
        float value2 = HeightValue(theHeight);
        float value = Interpolate(Param(), theTime, time1, time2, value1, value2);
        return value;
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::FindNearestTime(const NFmiMetTime &theTime,
                                        FmiDirection theDirection,
                                        unsigned long theTimeRangeInMinutes)
{
  try
  {
    if (NFmiQueryInfo::FindNearestTime(theTime, theDirection, theTimeRangeInMinutes))
    {
      itsTimeIndex = NFmiQueryInfo::TimeIndex();
      return true;
    }
    else
    {
      itsTimeIndex = static_cast<unsigned long>(-1);  // resetoidaan, kun yritetään mennä rajan yli
      return false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsInsideRange(std::vector<float> &theLevelValues, float theValue)
{
  try
  {
    if (theValue != kFloatMissing)
    {
      if (theLevelValues.size())
      {
        float value1 = theLevelValues[0];
        float value2 = theLevelValues[theLevelValues.size() - 1];
        if (value1 != kFloatMissing && value2 != kFloatMissing)
        {
          if ((value1 <= theValue && theValue <= value2) ||
              (value2 <= theValue && theValue <= value1))
            return true;
        }
      }
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// laskee halutun paramerin arvon haluttuun korkeuteen (korkeus matriisin avulla määritetty)
// halutussa sarakkeessa eli poikkileikkaus pisteessä.
static float GetValueAtHeight(NFmiDataMatrix<float> &theParValues,
                              NFmiDataMatrix<float> &theHValues,
                              float theHeight,
                              int theColumn,
                              FmiInterpolationMethod theInterpolationMethod,
                              const NFmiFastQueryInfo &theInfo)
{
  try
  {
    double value = kFloatMissing;
    if (theColumn > static_cast<int>(theHValues.size() - 1))
      return kFloatMissing;
    std::vector<float> &tmpVec = theHValues[theColumn];
    // Teen theHeight:ille range-checkin ensin ja jos ollaan ala ja ylä arvojen ulkopuolella,
    // palautetaan missing arvo.
    if (::IsInsideRange(tmpVec, theHeight) == false)
      return kFloatMissing;

    // haetaan 1. theHeight:ia suuremman arvon sijainti
    auto pos = std::lower_bound(tmpVec.begin(), tmpVec.end(), theHeight);
    if (pos == tmpVec.end())  // ei voida interpoloida korkeuden mukaan niss tapauksissa
      return kFloatMissing;
    else if (pos == tmpVec.begin())  // voidaan palauttaa suoraan 1. kohdan arvo
      return theParValues[theColumn][0];

    int index = static_cast<int>(pos - tmpVec.begin());
    // lasketaan alemman korkeuden kerroin suhteessa haluttuun korkeuteen
    double h1 = tmpVec[index - 1];  // alempi korkeus
    double h2 = tmpVec[index];      // ylempi korkeus
    if (h1 == kFloatMissing || h2 == kFloatMissing)
      return kFloatMissing;
    double ratio = 1. - (theHeight - h1) / (h2 - h1);
    double p1 = theParValues[theColumn][index - 1];  // alempi parametri
    double p2 = theParValues[theColumn][index];      // ylempi parametri
    if (p1 == kFloatMissing || p2 == kFloatMissing)
      return kFloatMissing;
    if (theInterpolationMethod == kLinearly)
    {
      auto param = static_cast<FmiParameterName>(theInfo.Param().GetParamIdent());
      if (param == kFmiWindDirection || param == kFmiWaveDirection)
        value = static_cast<float>(NFmiInterpolation::ModLinear(1 - ratio, p1, p2, 360));
      else
        value = ratio * p1 + (1 - ratio) * p2;
    }
    else if (theInterpolationMethod == kNearestPoint)
    {
      if (::fabs(theHeight - h1) < ::fabs(theHeight - h2))
        return static_cast<float>(p1);
      else
        return static_cast<float>(p2);
    }
    return static_cast<float>(value);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// laskee halutun paramerin arvon haluttuun paine kerrokseen (korkeus matriisin avulla mritetty)
// halutussa sarakkeessa eli poikkileikkaus pisteess.
static float GetValueAtPressure(NFmiDataMatrix<float> &theParValues,
                                NFmiDataMatrix<float> &thePValues,
                                float theP,
                                int theColumn,
                                FmiInterpolationMethod theInterpolationMethod,
                                FmiParameterName theParamId)
{
  try
  {
    float value = kFloatMissing;
    if (theColumn > static_cast<int>(thePValues.size() - 1))
      return kFloatMissing;
    std::vector<float> &tmpVec = thePValues[theColumn];
    // haetaan 1. theP:a pienemmn arvon sijainti, HUOM! lower_bound-funktioon pit antaa knteiset
    // iteraattorit, koska paine vektori on laskevassa jrjestyksess.
    // HUOM2! lower_bound-funktion palauttaa 1. theP:tä suuremman arvon sijainnin, toisin kuin
    // edellä sanotaan, en tiedä mitä tässä funktiossa loppujen lopuksi pitäisi tehdä, se pitäisi
    // joskus tarkistaa... (teen kuitenkin theP:lle range-checkin ensin ja jos ollaan ala ja ylä
    // arvojen ulkopuolella, palautetaan missing arvo.)
    if (::IsInsideRange(tmpVec, theP) == false)
      return kFloatMissing;

    auto pos = std::lower_bound(tmpVec.rbegin(), tmpVec.rend(), theP);
    if (pos == tmpVec.rend())  // ei voida interpoloida korkeuden mukaan niss tapauksissa
      return kFloatMissing;
    else if (pos == tmpVec.rbegin())  // voidaan palauttaa suoraan viimeisen kohdan arvo
      return theParValues[theColumn][theParValues.NY() - 1];

    size_t index = tmpVec.size() - (pos - tmpVec.rbegin());
    // lasketaan alemman painepinnan kerroin suhteessa haluttuun painepintaan
    float p1 = tmpVec[index - 1];  // lhempn pintaa oleva paine
    float p2 = tmpVec[index];      // kauempana pinnasta oleva paine
    if (p1 == kFloatMissing || p2 == kFloatMissing)
      return kFloatMissing;
    float value1 = theParValues[theColumn][index - 1];  // lhempn pintaa oleva parametri
    float value2 = theParValues[theColumn][index];      // kauempana pinnasta oleva parametri
    if (value1 == kFloatMissing ||
        value2 == kFloatMissing)  // or-tarkistus sen takia ett ei extrapoloida vahingossa
      return kFloatMissing;
    if (theParamId == kFmiWindDirection)
      return CalcLogModLinearInterpolatedValue(p1, p2, theP, value1, value2, 360);
    else if (theParamId == kFmiWindVectorMS)
      return CalcLogInterpolatedWindWectorValue(p1, p2, theP, value1, value2);
    else if (theInterpolationMethod == kLinearly)
      return static_cast<float>(CalcLogInterpolatedValue(p1, p2, theP, value1, value2));
    else if (theInterpolationMethod == kNearestPoint)
    {
      if (::fabs(theP - p1) < ::fabs(theP - p2))
        return value1;
      else
        return value2;
    }
    return value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Täyttää annetun matriisin halutun ajan ja parametrin poikkileikkaus datalla.
// Matriisi on poikkileikkaus pisteiden kokoinen ja data täytetään joka leveliltä.
NFmiDataMatrix<float> NFmiFastQueryInfo::CalcCrossSectionLeveldata(
    NFmiFastQueryInfo &theInfo, const std::vector<NFmiPoint> &thePoints, const NFmiMetTime &theTime)
{
  try
  {
    // matriisin pitää olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(thePoints.size(), theInfo.SizeLevels(), kFloatMissing);
    if (theInfo.IsInside(theTime))
    {
      float tmpValue = 0.f;
      bool fTimeFound = theInfo.Time(theTime);
      theInfo.FirstLevel();  // pitää varmistaa, että voidaan kysyä level tietoa
      if ((theInfo.HeightDataAvailable() && theInfo.HeightParamIsRising()) ||
          ((theInfo.PressureDataAvailable()) && theInfo.PressureParamIsRising() == false))
      {  // esim. painepinta data täytetään näin, kun levelit ovat alhaalta ylöspäin
        theInfo.ResetLevel();
        for (unsigned int j = 0; j < values.NY(); j++)
        {
          theInfo.NextLevel();
          for (unsigned int i = 0; i < values.NX(); i++)
          {
            if (fTimeFound)
              tmpValue = theInfo.InterpolatedValue(thePoints[i]);
            else
              tmpValue = theInfo.InterpolatedValue(thePoints[i], theTime, 360);
            values[i][j] = tmpValue;
          }
        }
      }
      else  // yleensä esim. hybridi datassa on levelit ylhäältä als, joten data pitää täyttää
            // toisin päin
      {
        theInfo.LastLevel();  // huom! ei ole ResetLastLevel-metodia, vään tämä on FirstLevel:in
        // vastin pari
        for (unsigned int j = 0; j < values.NY(); j++)
        {
          for (unsigned int i = 0; i < values.NX(); i++)
          {
            if (fTimeFound)
              tmpValue = theInfo.InterpolatedValue(thePoints[i]);
            else
              tmpValue = theInfo.InterpolatedValue(thePoints[i], theTime, 360);
            values[i][j] = tmpValue;
          }
          theInfo.PreviousLevel();  // previous vasta loopin tällä puolella!!!
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 05-Oct-2011 PKi
// Tytt annetun matriisin halutun ajan ja parametrin poikkileikkaus datalla.
// Matriisi on poikkileikkaus pisteiden kokoinen ja data tytetn annetuilta mallipinnoilta.
static NFmiDataMatrix<float> CalcCrossSectionLeveldataHybrid(
    NFmiFastQueryInfo &theInfo,
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &thePoints,
    const NFmiMetTime &theTime)
{
  try
  {
    // matriisin pit olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(thePoints.size(), theLevels.size(), kFloatMissing);
    if (theInfo.IsInside(theTime))
    {
      float tmpValue = 0.f;
      bool fTimeFound = theInfo.Time(theTime);
      for (unsigned int j = 0; (j < theLevels.size()); j++)
        // Asetetaan taso kutsumalla groundille FirstLevel() ja muille Level().
        //
        // HUOM: Tassa ei tarkisteta tuodun/tuotujen taso(je)n tyyppia vaan jos
        // datassa on vain 1 taso (= ground) niin sen data palautetaan automaattisesti.
        // Kutsuja siis huolehtii että kutsuu tata jarkevalla taso/data yhdistelmalla!
        if (((theInfo.SizeLevels() == 1) && theInfo.FirstLevel()) || theInfo.Level(theLevels[j]))
          for (unsigned int i = 0; i < values.NX(); i++)
          {
            if (fTimeFound)
              tmpValue = theInfo.InterpolatedValue(thePoints[i]);
            else
              tmpValue = theInfo.InterpolatedValue(thePoints[i], theTime, 360);
            values[i][j] = tmpValue;
          }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Laskee paine datan painepinta level tietojen avulla. xSize kertoo matriisin x koon ja levelSize y
// koon.
// Jokaiseen columniin siis sijoitetaan vain samat paine arvot uudestaan ja uudestaan.
static NFmiDataMatrix<float> CalcCrossSectionLevelValuesFromLevelCache(
    NFmiFastQueryInfo &theInfo, int xSize, const std::vector<float> &theLevelValueCache)
{
  try
  {
    unsigned long oldLevelndex = theInfo.LevelIndex();
    NFmiDataMatrix<float> values(xSize, theInfo.SizeLevels(), kFloatMissing);
    if ((theInfo.HeightDataAvailable() && theInfo.HeightParamIsRising()) ||
        ((theInfo.PressureDataAvailable()) && !theInfo.PressureParamIsRising()))
    {
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        float levelValue = theLevelValueCache[j];
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          values[i][j] = levelValue;
        }
      }
    }
    else
    {
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        float levelValue = theLevelValueCache[values.NY() - j - 1];
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          values[i][j] = levelValue;
        }
      }
    }
    theInfo.LevelIndex(oldLevelndex);
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Funktio tekee poikkileikkauksen annetuista metri korkeuksista ja latlon pisteistä halutulle
// ajalle.
NFmiDataMatrix<float> NFmiFastQueryInfo::CrossSectionValues(
    const NFmiMetTime &theInterpolatedTime,
    const std::vector<float> &theHeights,
    const std::vector<NFmiPoint> &theLatlonPoints)
{
  try
  {
    NFmiDataMatrix<float> values(theLatlonPoints.size(), theHeights.size(), kFloatMissing);
    if (HeightDataAvailable())
    {
      FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
      // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> paramValues =
          CalcCrossSectionLeveldata(*this, theLatlonPoints, theInterpolatedTime);

      // 2. Kerää sitten level data korkeus parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> heightValues;
      if (fHeightValueAvailable)
      {
        unsigned long oldParamIndex = ParamIndex();
        bool oldFSubParamUsed = fUseSubParam;
        ParamIndex(itsHeightParamIndex);
        fUseSubParam = false;
        heightValues = CalcCrossSectionLeveldata(*this, theLatlonPoints, theInterpolatedTime);
        ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
        fUseSubParam = oldFSubParamUsed;
      }
      else if (fHeightLevelDataAvailable)
        heightValues = CalcCrossSectionLevelValuesFromLevelCache(
            *this, static_cast<int>(theLatlonPoints.size()), itsHeightLevelDataHeights);

      // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
      float tmpValue = 0.f;
      float tmpHeight = 0.f;
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpHeight = theHeights[j];
          tmpValue = GetValueAtHeight(paramValues, heightValues, tmpHeight, i, interp, *this);
          values[i][j] = tmpValue;
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::vector<float> NFmiFastQueryInfo::ConvertPressuresToHeights(
    const std::vector<float> &thePressures)
{
  try
  {
    std::vector<float> heigths(thePressures.size(), kFloatMissing);
    for (size_t i = 0; i < thePressures.size(); i++)
    {
      double heightValue =
          ::CalcHeightAtPressure(thePressures[i]) *
          1000.;  // CalcHeightAtPressure palauttaa kilometreissa, jotka muunnetaan metreiksi
      heigths[i] = static_cast<float>(heightValue);
    }

    return heigths;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::CrossSectionValuesLogP(
    const NFmiMetTime &theInterpolatedTime,
    const std::vector<float> &thePressures,
    const std::vector<NFmiPoint> &theLatlonPoints)
{
  try
  {
    if (PressureDataAvailable() == false && HeightDataAvailable())  // jos datasta ei löydy
                                                                    // paine-dataa, katsotaan
                                                                    // löytyykö siitä korkeus dataa
    {
      // Lasketaan paine vektorin avulla korkeus vektori ja lasketaan poikkileikkausarvot
      // korkeus-funktion avulla.
      std::vector<float> heightVector = ConvertPressuresToHeights(thePressures);
      return CrossSectionValues(theInterpolatedTime, heightVector, theLatlonPoints);
    }

    NFmiDataMatrix<float> values(theLatlonPoints.size(), thePressures.size(), kFloatMissing);

    if (!PressureDataAvailable())
      return values;

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    auto paramId = static_cast<FmiParameterName>(Param().GetParam()->GetIdent());
    // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
    // väliaikaiseen matriisiin
    NFmiDataMatrix<float> paramValues =
        CalcCrossSectionLeveldata(*this, theLatlonPoints, theInterpolatedTime);

    // 2. Kerää sitten level data paine parametrille (paikka+aika intepolointeineen) väliaikaiseen
    // matriisiin
    NFmiDataMatrix<float> pressureValues;
    if (fPressureValueAvailable)
    {
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;
      pressureValues = CalcCrossSectionLeveldata(*this, theLatlonPoints, theInterpolatedTime);
      ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fPressureLevelDataAvailable)
      pressureValues = CalcCrossSectionLevelValuesFromLevelCache(
          *this, static_cast<int>(theLatlonPoints.size()), itsPressureLevelDataPressures);

    // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
    float tmpValue = 0.f;
    float tmpPressure = 0.f;
    for (unsigned int j = 0; j < values.NY(); j++)
    {
      for (unsigned int i = 0; i < values.NX(); i++)
      {
        tmpPressure = thePressures[j];
        tmpValue = GetValueAtPressure(paramValues, pressureValues, tmpPressure, i, interp, paramId);
        values[i][j] = tmpValue;
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 05-Oct-2011 PKi
// Funktio tekee pintapoikkileikkauksen annetuista mallipinnoista ja latlon pisteist halutulle
// ajalle.
NFmiDataMatrix<float> NFmiFastQueryInfo::CrossSectionValuesHybrid(
    const NFmiMetTime &theInterpolatedTime,
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &theLatlonPoints)
{
  try
  {
    // Ker data halutulle parametrille (paikka+aika intepolointeineen)
    return CalcCrossSectionLeveldataHybrid(*this, theLevels, theLatlonPoints, theInterpolatedTime);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Täyttää annetun matriisin halutun pisteen ja parametrin aika-poikkileikkaus datalla.
// Matriisi on poikkileikkaus timebagin kokoinen ja data täytetään joka leveliltä.
NFmiDataMatrix<float> NFmiFastQueryInfo::CalcTimeCrossSectionLeveldata(NFmiFastQueryInfo &theInfo,
                                                                       const NFmiPoint &thePoint,
                                                                       NFmiTimeBag &theTimes)
{
  try
  {
    // matriisin pitää olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(theTimes.GetSize(), theInfo.SizeLevels(), kFloatMissing);
    float tmpValue = 0.f;
    if ((theInfo.HeightDataAvailable() && theInfo.HeightParamIsRising() == true) ||
        ((theInfo.PressureDataAvailable()) && theInfo.PressureParamIsRising() == false))
    {  // esim. painepinta data täytetään näin, kun levelit ovat alhaalta ylöspäin
      theInfo.ResetLevel();
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        theInfo.NextLevel();
        theTimes.Reset();
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          theTimes.Next();
          tmpValue = theInfo.InterpolatedValue(thePoint, theTimes.CurrentTime(), 360);
          values[i][j] = tmpValue;
        }
      }
    }
    else  // esim. hybridi datassa on levelit usein ylhäältä alas, joten data pitää täyttää toisin
          // päin
    {
      theInfo.LastLevel();  // huom! ei ole ResetLastLevel-metodia, vään tämä on FirstLevel:in
                            // vastin pari
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        theTimes.Reset();
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          theTimes.Next();
          tmpValue = theInfo.InterpolatedValue(thePoint, theTimes.CurrentTime(), 360);
          values[i][j] = tmpValue;
        }
        theInfo.PreviousLevel();  // previous vasta loopin tällä puolella!!!
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 05-Oct-2011 PKi
// Tytt annetun matriisin halutun pisteen ja parametrin aika-poikkileikkaus datalla.
// Matriisi on poikkileikkaus timebagin kokoinen ja data tytetn annetuilta leveleilt.
static NFmiDataMatrix<float> CalcTimeCrossSectionLeveldataHybrid(
    NFmiFastQueryInfo &theInfo,
    const std::vector<NFmiLevel> &theLevels,
    const NFmiPoint &thePoint,
    NFmiTimeBag &theTimes)
{
  try
  {
    // matriisin pit olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(theTimes.GetSize(), theLevels.size(), kFloatMissing);
    float tmpValue = 0.f;
    for (unsigned int j = 0; (j < theLevels.size()); j++)
    {
      // Asetetaan taso kutsumalla groundille FirstLevel() ja muille Level().
      //
      // HUOM: Tassa ei tarkisteta tuodun/tuotujen taso(je)n tyyppia vaan jos
      // datassa on vain 1 taso (= ground) niin sen data palautetaan automaattisesti.
      // Kutsuja siis huolehtii että kutsuu tata jarkevalla taso/data yhdistelmalla!
      if (((theInfo.SizeLevels() == 1) && theInfo.FirstLevel()) || theInfo.Level(theLevels[j]))
      {
        theTimes.Reset();
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          theTimes.Next();
          tmpValue = theInfo.InterpolatedValue(thePoint, theTimes.CurrentTime(), 360);
          values[i][j] = tmpValue;
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::TimeCrossSectionValues(std::vector<float> &theHeights,
                                                                const NFmiPoint &thePoint,
                                                                NFmiTimeBag &theWantedTimes)
{
  try
  {
    // kerää dataa matriisiin siten, että alhaalla (pinnalla) olevat datat ovat
    // matriisin y-akselin alapäässä.
    // x-akseli täytetään timebagistä tulevilla ajoilla

    NFmiDataMatrix<float> values(
        theWantedTimes.GetSize(), theHeights.size(), kFloatMissing);  // xnumberissa pitäisi olla
    // poikkileikkaus pisteiden
    // määrä ja ynumberissa
    // haluttujen korkeuksien määrä
    if (HeightDataAvailable())
    {
      FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
      // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> paramValues =
          CalcTimeCrossSectionLeveldata(*this, thePoint, theWantedTimes);

      // 2. Kerää sitten level data korkeus parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> heightValues;
      if (fHeightValueAvailable)
      {
        unsigned long oldParamIndex = ParamIndex();
        bool oldFSubParamUsed = fUseSubParam;
        ParamIndex(itsHeightParamIndex);
        fUseSubParam = false;
        heightValues = CalcTimeCrossSectionLeveldata(*this, thePoint, theWantedTimes);
        ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
        fUseSubParam = oldFSubParamUsed;
      }
      else if (fHeightLevelDataAvailable)
        heightValues = CalcCrossSectionLevelValuesFromLevelCache(
            *this, theWantedTimes.GetSize(), itsHeightLevelDataHeights);

      // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
      float tmpValue = 0.f;
      float tmpHeight = 0.f;
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpHeight = theHeights[j];
          tmpValue = GetValueAtHeight(paramValues, heightValues, tmpHeight, i, interp, *this);
          values[i][j] = tmpValue;
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::TimeCrossSectionValuesLogP(
    std::vector<float> &thePressures,
    const NFmiPoint &thePoint,
    NFmiTimeBag &theWantedTimes,
    unsigned int theStartTimeIndex)
{
  try
  {
    // kerää dataa matriisiin siten, että alhaalla (pinnalla) olevat datat ovat
    // matriisin y-akselin alapäässä.
    // x-akseli täytetään timebagistä tulevilla ajoilla

    if (PressureDataAvailable() == false && HeightDataAvailable())  // jos datasta ei löydy
                                                                    // paine-dataa, katsotaan
                                                                    // löytyykö siitä korkeus dataa
    {
      // Lasketaan paine vektorin avulla korkeus vektori ja lasketaan poikkileikkausarvot
      // korkeus-funktion avulla.
      std::vector<float> heightVector = ConvertPressuresToHeights(thePressures);
      return TimeCrossSectionValues(heightVector, thePoint, theWantedTimes);
    }

    NFmiDataMatrix<float> values(
        theWantedTimes.GetSize(),
        thePressures.size(),
        kFloatMissing);  // xnumberissa pitäisi olla poikkileikkaus pisteiden
    // määrä ja ynumberissa haluttujen korkeuksien määrä

    if (!PressureDataAvailable())
      return values;

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    auto paramId = static_cast<FmiParameterName>(Param().GetParam()->GetIdent());
    // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
    // väliaikaiseen matriisiin
    NFmiDataMatrix<float> paramValues =
        CalcTimeCrossSectionLeveldata(*this, thePoint, theWantedTimes);

    // 2. Kerää sitten level data korkeus parametrille (paikka+aika intepolointeineen) väliaikaiseen
    // matriisiin
    NFmiDataMatrix<float> pressureValues;
    if (fPressureValueAvailable)
    {
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;
      pressureValues = CalcTimeCrossSectionLeveldata(*this, thePoint, theWantedTimes);
      ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fPressureLevelDataAvailable)
      pressureValues = CalcCrossSectionLevelValuesFromLevelCache(
          *this, theWantedTimes.GetSize(), itsPressureLevelDataPressures);

    // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
    float tmpValue = 0.f;
    float tmpPressure = 0.f;
    for (unsigned int j = 0; j < values.NY(); j++)
    {
      for (unsigned int i = theStartTimeIndex; i < values.NX(); i++)
      {
        tmpPressure = thePressures[j];
        tmpValue = GetValueAtPressure(paramValues, pressureValues, tmpPressure, i, interp, paramId);
        values[i][j] = tmpValue;
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::TimeCrossSectionValuesHybrid(
    const std::vector<NFmiLevel> &theLevels, const NFmiPoint &thePoint, NFmiTimeBag &theWantedTimes)
{
  try
  {
    // ker mallipintadataa (mys ground) matriisiin timebagist tulevilla ajoilla
    // Ker data halutulle parametrille (paikka+aika intepolointeineen)
    return CalcTimeCrossSectionLeveldataHybrid(*this, theLevels, thePoint, theWantedTimes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Täyttää annetun matriisin reitti poikkileikkaus datalla.
// data täytetään joka leveliltä.
NFmiDataMatrix<float> NFmiFastQueryInfo::CalcRouteCrossSectionLeveldata(
    NFmiFastQueryInfo &theInfo,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // matriisin pitää olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(theLatlonPoints.size(), theInfo.SizeLevels(), kFloatMissing);
    float tmpValue = 0.f;
    if ((theInfo.HeightDataAvailable() && theInfo.HeightParamIsRising()) ||
        ((theInfo.PressureDataAvailable()) && theInfo.PressureParamIsRising() == false))
    {  // esim. painepinta data täytetään näin, kun levelit ovat alhaalta ylöspäin
      theInfo.ResetLevel();
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        theInfo.NextLevel();
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpValue = theInfo.InterpolatedValue(theLatlonPoints[i], thePointTimes[i], 360);
          values[i][j] = tmpValue;
        }
      }
    }
    else  // esim. hybridi datassa on levelit usein ylhäältä alas, joten data pitää täyttää toisin
          // päin
    {
      theInfo
          .LastLevel();  // huom! ei ole ResetLastLevel-metodia, vään tämä on FirstLevel:in vastin
      // pari
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpValue = theInfo.InterpolatedValue(theLatlonPoints[i], thePointTimes[i], 360);
          values[i][j] = tmpValue;
        }
        theInfo.PreviousLevel();  // previous vasta loopin tällä puolella!!!
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 05-Oct-2011 PKi
// Tytt annetun matriisin reitti poikkileikkaus datalla.
// data tytetn annetuilta leveleilt.
static NFmiDataMatrix<float> CalcRouteCrossSectionLeveldataHybrid(
    NFmiFastQueryInfo &theInfo,
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // matriisin pit olla siis pisteiden ja leveleiden kokoinen
    NFmiDataMatrix<float> values(theLatlonPoints.size(), theLevels.size(), kFloatMissing);
    float tmpValue = 0.f;
    for (unsigned int j = 0; (j < theLevels.size()); j++)
    {
      // Asetetaan taso kutsumalla groundille FirstLevel() ja muille Level().
      //
      // HUOM: Tassa ei tarkisteta tuodun/tuotujen taso(je)n tyyppia vaan jos
      // datassa on vain 1 taso (= ground) niin sen data palautetaan automaattisesti.
      // Kutsuja siis huolehtii että kutsuu tata jarkevalla taso/data yhdistelmalla!
      if (((theInfo.SizeLevels() == 1) && theInfo.FirstLevel()) || theInfo.Level(theLevels[j]))
      {
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpValue = theInfo.InterpolatedValue(theLatlonPoints[i], thePointTimes[i], 360);
          values[i][j] = tmpValue;
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 09-Mar-2015 PKi
// Palauttaa suurimman arvon. Kunkin arvon tulee olla 1 tai sama N, muuten palautetaan 0.
//
static size_t MaxValueOf_1_Or_N(size_t n1, size_t n2, size_t n3)
{
  try
  {
    size_t n = std::max(std::max(n1, n2), n3);

    if (((n1 == 1) || (n1 == n)) && ((n2 == 1) || (n2 == n)) && ((n3 == 1) || (n3 == n)))
      return n;

    return 0;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 09-Mar-2015 PKi
// Lentoreittihaku. Tayttaa matriisin ([N,1]) annetuille pisteille/mallipinnoille/ajoille.
//
// Levelina voi olla myos ground level
//
static NFmiDataMatrix<float> CalcFlightRouteDataHybrid(
    NFmiFastQueryInfo &theInfo,
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // Pisteita/leveleita/aikoja pitaa olla 1 tai sama N kpl, muuten palautetaan tyhja tulos
    //
    size_t N = MaxValueOf_1_Or_N(theLevels.size(), theLatlonPoints.size(), thePointTimes.size());

    if (N == 0)
      return NFmiDataMatrix<float>();

    NFmiDataMatrix<float> values(N, 1, kFloatMissing);

    // Piste/level/aika -vektoreiden indeksi-inkrementit; 1 (jos N alkiota) tai 0 jos (1 alkio)
    //
    size_t incL = (theLevels.size() > 1) ? 1 : 0, incP = (theLatlonPoints.size() > 1) ? 1 : 0,
           incT = (thePointTimes.size() > 1) ? 1 : 0;

    // Haetaan tulosarvo yksi piste/level/aika -indeksi kerrallaan
    //
    for (size_t n = 0, iL = 0, iP = 0, iT = 0; (n < N); n++, iL += incL, iP += incP, iT += incT)
    {
      // Asetetaan taso kutsumalla groundille FirstLevel() ja muille Level().
      //
      // HUOM: Tassa ei tarkisteta tuodun/tuotujen taso(je)n tyyppia vaan jos
      // datassa on vain 1 taso (= ground) niin sen data palautetaan automaattisesti.
      // Kutsuja siis huolehtii ettï¿½ kutsuu tata jarkevalla taso/data yhdistelmalla!
      if (((theInfo.SizeLevels() == 1) && theInfo.FirstLevel()) || theInfo.Level(theLevels[iL]))
        values[n][0] = theInfo.InterpolatedValue(theLatlonPoints[iP], thePointTimes[iT], 360);
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// kerää dataa matriisiin siten, että alhaalla (pinnalla) olevat datat ovat
// matriisin y-akselin alapäässä.
// Reittipoikkileikkaus eli on alku ja loppu paikat ja ajat. Jokaista paikkaa vastaa oma aika.
// aikoja ja paikkoja pitää olla yhtä paljon.
NFmiDataMatrix<float> NFmiFastQueryInfo::RouteCrossSectionValues(
    const std::vector<float> &theHeights,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    NFmiDataMatrix<float> values(
        theLatlonPoints.size(), theHeights.size(), kFloatMissing);  // xnumberissa pitäisi olla
    // poikkileikkaus pisteiden määrä
    // ja ynumberissa haluttujen
    // korkeuksien määrä

    if (HeightDataAvailable())
    {
      FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
      // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> paramValues =
          CalcRouteCrossSectionLeveldata(*this, theLatlonPoints, thePointTimes);

      // 2. Kerää sitten level data korkeus parametrille (paikka+aika intepolointeineen)
      // väliaikaiseen matriisiin
      NFmiDataMatrix<float> heightValues;
      if (fHeightValueAvailable)
      {
        unsigned long oldParamIndex = ParamIndex();
        bool oldFSubParamUsed = fUseSubParam;
        ParamIndex(itsHeightParamIndex);
        fUseSubParam = false;
        heightValues = CalcRouteCrossSectionLeveldata(*this, theLatlonPoints, thePointTimes);
        ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
        fUseSubParam = oldFSubParamUsed;
      }
      else if (fHeightLevelDataAvailable)
        heightValues = CalcCrossSectionLevelValuesFromLevelCache(
            *this, static_cast<int>(theLatlonPoints.size()), itsHeightLevelDataHeights);

      // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
      float tmpValue = 0.f;
      float tmpHeight = 0.f;
      for (unsigned int j = 0; j < values.NY(); j++)
      {
        for (unsigned int i = 0; i < values.NX(); i++)
        {
          tmpHeight = theHeights[j];
          tmpValue = GetValueAtHeight(paramValues, heightValues, tmpHeight, i, interp, *this);
          values[i][j] = tmpValue;
        }
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 09-Mar-2015 PKi
// Lentoreittihaku. Tayttaa matriisin ([N,1]) annetuille pisteille/korkeuksille/ajoille.
//
NFmiDataMatrix<float> NFmiFastQueryInfo::FlightRouteValues(
    const std::vector<float> &theHeights,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    if (!HeightDataAvailable())
      return NFmiDataMatrix<float>();

    // Pisteita/korkeuksia/aikoja pitaa olla 1 tai sama N kpl, muuten palautetaan tyhja tulos.
    //
    size_t N = MaxValueOf_1_Or_N(theHeights.size(), theLatlonPoints.size(), thePointTimes.size());

    if (N == 0)
      return NFmiDataMatrix<float>();

    NFmiDataMatrix<float> values(N, 1, kFloatMissing);

    // Ao. koodi vaatii samanpituiset paikka- ja aikavektorit, taytetaan toinen niista tarvittaessa.
    //
    std::vector<NFmiPoint> latlonPoints;
    std::vector<NFmiMetTime> pointTimes;
    bool localPoints = false, localTimes = false;

    if (theLatlonPoints.size() != thePointTimes.size())
    {
      if (theLatlonPoints.size() == 1)
      {
        latlonPoints.resize(N, theLatlonPoints[0]);
        localPoints = true;
      }
      else
      {
        pointTimes.resize(N, thePointTimes[0]);
        localTimes = true;
      }
    }

    const std::vector<NFmiPoint> &points = localPoints ? latlonPoints : theLatlonPoints;
    const std::vector<NFmiMetTime> &times = localTimes ? pointTimes : thePointTimes;

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    // 1. Kerï¿½ï¿½ ensin level data halutulle parametrille (paikka+aika intepolointeineen)
    // vï¿½liaikaiseen matriisiin
    NFmiDataMatrix<float> paramValues = CalcRouteCrossSectionLeveldata(*this, points, times);

    // 2. Kerï¿½ï¿½ sitten level data korkeus parametrille (paikka+aika intepolointeineen)
    // vï¿½liaikaiseen matriisiin
    NFmiDataMatrix<float> heightValues;
    if (fHeightValueAvailable)
    {
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsHeightParamIndex);
      fUseSubParam = false;
      heightValues = CalcRouteCrossSectionLeveldata(*this, points, times);
      ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperï¿½istï¿½ parametria
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fHeightLevelDataAvailable)
      heightValues = CalcCrossSectionLevelValuesFromLevelCache(
          *this, static_cast<int>(N), itsHeightLevelDataHeights);

    // 3. tï¿½ytetï¿½ï¿½n lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
    float tmpValue = 0.f;
    float tmpHeight = 0.f;
    for (unsigned int i = 0; i < values.NX(); i++)
    {
      tmpHeight = theHeights[i];
      tmpValue = GetValueAtHeight(paramValues, heightValues, tmpHeight, i, interp, *this);
      values[i][0] = tmpValue;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::RouteCrossSectionValuesLogP(
    const std::vector<float> &thePressures,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // määrä ja ynumberissa
    // haluttujen korkeuksien määrä
    if (PressureDataAvailable() == false && HeightDataAvailable())  // jos datasta ei löydy
                                                                    // paine-dataa, katsotaan
                                                                    // löytyykö siitä korkeus dataa
    {
      // Lasketaan paine vektorin avulla korkeus vektori ja lasketaan poikkileikkausarvot
      // korkeus-funktion avulla.
      std::vector<float> heightVector = ConvertPressuresToHeights(thePressures);
      return RouteCrossSectionValues(heightVector, theLatlonPoints, thePointTimes);
    }

    if (!PressureDataAvailable())
      return NFmiDataMatrix<float>();

    NFmiDataMatrix<float> values(
        theLatlonPoints.size(), thePressures.size(), kFloatMissing);  // xnumberissa pitäisi olla
    // poikkileikkaus pisteiden

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    auto paramId = static_cast<FmiParameterName>(Param().GetParam()->GetIdent());
    // 1. Kerää ensin level data halutulle parametrille (paikka+aika intepolointeineen)
    // väliaikaiseen matriisiin
    NFmiDataMatrix<float> paramValues =
        CalcRouteCrossSectionLeveldata(*this, theLatlonPoints, thePointTimes);

    // 2. Kerää sitten level data korkeus parametrille (paikka+aika intepolointeineen) väliaikaiseen
    // matriisiin
    NFmiDataMatrix<float> pressureValues;
    if (fPressureValueAvailable)
    {
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;
      pressureValues = CalcRouteCrossSectionLeveldata(*this, theLatlonPoints, thePointTimes);
      ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperäistä parametria
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fPressureLevelDataAvailable)
      pressureValues = CalcCrossSectionLevelValuesFromLevelCache(
          *this, static_cast<int>(theLatlonPoints.size()), itsPressureLevelDataPressures);

    // 3. täytetään lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
    float tmpValue = 0.f;
    float tmpPressure = 0.f;
    for (unsigned int j = 0; j < values.NY(); j++)
    {
      for (unsigned int i = 0; i < values.NX(); i++)
      {
        tmpPressure = thePressures[j];
        tmpValue = GetValueAtPressure(paramValues, pressureValues, tmpPressure, i, interp, paramId);
        values[i][j] = tmpValue;
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 09-Mar-2015 PKi
// Lentoreittihaku. Tayttaa matriisin ([N,1]) annetuille pisteille/paineille/ajoille.
//
NFmiDataMatrix<float> NFmiFastQueryInfo::FlightRouteValuesLogP(
    const std::vector<float> &thePressures,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    if (PressureDataAvailable() == false && HeightDataAvailable())  // jos datasta ei lï¿½ydy
                                                                    // paine-dataa, katsotaan
                                                                    // lï¿½ytyykï¿½ siitï¿½ korkeus
                                                                    // dataa
    {
      // Lasketaan paine vektorin avulla korkeus vektori ja lasketaan poikkileikkausarvot
      // korkeus-funktion avulla.
      std::vector<float> heightVector = ConvertPressuresToHeights(thePressures);
      return FlightRouteValues(heightVector, theLatlonPoints, thePointTimes);
    }

    if (!PressureDataAvailable())
      return NFmiDataMatrix<float>();

    // Pisteita/leveleita/aikoja pitaa olla 1 tai sama N kpl, muuten palautetaan tyhja tulos.
    //
    size_t N = MaxValueOf_1_Or_N(thePressures.size(), theLatlonPoints.size(), thePointTimes.size());

    if (N == 0)
      return NFmiDataMatrix<float>();

    NFmiDataMatrix<float> values(N, 1, kFloatMissing);

    // Ao. koodi vaatii samanpituiset paikka- ja aikavektorit, taytetaan toinen vektori
    // tarvittaessa.
    //
    std::vector<NFmiPoint> latlonPoints;
    std::vector<NFmiMetTime> pointTimes;
    bool localPoints = false, localTimes = false;

    if (theLatlonPoints.size() != thePointTimes.size())
    {
      if (theLatlonPoints.size() == 1)
      {
        latlonPoints.resize(N, theLatlonPoints[0]);
        localPoints = true;
      }
      else
      {
        pointTimes.resize(N, thePointTimes[0]);
        localTimes = true;
      }
    }

    const std::vector<NFmiPoint> &points = localPoints ? latlonPoints : theLatlonPoints;
    const std::vector<NFmiMetTime> &times = localTimes ? pointTimes : thePointTimes;

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();
    auto paramId = static_cast<FmiParameterName>(Param().GetParam()->GetIdent());
    // 1. Kerï¿½ï¿½ ensin level data halutulle parametrille (paikka+aika intepolointeineen)
    // vï¿½liaikaiseen matriisiin
    NFmiDataMatrix<float> paramValues = CalcRouteCrossSectionLeveldata(*this, points, times);

    // 2. Kerï¿½ï¿½ sitten level data korkeus parametrille (paikka+aika intepolointeineen)
    // vï¿½liaikaiseen matriisiin
    NFmiDataMatrix<float> pressureValues;
    if (fPressureValueAvailable)
    {
      unsigned long oldParamIndex = ParamIndex();
      bool oldFSubParamUsed = fUseSubParam;
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;
      pressureValues = CalcRouteCrossSectionLeveldata(*this, points, times);
      ParamIndex(oldParamIndex);  // laitetaan data osoittamaan takaisin alkuperï¿½istï¿½ parametria
      fUseSubParam = oldFSubParamUsed;
    }
    else if (fPressureLevelDataAvailable)
      pressureValues = CalcCrossSectionLevelValuesFromLevelCache(
          *this, static_cast<int>(N), itsPressureLevelDataPressures);

    // 3. tï¿½ytetï¿½ï¿½n lopullinen arvo-matriisi eli lasketaan data halutuille korkeuksille
    float tmpValue = 0.f;
    float tmpPressure = 0.f;
    for (unsigned int i = 0; i < values.NX(); i++)
    {
      tmpPressure = thePressures[i];
      tmpValue = GetValueAtPressure(paramValues, pressureValues, tmpPressure, i, interp, paramId);
      values[i][0] = tmpValue;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 05-Oct-2011 PKi
// ker dataa matriisiin
// Reittipoikkileikkaus eli on alku ja loppu paikat ja ajat. Jokaista paikkaa vastaa oma aika.
// aikoja ja paikkoja pit olla yht paljon.

NFmiDataMatrix<float> NFmiFastQueryInfo::RouteCrossSectionValuesHybrid(
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // Ker data halutulle parametrille (paikka+aika intepolointeineen)
    return CalcRouteCrossSectionLeveldataHybrid(*this, theLevels, theLatlonPoints, thePointTimes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// 09-Mar-2015 PKi
// Lentoreittihaku. Tayttaa matriisin ([N,1]) annetuille pisteille/mallipinnoille/ajoille.
//
// Levelina voi olla myos ground level
//
NFmiDataMatrix<float> NFmiFastQueryInfo::FlightRouteValuesHybrid(
    const std::vector<NFmiLevel> &theLevels,
    const std::vector<NFmiPoint> &theLatlonPoints,
    const std::vector<NFmiMetTime> &thePointTimes)
{
  try
  {
    // Ker data halutulle parametrille (paikka+aika intepolointeineen)
    return CalcFlightRouteDataHybrid(*this, theLevels, theLatlonPoints, thePointTimes);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle painepinnalle.
NFmiDataMatrix<float> NFmiFastQueryInfo::PressureValues(const NFmiMetTime &theInterpolatedTime,
                                                        float wantedPressureLevel)
{
  try
  {
    if (PressureDataAvailable() == false)
      throw Fmi::Exception(
          BCP,
          "Error: NFmiFastQueryInfo::PressureValues - Can't calculate pressure values, data "
          "unsuitable.");
    NFmiDataMatrix<float> values(GridXNumber(), GridYNumber(), kFloatMissing);
    for (ResetLocation(); NextLocation();)
    {
      float value = PressureLevelValue(wantedPressureLevel, theInterpolatedTime);
      values[LocationIndex() % GridXNumber()][LocationIndex() / GridXNumber()] = value;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiFastQueryInfo::DoWindComponentFix(const NFmiGrid &usedGrid,
                                           float u,
                                           float v,
                                           FmiParameterName id,
                                           NFmiDataMatrix<float> &theValues)
{
  try
  {
    float value = kFloatMissing;

    if ((u != kFloatMissing) && (v != kFloatMissing))
    {
      double azimuth1 = Grid()->Area()->TrueNorthAzimuth(usedGrid.LatLon()).ToRad();
      double azimuth2 = usedGrid.Area()->TrueNorthAzimuth(usedGrid.LatLon()).ToRad();
      double da = azimuth2 - azimuth1;

      float uu = u * cos(da) + v * sin(da);
      float vv = v * cos(da) - u * sin(da);

      value = (id == kFmiWindUMS ? uu : vv);
    }

    theValues[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] = value;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static void valBufDeleter(float *ptr)
{
  try
  {
    if (ptr)
      delete[] ptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tämä hakee haluttuun hilaan ja alueeseen dataa.
NFmiDataMatrix<float> NFmiFastQueryInfo::PressureValues(const NFmiGrid &theWantedGrid,
                                                        const NFmiMetTime &theInterpolatedTime,
                                                        float wantedPressureLevel)
{
  try
  {
    if (PressureDataAvailable() == false)
      throw Fmi::Exception(
          BCP,
          "Error: NFmiFastQueryInfo::PressureValues - Can't calculate pressure values, data "
          "unsuitable.");
    NFmiGrid usedGrid(theWantedGrid);

    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);

    auto id = FmiParameterName(Param().GetParam()->GetIdent());

    if (id == kFmiWindUMS || id == kFmiWindVMS)
    {
      // Rotate U- and V-components by the difference of the true north azimuthal angles.
      //
      // Note: getting/setting isSubParamUsed flag is obsolete when no more setting the parameter
      // index;
      //           for now keeping it.

      bool isSubParamUsed = IsSubParamUsed();

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind U-component");
      if (!Param(kFmiWindVMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind V-component");

      SetIsSubParamUsed(isSubParamUsed);

      // Get V values

      typedef std::unique_ptr<float, void (*)(float *)> valBuf;

      valBuf vValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); vPtr++)
        *vPtr = PressureLevelValue(wantedPressureLevel, usedGrid.LatLon(), theInterpolatedTime);

      // Get U values

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Internal error: could not switch to parameter U");
      SetIsSubParamUsed(isSubParamUsed);

      valBuf uValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *uPtr = uValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++)
        *uPtr = PressureLevelValue(wantedPressureLevel, usedGrid.LatLon(), theInterpolatedTime);

      // Rotate

      uPtr = uValues.get();
      vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++, vPtr++)
        DoWindComponentFix(usedGrid, *uPtr, *vPtr, id, values);

      if (!Param(id))
        throw Fmi::Exception(BCP,
                             "Internal error: could not switch to parameter " +
                                 boost::lexical_cast<std::string>(id));
      SetIsSubParamUsed(isSubParamUsed);
    }

    else
    {
      // Normal access

      for (usedGrid.Reset(); usedGrid.Next();)
      {
        float value =
            PressureLevelValue(wantedPressureLevel, usedGrid.LatLon(), theInterpolatedTime);
        values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] =
            value;
      }
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::PressureValues(const NFmiGrid &theWantedGrid,
                                                        const NFmiMetTime &theInterpolatedTime,
                                                        float wantedPressureLevel,
                                                        bool relative_uv)
{
  try
  {
    if (relative_uv)
      return PressureValues(theWantedGrid, theInterpolatedTime, wantedPressureLevel);

    if (PressureDataAvailable() == false)
      throw Fmi::Exception(
          BCP,
          "Error: NFmiFastQueryInfo::PressureValues - Can't calculate pressure values, data "
          "unsuitable.");

    NFmiGrid usedGrid(theWantedGrid);
    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);

    for (usedGrid.Reset(); usedGrid.Next();)
    {
      float value = PressureLevelValue(wantedPressureLevel, usedGrid.LatLon(), theInterpolatedTime);
      values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] = value;
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::GridValues(const NFmiGrid &theWantedGrid,
                                                    const NFmiMetTime &theInterpolatedTime)
{
  try
  {
    NFmiGrid usedGrid(theWantedGrid);

    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);
    bool timeInterpolationNeeded = (Time(theInterpolatedTime) == false);

    auto id = FmiParameterName(Param().GetParam()->GetIdent());

    if (id == kFmiWindUMS || id == kFmiWindVMS)
    {
      // Rotate U- and V-components by the difference of the true north azimuthal angles.
      //
      // Note: getting/setting isSubParamUsed flag is obsolete when no more setting the parameter
      // index;
      //           for now keeping it.

      bool isSubParamUsed = IsSubParamUsed();

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind U-component");
      if (!Param(kFmiWindVMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind V-component");

      SetIsSubParamUsed(isSubParamUsed);

      // Get V values

      typedef std::unique_ptr<float, void (*)(float *)> valBuf;

      valBuf vValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); vPtr++)
        *vPtr = timeInterpolationNeeded
                    ? InterpolatedValue(usedGrid.LatLon(), theInterpolatedTime, 180)
                    : InterpolatedValue(usedGrid.LatLon());

      // Get U values

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Internal error: could not switch to parameter U");
      SetIsSubParamUsed(isSubParamUsed);

      valBuf uValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *uPtr = uValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++)
        *uPtr = timeInterpolationNeeded
                    ? InterpolatedValue(usedGrid.LatLon(), theInterpolatedTime, 180)
                    : InterpolatedValue(usedGrid.LatLon());

      // Rotate

      uPtr = uValues.get();
      vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++, vPtr++)
        DoWindComponentFix(usedGrid, *uPtr, *vPtr, id, values);

      if (!Param(id))
        throw Fmi::Exception(BCP,
                             "Internal error: could not switch to parameter " +
                                 boost::lexical_cast<std::string>(id));
      SetIsSubParamUsed(isSubParamUsed);
    }

    else
    {
      // Normal access

      for (usedGrid.Reset(); usedGrid.Next();)
      {
        float value = timeInterpolationNeeded
                          ? InterpolatedValue(usedGrid.LatLon(), theInterpolatedTime, 180)
                          : InterpolatedValue(usedGrid.LatLon());
        values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] =
            value;
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::GridValues(const NFmiGrid &theWantedGrid,
                                                    const NFmiMetTime &theInterpolatedTime,
                                                    bool relative_uv)
{
  try
  {
    if (relative_uv)
      return GridValues(theWantedGrid, theInterpolatedTime);

    NFmiGrid usedGrid(theWantedGrid);
    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);
    bool timeInterpolationNeeded = (Time(theInterpolatedTime) == false);

    for (usedGrid.Reset(); usedGrid.Next();)
    {
      float value = timeInterpolationNeeded
                        ? InterpolatedValue(usedGrid.LatLon(), theInterpolatedTime, 180)
                        : InterpolatedValue(usedGrid.LatLon());
      values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] = value;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle korkeudelle [m].
// Jos haluat lentopinnoille dataa (Flight Level) on lentopinta -> metri kerroin = 30.5
// eli esim. lentopinta 50 saadaan laskulla 50 * 30.5 eli 1525 [m].
NFmiDataMatrix<float> NFmiFastQueryInfo::HeightValues(const NFmiMetTime &theInterpolatedTime,
                                                      float wantedHeightLevel)
{
  try
  {
    if (HeightDataAvailable() == false)
      throw Fmi::Exception(BCP,
                           "Error: NFmiFastQueryInfo::HeightValues - Can't calculate height "
                           "values, data unsuitable.");

    NFmiDataMatrix<float> values(GridXNumber(), GridYNumber(), kFloatMissing);

    for (ResetLocation(); NextLocation();)
    {
      float value = HeightValue(wantedHeightLevel, theInterpolatedTime);
      values[LocationIndex() % GridXNumber()][LocationIndex() / GridXNumber()] = value;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Sama korkeus haku, mutta haluttuun hilaan ja projektioon.
NFmiDataMatrix<float> NFmiFastQueryInfo::HeightValues(const NFmiGrid &theWantedGrid,
                                                      const NFmiMetTime &theInterpolatedTime,
                                                      float wantedHeightLevel)
{
  try
  {
    if (HeightDataAvailable() == false)
      throw Fmi::Exception(BCP,
                           "Error: NFmiFastQueryInfo::HeightValues - Can't calculate height "
                           "values, data unsuitable.");
    NFmiGrid usedGrid(theWantedGrid);
    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);

    auto id = FmiParameterName(Param().GetParam()->GetIdent());

    if (id == kFmiWindUMS || id == kFmiWindVMS)
    {
      // Rotate U- and V-components by the difference of the true north azimuthal angles.
      //
      // Note: getting/setting isSubParamUsed flag is obsolete when no more setting the parameter
      // index;
      //           for now keeping it.

      bool isSubParamUsed = IsSubParamUsed();

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind U-component");
      if (!Param(kFmiWindVMS))
        throw Fmi::Exception(BCP, "Data does not contain Wind V-component");

      SetIsSubParamUsed(isSubParamUsed);

      // Get V values

      typedef std::unique_ptr<float, void (*)(float *)> valBuf;

      valBuf vValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); vPtr++)
        *vPtr = HeightValue(wantedHeightLevel, usedGrid.LatLon(), theInterpolatedTime);

      // Get U values

      if (!Param(kFmiWindUMS))
        throw Fmi::Exception(BCP, "Internal error: could not switch to parameter U");
      SetIsSubParamUsed(isSubParamUsed);

      valBuf uValues(new float[usedGrid.XNumber() * usedGrid.YNumber()], valBufDeleter);
      float *uPtr = uValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++)
        *uPtr = HeightValue(wantedHeightLevel, usedGrid.LatLon(), theInterpolatedTime);

      // Rotate

      uPtr = uValues.get();
      vPtr = vValues.get();

      for (usedGrid.Reset(); usedGrid.Next(); uPtr++, vPtr++)
        DoWindComponentFix(usedGrid, *uPtr, *vPtr, id, values);

      if (!Param(id))
        throw Fmi::Exception(BCP,
                             "Internal error: could not switch to parameter " +
                                 boost::lexical_cast<std::string>(id));
      SetIsSubParamUsed(isSubParamUsed);
    }

    else
    {
      // Normal access

      for (usedGrid.Reset(); usedGrid.Next();)
      {
        float value = HeightValue(wantedHeightLevel, usedGrid.LatLon(), theInterpolatedTime);
        values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] =
            value;
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiDataMatrix<float> NFmiFastQueryInfo::HeightValues(const NFmiGrid &theWantedGrid,
                                                      const NFmiMetTime &theInterpolatedTime,
                                                      float wantedHeightLevel,
                                                      bool relative_uv)
{
  try
  {
    if (relative_uv)
      return HeightValues(theWantedGrid, theInterpolatedTime, wantedHeightLevel);

    if (HeightDataAvailable() == false)
      throw Fmi::Exception(BCP,
                           "Error: NFmiFastQueryInfo::HeightValues - Can't calculate height "
                           "values, data unsuitable.");

    NFmiGrid usedGrid(theWantedGrid);
    NFmiDataMatrix<float> values(usedGrid.XNumber(), usedGrid.YNumber(), kFloatMissing);

    for (usedGrid.Reset(); usedGrid.Next();)
    {
      float value = HeightValue(wantedHeightLevel, usedGrid.LatLon(), theInterpolatedTime);
      values[usedGrid.Index() % usedGrid.XNumber()][usedGrid.Index() / usedGrid.XNumber()] = value;
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Data pitää sisällään lokaatio tietoja datassaan, jos se on asemadataa ja sille löytyy
// kFmiLongitude ja kFmiLatitude parametrit
bool NFmiFastQueryInfo::HasLatlonInfoInData() const
{
  try
  {
    if (!Grid())
    {
      if (itsLongitudeParamIndex != gMissingIndex && itsLatitudeParamIndex != gMissingIndex)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// jos datassa on kFmiLongitude ja kFmiLatitude parametrit, lasketaan sijanti asetetun ajan ja
// paikan suhteen niiden avulla
NFmiPoint NFmiFastQueryInfo::GetLatlonFromData()
{
  try
  {
    double lon = PeekParamValue(itsLongitudeParamIndex);
    double lat = PeekParamValue(itsLatitudeParamIndex);
    return NFmiPoint(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// jos tiedetään jonkin parametrin indeksi, tämän avulla voidaan kurkata sen parametrin arvoa
// (aika,paikka ja leveli jo asetettuja)
// HUOM! Ei toimi aliparametrien (TotalWind ja WeatherAndCloudiness) kanssa!!!!
float NFmiFastQueryInfo::PeekParamValue(unsigned long theParamIndex)
{
  try
  {
    if (theParamIndex < SizeParams())
    {
      if (IsSubParamUsed())
      {  // Jos ali parametri on käytössä, ei voida optimoida
        auto oldParam = static_cast<FmiParameterName>(Param().GetParamIdent());
        ParamIndex(theParamIndex);
        SetIsSubParamUsed(false);
        float value = FloatValue();
        Param(oldParam);
        return value;
      }
      else
      {
        size_t idx = Index(theParamIndex, itsLocationIndex, itsLevelIndex, itsTimeIndex);
        return NFmiQueryInfo::PeekValue(idx);
      }
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Näillä Start/Restore -funktioilla otetaan nykyinen parametri tila talteen ja otetaan käyttöön
// 'erikois' korkeus-parametri.
// Palauttaa true, jos muutos oli mahdollinen. Jos kyseistä parametria ei voi käyttää, palauttaa
// false:n.
// HUOM! jos tämän kutsu palauttaa true:n, voidaan korkeus parametria käyttää ja käytön jälkeen
// pitää kutsua vastaavaa RestoreAfterUsingPressureParam -funktiota.
bool NFmiFastQueryInfo::StartUsingPressureParam()
{
  try
  {
    if (fPressureValueAvailable)  // pitää tarkistaa onko tälle datalle paineparametria tarjolla
                                  // ollenkaan.
    {
      itsOldParamIndex = ParamIndex();  // otetaan talteen originaali parametri indeksi
      fOldFSubParamUsed =
          fUseSubParam;  // otetaan talteen tieto oliko originaali parametri ali-parametri vai ei
      ParamIndex(itsPressureParamIndex);
      fUseSubParam = false;
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Ja käytön jälkeen palautus.
void NFmiFastQueryInfo::RestoreAfterUsingPressureParam()
{
  try
  {
    if (itsOldParamIndex !=
        gMissingIndex)  // ei tehdä palautusta, jos vanha indeksi on missing-arvossa
    {
      ParamIndex(itsOldParamIndex);
      fUseSubParam = fOldFSubParamUsed;

      // asetetaan vielä old-valuet missing-tilaan
      itsOldParamIndex = gMissingIndex;
      fOldFSubParamUsed = false;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiFastQueryInfo::StartUsingHeightParam()
{
  try
  {
    if (fHeightValueAvailable)  // pitää tarkistaa onko tälle datalle korkeusparametria tarjolla
                                // ollenkaan.
    {
      itsOldParamIndex = ParamIndex();  // otetaan talteen originaali parametri indeksi
      fOldFSubParamUsed =
          fUseSubParam;  // otetaan talteen tieto oliko originaali parametri ali-parametri vai ei
      ParamIndex(itsHeightParamIndex);
      fUseSubParam = false;
      return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiFastQueryInfo::RestoreAfterUsingHeightParam()
{
  try
  {
    RestoreAfterUsingPressureParam();  // tämä tekee tarvittavat asetukset
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Universaali funktio, jolla saa halutunlaisen korkeus parametrin kyseiseltä leveliltä haluttuun
// kohtaan ja aikaa interpoloituna
float NFmiFastQueryInfo::GetLevelHeightValue(FmiParameterName theParId,
                                             const NFmiPoint &theLatlon,
                                             const NFmiMetTime &theTime)
{
  try
  {
    float heightValue = kFloatMissing;
    if (theParId == kFmiPressure)
    {
      if (StartUsingPressureParam())
      {
        heightValue = InterpolatedValue(theLatlon, theTime);
        RestoreAfterUsingPressureParam();
      }
      else if (fPressureLevelDataAvailable)
        heightValue = itsPressureLevelDataPressures[itsLevelIndex];
    }
    if (theParId == kFmiGeomHeight || theParId == kFmiGeopHeight)
    {
      if (StartUsingHeightParam())
      {
        heightValue = InterpolatedValue(theLatlon, theTime);
        RestoreAfterUsingHeightParam();
      }
    }

    return heightValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::GetLevelHeightValue(FmiParameterName theParId,
                                             const NFmiLocationCache &theLocationCache,
                                             const NFmiTimeCache &theTimeCache)
{
  try
  {
    float heightValue = kFloatMissing;
    if (theParId == kFmiPressure)
    {
      if (StartUsingPressureParam())
      {
        heightValue = CachedInterpolation(theLocationCache, theTimeCache);
        RestoreAfterUsingPressureParam();
      }
      else if (fPressureLevelDataAvailable)
        heightValue = itsPressureLevelDataPressures[itsLevelIndex];
    }
    else if (theParId == kFmiGeomHeight || theParId == kFmiGeopHeight)
    {
      if (StartUsingHeightParam())
      {
        heightValue = CachedInterpolation(theLocationCache, theTimeCache);
        RestoreAfterUsingHeightParam();
      }
      else if (fHeightLevelDataAvailable)
        heightValue = itsHeightLevelDataHeights[itsLevelIndex];
    }

    return heightValue;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Geographic interpolation to specific height with land/sea handling
 */
// ----------------------------------------------------------------------

float NFmiFastQueryInfo::LandscapeInterpolatedValue(float theHeight,
                                                    const NFmiPoint &theLatLon,
                                                    const NFmiMetTime &theTime,
                                                    int theMaxMinuteRange)
{
  try
  {
    bool isWater = (theHeight == 0.0);
    return LandscapeInterpolatedValue(theHeight, isWater, theLatLon, theTime, theMaxMinuteRange);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Lapse rate adjustment to temperature

float lapseratefix(float theLapseRate, float theTrueHeight, float theModelHeight, bool theWaterFlag)
{
  try
  {
    // Limit inversion in Norwegian fjords
    const float sea_lapse_rate_limit = -3.0;

    if (theWaterFlag)
      theLapseRate = std::min(theLapseRate, sea_lapse_rate_limit);

    float diff = theTrueHeight - theModelHeight;
    if (theLapseRate > 0)
      diff = std::min(+150.f, std::max(-300.f, diff));  // Inversion: limit to -300...+150 meters
    else
      diff =
          std::min(+2000.f, std::max(-1500.f, diff));  // Normal case: limit to -1500...+2000 meters

    return theLapseRate / 1000 *
           diff;  // lapse rate unit is km, hence we divide by 1000 to get change per meters
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::LandscapeInterpolatedValue(float theHeight,
                                                    bool theWaterFlag,
                                                    const NFmiPoint &theLatLon,
                                                    const NFmiMetTime &theTime,
                                                    int theMaxMinuteRange)
{
  try
  {
    // Special handling for DewPoint

    if (Param().GetParam()->GetIdent() == kFmiDewPoint)
      return LandscapeInterpolatedValueDewPoint(theHeight, theLatLon, theTime, theMaxMinuteRange);

    // Never correct if
    //  a) the data is not in a grid : we cannot interpolate
    //  b) the height is not known: we do not even know if we're at sea or not
    if (!IsGrid() || theHeight == kFloatMissing || std::isnan(theHeight))
    {
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }

    // Never interpolate outside the grid...
    NFmiPoint xy(0, 0);
    if (!Location(theLatLon, &xy))
    {
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }

    // Peek offsets
    int dx = -static_cast<int>(std::round(xy.X()) - std::floor(xy.X()));
    int dy = -static_cast<int>(std::round(xy.Y()) - std::floor(xy.Y()));

    // Get the values from which to interpolate

    float bottomleft, bottomright, topleft, topright;

    bool time_exact = Time(theTime);

    if (time_exact)
      PeekCellValues(
          *this, dx, dy, bottomleft, bottomright, topleft, topright);  // TODO: maxminuterange???
    else
      PeekCellValues(*this,
                     dx,
                     dy,
                     theTime,
                     bottomleft,
                     bottomright,
                     topleft,
                     topright);  // TODO: maxminuterange???

    // If any value is missing, use default methods

    if (bottomleft == kFloatMissing || bottomright == kFloatMissing || topleft == kFloatMissing ||
        topright == kFloatMissing)
    {
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }

    // Do height corrections if possible

    long paramindex = ParamIndex();
    if (Param(kFmiGeopHeight))
    {
      float zbl, zbr, ztl, ztr;
      if (time_exact)
        PeekCellValues(*this, dx, dy, zbl, zbr, ztl, ztr);
      else
        PeekCellValues(*this, dx, dy, theTime, zbl, zbr, ztl, ztr);

      if (zbl != kFloatMissing && zbr != kFloatMissing && ztl != kFloatMissing &&
          ztr != kFloatMissing)
      {
        // FMI uses premultiplied (1/f = 0.102) values

        float default_lapserate = -6.5;  // degrees per kilometer

        float lapserate_bl = default_lapserate;
        float lapserate_br = default_lapserate;
        float lapserate_tl = default_lapserate;
        float lapserate_tr = default_lapserate;

        if (Param(kFmiLapseRate))
        {
          PeekCellValues(*this, dx, dy, lapserate_bl, lapserate_br, lapserate_tl, lapserate_tr);
          if (lapserate_bl == kFloatMissing)
            lapserate_bl = default_lapserate;
          if (lapserate_br == kFloatMissing)
            lapserate_br = default_lapserate;
          if (lapserate_tl == kFloatMissing)
            lapserate_tl = default_lapserate;
          if (lapserate_tr == kFloatMissing)
            lapserate_tr = default_lapserate;
        }

        // Convert the values to the desired height

        bottomleft += lapseratefix(lapserate_bl, theHeight, zbl, theWaterFlag);
        bottomright += lapseratefix(lapserate_br, theHeight, zbr, theWaterFlag);
        topleft += lapseratefix(lapserate_tl, theHeight, ztl, theWaterFlag);
        topright += lapseratefix(lapserate_tr, theHeight, ztr, theWaterFlag);
      }
    }
    ParamIndex(paramindex);

    // Coefficients for bilinear interpolation at the desired height

    float wx = xy.X() - std::floor(xy.X());
    float wy = xy.Y() - std::floor(xy.Y());

    float wbl = (1 - wx) * (1 - wy);
    float wbr = wx * (1 - wy);
    float wtl = (1 - wx) * wy;
    float wtr = wx * wy;

    // Modify the coefficients based on the land sea mask

    if (Param(kFmiLandSeaMask))
    {
      // These are zero for sea and 1.0 for land without any lakes either
      float landbl, landbr, landtl, landtr;
      if (time_exact)
        PeekCellValues(*this, dx, dy, landbl, landbr, landtl, landtr);
      else
        PeekCellValues(*this, dx, dy, theTime, landbl, landbr, landtl, landtr);

      if (landbl != kFloatMissing && landbr != kFloatMissing && landtl != kFloatMissing &&
          landtr != kFloatMissing)
      {
        // Minimum weight for any value selected by Mikko Rauhala
        const float wlimit = 0.3;

        // Handle land areas
        if (!theWaterFlag)
        {
          // Scale percentage from 0...1 to wlimit...1
          wbl *= (landbl + wlimit) / (1 + wlimit);
          wbr *= (landbr + wlimit) / (1 + wlimit);
          wtl *= (landtl + wlimit) / (1 + wlimit);
          wtr *= (landtr + wlimit) / (1 + wlimit);
        }
        else
        {
          // Scale percentage from 0...1 to 1...wlimit
          wbl *= (1 - landbl + wlimit) / (1 + wlimit);
          wbr *= (1 - landbr + wlimit) / (1 + wlimit);
          wtl *= (1 - landtl + wlimit) / (1 + wlimit);
          wtr *= (1 - landtr + wlimit) / (1 + wlimit);
        }
      }
    }
    ParamIndex(paramindex);

    // Perform combined interpolation

    return (wbl * bottomleft + wbr * bottomright + wtl * topleft + wtr * topright) /
           (wbl + wbr + wtl + wtr);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Geographic interpolation to specific height with land/sea handling
 */
// ----------------------------------------------------------------------

float NFmiFastQueryInfo::LandscapeInterpolatedValueDewPoint(float theHeight,
                                                            const NFmiPoint &theLatLon,
                                                            const NFmiMetTime &theTime,
                                                            int theMaxMinuteRange)
{
  try
  {
    bool isWater = (theHeight == 0.0);
    return LandscapeInterpolatedValueDewPoint(
        theHeight, isWater, theLatLon, theTime, theMaxMinuteRange);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

float NFmiFastQueryInfo::LandscapeInterpolatedValueDewPoint(float theHeight,
                                                            bool theWaterFlag,
                                                            const NFmiPoint &theLatLon,
                                                            const NFmiMetTime &theTime,
                                                            int theMaxMinuteRange)
{
  try
  {
    // Never correct if
    //  a) the data is not in a grid : we cannot interpolate
    //  b) the height is not known: we do not even know if we're at sea or not
    if (!IsGrid() || theHeight == kFloatMissing || std::isnan(theHeight))
    {
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }

    // Never interpolate outside the grid...
    NFmiPoint xy(0, 0);
    if (!Location(theLatLon, &xy))
    {
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }

    // Calculate from corrected temperature and normally interpolated humidity

    float t2m = kFloatMissing;
    float rh = kFloatMissing;

    // Get the helper variables first

    long tdewindex = ParamIndex();
    if (!Param(kFmiHumidity))
    {
      ParamIndex(tdewindex);
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }
    rh = InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);

    if (!Param(kFmiTemperature))
    {
      ParamIndex(tdewindex);
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);
    }
    t2m =
        LandscapeInterpolatedValue(theHeight, theWaterFlag, theLatLon, theTime, theMaxMinuteRange);
    ParamIndex(tdewindex);

    if (rh == kFloatMissing || t2m == kFloatMissing)
      return InterpolatedValue(theLatLon, theTime, theMaxMinuteRange);

    // The actual calculated value from eq 11 in
    // http://journals.ametsoc.org/doi/pdf/10.1175/BAMS-86-2-225
    // See also: https://wiki.fmi.fi/display/PROD/dewpoint_plugin.
    // Note table 1 in the reference shows how big the errors using
    // the linear approximations are, so we do not use it.

    const float Rw = 641.5;   // gas constant of water vapor
    const float L = 2.501e6;  // specific latent heat of evaporation of water

    t2m += 273.15;  // convert to Kelvins
    return t2m / (1 - t2m * log(rh / 100) * Rw / L) - 273.15;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping for dewpoint matrix
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeInterpolatedValuesDewPoint(
    const NFmiDataMatrix<float> &tdewMatrix,
    const NFmiDataMatrix<float> &temperatureMatrix,
    NFmiDataMatrix<float> &humidityMatrix,
    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  try
  {
    const float Rw = 641.5;   // gas constant of water vapor
    const float L = 2.501e6;  // specific latent heat of evaporation of water

    const bool cropNativeGrid = (theLocationCache.NX() == 0);
    const auto nx = temperatureMatrix.NX();
    const auto ny = temperatureMatrix.NY();

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    if (!cropNativeGrid)
    {
      if ((theLocationCache.NX() != nx) || (theLocationCache.NY() != ny))
        throw Fmi::Exception(BCP,
                             "NFmiFastQueryInfo::LandscapeInterpolatedValuesDewPoint: Location and "
                             "temperature matrix "
                             "dimensions are not equal");

      auto oldParamIndex = ParamIndex();
      bool hasHumidity = Param(kFmiHumidity);

      if (!hasHumidity)
        ParamIndex(oldParamIndex);

      // Get humidity (or dewpoint if humidity is not available) values for given locations

      NFmiDataMatrix<float> &matrix = (hasHumidity ? humidityMatrix : values);
      matrix.Resize(nx, ny);

      for (unsigned long i = 0; (i < nx); i++)
        for (unsigned long j = 0; (j < ny); j++)
        {
          const NFmiLocationCache &lc = theLocationCache[i][j];
          matrix[i][j] = CachedInterpolation(lc);
        }

      ParamIndex(oldParamIndex);

      if (!hasHumidity)
        return values;
    }
    else if ((tdewMatrix.NX() != nx) || (tdewMatrix.NY() != ny))
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeInterpolatedValuesDewPoint: Dewpoint and temperature matrix "
          "dimensions are not equal");

    if ((nx == 0) || (ny == 0) || (humidityMatrix.NX() != nx) || (humidityMatrix.NY() != ny))
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeInterpolatedValuesDewPoint: Humidity and temperature matrix "
          "dimensions are not equal");

    for (unsigned long i = 0; i < nx; i++)
      for (unsigned long j = 0; j < ny; j++)
      {
        float t2m = temperatureMatrix[i][j];
        float rh = humidityMatrix[i][j];

        if ((t2m != kFloatMissing) && (rh != kFloatMissing))
        {
          t2m += 273.15f;
          values[i][j] = t2m / (1 - t2m * log(rh / 100) * Rw / L) - 273.15f;
        }
        else if (cropNativeGrid)
          values[i][j] = tdewMatrix[i][j];
        else
        {
          const NFmiLocationCache &lc = theLocationCache[i][j];
          values[i][j] = CachedInterpolation(lc);
        }
      }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Temperature matrix interpolation to specific height with land/sea handling
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeInterpolatedValues(
    const NFmiDataMatrix<float> &theMatrix,
    const Fmi::CoordinateMatrix &gridPointMatrix,
    const NFmiDataMatrix<float> &demMatrix,
    const NFmiDataMatrix<bool> &waterFlagMatrix,
    const NFmiDataMatrix<float> &heightMatrix,
    const NFmiDataMatrix<float> &lapseRateMatrix,
    const NFmiDataMatrix<float> &maskMatrix)
{
  try
  {
    const double eps = 0.000001;

    auto nx = theMatrix.NX();
    auto ny = theMatrix.NY();

    if ((nx == 0) || (ny == 0) ||
        ((heightMatrix.NX() > 0) && ((heightMatrix.NX() != nx) || (heightMatrix.NY() != ny))) ||
        ((lapseRateMatrix.NX() > 0) &&
         ((lapseRateMatrix.NX() != nx) || (lapseRateMatrix.NY() != ny))) ||
        ((maskMatrix.NX() > 0) && ((maskMatrix.NX() != nx) || (maskMatrix.NY() != ny))))
      throw Fmi::Exception(BCP,
                           "NFmiFastQueryInfo::LandscapeInterpolatedValues: Input parameter matrix "
                           "dimensions are not "
                           "equal");

    auto ngx = gridPointMatrix.width();
    auto ngy = gridPointMatrix.height();

    if (ngx == 0 || ngy == 0 || demMatrix.NX() != ngx || demMatrix.NY() != ngy ||
        waterFlagMatrix.NX() != ngx || waterFlagMatrix.NY() != ngy)
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeInterpolatedValues: Gridpoint, dem and waterflag matrix "
          "dimensions are not equal");

    NFmiDataMatrix<float> values(ngx, ngy, kFloatMissing);

    for (std::size_t i = 0; i < ngx; i++)
    {
      for (std::size_t j = 0; j < ngy; j++)
      {
        // Get the values from which to interpolate. Don't correct if any value is
        // unavailable/missing
        //
        const auto xy = gridPointMatrix(i, j);
        int dx = static_cast<int>(floor(xy.first));
        int dy = static_cast<int>(floor(xy.second));

        if (!((dx >= 0) && ((dx + 1) < (int)nx) && (dy >= 0) && ((dy + 1) < (int)ny)))
        {
          if ((dx >= 0) && (dx < (int)nx) && (fabs(xy.first - dx) <= eps) && (dy >= 0) &&
              (dy < (int)ny) && (fabs(xy.second - dy) <= eps))
            values[i][j] = theMatrix[dx][dy];
          else
            values[i][j] = kFloatMissing;

          continue;
        }

        float dem = demMatrix[i][j];
        float bottomleft = theMatrix[dx][dy];
        float bottomright = theMatrix[dx + 1][dy];
        float topleft = theMatrix[dx][dy + 1];
        float topright = theMatrix[dx + 1][dy + 1];

        if ((dem == kFloatMissing) || std::isnan(dem) || (bottomleft == kFloatMissing) ||
            (bottomright == kFloatMissing) || (topleft == kFloatMissing) ||
            (topright == kFloatMissing))
        {
          values[i][j] = static_cast<float>(NFmiInterpolation::BiLinear(
              xy.first - dx, xy.second - dy, topleft, topright, bottomleft, bottomright));
          continue;
        }

        // Do height corrections if possible

        bool waterFlag = waterFlagMatrix[i][j];

        if (heightMatrix.NX() > 0)
        {
          float zbl = heightMatrix[dx][dy];
          float zbr = heightMatrix[dx + 1][dy];
          float ztl = heightMatrix[dx][dy + 1];
          float ztr = heightMatrix[dx + 1][dy + 1];

          if ((zbl != kFloatMissing) && (zbr != kFloatMissing) && (ztl != kFloatMissing) &&
              (ztr != kFloatMissing))
          {
            // FMI uses premultiplied (1/f = 0.102) values

            float default_lapserate = -6.5;  // degrees per kilometer

            float lapserate_bl = default_lapserate;
            float lapserate_br = default_lapserate;
            float lapserate_tl = default_lapserate;
            float lapserate_tr = default_lapserate;

            if (lapseRateMatrix.NX() > 0)
            {
              lapserate_bl = lapseRateMatrix[dx][dy];
              lapserate_br = lapseRateMatrix[dx + 1][dy];
              lapserate_tl = lapseRateMatrix[dx][dy + 1];
              lapserate_tr = lapseRateMatrix[dx + 1][dy + 1];

              if (lapserate_bl == kFloatMissing)
                lapserate_bl = default_lapserate;
              if (lapserate_br == kFloatMissing)
                lapserate_br = default_lapserate;
              if (lapserate_tl == kFloatMissing)
                lapserate_tl = default_lapserate;
              if (lapserate_tr == kFloatMissing)
                lapserate_tr = default_lapserate;
            }

            // Convert the values to the desired height

            bottomleft += lapseratefix(lapserate_bl, dem, zbl, waterFlag);
            bottomright += lapseratefix(lapserate_br, dem, zbr, waterFlag);
            topleft += lapseratefix(lapserate_tl, dem, ztl, waterFlag);
            topright += lapseratefix(lapserate_tr, dem, ztr, waterFlag);
          }
        }

        // Coefficients for bilinear interpolation at the desired height

        float wx = static_cast<float>(xy.first - floor(xy.first));
        float wy = static_cast<float>(xy.second - floor(xy.second));

        float wbl = (1 - wx) * (1 - wy);
        float wbr = wx * (1 - wy);
        float wtl = (1 - wx) * wy;
        float wtr = wx * wy;

        // Modify the coefficients based on the land sea mask

        if (maskMatrix.NX() > 0)
        {
          // These are zero for sea and 1.0 for land without any lakes either

          float landbl = maskMatrix[dx][dy];
          float landbr = maskMatrix[dx + 1][dy];
          float landtl = maskMatrix[dx][dy + 1];
          float landtr = maskMatrix[dx + 1][dy + 1];

          if ((landbl != kFloatMissing) && (landbr != kFloatMissing) && (landtl != kFloatMissing) &&
              (landtr != kFloatMissing))
          {
            // Minimum weight for any value selected by Mikko Rauhala

            const float wlimit = 0.3f;

            // Handle land areas

            if (!waterFlag)
            {
              // Scale percentage from 0...1 to wlimit...1

              wbl *= (landbl + wlimit) / (1 + wlimit);
              wbr *= (landbr + wlimit) / (1 + wlimit);
              wtl *= (landtl + wlimit) / (1 + wlimit);
              wtr *= (landtr + wlimit) / (1 + wlimit);
            }
            else
            {
              // Scale percentage from 0...1 to 1...wlimit

              wbl *= (1 - landbl + wlimit) / (1 + wlimit);
              wbr *= (1 - landbr + wlimit) / (1 + wlimit);
              wtl *= (1 - landtl + wlimit) / (1 + wlimit);
              wtr *= (1 - landtr + wlimit) / (1 + wlimit);
            }
          }
        }

        // Perform combined interpolation

        values[i][j] = (wbl * bottomleft + wbr * bottomright + wtl * topleft + wtr * topright) /
                       (wbl + wbr + wtl + wtr);
      }
    }
    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping for native grid temperature or dewpoint
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeValues(
    const NFmiDataMatrix<float> &theDEMMatrix, const NFmiDataMatrix<bool> &theWaterFlagMatrix)
{
  try
  {
    if (!IsGrid())
      throw Fmi::Exception(
          BCP, "NFmiFastQueryInfo::LandscapedValues: Can only be used for gridded data");

    auto ident = Param().GetParam()->GetIdent();

    if ((ident != kFmiTemperature) && (ident != kFmiDewPoint))
      throw Fmi::Exception(
          BCP, "NFmiFastQueryInfo::LandscapeValues: Can only be used for temperature or dewpoint");

    unsigned long nx = itsGridXNumber;
    unsigned long ny = itsGridYNumber;

    if ((theDEMMatrix.NX() != nx) || (theDEMMatrix.NX() != theWaterFlagMatrix.NX()) ||
        (theDEMMatrix.NY() != ny) || (theDEMMatrix.NY() != theWaterFlagMatrix.NY()))
      throw Fmi::Exception(BCP,
                           "NFmiFastQueryInfo::LandscapeValues: Dem and waterflag matrix "
                           "dimensions must equal native "
                           "grid size");

    auto values = Values();

    NFmiDataMatrix<float> temperatureMatrix;
    NFmiDataMatrix<float> &correctedTemperatureMatrix =
        ((ident == kFmiDewPoint) ? temperatureMatrix : values);
    NFmiDataMatrix<float> humidityMatrix;
    long oldParamIndex = ParamIndex();

    if (ident == kFmiDewPoint)
    {
      // Dewpoint is calculated from corrected temperature and humidity if available
      //
      if (Param(kFmiTemperature) && Param(kFmiHumidity))
      {
        humidityMatrix = Values();
        Param(kFmiTemperature);
        auto temperatureMatrix = Values();
      }
      if (temperatureMatrix.NX() == 0)
      {
        // Both temperature and humidity not available
        //
        ParamIndex(oldParamIndex);
        return values;
      }
    }

    // Get geopheight, lapserate and landseamask values if available

    NFmiDataMatrix<float> heightMatrix;
    NFmiDataMatrix<float> lapseRateMatrix;
    NFmiDataMatrix<float> maskMatrix;

    if (Param(kFmiGeopHeight))
      heightMatrix = Values();
    if (Param(kFmiLapseRate))
      lapseRateMatrix = Values();
    if (Param(kFmiLandSeaMask))
      maskMatrix = Values();

    ParamIndex(oldParamIndex);

    // Load gridpoint matrix

    Fmi::CoordinateMatrix gridPointMatrix(nx, ny, 0, 0, nx - 1, ny - 1);

    // Landscaping for temperature

    correctedTemperatureMatrix = LandscapeInterpolatedValues(correctedTemperatureMatrix,
                                                             gridPointMatrix,
                                                             theDEMMatrix,
                                                             theWaterFlagMatrix,
                                                             heightMatrix,
                                                             lapseRateMatrix,
                                                             maskMatrix);

    if (ident == kFmiDewPoint)
      // Landscaping for dewpoint

      values =
          LandscapeInterpolatedValuesDewPoint(values, correctedTemperatureMatrix, humidityMatrix);

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping with time interpolation for native grid temperature
 *                or dewpoint
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeValues(
    const NFmiMetTime &theInterpolatedTime,
    const NFmiDataMatrix<float> &theDEMMatrix,
    const NFmiDataMatrix<bool> &theWaterFlagMatrix)

{
  try
  {
    int oldTimeIndex = TimeIndex();

    // Handle exact existing time
    if (Time(theInterpolatedTime))
    {
      auto values = LandscapeValues(theDEMMatrix, theWaterFlagMatrix);
      TimeIndex(oldTimeIndex);
      return values;
    }

    int nx = itsGridXNumber;
    int ny = itsGridYNumber;
    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    // Cannot interpolate outside data range
    if (!IsInside(theInterpolatedTime))
    {
      TimeIndex(oldTimeIndex);
      throw Fmi::Exception(BCP,
                           std::string("Requested time ") +
                               theInterpolatedTime.ToStr(kYYYYMMDDHHMM).CharPtr() +
                               " outside the data time range");
    }

    // Extract leftside and rightside data values

    NFmiDataMatrix<float> values1;
    NFmiDataMatrix<float> values2;

    // pitää löytyä, koska isinside on tarkastettu edellä!!
    if (TimeToNearestStep(theInterpolatedTime, kBackward))
      values1 = LandscapeValues(theDEMMatrix, theWaterFlagMatrix);

    NFmiMetTime time1(Time());

    // pitää löytyä, koska isinside on tarkastettu edellä!!
    if (TimeToNearestStep(theInterpolatedTime, kForward))
      values2 = LandscapeValues(theDEMMatrix, theWaterFlagMatrix);

    NFmiMetTime time2(Time());

    auto diff1 = static_cast<float>(theInterpolatedTime.DifferenceInMinutes(time1));
    auto diff2 = static_cast<float>(time2.DifferenceInMinutes(time1));

    float factor = 1 - diff1 / diff2;

    // Then interpolate the data

    for (int i = 0; i < nx; i++)
      for (int j = 0; j < ny; j++)
        values[i][j] = InterpolationHelper(values1[i][j], values2[i][j], factor);

    TimeIndex(oldTimeIndex);

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping for temperature or dewpoint subgrid or for given locations
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeCroppedValues(
    int x1,
    int y1,
    int x2,
    int y2,
    const NFmiDataMatrix<float> &theDEMMatrix,
    const NFmiDataMatrix<bool> &theWaterFlagMatrix,
    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  try
  {
    if (!IsGrid())
      throw Fmi::Exception(
          BCP, "NFmiFastQueryInfo::LandscapeCroppedValues: Can only be used for gridded data");

    auto ident = Param().GetParam()->GetIdent();

    if ((ident != kFmiTemperature) && (ident != kFmiDewPoint))
      throw Fmi::Exception(BCP,
                           "NFmiFastQueryInfo::LandscapeCroppedValues: Can only be used for "
                           "temperature or dewpoint");

    bool cropNativeGrid = (theLocationCache.NX() == 0);
    bool hasCroppedDEM = false;
    bool xExtended = false, yExtended = false;
    int nx, ny;

    // Crop values for given subgrid or for subgrid covering given locations

    if (cropNativeGrid)
    {
      // When possible, extend the subgrid dimensions by 1 to interpolate values for the last column
      // and row
      //
      // theDEMMatrix and theWaterFlagMatrix can cover either the hole grid or the (extended)
      // subgrid

      nx = x2 - x1 + 1;
      ny = y2 - y1 + 1;

      if ((xExtended = ((nx < (int)theDEMMatrix.NX()) && (x2 < ((int)itsGridXNumber - 1)))))
      {
        x2++;
        nx++;
      }

      if ((yExtended = ((ny < (int)theDEMMatrix.NY()) && (y2 < ((int)itsGridYNumber - 1)))))
      {
        y2++;
        ny++;
      }

      hasCroppedDEM = (((int)theDEMMatrix.NX() == nx) && ((int)theDEMMatrix.NY() == ny));

      if (((!hasCroppedDEM) &&
           ((theDEMMatrix.NX() != itsGridXNumber) || (theDEMMatrix.NY() != itsGridYNumber))) ||
          (theDEMMatrix.NX() != theWaterFlagMatrix.NX()) ||
          (theDEMMatrix.NY() != theWaterFlagMatrix.NY()))
        throw Fmi::Exception(
            BCP,
            "NFmiFastQueryInfo::LandscapeCroppedValues: Dem and waterflag matrix dimensions must "
            "equal native or cropped grid size");
    }
    else
    {
      nx = static_cast<int>(theLocationCache.NX());
      ny = static_cast<int>(theLocationCache.NY());

      if (((int)theDEMMatrix.NX() != nx) || (theDEMMatrix.NX() != theWaterFlagMatrix.NX()) ||
          ((int)theDEMMatrix.NY() != ny) || (theDEMMatrix.NY() != theWaterFlagMatrix.NY()))
        throw Fmi::Exception(
            BCP,
            "NFmiFastQueryInfo::LandscapeCachedInterpolation: Location, dem and waterflag matrix "
            "dimensions must be equal");

      x1 = itsGridXNumber;
      x2 = 0;
      y1 = itsGridYNumber;
      y2 = 0;

      for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
          if (theLocationCache[i][j].itsLocationIndex != static_cast<unsigned long>(-1))
          {
            int x = static_cast<int>(floor(theLocationCache[i][j].itsGridPoint.X()));
            if (x < x1)
              x1 = x;
            x++;
            if (x > x2)
              x2 = x;

            int y = static_cast<int>(floor(theLocationCache[i][j].itsGridPoint.Y()));
            if (y < y1)
              y1 = y;
            y++;
            if (y > y2)
              y2 = y;
          }

      if (x1 < 0)
        x1 = 0;
      if (x2 >= (int)itsGridXNumber)
        x2 = itsGridXNumber - 1;
      if (y1 < 0)
        y1 = 0;
      if (y2 >= (int)itsGridYNumber)
        y2 = itsGridYNumber - 1;

      if (!((x2 > x1) && (y2 > y1)))
      {
        NFmiDataMatrix<float> values(nx, ny, kFloatMissing);
        return values;
      }
    }

    auto values = CroppedValues(x1, y1, x2, y2);

    NFmiDataMatrix<float> correctedTemperatureMatrix;
    NFmiDataMatrix<float> temperatureMatrix;
    NFmiDataMatrix<float> &inputTemperatureMatrix =
        ((ident == kFmiTemperature) ? values : temperatureMatrix);
    NFmiDataMatrix<float> &outputTemperatureMatrix =
        ((ident == kFmiTemperature) ? temperatureMatrix : correctedTemperatureMatrix);
    NFmiDataMatrix<float> humidityMatrix;
    long oldParamIndex = ParamIndex();

    if (ident == kFmiDewPoint)
    {
      // Dewpoint is calculated from corrected temperature and humidity if available
      //
      if (Param(kFmiTemperature) && Param(kFmiHumidity))
      {
        if (cropNativeGrid)
          humidityMatrix = CroppedValues(x1, y1, x2, y2);

        Param(kFmiTemperature);
        temperatureMatrix = CroppedValues(x1, y1, x2, y2);
      }

      if (temperatureMatrix.NX() == 0)
      {
        // Both temperature and humidity not available
        //
        // Interpolate dewpoint values for given locations or keep the cropped values

        ParamIndex(oldParamIndex);

        if (!cropNativeGrid)
        {
          values.Resize(nx, ny);

          for (int i = 0; (i < nx); i++)
            for (int j = 0; (j < ny); j++)
            {
              const NFmiLocationCache &lc = theLocationCache[i][j];
              values[i][j] = CachedInterpolation(lc);
            }
        }
        else
        {
          if (xExtended)
            values.RemoveColumn(nx - 1);
          if (yExtended)
            values.RemoveRow(ny - 1);
        }

        return values;
      }
    }

    NFmiDataMatrix<float> demMatrix;
    NFmiDataMatrix<bool> waterFlagMatrix;
    NFmiDataMatrix<float> heightMatrix;
    NFmiDataMatrix<float> lapseRateMatrix;
    NFmiDataMatrix<float> maskMatrix;

    const NFmiDataMatrix<float> &demValues =
        (((!cropNativeGrid) || hasCroppedDEM) ? theDEMMatrix : demMatrix);
    const NFmiDataMatrix<bool> &waterFlags =
        (((!cropNativeGrid) || hasCroppedDEM) ? theWaterFlagMatrix : waterFlagMatrix);

    // Get geopheight, lapserate and landseamask values if available

    if (Param(kFmiGeopHeight))
      heightMatrix = CroppedValues(x1, y1, x2, y2);
    if (Param(kFmiLapseRate))
      lapseRateMatrix = CroppedValues(x1, y1, x2, y2);
    if (Param(kFmiLandSeaMask))
      maskMatrix = CroppedValues(x1, y1, x2, y2);

    ParamIndex(oldParamIndex);

    Fmi::CoordinateMatrix gridPointMatrix(nx, ny, 0, 0, nx - 1, ny - 1);

    if (cropNativeGrid)
    {
      // Cropping native grid; load native gridpoints, and dem and waterflags for the subgrid
      // if the input matrices cover the hole native grid
      //
      if (!hasCroppedDEM)
      {
        demMatrix.Resize(nx, ny);
        waterFlagMatrix.Resize(nx, ny);
      }

      for (int i = 0, i1 = x1; i < nx; i++, i1++)
        for (int j = 0, j1 = y1; j < ny; j++, j1++)
        {
          if (!hasCroppedDEM)
          {
            // DEM data is more accurate

            auto dem = theDEMMatrix[i1][j1];

            demMatrix[i][j] = dem;
            waterFlagMatrix[i][j] = ((dem == 0) || theWaterFlagMatrix[i1][j1]);
          }
        }
    }
    else
    {
      // Load given gridpoints
      //
      for (int i = 0; i < nx; i++)
        for (int j = 0; j < ny; j++)
        {
          // Note: The subgrid x/y offsets are substracted from given gridpoints to correlate to the
          //               cropped data at indexes [0..nx-1][0..ny-1]
          //
          auto const &loc = theLocationCache[i][j];
          gridPointMatrix.set(
              i, j, NFmiPoint(loc.itsGridPoint.X() - x1, loc.itsGridPoint.Y() - y1));
        }
    }

    // Landscaping for temperature

    outputTemperatureMatrix = LandscapeInterpolatedValues(inputTemperatureMatrix,
                                                          gridPointMatrix,
                                                          demValues,
                                                          waterFlags,
                                                          heightMatrix,
                                                          lapseRateMatrix,
                                                          maskMatrix);

    if (ident == kFmiDewPoint)
    {
      // Landscaping for dewpoint
      //
      values = LandscapeInterpolatedValuesDewPoint(
          values, outputTemperatureMatrix, humidityMatrix, theLocationCache);
    }
    else
      values = outputTemperatureMatrix;

    if (cropNativeGrid)
    {
      if (xExtended)
        values.RemoveColumn(nx - 1);
      if (yExtended)
        values.RemoveRow(ny - 1);
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Time interpolated landscaped values for a subgrid
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeCroppedValues(
    const NFmiMetTime &theInterpolatedTime,
    int x1,
    int y1,
    int x2,
    int y2,
    const NFmiDataMatrix<float> &theDEMMatrix,
    const NFmiDataMatrix<bool> &theWaterFlagMatrix,
    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  try
  {
    // Only grids can be returned as matrices
    if (!IsGrid())
      return {};

    int oldTimeIndex = TimeIndex();

    // Handle exact existing time
    if (Time(theInterpolatedTime))
    {
      auto values = LandscapeCroppedValues(
          x1, y1, x2, y2, theDEMMatrix, theWaterFlagMatrix, theLocationCache);
      TimeIndex(oldTimeIndex);
      return values;
    }

    // Cannot interpolate outside data range
    if (!IsInside(theInterpolatedTime))
    {
      TimeIndex(oldTimeIndex);
      throw Fmi::Exception(BCP,
                           std::string("Requested time ") +
                               theInterpolatedTime.ToStr(kYYYYMMDDHHMM).CharPtr() +
                               " outside the data time range");
    }

    // Extract leftside and rightside data values

    auto nx = static_cast<int>(x2 - x1 + 1);
    auto ny = static_cast<int>(y2 - y1 + 1);

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    NFmiDataMatrix<float> values1;
    NFmiDataMatrix<float> values2;

    // pitää löytyä, koska isinside on tarkastettu edellä!!
    if (TimeToNearestStep(theInterpolatedTime, kBackward))
      values1 = LandscapeCroppedValues(
          x1, y1, x2, y2, theDEMMatrix, theWaterFlagMatrix, theLocationCache);
    NFmiMetTime time1(Time());

    // pitää löytyä, koska isinside on tarkastettu edellä!!
    if (TimeToNearestStep(theInterpolatedTime, kForward))
      values2 = LandscapeCroppedValues(
          x1, y1, x2, y2, theDEMMatrix, theWaterFlagMatrix, theLocationCache);
    NFmiMetTime time2(Time());

    auto diff1 = static_cast<float>(theInterpolatedTime.DifferenceInMinutes(time1));
    auto diff2 = static_cast<float>(time2.DifferenceInMinutes(time1));

    float factor = 1 - diff1 / diff2;

    // Then interpolate the data. We must be careful to obey the interpolation
    // rules of both discrete data and combined parameters

    FmiInterpolationMethod interp = Param().GetParam()->InterpolationMethod();

    if (interp != kLinearly)
    {
      if (factor > 0.5)
        values = values1;
      else
        values = values2;
    }
    else
    {
      for (int j = 0; j < ny; j++)
        for (int i = 0; i < nx; i++)
          values[i][j] = InterpolationHelper(values1[i][j], values2[i][j], factor);
    }

    TimeIndex(oldTimeIndex);

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping for temperature or dewpoint using given locations
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeCachedInterpolation(
    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache,
    const NFmiDataMatrix<float> &theDEMMatrix,
    const NFmiDataMatrix<bool> &theWaterFlagMatrix)
{
  try
  {
    if (!IsGrid())
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeCachedInterpolation: Can only be used for gridded data");

    auto ident = Param().GetParam()->GetIdent();

    if ((ident != kFmiTemperature) && (ident != kFmiDewPoint))
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeCachedInterpolation: Can only be used for temperature or "
          "dewpoint");

    int nx = static_cast<int>(theLocationCache.NX());
    int ny = static_cast<int>(theLocationCache.NY());

    if (((int)theDEMMatrix.NX() != nx) || (theDEMMatrix.NX() != theWaterFlagMatrix.NX()) ||
        ((int)theDEMMatrix.NY() != ny) || (theDEMMatrix.NY() != theWaterFlagMatrix.NY()))
      throw Fmi::Exception(
          BCP,
          "NFmiFastQueryInfo::LandscapeCachedInterpolation: Location, dem and waterflag matrix "
          "dimensions must be equal");

    // Landscaping

    auto croppedMatrix =
        LandscapeCroppedValues(0, 0, 0, 0, theDEMMatrix, theWaterFlagMatrix, theLocationCache);

    // Load landscaped values for given locations

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    for (int i = 0; i < nx; i++)
      for (int j = 0; j < ny; j++)
        values[i][j] = ((theLocationCache[i][j].itsLocationIndex != static_cast<unsigned long>(-1))
                            ? croppedMatrix[i][j]
                            : kFloatMissing);

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Landscaping with time interpolation for temperature or dewpoint
 *                using given locations
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::LandscapeCachedInterpolation(
    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache,
    const NFmiTimeCache &theTimeCache,
    const NFmiDataMatrix<float> &theDEMMatrix,
    const NFmiDataMatrix<bool> &theWaterFlagMatrix)
{
  try
  {
    if (theTimeCache.NoValue())
      return LandscapeCachedInterpolation(theLocationCache, theDEMMatrix, theWaterFlagMatrix);

    auto oldTimeIndex = TimeIndex();

    TimeIndex(theTimeCache.itsTimeIndex1);
    auto valueMatrix1 =
        LandscapeCachedInterpolation(theLocationCache, theDEMMatrix, theWaterFlagMatrix);

    TimeIndex(theTimeCache.itsTimeIndex2);
    auto valueMatrix2 =
        LandscapeCachedInterpolation(theLocationCache, theDEMMatrix, theWaterFlagMatrix);

    TimeIndex(oldTimeIndex);

    int nx = static_cast<int>(theLocationCache.NX());
    int ny = static_cast<int>(theLocationCache.NY());

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    for (int i = 0; i < nx; i++)
      for (int j = 0; j < ny; j++)
        values[i][j] =
            InterpolationHelper(valueMatrix1[i][j], valueMatrix2[i][j], theTimeCache.itsOffset);

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return native coordinates in a 2D matrix
 *
 * Note: NFmiDataMatrix is deprecated
 */
// ----------------------------------------------------------------------

Fmi::CoordinateMatrix NFmiFastQueryInfo::CoordinateMatrix(bool wrapped) const
{
  try
  {
    return itsHPlaceDescriptor->CoordinateMatrix(wrapped);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test whether the data is global apart from needing a wrap around
 */
// ----------------------------------------------------------------------

bool NFmiFastQueryInfo::NeedsGlobeWrap() const
{
  try
  {
    return itsHPlaceDescriptor->NeedsGlobeWrap();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot
 * Muutettu siten että hiladata menee kuten ennenkin, mutta
 * asemadata ladataankin 'yksiulotteiseen'-matriisiin (eli 1 x N).
 * Huom! x-dimension pitää olla 1 ja y-dimensioon laitetaan paikkojen
 * määrä ja sen pitää tapahtua näin jolloin matrix-luokka tekee
 * vain 1 + 1 vector-luokan instanssia (toisin päin tekisi 1 + N kpl).
 *
 * \param theMatrix The matrix in which to store the values
 */
// ----------------------------------------------------------------------

NFmiDataMatrix<float> NFmiFastQueryInfo::Values() const
{
  try
  {
    bool fIsGrid = IsGrid();
    unsigned long nx = fIsGrid ? itsGridXNumber : 1;
    unsigned long ny = fIsGrid ? itsGridYNumber : itsLocationSize;

    NFmiDataMatrix<float> values(nx, ny, kFloatMissing);

    size_t idx = Index(itsParamIndex, 0, itsLevelIndex, itsTimeIndex);
    size_t offset = Index(itsParamIndex, 1, itsLevelIndex, itsTimeIndex) - idx;

    // Mika: Must have this loop order so that the offset trick works

    for (unsigned long j = 0; j < ny; j++)
    {
      for (unsigned long i = 0; i < nx; i++)
      {
        values[i][j] = NFmiQueryInfo::PeekValue(idx);
        idx += offset;
      }
    }

    return values;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
