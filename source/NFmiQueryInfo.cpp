// ======================================================================
/*!
 * \file NFmiQueryInfo.cpp
 * \brief Implementation of class NFmiQueryInfo
 * \todo Refactor
 */
// ======================================================================
/*!
 * \class NFmiQueryInfo
 *
 * Undocumented
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // poistaa MSVC++2008 k��nt�j�n tekemi� varoituksia: "warning
                                 // C4996: 'sprintf': This function or variable may be unsafe.
                                 // Consider using sprintf_s instead"
#endif

#include "NFmiQueryInfo.h"
#include "NFmiBitMask.h"
//#include "NFmiDataModifier.h"
#include "NFmiDataModifierExtreme.h"
#include "NFmiDataModifierExtremePlace.h"
#include "NFmiFastQueryInfo.h"  // Rumaa lapsiluokka riippuvaista koodia optimoinnin nimiss� ModifyTimesLocationData_FullMT -metodissa
#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiInterpolation.h"
#include "NFmiQueryData.h"
#include "NFmiQueryInfoSatel.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiStringList.h"
#include "NFmiTimeList.h"
#include "NFmiTotalWind.h"
#include "NFmiWeatherAndCloudiness.h"
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
// abort()

#ifdef FMI_COMPRESSION
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#endif

// Mika: isspace on m��ritelty ctypess�
#ifdef UNIX
#include <cctype>
#endif

#ifdef _MSC_VER
#pragma warning( \
    disable : 4244 4267 4512)  // boost:in thread kirjastosta tulee ik�v�sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244 4267 4512)  // laitetaan 4244 takaisin p��lle, koska se on t�rke�
                                           // (esim. double -> int auto castaus varoitus)
#endif

using namespace std;

static const NFmiString gInfoHeaderKeyWord("FMI_");
#include "NFmiVersion.h"

#ifndef NDEBUG
int NFmiQueryInfo::itsConstructorCalls = 0;
int NFmiQueryInfo::itsDestructorCalls = 0;
#endif  // NDEBUG

// Jos on maailma dataa, joka ei mene maailman ymp�ri kokonaan, vaan esim. 0 -> 359.75
// koska tuo 360 asteen sarake olisi sama kuin 0 asteen sarake. T�ll�in jos tarvitaan interpoloida
// alueelle 359.75 - 360 arvoja, k�ytet��n t�t� indeksi� merkitsem��n NFmiLocationCache -luokan
// itsLocationIndex -dataosan arvoa, jotta osataan toimia oikein haettaessa interpolaatio arvoja
// datan 1. ja viimeisest� sarakkeesta.
static const unsigned long gStrechedGlobalGridIndex = static_cast<unsigned long>(-2);

#if 0
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \param topLeftValue Undocumented
 * \param topRightValue Undocumented
 * \param bottomLeftValue Undocumented
 * \param bottomRightValue Undocumented
 * \return Undocumented
 * \todo Refactor this into an external file
 *
 * The modular mean is calculated using the Mitsuta algorithm
 * for wind direction means.
 *
 * Reference: Mori, Y., 1986.<br>
 * <em>Evaluation of Several Single-Pass Estimators of the Mean and
 *     the Standard Deviation of Wind Direction.</em><br>
 * J Climate Appl. Metro., 25, 1387-1397.
 *
 * The formula for weighted mean is easily extrapolated.
 *
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ModBiLinearInterpolation(double x,
											 double y,
											 float & theValue,
											 float topLeftValue,
											 float topRightValue,
											 float bottomLeftValue,
											 float bottomRightValue)
{
  NFmiModMeanCalculator calculator;

  double dx = x - int(x);
  double dy = y - int(y);

  if( (dx != 0.) && (dx < 0.0001)) { dx = 0;}
  if( (dy != 0.) && (dy < 0.0001)) { dy = 0;}

  if(dx > (1-0.0001)) { dx = 1; }
  if(dy > (1-0.0001)) { dy = 1; }

  if(topLeftValue != kFloatMissing &&
	 topRightValue != kFloatMissing &&
	 bottomLeftValue != kFloatMissing &&
	 bottomRightValue != kFloatMissing)
	{
	  calculator(topRightValue, static_cast<float>(dx*dy));
	  calculator(topLeftValue, static_cast<float>((1-dx)*dy));
	  calculator(bottomRightValue, static_cast<float>(dx*(1-dy)));
	  calculator(bottomLeftValue, static_cast<float>((1-dx)*(1-dy)));
	  theValue = calculator();
	  return true;
	}

  if(dy == 0. &&
	 bottomLeftValue != kFloatMissing &&
	 bottomRightValue != kFloatMissing)
	{
	  calculator(bottomRightValue, static_cast<float>(dx));
	  calculator(bottomLeftValue, static_cast<float>(1-dx));
	  theValue =  calculator();
	  return true;
	}

  if(dx == 0. &&
	 bottomLeftValue != kFloatMissing &&
	 topLeftValue != kFloatMissing)
	{
	  calculator(topLeftValue, static_cast<float>(dy));
	  calculator(bottomLeftValue, static_cast<float>(1-dy));
	  theValue =  calculator();
	  return true;
	}

  if(dy == 1. &&
	 topLeftValue != kFloatMissing &&
	 topRightValue != kFloatMissing)
	{
	  calculator(topRightValue, static_cast<float>(dx));
	  calculator(topLeftValue, static_cast<float>(1-dx));
	  theValue =  calculator();
	  return true;
	}

  if(dx == 1. &&
	 bottomRightValue != kFloatMissing &&
	 topRightValue != kFloatMissing)
	{
	  calculator(topRightValue, static_cast<float>(dy));
	  calculator(bottomRightValue, static_cast<float>(1-dy));
	  theValue =  calculator();
	  return true;
	}

  if(dx == 0.f &&
	 dy == 0.f &&
	 bottomLeftValue != kFloatMissing)
	{
	  theValue =  bottomLeftValue;
	  return true;
	}

  if(dx == 0.f &&
	 dy == 1.f &&
	 topLeftValue != kFloatMissing)
	{
	  theValue =  topLeftValue;
	  return true;
	}

  if(dx == 1.f &&
	 dy == 0.f &&
	 bottomRightValue != kFloatMissing)
	{
	  theValue =  bottomRightValue;
	  return true;
	}

  if(dx == 1.f &&
	 dy == 1.f &&
	 topRightValue != kFloatMissing)
	{
	  theValue =  topRightValue;
	  return true;
	}

  theValue = kFloatMissing;
  return false;
}

// tein my�s aikainterpolaatiossa k�ytetyn version, eli
// minne annetaan kahden ajan v�linen kerroin ja kaksi arvoa.
bool NFmiQueryInfo::ModBiLinearInterpolation(double factor,
											 float & theValue,
											 float value1,
											 float value2)
{
  if(value1 != kFloatMissing &&
	 value2 != kFloatMissing)
	{
	  NFmiModMeanCalculator calculator;
	  calculator(value2, static_cast<float>(1-factor));
	  calculator(value1, static_cast<float>(factor));
	  theValue = calculator();
	  return true;
	}
  else if(value1 == kFloatMissing &&
		  value2 != kFloatMissing)
  {
	 theValue = value2;
	 return true;
  }
  else if(value1 != kFloatMissing &&
		  value2 == kFloatMissing)
  {
	 theValue = value1;
	 return true;
  }

  theValue = kFloatMissing;
  return false;
}
#endif

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(double theInfoVersion)
    : itsRefRawData(0),
      itsRefQueryData(0)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(0),
      itsHPlaceDescriptor(0),
      itsVPlaceDescriptor(0),
      itsTimeDescriptor(0),
      itsHeaderText(0),
      itsPostProc(0),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(0),
      itsInfoVersion(theInfoVersion),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  itsHeaderText = new NFmiStringList;
#ifndef NDEBUG
  NFmiQueryInfo::itsConstructorCalls++;
#endif  // NDEBUG
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParamDescriptor Undocumented
 * \param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 * \param theInfoVersion Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                             const NFmiTimeDescriptor &theTimeDescriptor,
                             const NFmiHPlaceDescriptor &theHPlaceDescriptor,
                             const NFmiVPlaceDescriptor &theVPlaceDescriptor,
                             double theInfoVersion)

    : itsRefRawData(0),
      itsRefQueryData(0)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(new NFmiParamDescriptor(theParamDescriptor)),
      itsHPlaceDescriptor(0),
      itsVPlaceDescriptor(0),
      itsTimeDescriptor(new NFmiTimeDescriptor(theTimeDescriptor)),
      itsHeaderText(0),
      itsPostProc(0),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(0),
      itsInfoVersion(theInfoVersion),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  if (theVPlaceDescriptor.Size())
  {
    itsVPlaceDescriptor = new NFmiVPlaceDescriptor(theVPlaceDescriptor);
  }
  else
  {
    NFmiLevelBag theLevelBag(kFmiAnyLevelType, 0, 0, 1);
    itsVPlaceDescriptor = new NFmiVPlaceDescriptor(theLevelBag);
  }

  if (theHPlaceDescriptor.Size())
  {
    itsHPlaceDescriptor = new NFmiHPlaceDescriptor(theHPlaceDescriptor);
  }
  else
  {
    NFmiLocationBag theLocationBag(new NFmiStation(), 1);
    itsHPlaceDescriptor = new NFmiHPlaceDescriptor(theLocationBag);
  }
  itsHeaderText = new NFmiStringList;
#ifndef NDEBUG
  NFmiQueryInfo::itsConstructorCalls++;
#endif  // NDEBUG
}

// ----------------------------------------------------------------------
/*!
 * \param theInfo Undocumented
 * \param theParamDescriptor Undocumented
 *�\param theTimeDescriptor Undocumented
 * \param theHPlaceDescriptor Undocumented
 * \param theVPlaceDescriptor Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(NFmiQueryData *theInfo,
                             NFmiParamDescriptor *theParamDescriptor,
                             NFmiTimeDescriptor *theTimeDescriptor,
                             NFmiHPlaceDescriptor *theHPlaceDescriptor,
                             NFmiVPlaceDescriptor *theVPlaceDescriptor)
    : itsRefRawData(theInfo->itsRawData),
      itsRefQueryData(theInfo)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(theInfo->Info()->itsParamDescriptor
                             ? new NFmiParamDescriptor(*(theInfo->Info()->itsParamDescriptor))
                             : 0),
      itsHPlaceDescriptor(theInfo->Info()->itsHPlaceDescriptor
                              ? new NFmiHPlaceDescriptor(*(theInfo->Info()->itsHPlaceDescriptor))
                              : 0),
      itsVPlaceDescriptor(theInfo->Info()->itsVPlaceDescriptor
                              ? new NFmiVPlaceDescriptor(*(theInfo->Info()->itsVPlaceDescriptor))
                              : 0),
      itsTimeDescriptor(theInfo->Info()->itsTimeDescriptor
                            ? new NFmiTimeDescriptor(*(theInfo->Info()->itsTimeDescriptor))
                            : 0),
      itsHeaderText(0),
      itsPostProc(0),
      itsNewClassIdent(theInfo->Info()->itsNewClassIdent),
      itsCombinedParamParser(0),
      itsInfoVersion(theInfo->InfoVersion()),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(0),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
  if (theParamDescriptor)
  {
    itsParamDescriptor->Reset();
    while (itsParamDescriptor->Next())
      itsParamDescriptor->SetActivity(false);

    theParamDescriptor->Reset();
    while (theParamDescriptor->Next())
    {
      if (Param(theParamDescriptor->Param()))
      {
        itsParamDescriptor->SetActivity(true);
      }
    }
  }

  if (theTimeDescriptor)
  {
    itsTimeDescriptor->Reset();
    while (itsTimeDescriptor->Next())
      itsTimeDescriptor->SetActivity(false);

    theTimeDescriptor->Reset();
    while (theTimeDescriptor->Next())
    {
      if (Time(theTimeDescriptor->Time()))
      {
        itsTimeDescriptor->SetActivity(true);
      }
    }
  }

  if (theHPlaceDescriptor)
  {
    itsHPlaceDescriptor->Reset();
    while (itsHPlaceDescriptor->Next())
      itsHPlaceDescriptor->SetActivity(false);

    theHPlaceDescriptor->Reset();
    while (theHPlaceDescriptor->Next())
    {
      if (Location(*(theHPlaceDescriptor->Location())))
      {
        itsHPlaceDescriptor->SetActivity(true);
      }
    }
  }

  if (theVPlaceDescriptor)
  {
    itsVPlaceDescriptor->Reset();
    while (itsVPlaceDescriptor->Next())
      itsVPlaceDescriptor->SetActivity(false);

    theVPlaceDescriptor->Reset();
    while (theVPlaceDescriptor->Next())
    {
      if (Level(*(theVPlaceDescriptor->Level())))
      {
        itsVPlaceDescriptor->SetActivity(true);
      }
    }
  }

  if (theInfo->Info()->Header()) itsHeaderText = new NFmiStringList(*(theInfo->Info()->Header()));

  if (theInfo->Info()->PostProcText())
    itsPostProc = new NFmiStringList(*(theInfo->Info()->PostProcText()));

  // tehd��n First, niin Info-iteraattori on heti k�ytett�viss�
  // (ja mm. levelit osoittaa 1. leveliin, mik� unohtuu helposti k�ytt�jilt�)

  First();
#ifndef NDEBUG
  NFmiQueryInfo::itsConstructorCalls++;
#endif  // NDEBUG
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theInfo The object being copied
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const NFmiQueryInfo &theInfo)
    : itsRefRawData(theInfo.itsRefRawData),
      itsRefQueryData(theInfo.itsRefQueryData)
      //  , itsStaticDataMask(theInfo.itsStaticDataMask)
      ,
      itsParamDescriptor(
          theInfo.itsParamDescriptor ? new NFmiParamDescriptor(*(theInfo.itsParamDescriptor)) : 0),
      itsHPlaceDescriptor(theInfo.itsHPlaceDescriptor
                              ? new NFmiHPlaceDescriptor(*(theInfo.itsHPlaceDescriptor))
                              : 0),
      itsVPlaceDescriptor(theInfo.itsVPlaceDescriptor
                              ? new NFmiVPlaceDescriptor(*(theInfo.itsVPlaceDescriptor))
                              : 0),
      itsTimeDescriptor(
          theInfo.itsTimeDescriptor ? new NFmiTimeDescriptor(*(theInfo.itsTimeDescriptor)) : 0),
      itsHeaderText(0),
      itsPostProc(0),
      itsNewClassIdent(theInfo.itsNewClassIdent),
      itsCombinedParamParser(
          theInfo.itsCombinedParamParser ? theInfo.itsCombinedParamParser->Clone() : 0),
      itsInfoVersion(theInfo.InfoVersion()),
      itsGridXNumber(theInfo.itsGridXNumber),
      itsGridYNumber(theInfo.itsGridYNumber)
      //  , fUseStaticDataMask(theInfo.fUseStaticDataMask)
      ,
      fIsStrechableGlobalGrid(theInfo.fIsStrechableGlobalGrid),
      itsTimeUnCertaintyStart(theInfo.itsTimeUnCertaintyStart),
      itsTimeUnCertaintyEnd(theInfo.itsTimeUnCertaintyEnd),
      itsAreaUnCertaintyStart(theInfo.itsAreaUnCertaintyStart),
      itsAreaUnCertaintyEnd(theInfo.itsAreaUnCertaintyEnd),
      fDoEndianByteSwap(theInfo.fDoEndianByteSwap),
      fHasNonFiniteValueSet(theInfo.fHasNonFiniteValueSet)
{
  if (theInfo.itsHeaderText)
  {
    itsHeaderText = new NFmiStringList;
    *itsHeaderText = *theInfo.itsHeaderText;
  }

  if (theInfo.itsPostProc)
  {
    itsPostProc = new NFmiStringList;
    *itsPostProc = *theInfo.itsPostProc;
  }
#ifndef NDEBUG
  NFmiQueryInfo::itsConstructorCalls++;
#endif  // NDEBUG
}

// ----------------------------------------------------------------------.
/*!
 * \brief Construct from file
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::NFmiQueryInfo(const string &filename)
    : itsRefRawData(0),
      itsRefQueryData(0)
      //  , itsStaticDataMask(0)
      ,
      itsParamDescriptor(0),
      itsHPlaceDescriptor(0),
      itsVPlaceDescriptor(0),
      itsTimeDescriptor(0),
      itsHeaderText(0),
      itsPostProc(0),
      itsNewClassIdent(kNFmiQueryInfo),
      itsCombinedParamParser(0),
      itsInfoVersion(7.0),
      itsGridXNumber(0),
      itsGridYNumber(0)
      //  , fUseStaticDataMask(false)
      ,
      fIsStrechableGlobalGrid(false),
      itsTimeUnCertaintyStart(0),
      itsTimeUnCertaintyEnd(0),
      itsAreaUnCertaintyStart(1),
      itsAreaUnCertaintyEnd(4),
      fDoEndianByteSwap(false),
      fHasNonFiniteValueSet(false)
{
#ifndef NDEBUG
  NFmiQueryInfo::itsConstructorCalls++;
#endif  // NDEBUG

  ifstream file(filename.c_str(), ios::in | ios::binary);
  if (!file) throw runtime_error("Could not open '" + filename + "' for reading");

#ifdef FMI_COMPRESSION
  if (NFmiFileSystem::IsCompressed(filename))
  {
    using namespace boost;
    using namespace boost::iostreams;
    filtering_stream<input> filter;
    if (iends_with(filename, ".gz"))
      filter.push(gzip_decompressor());
    else if (iends_with(filename, ".bz2"))
      filter.push(bzip2_decompressor());
    filter.push(file);
    Read(filter);
    if (!filter.good()) throw runtime_error("Error while reading '" + filename + "'");
  }
  else
  {
    Read(file);
    if (!file.good()) throw runtime_error("Error while reading '" + filename + "'");
  }
#else
  Read(file);
  if (!file.good()) throw runtime_error("Error while reading '" + filename + "'");
#endif
}

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiQueryInfo::~NFmiQueryInfo(void)
{
  Destroy();
#ifndef NDEBUG
  NFmiQueryInfo::itsDestructorCalls++;
#endif  // NDEBUG
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::Destroy(void)
{
  if (itsHeaderText)
  {
    delete itsHeaderText;
    itsHeaderText = 0;
  }

  if (itsPostProc)
  {
    delete itsPostProc;
    itsPostProc = 0;
  }

  if (itsTimeDescriptor)
  {
    delete itsTimeDescriptor;
    itsTimeDescriptor = 0;
  }

  if (itsHPlaceDescriptor)
  {
    delete itsHPlaceDescriptor;
    itsHPlaceDescriptor = 0;
  }

  if (itsVPlaceDescriptor)
  {
    delete itsVPlaceDescriptor;
    itsVPlaceDescriptor = 0;
  }

  if (itsParamDescriptor)
  {
    delete itsParamDescriptor;
    itsParamDescriptor = 0;
  }

  if (itsCombinedParamParser)
  {
    delete itsCombinedParamParser;
    itsCombinedParamParser = 0;
  }

  itsNewClassIdent = kNFmiQueryInfo;

  //  delete itsStaticDataMask;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::Reset(void)
{
  itsParamDescriptor->Reset();
  itsTimeDescriptor->Reset();
  itsHPlaceDescriptor->Reset();
  itsVPlaceDescriptor->Reset();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::First(void)
{
  Reset();
  return (itsTimeDescriptor->Next() && itsParamDescriptor->Next() && itsHPlaceDescriptor->Next() &&
          itsVPlaceDescriptor->Next());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FirstParam(bool fIgnoreSubParam)
{
  ResetParam();
  return NextParam(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

/*----------------------------------------------------------------------------------------------*/
bool NFmiQueryInfo::FirstLocation(void)
{
  ResetLocation();
  return NextLocation();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FirstLevel(void)
{
  ResetLevel();
  return NextLevel();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FirstTime(void)
{
  ResetTime();
  return NextTime();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiQueryInfo::SizeActiveLocations(void) const
{
  return 0;  // itsHPlaceDescriptor->SizeActive(); TARKISTA
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiQueryInfo::SizeActiveLevels(void) const
{
  return 0;  // itsVPlaceDescriptor?itsVPlaceDescriptor->SizeActive():0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \bug Does not work correctly
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiQueryInfo::EditParam(void)
{
  // ************** T�M� TOIMII V��RIN KORJAA!!!!! /Marko 1999.08.24 *********************
  //	if(fUseSubParam)
  //		return itsSubParam;
  return *(itsParamDescriptor->ParamBag()->Current());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

size_t NFmiQueryInfo::Index(void) const
{
  //  if(!IsLocationMasked(itsHPlaceDescriptor->Index()))
  //	return static_cast<unsigned long>(-1);

  long timeIndex = itsTimeDescriptor->Index();
  long theVPlaceIndex = itsVPlaceDescriptor->Index();

  if (theVPlaceIndex == -1) theVPlaceIndex = 0;

  return ((itsParamDescriptor->Index()) * (itsHPlaceDescriptor->Size()) *
              (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
          (itsHPlaceDescriptor->Index()) * (itsVPlaceDescriptor->Size()) *
              (itsTimeDescriptor->Size()) +
          theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
}

size_t NFmiQueryInfo::Index(unsigned long theParamIndex,
                            unsigned long theLocationIndex,
                            unsigned long theLevelIndex,
                            unsigned long theTimeIndex) const
{
  //  if(!IsLocationMasked(theLocationIndex))
  //	return static_cast<unsigned long>(-1);

  if (theLevelIndex == static_cast<unsigned long>(-1)) theLevelIndex = 0;

  size_t idx = ((theParamIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
                    (itsTimeDescriptor->Size()) +
                (theLocationIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
                theLevelIndex * itsTimeDescriptor->Size() + theTimeIndex);

  return idx;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiQueryInfo::Clone(void) const { return new NFmiQueryInfo(*this); }
// ----------------------------------------------------------------------
/*!
 * \param theCombine Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo NFmiQueryInfo::Combine(const NFmiQueryInfo &theCombine)
{
  return NFmiQueryInfo(itsParamDescriptor->Combine(*theCombine.itsParamDescriptor),
                       itsTimeDescriptor->Combine(*(theCombine).itsTimeDescriptor),
                       itsHPlaceDescriptor->Combine(*theCombine.itsHPlaceDescriptor),
                       *itsVPlaceDescriptor);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::Owner(void)
{
  if (!itsHeaderText) return NFmiString();

  if (!itsHeaderText->Reset()) return NFmiString("Unsigned");

  return *(itsHeaderText->Current());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::TimeStamp(void)
{
  if (!itsHeaderText) return NFmiString();

  if (!itsHeaderText->Reset()) return NFmiString("Unsigned");

  if (!itsHeaderText->Next()) return NFmiString("Unsigned");

  return *(itsHeaderText->Current());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::Leader(void)
{
  if (!itsHeaderText) return NFmiString();

  if (!itsHeaderText->Reset()) return NFmiString("Unsigned");

  if (!itsHeaderText->Next()) return NFmiString("Unsigned");

  if (!itsHeaderText->Next()) return NFmiString("Unsigned");

  return *(itsHeaderText->Current());
}

// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::Header(NFmiStringList *theHeader)
{
  if (theHeader)
  {
    itsHeaderText = new NFmiStringList(*theHeader);
  }
  else
  {
    itsHeaderText = new NFmiStringList();
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiStringList *NFmiQueryInfo::Header(void) const { return itsHeaderText; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::Text(void) const
{
  if (!itsHeaderText) return NFmiString();

  return *(itsHeaderText->Current());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ResetText(void)
{
  if (itsHeaderText) return false;

  if (!itsHeaderText->Reset()) return false;

  if (!itsHeaderText->Next()) return false;

  if (!itsHeaderText->Next()) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::NextText(void)
{
  if (itsHeaderText) return false;

  if (!itsHeaderText->Next()) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theHeader Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::PostProcText(NFmiStringList *theHeader)
{
  if (theHeader)
  {
    itsPostProc = new NFmiStringList(*theHeader);
  }
  else
  {
    itsPostProc = new NFmiStringList();
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiStringList *NFmiQueryInfo::PostProcText(void) const { return itsPostProc; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ResetPostProc(void)
{
  if (!itsPostProc) return false;

  if (!itsPostProc->Reset()) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::NextPostProc(void)
{
  if (!itsPostProc) return false;

  if (!itsPostProc->Next()) return false;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::PostProc(void) const
{
  if (!itsPostProc) return NFmiString();

  return *(itsPostProc->Current());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
void * NFmiQueryInfo::VoidValue(void)
{
  return itsRefQueryData->VoidValue(*this);
}
*/

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiGrid NFmiQueryInfo::GridValue(void)
{
  if (Grid())
  {
    NFmiDataPool theDataPool;
    NFmiGrid theGrid(*Grid());
    float *theFloatArray = new float[Grid()->Size()];
    unsigned long arrayCount = 0;
    ResetLocation();
    while (NextLocation())
    {
      theFloatArray[arrayCount++] = FloatValue();
    }
    theDataPool.Init(Grid()->Size(), theFloatArray);
    theGrid.Init(&theDataPool);
    delete[] theFloatArray;
    return theGrid;
  }

  return NFmiGrid();
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theInfo The other object being copied
 * \return The object assigned to
 *
 * \bug Leaks memory, since no Destroy is called
 * \bug Should protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiQueryInfo &NFmiQueryInfo::operator=(const NFmiQueryInfo &theInfo)
{
  // HUOM!!!! Eik� t�m� vuoda kun mit��n ei deletoida??????
  // Pit�isi kutsua Destroy():ta !!!!!!!!! /Marko

  itsParamDescriptor = new NFmiParamDescriptor(*theInfo.itsParamDescriptor);
  itsTimeDescriptor = new NFmiTimeDescriptor(*theInfo.itsTimeDescriptor);
  itsHPlaceDescriptor = new NFmiHPlaceDescriptor(*theInfo.itsHPlaceDescriptor);
  itsVPlaceDescriptor = new NFmiVPlaceDescriptor(*theInfo.itsVPlaceDescriptor);

  itsRefRawData = theInfo.itsRefRawData;
  itsRefQueryData = theInfo.itsRefQueryData;

  if (theInfo.Header()) itsHeaderText = new NFmiStringList(*(theInfo.Header()));

  if (theInfo.PostProcText()) itsPostProc = new NFmiStringList(*(theInfo.PostProcText()));

  itsCombinedParamParser = 0;  // ei tarvita, jos kutsutaan Destroy():ta
  if (theInfo.itsCombinedParamParser)
    itsCombinedParamParser = theInfo.itsCombinedParamParser->Clone();
  itsTimeUnCertaintyStart = theInfo.itsTimeUnCertaintyStart;
  itsTimeUnCertaintyEnd = theInfo.itsTimeUnCertaintyEnd;
  itsAreaUnCertaintyStart = theInfo.itsAreaUnCertaintyStart;
  itsAreaUnCertaintyEnd = theInfo.itsAreaUnCertaintyEnd;
  fDoEndianByteSwap = theInfo.fDoEndianByteSwap;
  fHasNonFiniteValueSet = theInfo.fHasNonFiniteValueSet;
  fIsStrechableGlobalGrid = theInfo.fIsStrechableGlobalGrid;
  itsInfoVersion = theInfo.InfoVersion();
  itsGridXNumber = theInfo.itsGridXNumber;
  itsGridYNumber = theInfo.itsGridYNumber;

  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiQueryInfo::Write(std::ostream &file) const
{
  static const int endianInfo = 0x4f464e49;  //  == 'INFO'

  // T�m� kun kirjoitetaan tiedostoon, tulee siit� dataan tieto miss�
  // arkkitehtuurissa data tiedosto on talletettu. Tekstieditorissa katsottuna
  // little-endian-koneissa (esim. Windowsissa) kirjoitetussa datatiedostossa
  // se n�kyy muodossa "INFO", mutta big-endian koneissa (esim. UNIX)
  // kirjoitettu datatiedossa n�kyy teksti "OFNI". T�m� johtuu siit�, ett�
  // integer luku kirjoitetaan stringi muodossa ja big-endian koneissa
  // bytet ovat erilailla j�rjestyksess� kuin little-endian koneissa.

  file << "@$��Q";
  char buffer[5];
  strncpy(buffer,
          reinterpret_cast<const char *>(&endianInfo),
          4);  // rumaa, kirjoitetaan buffer:iin integer tavuina!
  buffer[4] = '\0';
  file.write(
      buffer,
      4);  // t�m� kirjoittaa tiedostoon little-endian koneessa "INFO" ja big-endianissa "OFNI".
  file << "@$��"
       << " ";

  file << "VER " << itsInfoVersion << std::endl;

  file << ClassId() << " " << ClassName() << std::endl;
  file << "0 "
       << "0 "
       << "0 "
       << "0 " << std::endl;  // Varalla tulevaisuuta varten

  // Lis�tty 9.3.1998/Vili
  if (itsHeaderText)
    file << *itsHeaderText;
  else
  {
    NFmiStringList tmp;
    file << tmp;
  }

  if (itsPostProc)
    file << *itsPostProc;
  else
  {
    NFmiStringList tmp;
    file << tmp;
  }

  if (itsParamDescriptor)
    file << *itsParamDescriptor;
  else
    file << 0 << " NFmiParamDescriptor" << std::endl;

  if (itsHPlaceDescriptor)
    file << *itsHPlaceDescriptor;
  else
    file << 0 << " NFmiHPlaceDescriptor" << std::endl;

  if (itsVPlaceDescriptor)
    file << *itsVPlaceDescriptor;
  else
  {
    NFmiVPlaceDescriptor theVPlaceDescriptor;
    file << theVPlaceDescriptor;
  }

  if (itsTimeDescriptor)
    file << *itsTimeDescriptor;
  else
    file << 0 << " NFmiTimeDescriptor" << std::endl;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 * \todo Kill the dog
 */
// ----------------------------------------------------------------------

std::istream &NFmiQueryInfo::Read(std::istream &file)
{
  const int correctEndianInfo = 0x4f464e49;  //  == 'INFO' // little-endian systeemeiss� mm. Windows
  const int swappedEndianInfo = 0x494e464f;  //  == 'OFNI' // big-endian systeemeiss� mm. unix

  Destroy();

  char dirty[30];
  unsigned char buffer[6];
  file.read(reinterpret_cast<char *>(buffer), 5);  // t�m� on luettava (= "@$��Q")
  file.read(reinterpret_cast<char *>(buffer), 4);  // t�ss� tulee INFO bin��risen� integerin�

  // This is needed in case the stream is for example empty
  if (!file.good())
    throw runtime_error("NFmiQueryInfo::Read : Input stream does not contain querydata");

  const int infoInt = *reinterpret_cast<int *>(buffer);

  if (infoInt == correctEndianInfo)
    fDoEndianByteSwap = false;
  else if (infoInt == swappedEndianInfo)
    fDoEndianByteSwap = true;
  else
    throw runtime_error("NFmiQueryInfo::File does not contain endian-information bytes.");

  file.read(reinterpret_cast<char *>(buffer), 4);  // t�m� on luettava (= "@$��")

  // VERSIONHALLINTAA
  char space;
  file.get(space);

  char ver;
  file.get(ver);
  if (ver == 'V')
  {
    char tmp[3];
    file >> tmp;
    file >> itsInfoVersion;
    FmiInfoVersion = static_cast<unsigned short>(itsInfoVersion);
  }
  else
  {
    throw runtime_error("NFmiQueryInfo::File is not QueryInfo-class");
  }

  if (itsInfoVersion < 4)
  {
    throw runtime_error("NFmiQueryInfo::File is old QueryInfo-class");
  }

  // VERSIOHALLINTAA

  unsigned long classIdent;
  file >> classIdent >> dirty;

  if (classIdent != ClassId())
  {
    if (classIdent != kNFmiRadarQueryInfo)
    {
      itsNewClassIdent = classIdent;
      return file;
    }
    else
    {
      itsNewClassIdent = kNFmiRadarQueryInfo;
    }
  }

  unsigned long theReserve;
  file >> theReserve >> theReserve >> theReserve >> theReserve;

  try
  {
    itsHeaderText = new NFmiStringList;
    file >> *itsHeaderText;

    itsPostProc = new NFmiStringList;
    file >> *itsPostProc;

    itsParamDescriptor = new NFmiParamDescriptor;
    file >> *itsParamDescriptor;

    itsHPlaceDescriptor = new NFmiHPlaceDescriptor;
    file >> *itsHPlaceDescriptor;

    itsVPlaceDescriptor = new NFmiVPlaceDescriptor;
    file >> *itsVPlaceDescriptor;

    if (!itsVPlaceDescriptor->Size())
    {
      delete itsVPlaceDescriptor;
      itsVPlaceDescriptor = 0;
    }

    itsTimeDescriptor = new NFmiTimeDescriptor;
    file >> *itsTimeDescriptor;
  }
  catch (...)
  {
    Destroy();
    throw;
  }

  // One final check on the validity of the input
  if (!file.good())
    throw runtime_error("NFmiQueryInfo::Read : Input stream does not contain valid querydata");

  // FmiInfoVersion = OldFmiInfoVersion;
  return file;
}

// Parissa interpolointi funktiossa halutaan tietyille parametreille
// tehd� erikois interpolaatio. Siksi laitoin ehdon yhteen funktioon, ett�
// koodi toimisi aina samoin kaikkialla. Nyt ehdot olivat hieman
// erilaisia eri paikoissa.

static bool IsWeatherSubParam(const NFmiQueryInfo &theInfo)
{
  FmiParameterName param = static_cast<FmiParameterName>(theInfo.Param().GetParam()->GetIdent());

  return (param == kFmiWeatherSymbol3 || param == kFmiWeatherSymbol1 ||
          param == kFmiPrecipitationForm || param == kFmiPrecipitationType ||
          param == kFmiMiddleAndLowCloudCover || param == kFmiProbabilityThunderstorm);
}

static const double gInterpolatedValueEps = 0.00001;

float NFmiQueryInfo::InterpolatedValue(const NFmiPoint &theLatLonPoint)
{
  if (!Param().GetParam()) return kFloatMissing;  // parametria ei ole asetettu kohdalleen!!!!

  if (IsGrid())
  {
    NFmiLocationCache locCache = CalcLocationCache(theLatLonPoint);
    return CachedInterpolation(locCache);
  }
  else
  {
    Location(theLatLonPoint);  // asemadatalle haetaan l�himm�n pisteen arvo
    return FloatValue();
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValue_old(const NFmiPoint &theLatLonPoint)
{
  float theValue = kFloatMissing;
  NFmiParam *parameter = Param().GetParam();
  if (parameter == 0)  // parametria ei ole asetettu kohdalleen!!!!
  {
    assert(parameter);

    return theValue;
  }

  FmiInterpolationMethod interp = parameter->InterpolationMethod();
  FmiParameterName param = static_cast<FmiParameterName>(parameter->GetIdent());

  bool wcParam = IsSubParamUsed() && ::IsWeatherSubParam(*this);

  if (IsGrid() && ((interp != kNoneInterpolation && interp != kNearestPoint) || wcParam))
  {
    NFmiPoint gpoint;
    if (Location(theLatLonPoint, &gpoint))
    {
      // optimointia, jos ollaan tarpeeksi l�hell� hilaa, palautetaan suoraan hilan arvo
      if (fabs(gpoint.X() - round(gpoint.X())) < gInterpolatedValueEps &&
          fabs(gpoint.Y() - round(gpoint.Y())) < gInterpolatedValueEps)
        return FloatValue();

      // Location(theLatLonPoint)-metodi asettaa l�himp��n hilapisteeseen, josta seuraa se,
      // ett� PeekLocationValue:lle pit�� laskea mahdollinen siirtym�

      int xShift = static_cast<int>(round(gpoint.X()) - static_cast<int>(gpoint.X()));
      int yShift = static_cast<int>(round(gpoint.Y()) - static_cast<int>(gpoint.Y()));

      // HESSAA, hsade, sateenolomuoto ja sateen tyypit pit�� hoitaa erikoistapauksina
      if (wcParam)
        return InterpolatedValueForCombinedParam_old(gpoint, xShift, yShift);
      else if (param == kFmiWindDirection)
      {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
        float blWD = PeekLocationValue(0 - xShift, 0 - yShift);
        float brWD = PeekLocationValue(1 - xShift, 0 - yShift);
        float tlWD = PeekLocationValue(0 - xShift, 1 - yShift);
        float trWD = PeekLocationValue(1 - xShift, 1 - yShift);
        Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
        float blWS = PeekLocationValue(0 - xShift, 0 - yShift);
        float brWS = PeekLocationValue(1 - xShift, 0 - yShift);
        float tlWS = PeekLocationValue(0 - xShift, 1 - yShift);
        float trWS = PeekLocationValue(1 - xShift, 1 - yShift);
        Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
        double dx = gpoint.X() - floor(gpoint.X());
        double dy = gpoint.Y() - floor(gpoint.Y());
        NFmiInterpolation::WindInterpolator windInterpolator;
        windInterpolator.operator()(blWS, blWD, (1 - dx) * (1 - dy));
        windInterpolator.operator()(brWS, brWD, dx * (1 - dy));
        windInterpolator.operator()(trWS, trWD, dx * dy);
        windInterpolator.operator()(tlWS, tlWD, (1 - dx) * dy);

        theValue = static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa vain
                                                                      // tuulen suunta (t�m� pit��
                                                                      // tehd� jotenkin fiksummin)
        return theValue;
      }
      else if (param == kFmiWaveDirection)
      {
        // modulo 360 interpolation
        float bottomLeftValue = PeekLocationValue(0 - xShift, 0 - yShift);
        float bottomRightValue = PeekLocationValue(1 - xShift, 0 - yShift);
        float topLeftValue = PeekLocationValue(0 - xShift, 1 - yShift);
        float topRightValue = PeekLocationValue(1 - xShift, 1 - yShift);

        theValue = static_cast<float>(NFmiInterpolation::ModBiLinear(gpoint.X() - floor(gpoint.X()),
                                                                     gpoint.Y() - floor(gpoint.Y()),
                                                                     topLeftValue,
                                                                     topRightValue,
                                                                     bottomLeftValue,
                                                                     bottomRightValue));
        if (theValue != kFloatMissing) return theValue;
      }
      else if (interp == kByCombinedParam)
      {
        if (param == kFmiWeatherAndCloudiness)
          return InterpolatedValueForWeatherAndCloudiness(gpoint, xShift, yShift);
        else if (param == kFmiTotalWindMS)
          return InterpolatedValueForTotalWind(gpoint, xShift, yShift);

        return theValue;
      }
      else
      {
        float bottomLeftValue = PeekLocationValue(0 - xShift, 0 - yShift);
        float bottomRightValue = PeekLocationValue(1 - xShift, 0 - yShift);
        float topLeftValue = PeekLocationValue(0 - xShift, 1 - yShift);
        float topRightValue = PeekLocationValue(1 - xShift, 1 - yShift);

        theValue = static_cast<float>(NFmiInterpolation::BiLinear(gpoint.X() - floor(gpoint.X()),
                                                                  gpoint.Y() - floor(gpoint.Y()),
                                                                  topLeftValue,
                                                                  topRightValue,
                                                                  bottomLeftValue,
                                                                  bottomRightValue));

        if (theValue != kFloatMissing) return theValue;

        // 4.6.2003/Marko Laitoin menem��n puuttuvassa tapauksessa sittenkin
        // 'nearest'-metodiin, koska muuten puuttuvan alue datan tapauksissa
        // laskuissa j��v�n datan alue pienenee ehk� liikaakin.
      }
    }
  }
  Location(theLatLonPoint);  // 31.8.99 Lasse korvasi seuraavan t�ll� ja toimii
  theValue = FloatValue();
  return theValue;
}

float NFmiQueryInfo::InterpolatedValueForCombinedParam(const NFmiPoint &theGridPoint)
{
  // jos esim. hessaa ei olekaan weatherandcloudinesissa vaan omana parametrinaa,
  // homma kaatuu t�h�n, t�m� on vain h�t� korjaus kaatumatautia vastaan itsCombinedParamParser
  // on nolla-pointteri, ja sit� ei auta kuitenkaan luoda, koska siihen ei saa kuitenkaan arvoja.
  if (!IsSubParamUsed())
    return FloatValue();  // kokeilen korjata t�t� n�in, jolloin palautetaan nearest arvo (t�m� on
                          // h�t� paskan h�t� paska)

  int xShift = static_cast<int>(round(theGridPoint.X()) - static_cast<int>(theGridPoint.X()));
  int yShift = static_cast<int>(round(theGridPoint.Y()) - static_cast<int>(theGridPoint.Y()));

  PeekLocationValue(0 - xShift, 0 - yShift);
  NFmiWeatherAndCloudiness bottomLeftweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(1 - xShift, 0 - yShift);
  NFmiWeatherAndCloudiness bottomRightweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(0 - xShift, 1 - yShift);
  NFmiWeatherAndCloudiness topLeftweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(1 - xShift, 1 - yShift);
  NFmiWeatherAndCloudiness topRightweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));
  return float(itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 *
 * \bug dummy muuttujaa ei kayteta ollenkaan
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForCombinedParam_old(const NFmiPoint &theGridPoint,
                                                           int theXShift,
                                                           int theYShift)
{
  // jos esim. hessaa ei olekaan weatherandcloudinesissa vaan omana parametrinaa,
  // homma kaatuu t�h�n, t�m� on vain h�t� korjaus kaatumatautia vastaan itsCombinedParamParser
  // on nolla-pointteri, ja sit� ei auta kuitenkaan luoda, koska siihen ei saa kuitenkaan arvoja.

  if (!IsSubParamUsed())
    return FloatValue();  // kokeilen korjata t�t� n�in, jolloin palautetaan nearest arvo (t�m� on
                          // h�t� paskan h�t� paska)

  PeekLocationValue(0 - theXShift, 0 - theYShift);
  NFmiWeatherAndCloudiness bottomLeftweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(1 - theXShift, 0 - theYShift);
  NFmiWeatherAndCloudiness bottomRightweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(0 - theXShift, 1 - theYShift);
  NFmiWeatherAndCloudiness topLeftweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));
  PeekLocationValue(1 - theXShift, 1 - theYShift);
  NFmiWeatherAndCloudiness topRightweather(
      *static_cast<NFmiWeatherAndCloudiness *>(itsCombinedParamParser));

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));
  return float(itsCombinedParamParser->SubValue(
      FmiParameterName(Param().GetParam()->GetIdent())));  // kFmiWeatherSymbol3));
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForWeatherAndCloudiness(const NFmiPoint &theGridPoint,
                                                              int theXShift,
                                                              int theYShift)
{
  NFmiWeatherAndCloudiness bottomLeftweather(itsInfoVersion);
  bottomLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 0 - theYShift));

  NFmiWeatherAndCloudiness bottomRightweather(itsInfoVersion);
  bottomRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 0 - theYShift));

  NFmiWeatherAndCloudiness topLeftweather(itsInfoVersion);
  topLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 1 - theYShift));

  NFmiWeatherAndCloudiness topRightweather(itsInfoVersion);
  topRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 1 - theYShift));

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));

  return itsCombinedParamParser->TransformedFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForWeatherAndCloudiness(const NFmiMetTime &theTime,
                                                              int theMaxMinuteRange,
                                                              const NFmiPoint &theGridPoint,
                                                              int theXShift,
                                                              int theYShift)
{
  unsigned int oldLocationIndex = LocationIndex();

  // t�m� liikkuminen on melkoista virityst�
  MoveLeft(theXShift);
  MoveDown(theYShift);
  NFmiWeatherAndCloudiness bottomLeftweather(itsInfoVersion);
  bottomLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveRight(1);
  NFmiWeatherAndCloudiness bottomRightweather(itsInfoVersion);
  bottomRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveUp(1);
  NFmiWeatherAndCloudiness topRightweather(itsInfoVersion);
  topRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveLeft(1);
  NFmiWeatherAndCloudiness topLeftweather(itsInfoVersion);
  topLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));

  LocationIndex(oldLocationIndex);

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));

  return itsCombinedParamParser->TransformedFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForTotalWind(const NFmiPoint &theGridPoint,
                                                   int theXShift,
                                                   int theYShift)
{
  NFmiTotalWind bottomLeftweather(itsInfoVersion);
  bottomLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 0 - theYShift));

  NFmiTotalWind bottomRightweather(itsInfoVersion);
  bottomRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 0 - theYShift));

  NFmiTotalWind topLeftweather(itsInfoVersion);
  topLeftweather.TransformFromFloatValue(PeekLocationValue(0 - theXShift, 1 - theYShift));

  NFmiTotalWind topRightweather(itsInfoVersion);
  topRightweather.TransformFromFloatValue(PeekLocationValue(1 - theXShift, 1 - theYShift));

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));
  return itsCombinedParamParser->TransformedFloatValue();
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theGridPoint Undocumented
 * \param theXShift Undocumented
 * \param theYShift Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValueForTotalWind(const NFmiMetTime &theTime,
                                                   int theMaxMinuteRange,
                                                   const NFmiPoint &theGridPoint,
                                                   int theXShift,
                                                   int theYShift)
{
  unsigned int oldLocationIndex = LocationIndex();
  // t�m� liikkuminen on melkoista virityst�
  MoveLeft(theXShift);
  MoveDown(theYShift);
  NFmiTotalWind bottomLeftweather(itsInfoVersion);
  bottomLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveRight(1);
  NFmiTotalWind bottomRightweather(itsInfoVersion);
  bottomRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveUp(1);
  NFmiTotalWind topRightweather(itsInfoVersion);
  topRightweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));
  MoveLeft(1);
  NFmiTotalWind topLeftweather(itsInfoVersion);
  topLeftweather.TransformFromFloatValue(InterpolatedValue(theTime, theMaxMinuteRange));

  LocationIndex(oldLocationIndex);

  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  if ((dx != 0.) && (dx < 0.0001))
  {
    dx = 0;
  }
  if ((dy != 0.) && (dy < 0.0001))
  {
    dy = 0;
  }

  if (dx > (1 - 0.0001))
  {
    dx = 1;
  }
  if (dy > (1 - 0.0001))
  {
    dy = 1;
  }

  if (itsCombinedParamParser) delete itsCombinedParamParser;
  itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);
  itsCombinedParamParser->SetToWeightedMean(&bottomLeftweather,
                                            float((1. - dx) * (1. - dy)),
                                            &bottomRightweather,
                                            float(dx * (1. - dy)),
                                            &topLeftweather,
                                            float((1. - dx) * dy),
                                            &topRightweather,
                                            float(dx * dy));
  return itsCombinedParamParser->TransformedFloatValue();
}

#if 0
// ----------------------------------------------------------------------
/*!
 * \param x Undocumented
 * \param y Undocumented
 * \param theValue Undocumented
 * \param topLeftValue Undocumented
 * \param topRightValue Undocumented
 * \param bottomLeftValue Undocumented
 * \param bottomRightValue Undocumented
 * \return Undocumented
 * \todo Refactor this into an external file
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::BiLinearInterpolation(double x,
										  double y,
										  float & theValue,
										  float topLeftValue,
										  float topRightValue,
										  float bottomLeftValue,
										  float bottomRightValue)
{
  double dx = x - int(x);
  double dy = y - int(y);

  if( (dx != 0.) && (dx < 0.0001)) { dx = 0;}
  if( (dy != 0.) && (dy < 0.0001)) { dy = 0;}

  if(dx > (1-0.0001)) { dx = 1; }
  if(dy > (1-0.0001)) { dy = 1; }

  if((topLeftValue != kFloatMissing) && (topRightValue != kFloatMissing)	&&
	 (bottomLeftValue != kFloatMissing) && (bottomRightValue != kFloatMissing))
	{
	  float interpolatedTopValue = float(dx*topRightValue + (1.f - dx)*topLeftValue);
	  float interpolatedBottomValue = float(dx*bottomRightValue + (1.f - dx)*bottomLeftValue);

	  theValue = float(dy*interpolatedTopValue + (1.f - dy)*interpolatedBottomValue);
	  return true;
	}

  if(	(dy == 0.)	&& (bottomLeftValue != kFloatMissing)
		&& (bottomRightValue != kFloatMissing))
	{
	  theValue =  float(dx*bottomRightValue + (1.f - dx)*bottomLeftValue);
	  return true;
	}
  if(	(dx == 0.)	&& (bottomLeftValue != kFloatMissing)
		&& (topLeftValue != kFloatMissing))
	{
	  theValue =  float(dy*topLeftValue + (1.f - dy)*bottomLeftValue);
	  return true;
	}
  if(	(dy == 1.)	&& (topLeftValue != kFloatMissing)
		&& (topRightValue != kFloatMissing))
	{
	  theValue =  float(dx*topRightValue + (1.f - dx)*topLeftValue);
	  return true;
	}
  if(	(dx == 1.)	&& (bottomRightValue != kFloatMissing)
		&& (topRightValue != kFloatMissing))
	{
	  theValue =  float(dy*topRightValue + (1.f - dy)*bottomRightValue);
	  return true;
	}
  if(	(dx == 0.f && dy == 0.f)	&& (bottomLeftValue != kFloatMissing))
	{
	  theValue =  bottomLeftValue;
	  return true;
	}
  if(	(dx == 0.f && dy == 1.f)	&& (topLeftValue != kFloatMissing))
	{
	  theValue =  topLeftValue;
	  return true;
	}
  if(	(dx == 1.f && dy == 0.f)	&& (bottomRightValue != kFloatMissing))
	{
	  theValue =  bottomRightValue;
	  return true;
	}
  if(	(dx == 1.f && dy == 1.f)	&& (topRightValue != kFloatMissing))
	{
	  theValue =  topRightValue;
	  return true;
	}

  theValue = kFloatMissing;
  return false;
}
#endif

// ----------------------------------------------------------------------
/*!
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// l�hinn� gridi-datan tutkimiseen, offsettien avulla voidaan pyyt�� currentista
// paikasta poikkevia paikkoja ei toimi kaikissa tilanteissa oikein, parempi versio
//  fastqueryinfossa

float NFmiQueryInfo::PeekLocationValue(int theXOffset, int theYOffset) const
{
  unsigned long paramIndex = itsParamDescriptor->Index();
  unsigned long timeIndex = itsTimeDescriptor->Index();
  unsigned long theVPlaceIndex = itsVPlaceDescriptor->Index();
  unsigned long theHPlaceIndex = itsHPlaceDescriptor->Index();

  if (IsGrid())
    theHPlaceIndex = theHPlaceIndex + theYOffset * Grid()->XNumber() + theXOffset;
  else
    theHPlaceIndex = theHPlaceIndex + theXOffset;

  unsigned long idx =
      ((paramIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
           (itsTimeDescriptor->Size()) +
       (theHPlaceIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
       theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
  return PeekValue(idx);
}

// HUOM!!!! En toteuta t�t� ainakaan viel� qinfoon, vaan vain fastinfoon.
float NFmiQueryInfo::PeekLocationValue(int /* theXOffset */,
                                       int /* theYOffset*/,
                                       const NFmiMetTime & /* theTime */)
{
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::NextParam(bool fIgnoreSubParam)
{
  bool status = itsParamDescriptor->Next(fIgnoreSubParam);
  bool useSubParam = itsParamDescriptor->IsSubParamUsed();
  if (useSubParam)
  {
    NFmiDataIdent &param =
        itsParamDescriptor->Param(true);  // huom! t�ss� pit�� pyyt�� yliparametria (->true)
    ChangeCombinedParamParser(param);
  }
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param fIgnoreSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::PreviousParam(bool fIgnoreSubParam)
{
  bool status = itsParamDescriptor->Previous(fIgnoreSubParam);
  bool useSubParam = itsParamDescriptor->IsSubParamUsed();
  if (useSubParam)
  {
    NFmiDataIdent &param = itsParamDescriptor->Param();
    ChangeCombinedParamParser(param);
  }
  return status;
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiDataIdent &theDataIdent)
{
  if (itsParamDescriptor->Param(theDataIdent))
    return true;
  else if (FindSubParam(theDataIdent))  // katsotaan l�ytyyk� parametri jonkin yhdistelm� parametrin
                                        // aliparametrina
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiParam &theParam)
{
  if (itsParamDescriptor->Param(theParam))
    return true;
  else if (FindSubParam(theParam))  // katsotaan l�ytyyk� parametri jonkin yhdistelm� parametrin
                                    // aliparametrina
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \param theSubParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Param(const NFmiParam &theParam, const NFmiParam &theSubParam)
{
  // T�t� metodia tarvitaan silloin, kun on mahdollista, ett� jokin parametri l�ytyy
  // paitsi varsinaisena parametrina my�s jonkin toisen parametrin aliparametrina tai
  // kenties jokin parametri l�ytyy kahden eri parametrin sis�lt� aliparametrin�.
  //
  // Esim.
  // Tunturis��palvelussa tuotetaan NFmiQueryData-olio, jossa on sek� kFmiWeatherAndCloudiness
  // ett� kFmiTopWeatherAndCloudiness	paramerit. N�iden avulla luotujen NFmiDataIdent-olioiden
  // sis��n on luettu useita saman nimisi� parametreja. Jos k�ytt�j� haluaa esim. lukea tunturin
  // huipun kFmiFogIntensity:n, asetetaan parametri t�ll� metodilla
  //
  //		NFmiQueryInfo::Param(kFmiTopWeatherAndCloudiness, kFmiFogIntensity)
  //
  // Mik�li kFmiFogIntensity yritet��n asettaa suoraan kutsumalla Param(kFmiFogIntensity),
  // asettuu itsParamDescriptor osoittamaan esimm�isen� (aliparametrina )l�ydetty� itemi�,
  // jonka nimi on kFmiFogIntensity.

  if (!itsParamDescriptor->Param(theParam))
    return false;

  else if (FindSubParam(theSubParam))  // katsotaan l�ytyyk� parametri jonkin yhdistelm� parametrin
                                       // aliparametrina
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// katsoo, l�ytyyk� mink��n parametrin aliparametreist� annettua parametri�. Jos l�ytyy
// palauttaa true ja asettaa itsSubParam:in arvoksi kyseisen parametrin

bool NFmiQueryInfo::FindSubParam(const NFmiParam &theParam)
{
  if (itsParamDescriptor->FindSubParam(theParam))
  {
    NFmiDataIdent &param = itsParamDescriptor->Param();
    ChangeCombinedParamParser(param);
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theDataIdent Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// katsoo, l�ytyyk� mink��n parametrin aliparametreist� annettua parametri�. Jos l�ytyy
// palauttaa true ja asettaa itsSubParam:in arvoksi kyseisen parametrin

bool NFmiQueryInfo::FindSubParam(const NFmiDataIdent &theDataIdent)
{
  if (itsParamDescriptor->FindSubParam(theDataIdent))
  {
    NFmiDataIdent &param = itsParamDescriptor->Param();
    ChangeCombinedParamParser(param);
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// luo annetun parametrin (yhdistelm� parametrin!) mukaisen parserin

bool NFmiQueryInfo::ChangeCombinedParamParser(const NFmiDataIdent &theParam)
{
  if (itsCombinedParamParser)
  {
    delete itsCombinedParamParser;
    itsCombinedParamParser = 0;
  }
  FmiParameterName paramName = FmiParameterName(theParam.GetParam()->GetIdent());
  switch (paramName)
  {
    case kFmiTotalWindMS:
      itsCombinedParamParser = new NFmiTotalWind(itsInfoVersion);  // Marko testi
      return true;
    case kFmiWeatherAndCloudiness:
      itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);  // Marko testi
      return true;
    default:
      break;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiProducer *NFmiQueryInfo::Producer(void)
{
  return (itsParamDescriptor ? itsParamDescriptor->ParamBag()->Current()->GetProducer() : 0);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataIdent &NFmiQueryInfo::Param(void) const { return itsParamDescriptor->Param(false); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::SubParamFloatValue(void) const
{
  if (itsCombinedParamParser)
  {
    float fValue = IndexFloatValue(Index());
    itsCombinedParamParser->TransformFromFloatValue(fValue);
    return float(itsCombinedParamParser->SubValue(
        FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent())));
  }
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theFloatData Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SubParamFloatValue(float theFloatData)
{
  if (itsCombinedParamParser)
  {
    size_t index = Index();
    float fValue = IndexFloatValue(index);
    itsCombinedParamParser->TransformFromFloatValue(fValue);
    itsCombinedParamParser->SubValue(
        theFloatData, FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent()));
    return IndexFloatValue(index, itsCombinedParamParser->TransformedFloatValue());
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::SubParamFloatValue(size_t theIndex) const
{
  if (itsCombinedParamParser)
  {
    float fValue = IndexFloatValue(theIndex);
    itsCombinedParamParser->TransformFromFloatValue(fValue);
    return float(itsCombinedParamParser->SubValue(
        FmiParameterName(itsParamDescriptor->Param(false).GetParam()->GetIdent())));
  }
  return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param period Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodFloatValue(unsigned long period,
                                          float startWeight,
                                          float centreWeight,
                                          float endWeight)
{
  if (UseSubParam())
  {
    if (itsCombinedParamParser) delete itsCombinedParamParser;
    // case: onko tuuli
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);

    itsCombinedParamParser->SetToWeightedPeriod(
        this, NFmiPoint(), period, true, startWeight, centreWeight, endWeight);
    return static_cast<float>(
        itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
  }
  else
    return TimePeriodValue(NFmiPoint(), period, startWeight, centreWeight, endWeight);
}

// ----------------------------------------------------------------------
/*!
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodFloatValue(const NFmiMetTime &theStartTime,
                                          const NFmiMetTime &theEndTime,
                                          float startWeight,
                                          float centreWeight,
                                          float endWeight)
{
  if (UseSubParam())
  {
    if (itsCombinedParamParser) delete itsCombinedParamParser;
    // case: onko tuuli
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedPeriod(
        this, NFmiPoint(), theStartTime, theEndTime, true, startWeight, centreWeight, endWeight);
    return static_cast<float>(
        itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
  }
  else
    return TimePeriodValue(
        NFmiPoint(), theStartTime, theEndTime, startWeight, centreWeight, endWeight);
}

// ----------------------------------------------------------------------
/*!
 * \param latlon Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param startWeight Undocumented
 * \param centreWeight Undocumented
 * \param endWeight Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedTimePeriodFloatValue(const NFmiPoint &latlon,
                                                      const NFmiMetTime &theStartTime,
                                                      const NFmiMetTime &theEndTime,
                                                      float startWeight,
                                                      float centreWeight,
                                                      float endWeight)
{
  if (UseSubParam())
  {
    if (itsCombinedParamParser) delete itsCombinedParamParser;
    itsCombinedParamParser = new NFmiWeatherAndCloudiness(itsInfoVersion);
    itsCombinedParamParser->SetToWeightedPeriod(this,
                                                latlon,
                                                theStartTime,
                                                theEndTime,
                                                true,
                                                startWeight,
                                                centreWeight,
                                                endWeight);  // 10.2.00
    return static_cast<float>(
        itsCombinedParamParser->SubValue(FmiParameterName(Param().GetParam()->GetIdent())));
  }
  else
    return TimePeriodValue(latlon, theStartTime, theEndTime, startWeight, centreWeight, endWeight);
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLat Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::TimePeriodValue(const NFmiPoint &theLonLat,
                                     const NFmiMetTime &theStartTime,
                                     const NFmiMetTime &theEndTime,
                                     float factor1,
                                     float factor2,
                                     float factor3)
{
  bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

  NFmiQueryInfo *newInfo = this->Clone();
  NFmiMetTime middleTime(theStartTime);
  double halfPeriod = theEndTime.DifferenceInHours(theStartTime) / 2.;
  middleTime.ChangeByHours(long(halfPeriod));

  double sum = 0.;
  double factorSum = 0;
  double factor;
  float value;

  if (!newInfo->TimeToNearestStep(theStartTime)) return kFloatMissing;

  while (newInfo->Time() <= theEndTime)
  {
    if (locationMissing)  // ei haluta interpoloida
      value = newInfo->FloatValue();
    else
      value = newInfo->InterpolatedValue(theLonLat);

    if (value != kFloatMissing)
    {
      if (newInfo->Time() < Time())
      {
        factor = (static_cast<float>(newInfo->Time().DifferenceInHours(theStartTime))) /
                     halfPeriod * (factor2 - factor1) +
                 factor1;
      }
      else
      {
        factor = (static_cast<float>(newInfo->Time().DifferenceInHours(middleTime))) / halfPeriod *
                     (factor3 - factor2) +
                 factor2;
      }

      if (value != kFloatMissing)
      {
        sum += value * factor;
        factorSum += factor;
      }
    }
    if (!newInfo->NextTime()) break;
  }

  delete newInfo;
  if (factorSum > 0.)
    return static_cast<float>(sum / factorSum);
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLat Undocumented
 * \param period Undocumented
 * \param factor1 Undocumented
 * \param factor2 Undocumented
 * \param factor3 Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// 10.2.2000/Lasse: JOLLEI KUKAAN K�YT� T�M�N VERSION VOI POISTAA

float NFmiQueryInfo::TimePeriodValue(
    const NFmiPoint &theLonLat, unsigned long period, float factor1, float factor2, float factor3)
{
  bool locationMissing = theLonLat.X() == 0. && theLonLat.Y() == 0.;

  NFmiQueryInfo newInfo(*this);
  NFmiMetTime startTime(Time());
  NFmiMetTime middleTime(startTime);
  NFmiMetTime endTime(startTime);
  long halfPeriod = period / 2;
  startTime.ChangeByHours(-halfPeriod);
  endTime.ChangeByHours(halfPeriod);

  double sum = 0.;
  double factorSum = 0;
  float factor;
  float value;

  if (!newInfo.TimeToNearestStep(startTime)) return kFloatMissing;

  while (newInfo.Time() <= endTime)
  {
    if (locationMissing)  // ei haluta interpoloida
      value = newInfo.FloatValue();
    else
      value = newInfo.InterpolatedValue(theLonLat);

    if (value != kFloatMissing)
    {
      if (newInfo.Time() < Time())
      {
        factor = (static_cast<float>(newInfo.Time().DifferenceInHours(startTime))) / halfPeriod *
                     (factor2 - factor1) +
                 factor1;
      }
      else
      {
        factor = (static_cast<float>(newInfo.Time().DifferenceInHours(middleTime))) / halfPeriod *
                     (factor3 - factor2) +
                 factor2;
      }

      if (value != kFloatMissing)
      {
        sum += value * factor;
        factorSum += factor;
      }
    }
    newInfo.NextTime();
  }

  if (factorSum > 0.)
    return static_cast<float>(sum / factorSum);
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theArea Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData(NFmiDataModifier *theModifier,
                                     NFmiArea *theArea,
                                     NFmiLocationBag *theLocationBag)
{
  if (theArea)
    CalcLocationData2(theModifier, theArea);
  else if (theLocationBag)
    CalcLocationData2(theModifier, theLocationBag);
  else
    CalcLocationData2(theModifier);
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLevelData(NFmiDataModifier *theModifier)
{
  for (ResetLevel(); NextLevel();)
    theModifier->Calculate(FloatValue());
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag)
{
  if (theTimeBag)
  {
    if (static_cast<unsigned long>(theTimeBag->Resolution()) == TimeResolution())
    {
      theTimeBag->First();
      do
      {
        if (Time(theTimeBag->CurrentTime())) break;
      } while (theTimeBag->Next());
      do
      {
        theModifier->Calculate(FloatValue());
      } while (theTimeBag->Next() && NextTime());
    }
    else
    {
      for (theTimeBag->Reset(); theTimeBag->Next();)
      {
        if (Time(theTimeBag->CurrentTime())) theModifier->Calculate(FloatValue());
      }
    }
  }
  else
  {
    for (ResetTime(); NextTime();)
      theModifier->Calculate(FloatValue());
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeDesc Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier, NFmiTimeDescriptor &theTimeDesc)
{
  if (theTimeDesc.ValidTimeBag())
    CalcTimeData(theModifier,
                 theTimeDesc.ValidTimeBag());  // jos on timebagi, k�ytt�� valmista funktiota
  else
  {
    // k�yd��n timeDesc:iss� oleva timelist l�pi
    for (theTimeDesc.Reset(); theTimeDesc.Next();)
      if (Time(theTimeDesc.Time())) theModifier->Calculate(FloatValue());
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeData(NFmiDataModifier *theModifier,
                                 const NFmiMetTime &theStartTime,
                                 const NFmiMetTime &theEndTime)
{
  // 2.5.02/Lasse: TOIMII SEURAAVASTI: ks CalcInterpolatedTimeData alla

  NFmiMetTime saveTime(Time());
  if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
  {
    theModifier->Calculate(FloatValue());
    while (NextTime() && Time() <= theEndTime)
      theModifier->Calculate(FloatValue());

    if (SizeTimes() > 1)
    {  // resoluutio voi olla eri alussa ja lopussa
      itsTimeDescriptor->FirstTime();
      long resStart = TimeResolution();
      itsTimeDescriptor->LastTime();
      long resEnd = TimeResolution();
      if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
          theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
      {
        theModifier->Calculate(kFloatMissing);
      }
    }
  }
  Time(saveTime);
}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcTimeDataWithExtremeTime(NFmiDataModifierExtreme *theModifier,
                                                const NFmiMetTime &theStartTime,
                                                const NFmiMetTime &theEndTime)
{
  // 2.5.02/Lasse: TOIMII SEURAAVASTI: ks CalcInterpolatedTimeData alla

  NFmiMetTime saveTime(Time());
  if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
  {
    theModifier->Calculate(FloatValue(), this);
    while (NextTime() && Time() <= theEndTime)
      theModifier->Calculate(FloatValue(), this);

    if (SizeTimes() > 1)
    {  // resoluutio voi olla eri alussa ja lopussa
      itsTimeDescriptor->FirstTime();
      long resStart = TimeResolution();
      itsTimeDescriptor->LastTime();
      long resEnd = TimeResolution();
      if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
          theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
      {
        theModifier->Calculate(kFloatMissing);
      }
    }
  }
  Time(saveTime);
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeDesc Undocumented
 * \param lonLat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                             NFmiTimeDescriptor &theTimeDesc,
                                             const NFmiPoint &lonLat)
{
  // HUOM oma aika s�ilyy
  NFmiMetTime saveTime(Time());
  for (theTimeDesc.Reset(); theTimeDesc.Next();)
  {
    if (Time(theTimeDesc.Time())) theModifier->Calculate(InterpolatedValue(lonLat));
  }
  Time(saveTime);
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theStartTime Undocumented
 * \param theEndTime Undocumented
 * \param lonLat Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcInterpolatedTimeData(NFmiDataModifier *theModifier,
                                             const NFmiMetTime &theStartTime,
                                             const NFmiMetTime &theEndTime,
                                             const NFmiPoint &lonLat)

// TOIMII SEURAAVASTI:
// .........D...D...D...D...D...D.......... datan aika-askeleet (res. t�ss� = 4h)
//               >         <                par:na tulevat alku/loppuaika;tulos= EI puuttuvia
//       >                       <          EI puuttuvia koska res. mukaan ei tarvita viel� t�ss�
//       alkupisteess�
//      >                        <          ON puuttuvia koska res. mukaan pit�isi olla dataa alussa
//                       >               <  ON puuttuvia
//           > <                            lopputulos puuttuva
//    >   <                                 lopputulos puuttuva
//                               >      <   lopputulos puuttuva
// jos yksikin data l�ytynyt ja ON puuttuvia on lopputulos kiinni modifierista eli sallitaanko niit�
// jos datassa vain yksi aika on tulos hyv�ksytty jos se vain sis�ltyy jaksoon (resoluutio tavallaan
// ��ret�n)

{
  NFmiMetTime saveTime(Time());

  if ((TimeToNearestStep(theStartTime, kForward)) && Time() <= theEndTime)
  {
    theModifier->Calculate(InterpolatedValue(lonLat));
    while (NextTime() && Time() <= theEndTime)
      theModifier->Calculate(InterpolatedValue(lonLat));

    if (SizeTimes() > 1)
    {  // resoluutio voi olla eri alussa ja lopussa
      itsTimeDescriptor->FirstTime();
      long resStart = TimeResolution();
      itsTimeDescriptor->LastTime();
      long resEnd = TimeResolution();
      if (TimeDescriptor().FirstTime().DifferenceInMinutes(theStartTime) >= resStart ||
          theEndTime.DifferenceInMinutes(TimeDescriptor().LastTime()) >= resEnd)
      {
        theModifier->Calculate(kFloatMissing);
      }
    }
  }

  Time(saveTime);
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyTimesLocationData(NFmiDataModifier *theModifier,
                                            NFmiQueryInfo *theQueryInfoCopy,
                                            NFmiTimeBag *theTimeBag)
{
  if (!theTimeBag)
  {
    if (theQueryInfoCopy) theQueryInfoCopy->ResetTime();
    for (ResetTime(); NextTime();)
    {
      if (theQueryInfoCopy) theQueryInfoCopy->NextTime();
      ModifyLocationData(theModifier, theQueryInfoCopy);
    }
  }
  else
  {
    for (theTimeBag->Reset(); theTimeBag->Next();)
    {
      if (!Time(theTimeBag->CurrentTime())) continue;
      if (theQueryInfoCopy)
        if (!theQueryInfoCopy->Time(theTimeBag->CurrentTime())) continue;
      ModifyLocationData(theModifier, theQueryInfoCopy);
    }
  }
}

void NFmiQueryInfo::ModifyTimesLocationData(NFmiDataModifier *theModifier,
                                            NFmiQueryInfo *theQueryInfoCopy,
                                            NFmiTimeDescriptor &theTimeDescriptor)
{
  for (theTimeDescriptor.Reset(); theTimeDescriptor.Next();)
  {
    if (!Time(theTimeDescriptor.Time())) continue;
    if (theQueryInfoCopy)
      if (!theQueryInfoCopy->Time(theTimeDescriptor.Time())) continue;
    ModifyLocationData(theModifier, theQueryInfoCopy);
  }
}

class TimeToModifyCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen t�h�n, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  TimeToModifyCalculator(const NFmiTimeDescriptor &theTimeDescriptor)
      : itsTimeDescriptor(theTimeDescriptor), fNoMoreWork(false)
  {
    itsTimeDescriptor.First();
  }

  bool GetCurrentTime(NFmiMetTime &theTime)
  {
    WriteLock lock(itsMutex);
    if (fNoMoreWork) return false;
    theTime = itsTimeDescriptor.Time();
    if (itsTimeDescriptor.Next() == false) fNoMoreWork = true;
    return true;
  }

 private:
  NFmiTimeDescriptor itsTimeDescriptor;
  MutexType itsMutex;
  bool fNoMoreWork;

  TimeToModifyCalculator(const TimeToModifyCalculator &);  // ei toteuteta kopio konstruktoria
};

static void ModifySingleTimeGridInThread(NFmiFastQueryInfo &theModifiedInfo,
                                         TimeToModifyCalculator &theTimeToModifyCalculator,
                                         NFmiDataModifier *theModifier)
{
  NFmiMetTime aTime;
  for (; theTimeToModifyCalculator.GetCurrentTime(aTime);)
  {
    if (theModifiedInfo.Time(aTime))
    {
      theModifier->SetTimeIndex(theModifiedInfo.TimeIndex());
      for (theModifiedInfo.ResetLocation(); theModifiedInfo.NextLocation();)
      {
        theModifier->SetLocationIndex(theModifiedInfo.LocationIndex());
        theModifiedInfo.FloatValue(theModifier->FloatOperation(theModifiedInfo.FloatValue()));
      }
    }
  }
}

void NFmiQueryInfo::ModifyTimesLocationData_FullMT(NFmiDataModifier *theModifier,
                                                   NFmiTimeDescriptor &theTimeDescriptor)
{
  unsigned int usedThreadCount = boost::thread::hardware_concurrency();
  unsigned long timeCount = theTimeDescriptor.Size();
  if (usedThreadCount > timeCount) usedThreadCount = timeCount;

  LatLon();  // multi-thread koodin varmistus, ett� latlon-cachet on alustettu
  theModifier->InitLatlonCache();
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > modifiedInfoVector(usedThreadCount);
  std::vector<boost::shared_ptr<NFmiDataModifier> > dataModifierVector(usedThreadCount);
  NFmiFastQueryInfo *thisFastInfo = dynamic_cast<NFmiFastQueryInfo *>(this);
  if (thisFastInfo == 0) return;  // ei voi edet�, koska this info ei ollutkaan oikeasti fastInfo
  for (unsigned int i = 0; i < usedThreadCount; i++)
  {
    // HUOM! pakko tehd� fastQueryInfo copioita, ett� ei menetet� nopeutta, eik� saa tehd� Clone:ja,
    // koska t�ll�in kopioituu my�s data-osio!!!
    modifiedInfoVector[i] =
        boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*thisFastInfo));
    dataModifierVector[i] = boost::shared_ptr<NFmiDataModifier>(theModifier->Clone());
  }

  TimeToModifyCalculator timeIndexCalculator(theTimeDescriptor);
  boost::thread_group calcParts;
  for (unsigned int i = 0; i < usedThreadCount; i++)
    calcParts.add_thread(new boost::thread(::ModifySingleTimeGridInThread,
                                           boost::ref(*modifiedInfoVector[i]),
                                           boost::ref(timeIndexCalculator),
                                           dataModifierVector[i].get()));
  calcParts.join_all();  // odotetaan ett� threadit lopettavat
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theArea Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData(NFmiDataModifier *theModifier,
                                       NFmiQueryInfo *theQueryInfoCopy,
                                       NFmiArea *theArea,
                                       NFmiLocationBag *theLocationBag)
{
  if (theQueryInfoCopy) theQueryInfoCopy->ResetLocation();

  if (theArea)
    ModifyLocationData2(theModifier, theQueryInfoCopy, theArea);
  else if (theLocationBag)
    ModifyLocationData2(theModifier, theQueryInfoCopy, theLocationBag);
  else
    ModifyLocationData2(theModifier, theQueryInfoCopy);
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyTimeData(NFmiDataModifier *theModifier, NFmiTimeBag *theTimeBag)
{
  if (theTimeBag)
  {
    for (theTimeBag->Reset(); theTimeBag->Next();)
      if (Time(theTimeBag->CurrentTime())) FloatValue(theModifier->FloatOperation(FloatValue()));
  }
  else
  {
    for (ResetTime(); NextTime();)
      FloatValue(theModifier->FloatOperation(FloatValue()));
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLevelData(NFmiDataModifier *theModifier)
{
  for (ResetLevel(); NextLevel();)
    FloatValue(theModifier->FloatOperation(FloatValue()));
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::PeekTimeValue(int theTimeOffset)
{
  float returnValue = kFloatMissing;
  NFmiMetTime copyTime = ValidTime(), copyTime2 = ValidTime();

  copyTime2.ChangeByMinutes(theTimeOffset * TimeResolution());
  if (Time(copyTime2)) returnValue = FloatValue();
  Time(copyTime);

  return returnValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theQueryInfo Undocumented
 * \param fIgnoreLevel Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel)
{
  bool returnValue = true;
  if (theQueryInfo->IsParamUsable())  // T�llainen testaus, siksi, ett� ei ohjelma
  {  // ei kaadu, jos ollaan resetiss�.
    if (!Param(NFmiParam(theQueryInfo->Param().GetParam()->GetIdent())))
    {
      returnValue = false;
    }
  }
  else
    returnValue = false;

  // Mika 08.11.2001 : Paluuarvoa ei kayteta - onko sivuvaikutus??
  // bool boo = theQueryInfo->IsLevel();
  theQueryInfo->IsLevel();
  if (!fIgnoreLevel && theQueryInfo->Level())
  {
    if (!Level(*(theQueryInfo->Level()))) returnValue = false;
  }
  else
    returnValue = false;

  // Mika 08.11.2001 - paluuarvoa ei kayteta, onko sivuvaikutus?
  // const NFmiLocation *loac = theQueryInfo->Location();
  // bool poo = IsGrid();
  theQueryInfo->Location();
  IsGrid();

  if (theQueryInfo->IsLocationUsable())  // T�llainen testaus, siksi, ett� ei ohjelma
  {
    if (IsGrid())
    {
      if (!Location(theQueryInfo->LatLon())) returnValue = false;
    }
    else if (theQueryInfo->Location())
    {
      if (!Location(*(theQueryInfo->Location()))) returnValue = false;
    }
    else
      returnValue = false;
  }
  else
    returnValue = false;

  if (theQueryInfo->IsTimeUsable())  // T�llainen testaus, siksi, ett� ei ohjelma
    if (!Time(theQueryInfo->Time())) returnValue = false;

  return returnValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier)
{
  for (ResetLocation(); NextLocation();)
    //	if(IsLocationMasked(LocationIndex()))
    theModifier->Calculate(FloatValue());
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationDataWithExtremePlace(NFmiDataModifierExtremePlace *theModifier)
{
  for (ResetLocation(); NextLocation();)
    //	if(IsLocationMasked(LocationIndex()))
    theModifier->Calculate(FloatValue(), this);
}
// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier, NFmiArea *theArea)
{
  if (theArea)
  {
    for (ResetLocation(); NextLocation();)
    {
      //		  if (theArea->IsInside(LatLon()) && IsLocationMasked(LocationIndex()))
      if (theArea->IsInside(LatLon())) theModifier->Calculate(FloatValue());
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::CalcLocationData2(NFmiDataModifier *theModifier,
                                      NFmiLocationBag *theLocationBag)
{
  if (theLocationBag)
  {
    for (theLocationBag->Reset(); theLocationBag->Next();)
    {
      if (Location(*theLocationBag->Location())) theModifier->Calculate(FloatValue());
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy)
{
  if (theQueryInfoCopy) theQueryInfoCopy->ResetLocation();

  for (ResetLocation(); NextLocation();)
  {
    if (theQueryInfoCopy) theQueryInfoCopy->LocationIndex(LocationIndex());
    FloatValue(theModifier->FloatOperation(FloatValue()));
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy,
                                        NFmiArea *theArea)
{
  if (theQueryInfoCopy) theQueryInfoCopy->ResetLocation();

  if (theArea)
  {
    for (ResetLocation(); NextLocation();)
    {
      if (theArea->IsInside(LatLon()))
      {
        FloatValue(theModifier->FloatOperation(FloatValue()));
      }
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theModifier Undocumented
 * \param theQueryInfoCopy Undocumented
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiQueryInfo::ModifyLocationData2(NFmiDataModifier *theModifier,
                                        NFmiQueryInfo *theQueryInfoCopy,
                                        NFmiLocationBag *theLocationBag)
{
  if (theQueryInfoCopy) theQueryInfoCopy->ResetLocation();

  if (theLocationBag)
  {
    for (theLocationBag->Reset(); theLocationBag->Next();)
    {
      if (Location(*theLocationBag->Location()))
      {
        if (theQueryInfoCopy) theQueryInfoCopy->Location(*theLocationBag->Location());
        FloatValue(theModifier->FloatOperation(FloatValue()));
      }
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theTime Undocumented
 * \param theMaxMinuteRange Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiQueryInfo::InterpolatedValue(const NFmiMetTime &theTime, int theMaxMinuteRange)
{
  float tmpValue = kFloatMissing;
  unsigned long oldTimeIndex = TimeIndex();
  // Katsotaan ensin l�ytyyk� suoraan halutulle ajalle arvoa
  if (Time(theTime)) tmpValue = FloatValue();
  if (tmpValue == kFloatMissing)
  {  // jos ei l�ytynyt suoraan arvoa, aletaan interpolointi...
    if (itsTimeDescriptor->ValidTimeBag())
      tmpValue = InterpolatedValueFromTimeBag(theTime, theMaxMinuteRange);
    else if (itsTimeDescriptor->ValidTimeList())
      tmpValue = InterpolatedValueFromTimeList(theTime, theMaxMinuteRange);
  }
  TimeIndex(oldTimeIndex);  // asetetaan lopuksi indeksi takaisin siihen, miss� se oli
  return tmpValue;
}

float NFmiQueryInfo::InterpolatedValueFromTimeBag(const NFmiMetTime &theTime, int theMaxMinuteRange)
{
  NFmiTimeBag *timeBag = itsTimeDescriptor->ValidTimeBag();
  if (timeBag && timeBag->IsInside(theTime))
  {
    int sizeTimes = SizeTimes();
    int timeIndex = TimeIndex();
    if (timeIndex == -1 || sizeTimes < timeIndex)
    {
      // eli jos aika osoittaa kuuseen, laitetaan aika osoittamaan
      // 1. aikaan ja otetaan indeksi uudestaan
      FirstTime();
      timeIndex = TimeIndex();
    }
    float returnValue = kFloatMissing;
    NFmiMetTime firstTime(timeBag->FirstTime());
    int timeResolution = timeBag->Resolution();
    int minuteOffSetOfWantedTimeInTimeBag =
        theTime.DifferenceInMinutes(firstTime) - (timeResolution * timeIndex);
    float wantedTimeOffsetFloat = float(minuteOffSetOfWantedTimeInTimeBag) / timeResolution;
    int wantedTimeOffset = minuteOffSetOfWantedTimeInTimeBag / timeResolution;
    int backwardOffset = static_cast<int>(floor(wantedTimeOffsetFloat));
    int forwardOffset = static_cast<int>(ceil(wantedTimeOffsetFloat));
    if (backwardOffset == forwardOffset) forwardOffset++;
    if (minuteOffSetOfWantedTimeInTimeBag % timeResolution ==
        0)  // jos haluttu aika menee tasa aikasteppiin
    {
      backwardOffset = wantedTimeOffset - 1;
      forwardOffset = wantedTimeOffset + 1;
      returnValue = PeekTimeValue(wantedTimeOffset);
    }
    if (returnValue == kFloatMissing || returnValue == kTCombinedWeatherFloatMissing)
    {
      // aletaan etsi� l�himpi� arvoja molemmista suunnista
      int i;
      float value1 = kFloatMissing, value2 = kFloatMissing;
      for (i = backwardOffset; i + timeIndex >= 0; i--)
      {
        backwardOffset = i;
        value1 = PeekTimeValue(i);
        if (!(value1 == kFloatMissing || value1 == kTCombinedWeatherFloatMissing))
        {
          if (theMaxMinuteRange && theMaxMinuteRange < (wantedTimeOffsetFloat - i) * timeResolution)
            value1 = kFloatMissing;
          break;
        }
        else
          value1 = kFloatMissing;  // tarkastus helpottuu my�hemmin, kun vain yksi missing arvo
      }
      for (i = forwardOffset; i + timeIndex < int(sizeTimes); i++)
      {
        forwardOffset = i;
        value2 = PeekTimeValue(i);
        if (!(value2 == kFloatMissing || value2 == kTCombinedWeatherFloatMissing))
        {
          if (theMaxMinuteRange && theMaxMinuteRange < (i - wantedTimeOffsetFloat) * timeResolution)
            value2 = kFloatMissing;
          break;
        }
        else
          value2 = kFloatMissing;  // tarkastus helpottuu my�hemmin, kun vain yksi missing arvo
      }
      if (value1 == kFloatMissing || value2 == kFloatMissing)
        returnValue = kFloatMissing;
      else
      {
        float offset1 = (forwardOffset - wantedTimeOffsetFloat) / (forwardOffset - backwardOffset);

        // 30.10.2000/Marko Muutin ottamaan huomioon combinedparam jutut ja ep�lineaariset
        // parametrit
        NFmiParam *parameter = Param().GetParam();
        FmiInterpolationMethod interp = parameter->InterpolationMethod();
        unsigned long param = parameter->GetIdent();
        if (interp == kByCombinedParam ||
            param == kFmiTotalWindMS)  // HUOM!!!! totalwindill� ei ole t�t� arvoa (korjaa!)
        {
          switch (param)
          {
            case kFmiTotalWindMS:
            {  // korjasin k�ytt�m��n SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien
               // avulla
              NFmiTotalWind resultWind(itsInfoVersion);
              NFmiTotalWind tempWind1(value1, kFmiPackedWind, itsInfoVersion);
              NFmiTotalWind tempWind2(value2, kFmiPackedWind, itsInfoVersion);
              resultWind.SetToWeightedMean(
                  &tempWind1, offset1, &tempWind2, 1 - offset1, &tempWind1, 0, &tempWind1, 0);
              returnValue = resultWind.TransformedFloatValue();
            }
            break;
            case kFmiWeatherAndCloudiness:
            {
              NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
              NFmiWeatherAndCloudiness tempWeather1(
                  value1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
              NFmiWeatherAndCloudiness tempWeather2(
                  value2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

              resultWeather.SetToWeightedMean(&tempWeather1,
                                              offset1,
                                              &tempWeather2,
                                              1 - offset1,
                                              &tempWeather1,
                                              0,
                                              &tempWeather1,
                                              0);
              returnValue = resultWeather.TransformedFloatValue();
            }
            break;
          }
        }
        else if (interp != kLinearly)
        {
          if (offset1 > 0.5)
            returnValue = value1;
          else
            returnValue = value2;
        }
        else  // muuten lineaarinen interpolointi
        {
          if (param == kFmiWindDirection)
          {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
            Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
            float ws1 = PeekTimeValue(backwardOffset);
            float ws2 = PeekTimeValue(forwardOffset);
            Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
            NFmiInterpolation::WindInterpolator windInterpolator;
            windInterpolator.operator()(ws1, value1, offset1);
            windInterpolator.operator()(ws2, value2, (1 - offset1));
            returnValue = static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa
                                                                             // vain tuulen suunta
                                                                             // (t�m� pit�� tehd�
                                                                             // jotenkin fiksummin)
          }
          else if (param == kFmiWaveDirection)
          {
            returnValue =
                static_cast<float>(NFmiInterpolation::ModLinear(offset1, value1, value2, 360));
          }
          else
            returnValue = float(offset1 * value1 + (1.f - offset1) * value2);
        }
      }
    }
    return returnValue;
  }
  return kFloatMissing;
}

float NFmiQueryInfo::FindNearestNonMissingValueFromTimeList(const NFmiMetTime &theTime,
                                                            int theMaxMinuteRange,
                                                            int &theTimeIndexInOut,
                                                            NFmiMetTime &theFoundTime,
                                                            FmiDirection theDirection)
{
  // etsit��n l�hin aika takaa p�in, mist� l�ytyy ei puuttuva arvo
  bool backWard = (theDirection == kBackward);
  float value = kFloatMissing;
  NFmiTimeList *timeList = itsTimeDescriptor->ValidTimeList();
  if (timeList == 0)
    throw runtime_error(
        "Error: Programming error in NFmiQueryInfo::FindNearestNonMissingValueFromTimeList, no "
        "timelist found.");
  do
  {
    value = PeekValue(Index(ParamIndex(), LocationIndex(), LevelIndex(), theTimeIndexInOut));
    if (value != kFloatMissing)
    {
      if (timeList->Time(theTimeIndexInOut) == 0)
        throw runtime_error(
            "Error: Programming error in NFmiQueryInfo::FindNearestNonMissingValueFromTimeList, "
            "time not found.");
      theFoundTime = *timeList->Time(theTimeIndexInOut);
      if (theMaxMinuteRange != 0 &&
          ::abs(theTime.DifferenceInMinutes(theFoundTime)) > theMaxMinuteRange)
        value = kFloatMissing;  // aika josta l�ytyi ei puuttuva arvo, l�ytyi liian kaukaa
      break;
    }
    backWard ? theTimeIndexInOut-- : theTimeIndexInOut++;
  } while (backWard ? theTimeIndexInOut >= 0 : theTimeIndexInOut < timeList->NumberOfItems());
  return value;
}

// Laskee theTime et�isyyden theTime2:een kun se suhteutetaan time1:n ja time2:n erotukseen.
// Eli mit� l�hemp�n� theTime on time2:sta, sit� l�hemp�n� arvo on 0:aa ja jos theTime
// on l�hemp�n� time1:st�, arvo l�henee 1:st�.
// Jos time2:ll� ja time2:lla ei ole eroa, palautetaan 0.
float NFmiQueryInfo::CalcTimeOffsetToLastTime(const NFmiMetTime &theTime,
                                              const NFmiMetTime &time1,
                                              const NFmiMetTime &time2)
{
  float totalDiff = static_cast<float>(time2.DifferenceInMinutes(time1));
  float diff1 = static_cast<float>(theTime.DifferenceInMinutes(time1));
  if (totalDiff)
  {
    // offset1 nimi on ehk� h�m��v�, tarkoittaa ett� jos offset on 1, ollaan time1:n kohdalla ja
    // palautetaan theValue1
    // jos arvo 0, ollaan time2:n kohdalla ja palautetaan theValue2, muuten interpoloidaan niiden
    // v�lille.
    float offset1 = 1 - (diff1 / totalDiff);
    return offset1;
  }
  else
    return 1;  // eli palautetaan offset, jolla ollaan kiinni time1:ss�
}

// Jos X/Y on negatiivinen, mutta sen itseisarvo on pienempi kuin eps, annetaan arvoksi 0.
static double FixGridPointEpsErrorBelowZero(double theGridPointValue)
{
  if (theGridPointValue != kFloatMissing)
  {
    if (theGridPointValue < 0 && ::fabs(theGridPointValue) < gInterpolatedValueEps) return 0;
  }
  return theGridPointValue;
}

// Jos X/Y on suurempi kuin datan reuna piste (=edge), mutta niiden erotus on pienempi kuin eps,
// annetaan arvoksi reunapiste.
static double FixGridPointEpsErrorOverTopEdge(double theGridPointValue, unsigned long theGridSize)
{
  if (theGridPointValue != kFloatMissing)
  {
    double edge = theGridSize - 1.;
    if (theGridPointValue > edge && ::fabs(theGridPointValue - edge) < gInterpolatedValueEps)
      return edge;
  }
  return theGridPointValue;
}

// Jos theGridPoint on pikkuisen (k�ytetyn epsilonin rajoissa) data-alueen
// ulkopuolella (x/y suunnassa), korjataan hilapisteen arvoksi data-alueen reunaksi.
static void FixGridPointEpsError(NFmiPoint &theGridPoint,
                                 unsigned long theGridSizeX,
                                 unsigned long theGridSizeY)
{
  theGridPoint.X(::FixGridPointEpsErrorBelowZero(theGridPoint.X()));
  theGridPoint.Y(::FixGridPointEpsErrorBelowZero(theGridPoint.Y()));

  theGridPoint.X(::FixGridPointEpsErrorOverTopEdge(theGridPoint.X(), theGridSizeX));
  theGridPoint.Y(::FixGridPointEpsErrorOverTopEdge(theGridPoint.Y(), theGridSizeY));
}

// Laskee NFmiLocationCache -olin, miss� on tieto siit� mihin kohtaan annettu latlon-piste osuu
// hilassa.
// Jos thePossibleSourceSizeX + Y ovat arvoltaan muuta kuin gMissingIndex, p��tell��n tarvitaanko
// tehd� bilineaarista interpolaatiota,
// vai riitt��k� nearest piste interpolaatio.
// Osaa k�sitell� my�s ns. vajaan globaalin datan tapauksen, miss� data menee pituuspiiri suunnassa
// esim. 0 => 359.5. T�ll�in
// NFmiLocationCache -olion itsLocationIndex -dataosalle annetaan arvo gStrechedGlobalGridIndex.
// T�m�n j�lkeen GetCachedValues -metodi
// osaa ker�t� oikeat arvot hilan ��rilaidoilta (0-asteen sarake ja 359.5 -sarake) interpolointeja
// varten.
NFmiLocationCache NFmiQueryInfo::CalcLocationCache(const NFmiPoint &theLatlon,
                                                   unsigned long thePossibleSourceSizeX,
                                                   unsigned long thePossibleSourceSizeY)
{
  NFmiLocationCache locCache;
  if (IsGrid())  // laskut j�rkevi� vain jos on hiladatasta kysymys
  {
    if (Location(theLatlon, &locCache.itsGridPoint))
    {
      ::FixGridPointEpsError(locCache.itsGridPoint, itsGridXNumber, itsGridYNumber);
      locCache.itsLocationIndex = LocationIndex();
      if (thePossibleSourceSizeX != gMissingIndex && thePossibleSourceSizeY != gMissingIndex)
        locCache.CalcIsInterpolationNeeded(thePossibleSourceSizeX, thePossibleSourceSizeY);
    }
    else if (DoGlobalWrapFix(locCache.itsGridPoint))
    {
      locCache.itsLocationIndex = gStrechedGlobalGridIndex;
    }
  }
  return locCache;
}

// Halutaan laskea this-infon avulla interpoloituja arvoja theTargetInfo:lle. Sit� varten lasketaan
// kerran
// halutut hilapisteet (targetin hilan pisteet this-hilassa). Sis�lt�� my�s tiedon hilapisteen
// indeksist�.
// T�t� voidaan k�ytt�� apuna kun interpolidaan monta kertaan hilasta toiseen arvoja (esim. eri
// ajoille,
// leveleille ja parametreille), cache lasketaan vain kerran. Cache-pisteet voidaan sitten antaa eri
// InterpolatedValue-metodeille. Ks. mallia NFmiQueryDataUtil::FillGridData-metodista.
// Huom! toimii siis vain hiloille.
bool NFmiQueryInfo::CalcLatlonCachePoints(NFmiQueryInfo &theTargetInfo,
                                          NFmiDataMatrix<NFmiLocationCache> &theLocationCache)
{
  if (Grid() && theTargetInfo.Grid())
  {
    unsigned long sourceSizeX = Grid()->XNumber();
    unsigned long sourceSizeY = Grid()->YNumber();
    unsigned long targetSizeX = theTargetInfo.Grid()->XNumber();
    unsigned long targetSizeY = theTargetInfo.Grid()->YNumber();
    theLocationCache.Resize(targetSizeX, targetSizeY);
    for (theTargetInfo.ResetLocation(); theTargetInfo.NextLocation();)
    {
      NFmiLocationCache locCache =
          CalcLocationCache(theTargetInfo.LatLon(), sourceSizeX, sourceSizeY);
      theLocationCache[theTargetInfo.LocationIndex() % targetSizeX][theTargetInfo.LocationIndex() /
                                                                    targetSizeX] = locCache;
    }
    return true;
  }
  return false;
}

NFmiTimeCache NFmiQueryInfo::CalcTimeCache(const NFmiMetTime &theTime)
{
  NFmiTimeCache timeCache;
  if (IsInside(theTime))
  {
    if (Time(theTime))
      timeCache.itsTimeIndex2 = timeCache.itsTimeIndex1 = TimeIndex();
    else
    {
      FindNearestTime(theTime, kBackward);  // t�m� pit�� l�yty�!
      timeCache.itsTimeIndex1 = TimeIndex();
      timeCache.itsTimeIndex2 =
          timeCache.itsTimeIndex1 +
          1;  // vain per�kk�iset aikaindeksit mukaan, ei v�lit� onko niiss� oikeasti arvoa vai ei
      NFmiMetTime time1 = Time();
      NextTime();
      NFmiMetTime time2 = Time();
      timeCache.itsOffset = 1.f - CalcTimeOffsetToLastTime(theTime, time1, time2);
    }
    timeCache.CalcIsInterpolationNeeded();
  }
  return timeCache;
}

// Lasketaan this-infosta aika-laskuissa k�ytett�v� timeCache vektori (jokaiselle aika-askeleelle)
// arvot.
// Jos targetin aika-askel ei osu ollenkaan this-datan aikaDescriptorin sis��n, laitetaan
// cache-arvoksi
// oletus arvot eli puuttuvan arvot. T�ll�in aikainterpolointi laskuissa palautetaan vain puuttuvaa.
// Muuten katsotaan osuuko target aika suoraan kohdalle. Jos osuu, kertoimellea ai ole v�li�,
// asetetaan
// vain aikaindeksit samoiksi, jolloin tiedet��n ett� aika-interpolointia ei tarvitse tehd�.
// Jos aikainterpolointia tarvitaan, laitetaan aikaIndex1:ksi edelt�v� aika ja 2:een seuraava, milt�
// siis
// l�ytyy arvoja. Lis�sksi lasketaan offset-kerroin, jolloa interplointi tehd��n.
// HUOM! nopeassa aikainterpolointi laskuissa ei voi ottaa huomioon maksimi aikav�li�, koska sit� ei
// voi laskea
// yhden pisteen avulla, joka saatetaan haluta ottaa huomioon, t�ll�in pit�isi k�ytt�� vain
// normaalia aikainterpolaatiota.
bool NFmiQueryInfo::CalcTimeCache(NFmiQueryInfo &theTargetInfo,
                                  checkedVector<NFmiTimeCache> &theTimeCache)
{
  theTimeCache.clear();
  for (theTargetInfo.ResetTime(); theTargetInfo.NextTime();)
  {
    NFmiTimeCache timeCache = CalcTimeCache(theTargetInfo.Time());
    theTimeCache.push_back(timeCache);
  }
  return true;
}

float NFmiQueryInfo::InterpolatedValueFromTimeList(const NFmiMetTime &theTime,
                                                   int theMaxMinuteRange)
{
  float returnValue = kFloatMissing;
  NFmiTimeList *timeList = itsTimeDescriptor->ValidTimeList();
  if (timeList)
  {
    if (theTime < timeList->FirstTime())
      return kFloatMissing;  // jos 1. aika oli jo suurempi kuin  haluttu, ei voida interpoloida.
    // T�m� tilanne pit�� tarkistaa, koska jos theMaxMinuteRange on 0, interpolointi
    // onnistuisi aina t�ll�isess� tapauksessa. T�m� korjaa rei�n koodissa, ylemmille tasoille voisi
    // laittaa
    // varmaan suoraan tarkastuksen, ett� jos haluttu aika ei ole timeDescriptorin sis�ll�, ei
    // interpolointi onnistu.

    NFmiMetTime time1 = NFmiMetTime::gMissingTime;  // this avoids calling SetZoneDifferenceHour
                                                    // which may cause locks
    NFmiMetTime time2 = NFmiMetTime::gMissingTime;

    int index1 = 0, index2 = 0;
    if ((index1 = timeList->FindNearestTimes(
             theTime, theMaxMinuteRange ? theMaxMinuteRange : kLongMissing, time1, time2)) != -1)
    {
      // etsit��n l�hin aika takaa p�in, mist� l�ytyy ei puuttuva arvo
      float value1 = FindNearestNonMissingValueFromTimeList(
          theTime, theMaxMinuteRange, index1, time1, kBackward);
      index2 = index1 + 1;
      float value2 = FindNearestNonMissingValueFromTimeList(
          theTime, theMaxMinuteRange, index2, time2, kForward);
      //			float value2 = PeekValue(Index(ParamIndex(), LocationIndex(),
      //LevelIndex(), index2));
      if (value1 != kFloatMissing && value1 != kTCombinedWeatherFloatMissing &&
          value2 != kFloatMissing && value2 != kTCombinedWeatherFloatMissing)
      {
        if (Param().GetParamIdent() == kFmiWindDirection)
        {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
          Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
          TimeIndex(index1);  // otetaan 1. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
          float ws1 = FloatValue();
          TimeIndex(index2);  // otetaan 2. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
          float ws2 = FloatValue();
          Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
          NFmiInterpolation::WindInterpolator windInterpolator;
          float offset1 = CalcTimeOffsetToLastTime(theTime, time1, time2);
          windInterpolator.operator()(ws1, value1, offset1);
          windInterpolator.operator()(ws2, value2, (1 - offset1));
          returnValue = static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa vain
                                                                           // tuulen suunta (t�m�
                                                                           // pit�� tehd� jotenkin
                                                                           // fiksummin)
        }
        else
          returnValue = Interpolate(Param(), theTime, time1, time2, value1, value2);
      }
      else
        returnValue = kFloatMissing;
    }
  }
  return returnValue;
}

float NFmiQueryInfo::Interpolate(const NFmiDataIdent &theDataIdent,
                                 const NFmiMetTime &theTime,
                                 const NFmiMetTime &theTime1,
                                 const NFmiMetTime &theTime2,
                                 float theValue1,
                                 float theValue2)
{
  if (theValue1 == kFloatMissing && theValue2 == kFloatMissing) return kFloatMissing;

  float returnValue = kFloatMissing;
  float offset1 = CalcTimeOffsetToLastTime(theTime, theTime1, theTime2);
  if (offset1 == 1)
    returnValue = theValue1;
  else if (offset1 == 0)
    returnValue = theValue2;

  // Jos toinen arvoista puuttuva, palautetaan suoraan se arvo, jota l�hemp�n� offset1 on, oli se
  // puuttuva tai ei
  if (theValue1 == kFloatMissing || theValue2 == kFloatMissing)
  {
    if (offset1 <= 0.5)
      return theValue2;
    else
      return theValue1;
  }

  // 30.10.2000/Marko Muutin ottamaan huomioon combinedparam jutut ja ep�lineaariset parametrit
  FmiInterpolationMethod interp = theDataIdent.GetParam()->InterpolationMethod();
  unsigned long param = theDataIdent.GetParam()->GetIdent();
  if (interp == kByCombinedParam ||
      param == kFmiTotalWindMS)  // HUOM!!!! totalwindill� ei ole t�t� arvoa (korjaa!)
  {
    switch (param)
    {
      case kFmiTotalWindMS:
      {  // korjasin k�ytt�m��n SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien avulla
        NFmiTotalWind resultWind(itsInfoVersion);
        NFmiTotalWind tempWind1(theValue1, kFmiPackedWind, itsInfoVersion);
        NFmiTotalWind tempWind2(theValue2, kFmiPackedWind, itsInfoVersion);
        resultWind.SetToWeightedMean(
            &tempWind1, offset1, &tempWind2, 1 - offset1, &tempWind1, 0, &tempWind1, 0);
        returnValue = resultWind.TransformedFloatValue();
      }
      break;
      case kFmiWeatherAndCloudiness:
      {
        NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
        NFmiWeatherAndCloudiness tempWeather1(
            theValue1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
        NFmiWeatherAndCloudiness tempWeather2(
            theValue2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

        resultWeather.SetToWeightedMean(
            &tempWeather1, offset1, &tempWeather2, 1 - offset1, &tempWeather1, 0, &tempWeather1, 0);
        returnValue = resultWeather.TransformedFloatValue();
      }
      break;
    }
  }
  else if (param == kFmiWindVectorMS)  // windvector laskut interpoloituina, vaikka se on 'nearest'
    returnValue =
        static_cast<float>(NFmiInterpolation::WindVector(1 - offset1, theValue1, theValue2));
  else if (interp != kLinearly)
  {
    if (offset1 <= 0.5)
      returnValue = theValue2;
    else
      returnValue = theValue1;
  }
  else  // muuten lineaarinen interpolointi
  {
    if (param == kFmiWindDirection || param == kFmiWaveDirection)
    {  // HUOM!! korjaa koodeja niin ett� kFmiWindDirection -parametrilla ei t�t� kutsuta!!!!
      returnValue =
          static_cast<float>(NFmiInterpolation::ModLinear(1 - offset1, theValue1, theValue2));
    }
    else
      returnValue = float(offset1 * theValue1 + (1.f - offset1) * theValue2);
  }
  return returnValue;
}

// ----------------------------------------------------------------------
/*!
 * \param theOtherInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// 1999.10.18/Marko Yhdist�� this-infon ja parametrina annetun infon datan.
// Luo niist� uuden yhdistetyn querydatan. Tein metodin NFmiQueryInfo:oon,
// enk� NFmiQueryData:an, koska n�in k�yt�n saa optimoitua (k�ytt�m�ll�
// NFmiFastQueryInfo:ja).
// Huom!!! T�t� ei ole optimoitu viel� yht��n. Esim. tarkastelemalla
// aika,paikka descriptoreita, voitaisiin teh� huomattavia nopeutuksia

NFmiQueryData *NFmiQueryInfo::CreateCombinedData(NFmiQueryInfo *theOtherInfo)
{
  NFmiQueryInfo *combinedInfo1 = CreateCombinedInfo(theOtherInfo);
  NFmiQueryData *combinedData = new NFmiQueryData(*combinedInfo1);
  combinedData->Init();

  // ongelmia ottaa k�ytt��n fastqueryinfo sujuvasti pit�� nyt luoda t�ss� tavallinen info

  NFmiQueryInfo *combinedInfo = new NFmiQueryInfo(combinedData);
  First();
  combinedInfo->First();
  theOtherInfo->First();

  for (combinedInfo->ResetTime(); combinedInfo->NextTime();)
  {
    for (combinedInfo->ResetParam(); combinedInfo->NextParam();)
    {
      for (combinedInfo->ResetLocation(); combinedInfo->NextLocation();)
      {
        if (combinedInfo->IsGrid())
        {
          if (Time(combinedInfo->Time()) && Param(combinedInfo->Param()) &&
              Location(combinedInfo->LatLon()))
          {
            combinedInfo->FloatValue(FloatValue());
          }
          else if (theOtherInfo->Time(combinedInfo->Time()) &&
                   theOtherInfo->Param(combinedInfo->Param()) &&
                   theOtherInfo->Location(combinedInfo->LatLon()))
          {
            combinedInfo->FloatValue(theOtherInfo->FloatValue());
          }
        }
        else
        {
          if (Time(combinedInfo->Time()) && Param(combinedInfo->Param()) &&
              Location(*combinedInfo->Location()))
          {
            combinedInfo->FloatValue(FloatValue());
          }
          else if (theOtherInfo->Time(combinedInfo->Time()) &&
                   theOtherInfo->Param(combinedInfo->Param()) &&
                   theOtherInfo->Location(*combinedInfo->Location()))
          {
            combinedInfo->FloatValue(theOtherInfo->FloatValue());
          }
        }
      }
    }
  }

  delete combinedInfo1;
  delete combinedInfo;
  return combinedData;
}

// ----------------------------------------------------------------------
/*!
 * \param theOtherInfo Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiQueryInfo *NFmiQueryInfo::CreateCombinedInfo(NFmiQueryInfo *theOtherInfo)
{
  return new NFmiQueryInfo(itsParamDescriptor->Combine(theOtherInfo->ParamDescriptor()),
                           itsTimeDescriptor->Combine(theOtherInfo->TimeDescriptor()),
                           itsHPlaceDescriptor->Combine(theOtherInfo->HPlaceDescriptor()),
                           *itsVPlaceDescriptor);
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa gridin tapauksessa currentista hilapisteest� offsettien avulla
 * osoitetun paikan paikan. Jos ei ole gridi tai menee hilan ohi, palauttaa
 * pisteen, jonka x ja y koordinaatit ovat kFloatMissing
 *
 * \param theXOffset Undocumented, unused
 * \param theYOffset Undocumented, unused
 * \return Undocumented
 * \todo Not implemented properly
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiQueryInfo::PeekLocationLatLon(int /* theXOffset */, int /* theYOffset */)
{
  if (IsGrid())
  {
    // katso toteutuksesta mallia NFmiFastQueryInfo:sta
    // JA toteuta T�M� sitten!!! (1999.10.28/Marko)
  }
  return NFmiPoint(kFloatMissing, kFloatMissing);
}

float NFmiQueryInfo::CachedInterpolation(const NFmiLocationCache &theLocationCache)
{
  if (theLocationCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua paikka indeksi ei saa olla muuttunut, indeksi talteen t�ss�!!!
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    float value = kFloatMissing;
    if (theLocationCache.NoInterpolation())
      value = FloatValue();
    else
    {
      // optimointia, jos ollaan tarpeeksi l�hell� hilaa, palautetaan suoraan hilan arvo
      if (fabs(theLocationCache.itsGridPoint.X() - round(theLocationCache.itsGridPoint.X())) <
              gInterpolatedValueEps &&
          fabs(theLocationCache.itsGridPoint.Y() - round(theLocationCache.itsGridPoint.Y())) <
              gInterpolatedValueEps)
        return FloatValue();

      NFmiDataIdent &param = Param();
      FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
      bool wcParam = IsSubParamUsed() && ::IsWeatherSubParam(*this);
      if (wcParam)  // HESSAA, hsade, sateenolomuoto ja sateen tyypit pit�� hoitaa erikoistapauksina
        return InterpolatedValueForCombinedParam(theLocationCache.itsGridPoint);
      else if (interp == kNearestPoint || interp == kNoneInterpolation)
      {
        if (theLocationCache.itsLocationIndex == gStrechedGlobalGridIndex)
        {
          unsigned long gridSizeX = Grid()->XNumber();
          double xFraction = theLocationCache.itsGridPoint.X() -
                             static_cast<int>(theLocationCache.itsGridPoint.X());
          // xFraction:ista riippuen otetaan l�hin hilapiste joko maailman oikeasta tai vasemmasta
          // reunasta
          int xInd = (xFraction > 0.5) ? 0 : gridSizeX - 1;
          int yInd = round(theLocationCache.itsGridPoint
                               .Y());  // y-indeksi vain py�ristet��n l�himp��n pisteeseen
          LocationIndex(yInd * gridSizeX + xInd);
          value = FloatValue();
        }
        else
          value = FloatValue();
      }
      else
      {
        std::vector<float> values(4, kFloatMissing);  // indexies 0..3 => bl,br,tr,tl
        GetCachedValues(theLocationCache, values);
        FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
        value = CachedLocationInterpolatedValue(values, 0, theLocationCache, interp, parId);
      }
    }
    // palautetaan indeksi lopuksi
    LocationIndex(oldLocationIndex);
    return value;
  }
}

float NFmiQueryInfo::CachedInterpolation(const NFmiTimeCache &theTimeCache)
{
  if (theTimeCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua aika indeksi ei saa olla muuttunut, indeksi talteen t�ss�!!!
    unsigned long oldTimeIndex = TimeIndex();
    std::vector<float> values(2, kFloatMissing);
    GetCachedValues(theTimeCache, values);
    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
    float value = CachedTimeInterpolatedValue(values[0], values[1], theTimeCache, interp, parId);
    // palautetaan indeksi lopuksi
    TimeIndex(oldTimeIndex);
    return value;
  }
}

static float InterpolateWandC(std::vector<float> &theValues,
                              size_t theStartIndex,
                              const NFmiPoint &theGridPoint,
                              double theInfoVersion)
{
  NFmiWeatherAndCloudiness bl(
      theValues[theStartIndex + 0], kFmiPackedWeather, kFloatMissing, theInfoVersion);
  NFmiWeatherAndCloudiness br(
      theValues[theStartIndex + 1], kFmiPackedWeather, kFloatMissing, theInfoVersion);
  NFmiWeatherAndCloudiness tr(
      theValues[theStartIndex + 2], kFmiPackedWeather, kFloatMissing, theInfoVersion);
  NFmiWeatherAndCloudiness tl(
      theValues[theStartIndex + 3], kFmiPackedWeather, kFloatMissing, theInfoVersion);
  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  NFmiWeatherAndCloudiness weather(theInfoVersion);
  weather.SetToWeightedMean(&bl,
                            float((1. - dx) * (1. - dy)),
                            &br,
                            float(dx * (1. - dy)),
                            &tl,
                            float((1. - dx) * dy),
                            &tr,
                            float(dx * dy));

  return weather.TransformedFloatValue();
}

static float InterpolateTotalWind(std::vector<float> &theValues,
                                  size_t theStartIndex,
                                  const NFmiPoint &theGridPoint,
                                  double theInfoVersion)
{
  NFmiTotalWind bl(theValues[theStartIndex + 0], kFmiPackedWind, theInfoVersion);
  NFmiTotalWind br(theValues[theStartIndex + 1], kFmiPackedWind, theInfoVersion);
  NFmiTotalWind tr(theValues[theStartIndex + 2], kFmiPackedWind, theInfoVersion);
  NFmiTotalWind tl(theValues[theStartIndex + 3], kFmiPackedWind, theInfoVersion);
  double dx = theGridPoint.X() - int(theGridPoint.X());
  double dy = theGridPoint.Y() - int(theGridPoint.Y());

  NFmiTotalWind wind(theInfoVersion);
  wind.SetToWeightedMean(&bl,
                         float((1. - dx) * (1. - dy)),
                         &br,
                         float(dx * (1. - dy)),
                         &tl,
                         float((1. - dx) * dy),
                         &tr,
                         float(dx * dy));

  return wind.TransformedFloatValue();
}

static float InterpolateWindDir(std::vector<float> &theWSvalues,
                                std::vector<float> &theWDvalues,
                                size_t theWDStartIndex,
                                const NFmiPoint &theGridPoint)
{
  double dx = theGridPoint.X() - floor(theGridPoint.X());
  double dy = theGridPoint.Y() - floor(theGridPoint.Y());
  NFmiInterpolation::WindInterpolator windInterpolator;
  windInterpolator.operator()(
      theWSvalues[0], theWDvalues[theWDStartIndex + 0], (1 - dx) * (1 - dy));
  windInterpolator.operator()(theWSvalues[1], theWDvalues[theWDStartIndex + 1], dx * (1 - dy));
  windInterpolator.operator()(theWSvalues[2], theWDvalues[theWDStartIndex + 2], dx * dy);
  windInterpolator.operator()(theWSvalues[3], theWDvalues[theWDStartIndex + 3], (1 - dx) * dy);

  return static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa vain tuulen suunta
                                                            // (t�m� pit�� tehd� jotenkin fiksummin)
}

float NFmiQueryInfo::CachedLocationInterpolatedValue(std::vector<float> &theValues,
                                                     size_t theStartIndex,
                                                     const NFmiLocationCache &theLocationCache,
                                                     FmiInterpolationMethod theInterpolatioMethod,
                                                     FmiParameterName theParId)
{
  float value = kFloatMissing;

  if (theInterpolatioMethod == kByCombinedParam)
  {
    if (theParId == kFmiWeatherAndCloudiness)
      value = ::InterpolateWandC(
          theValues, theStartIndex, theLocationCache.itsGridPoint, InfoVersion());
    else if (theParId == kFmiTotalWindMS)
      value = ::InterpolateTotalWind(
          theValues, theStartIndex, theLocationCache.itsGridPoint, InfoVersion());
  }
  else
  {
    const NFmiPoint &gpoint = theLocationCache.itsGridPoint;
    // Nearest interpolaatio on otettu jo ennen t�m�n kutsua.
    // return InterpolatedValueForCombinedParam(gpoint, xShift, yShift);  // pit��k� n�m�kin ottaa
    // erikseen huomioon???
    if (theParId == kFmiWindDirection)
    {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
      Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
      std::vector<float> WSvalues(4, kFloatMissing);
      GetCachedValues(theLocationCache, WSvalues);
      Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
      value = ::InterpolateWindDir(WSvalues, theValues, theStartIndex, gpoint);
    }
    else if (theParId == kFmiWaveDirection)
    {
      // modulo 360 interpolation
      value = static_cast<float>(NFmiInterpolation::ModBiLinear(gpoint.X() - floor(gpoint.X()),
                                                                gpoint.Y() - floor(gpoint.Y()),
                                                                theValues[theStartIndex + 3],
                                                                theValues[theStartIndex + 2],
                                                                theValues[theStartIndex + 0],
                                                                theValues[theStartIndex + 1]));
    }
    else
    {
      value = static_cast<float>(NFmiInterpolation::BiLinear(gpoint.X() - floor(gpoint.X()),
                                                             gpoint.Y() - floor(gpoint.Y()),
                                                             theValues[theStartIndex + 3],
                                                             theValues[theStartIndex + 2],
                                                             theValues[theStartIndex + 0],
                                                             theValues[theStartIndex + 1]));
    }
  }
  return value;
}

float NFmiQueryInfo::CachedTimeInterpolatedValue(float theValue1,
                                                 float theValue2,
                                                 const NFmiTimeCache &theTimeCache,
                                                 FmiInterpolationMethod theInterpolatioMethod,
                                                 FmiParameterName theParId)
{
  float value = kFloatMissing;
  float offset = theTimeCache.itsOffset;
  if (theInterpolatioMethod == kByCombinedParam)
  {
    if (theParId == kFmiTotalWindMS)
    {  // korjasin k�ytt�m��n SetToWeightedMean-metodia, joka laskee u- ja v-komponenttien avulla
      NFmiTotalWind resultWind(itsInfoVersion);
      NFmiTotalWind tempWind1(theValue1, kFmiPackedWind, itsInfoVersion);
      NFmiTotalWind tempWind2(theValue2, kFmiPackedWind, itsInfoVersion);
      resultWind.SetToWeightedMean(
          &tempWind1, 1 - offset, &tempWind2, offset, &tempWind1, 0, &tempWind1, 0);
      value = resultWind.TransformedFloatValue();
    }
    else
    {                                                          // kFmiWeatherAndCloudiness:
      NFmiWeatherAndCloudiness resultWeather(itsInfoVersion);  // Marko testi
      NFmiWeatherAndCloudiness tempWeather1(
          theValue1, kFmiPackedWeather, kFloatMissing, itsInfoVersion);
      NFmiWeatherAndCloudiness tempWeather2(
          theValue2, kFmiPackedWeather, kFloatMissing, itsInfoVersion);

      resultWeather.SetToWeightedMean(
          &tempWeather1, 1 - offset, &tempWeather2, offset, &tempWeather1, 0, &tempWeather1, 0);
      value = resultWeather.TransformedFloatValue();
    }
  }
  else
  {
    if (theValue1 == kFloatMissing && theValue2 == kFloatMissing)
      value = kFloatMissing;
    else if (theValue1 == kFloatMissing)
    {
      if (offset > 0.5)
        value = theValue2;
      else
        value = kFloatMissing;
    }
    else if (theValue2 == kFloatMissing)
    {
      if (offset <= 0.5)
        value = theValue1;
      else
        value = kFloatMissing;
    }
    else
    {
      if (theInterpolatioMethod != kLinearly)
      {
        if (offset <= 0.5)
          value = theValue1;
        else
          value = theValue2;
      }
      else  // muuten lineaarinen interpolointi
      {
        if (theParId == kFmiWindDirection)
        {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
          unsigned long oldTimeIndex = TimeIndex();
          Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
          TimeIndex(theTimeCache.itsTimeIndex1);
          float ws1 = FloatValue();
          TimeIndex(theTimeCache.itsTimeIndex2);
          float ws2 = FloatValue();
          TimeIndex(oldTimeIndex);
          Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
          NFmiInterpolation::WindInterpolator windInterpolator;
          windInterpolator.operator()(ws1, theValue1, offset);
          windInterpolator.operator()(ws2, theValue2, (1 - offset));
          value = static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa vain tuulen
                                                                     // suunta (t�m� pit�� tehd�
                                                                     // jotenkin fiksummin)
        }
        else if (theParId == kFmiWaveDirection)
          value =
              static_cast<float>(NFmiInterpolation::ModLinear(offset, theValue1, theValue2, 360));
        else
          value = static_cast<float>(NFmiInterpolation::Linear(offset, theValue1, theValue2));
      }
    }
  }
  return value;
}

float NFmiQueryInfo::CachedInterpolation(const NFmiLocationCache &theLocationCache,
                                         const NFmiTimeCache &theTimeCache)
{
  if (theLocationCache.NoValue() || theTimeCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua paikka/aika indeksit eiv�t saa olla muuttuneita, indeksit talteen t�ss�!!!
    unsigned long oldTimeIndex = TimeIndex();
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    TimeIndex(theTimeCache.itsTimeIndex1);
    float value = kFloatMissing;
    if (theTimeCache.NoInterpolation())
    {
      if (theLocationCache.NoInterpolation())
        value = FloatValue();
      else
      {
        TimeIndex(theTimeCache.itsTimeIndex1);
        value = CachedInterpolation(theLocationCache);
      }
    }
    else
    {
      if (theLocationCache.NoInterpolation())
        value = CachedInterpolation(theTimeCache);
      else
      {  // tehd��n sitten sek� aika ett� paikka cached interpolaatio
        std::vector<float> values(8, kFloatMissing);
        GetCachedValues(theLocationCache, theTimeCache, values);
        NFmiDataIdent &param = Param();
        FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
        FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
        float value1 = CachedLocationInterpolatedValue(values, 0, theLocationCache, interp, parId);
        float value2 = CachedLocationInterpolatedValue(values, 4, theLocationCache, interp, parId);
        value = CachedTimeInterpolatedValue(value1, value2, theTimeCache, interp, parId);
      }
    }
    // palautetaan indeksit lopuksi
    TimeIndex(oldTimeIndex);
    LocationIndex(oldLocationIndex);
    return value;
  }
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseiss�:
// 0 -> time1
// 1 -> time2
void NFmiQueryInfo::GetCachedValues(const NFmiTimeCache &theTimeCache,
                                    std::vector<float> &theValues)
{
  TimeIndex(theTimeCache.itsTimeIndex1);
  theValues[0] = FloatValue();
  TimeIndex(theTimeCache.itsTimeIndex2);
  theValues[1] = FloatValue();
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseiss�:
// 0-3 -> time1 bl,br,tr,tl
// 4-7 -> time2 bl,br,tr,tl
void NFmiQueryInfo::GetCachedValues(const NFmiLocationCache &theLocationCache,
                                    const NFmiTimeCache &theTimeCache,
                                    std::vector<float> &theValues)
{
  TimeIndex(theTimeCache.itsTimeIndex1);
  GetCachedValues(theLocationCache, theValues, 0);
  TimeIndex(theTimeCache.itsTimeIndex2);
  GetCachedValues(theLocationCache, theValues, 4);
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiLocationCache &theLocationCache,
                                                 const NFmiTimeCache &theTimeCache,
                                                 std::vector<float> &theValues)
{
  TimeIndex(theTimeCache.itsTimeIndex1);
  GetCachedPressureLevelValues(P, theLocationCache, theValues, 0);
  TimeIndex(theTimeCache.itsTimeIndex2);
  GetCachedPressureLevelValues(P, theLocationCache, theValues, 4);
}

bool NFmiQueryInfo::DoGlobalWrapFix(const NFmiPoint &theGridPoint) const
{
  if (fIsStrechableGlobalGrid)
  {
    unsigned long xsize = Grid()->XNumber();
    if (theGridPoint.X() > xsize - 1 && theGridPoint.X() <= xsize) return true;
  }
  return false;
}

// oletus, vektori on jo alustettu sopivan kokoiseksi ja puuttuvilla arvoilla
// arvojen sijoitus annetun vektorin indekseiss�:
// theStartIndex + 0..3 -> time1 bl,br,tr,tl
void NFmiQueryInfo::GetCachedValues(const NFmiLocationCache &theLocationCache,
                                    std::vector<float> &theValues,
                                    size_t theStartIndex)
{
  const NFmiPoint &gpoint = theLocationCache.itsGridPoint;
  if (theLocationCache.itsLocationIndex == gStrechedGlobalGridIndex)
  {
    unsigned long xsize = Grid()->XNumber();
    // Lasketaan ensin hilan oikean reunan alempi hilapiste
    unsigned long newLocationIndex = static_cast<unsigned long>(gpoint.Y()) * xsize + (xsize - 1);
    LocationIndex(newLocationIndex);
    theValues[theStartIndex + 0] = PeekLocationValue(0, 0);
    theValues[theStartIndex + 3] = PeekLocationValue(0, 1);
    // Lasketaan sitten hilan vasemman reunan alempi hilapiste
    newLocationIndex = static_cast<unsigned long>(gpoint.Y()) * xsize + 0;
    LocationIndex(newLocationIndex);
    theValues[theStartIndex + 1] = PeekLocationValue(0, 0);
    theValues[theStartIndex + 2] = PeekLocationValue(0, 1);
  }
  else
  {
    LocationIndex(theLocationCache.itsLocationIndex);
    // Otettu hilapisteen liikuttelu koodi InterpolatedValue(latlon) -metodista.
    int xShift = static_cast<int>(::round(gpoint.X())) - static_cast<int>(gpoint.X());
    int yShift = static_cast<int>(::round(gpoint.Y())) - static_cast<int>(gpoint.Y());

    theValues[theStartIndex + 0] = PeekLocationValue(0 - xShift, 0 - yShift);
    theValues[theStartIndex + 3] = PeekLocationValue(0 - xShift, 1 - yShift);
    theValues[theStartIndex + 1] = PeekLocationValue(1 - xShift, 0 - yShift);
    theValues[theStartIndex + 2] = PeekLocationValue(1 - xShift, 1 - yShift);
  }
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiLocationCache &theLocationCache,
                                                 std::vector<float> &theValues,
                                                 size_t theStartIndex)
{
  LocationIndex(theLocationCache.itsLocationIndex);
  const NFmiPoint &gpoint = theLocationCache.itsGridPoint;
  // Otettu hilapisteen liikuttelu koodi InterpolatedValue(latlon) -metodista.
  int xShift = static_cast<int>(::round(gpoint.X())) - static_cast<int>(gpoint.X());
  int yShift = static_cast<int>(::round(gpoint.Y())) - static_cast<int>(gpoint.Y());

  if (xShift == 1) MoveLeft();
  if (yShift == 1) MoveDown();

  theValues[theStartIndex + 0] = PressureLevelValue(P);
  MoveRight();
  theValues[theStartIndex + 1] = PressureLevelValue(P);
  MoveLeft();
  MoveUp();
  theValues[theStartIndex + 3] = PressureLevelValue(P);
  MoveRight();
  theValues[theStartIndex + 2] = PressureLevelValue(P);
}

float NFmiQueryInfo::CachedPressureLevelValue(float P, const NFmiLocationCache &theLocationCache)
{
  if (theLocationCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua paikka indeksi ei saa olla muuttunut, indeksi talteen t�ss�!!!
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    float value = kFloatMissing;
    if (theLocationCache.NoInterpolation())
      value = PressureLevelValue(P);
    else
    {
      // optimointia, jos ollaan tarpeeksi l�hell� hilaa, palautetaan suoraan hilan arvo
      if (fabs(theLocationCache.itsGridPoint.X() - round(theLocationCache.itsGridPoint.X())) <
              gInterpolatedValueEps &&
          fabs(theLocationCache.itsGridPoint.Y() - round(theLocationCache.itsGridPoint.Y())) <
              gInterpolatedValueEps)
        return PressureLevelValue(P);

      NFmiDataIdent &param = Param();
      FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
      if (interp == kNearestPoint)
        value = PressureLevelValue(P);
      else
      {
        std::vector<float> values(4, kFloatMissing);
        GetCachedPressureLevelValues(P, theLocationCache, values);
        FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
        value = CachedLocationInterpolatedValue(values, 0, theLocationCache, interp, parId);
      }
    }
    // palautetaan indeksi lopuksi
    LocationIndex(oldLocationIndex);
    return value;
  }
}

float NFmiQueryInfo::CachedPressureLevelValue(float P, const NFmiTimeCache &theTimeCache)
{
  if (theTimeCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua aika indeksi ei saa olla muuttunut, indeksi talteen t�ss�!!!
    unsigned long oldTimeIndex = TimeIndex();
    std::vector<float> values(2, kFloatMissing);
    GetCachedPressureLevelValues(P, theTimeCache, values);
    NFmiDataIdent &param = Param();
    FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
    FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
    float value = CachedTimeInterpolatedValue(values[0], values[1], theTimeCache, interp, parId);
    // palautetaan indeksi lopuksi
    TimeIndex(oldTimeIndex);
    return value;
  }
}

void NFmiQueryInfo::GetCachedPressureLevelValues(float P,
                                                 const NFmiTimeCache &theTimeCache,
                                                 std::vector<float> &theValues)
{
  TimeIndex(theTimeCache.itsTimeIndex1);
  theValues[0] = PressureLevelValue(P);
  TimeIndex(theTimeCache.itsTimeIndex2);
  theValues[1] = PressureLevelValue(P);
}

float NFmiQueryInfo::CachedPressureLevelValue(float P,
                                              const NFmiLocationCache &theLocationCache,
                                              const NFmiTimeCache &theTimeCache)
{
  if (theLocationCache.NoValue() || theTimeCache.NoValue())
    return kFloatMissing;
  else
  {
    // metodin loputtua paikka/aika indeksit eiv�t saa olla muuttuneita, indeksit talteen t�ss�!!!
    unsigned long oldTimeIndex = TimeIndex();
    unsigned long oldLocationIndex = LocationIndex();
    LocationIndex(theLocationCache.itsLocationIndex);
    TimeIndex(theTimeCache.itsTimeIndex1);
    float value = kFloatMissing;
    if (theTimeCache.NoInterpolation())
    {
      if (theLocationCache.NoInterpolation())
        value = PressureLevelValue(P);
      else
      {
        TimeIndex(theTimeCache.itsTimeIndex1);
        value = CachedPressureLevelValue(P, theLocationCache);
      }
    }
    else
    {
      if (theLocationCache.NoInterpolation())
        value = CachedPressureLevelValue(P, theTimeCache);
      else
      {  // tehd��n sitten sek� aika ett� paikka cached interpolaatio
        std::vector<float> values(8, kFloatMissing);
        GetCachedPressureLevelValues(P, theLocationCache, theTimeCache, values);
        NFmiDataIdent &param = Param();
        FmiInterpolationMethod interp = param.GetParam()->InterpolationMethod();
        FmiParameterName parId = static_cast<FmiParameterName>(param.GetParamIdent());
        float value1 = CachedLocationInterpolatedValue(values, 0, theLocationCache, interp, parId);
        float value2 = CachedLocationInterpolatedValue(values, 4, theLocationCache, interp, parId);
        value = CachedTimeInterpolatedValue(value1, value2, theTimeCache, interp, parId);
      }
    }
    // palautetaan indeksit lopuksi
    TimeIndex(oldTimeIndex);
    LocationIndex(oldLocationIndex);
    return value;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Haluttu piste, mihin arvo halutaan.
 * \param theTime Haluttu aika, johon arvo halutaan.
 * \param theMaxMinuteRange Aika rajoitin aikainterpolointiin.
 * \return Haluttuun paikkaan ja aikaan interpoloitu arvo tai
 *   puuttuva arvo, jos interpolointi mahdotonta.
 */
// ----------------------------------------------------------------------

// 1.6.2004/Marko EI toimi viel� t�ysin!
// InterpolatedValueForCombinedParam ei toimi viel� ajan suhteen, mutta sill� ei ole mielest�ni
// v�li�
// Koska t�ll�in haluttaisiin WeatherAndCloudiness ali parametri� interpoloituna ajan ja paikan
// suhteen
// ja sen teen kuntoon jos sit� oikeasti tarvitaan.
// HUOM!!! T�m� voi muuttaa infon locationindexi�!!!!!
float NFmiQueryInfo::InterpolatedValue(const NFmiPoint &theLatLonPoint,
                                       const NFmiMetTime &theTime,
                                       int theMaxMinuteRange)
{
  if (Time(theTime))
    return InterpolatedValue(
        theLatLonPoint);  // jos aika l�ytyy suoraan, turha tehd� erillist� aikainterpolointia

  float theValue = kFloatMissing;
  NFmiParam *parameter = Param().GetParam();
  if (parameter == 0)  // parametria ei ole asetettu kohdalleen!!!!
  {
    assert(parameter);
    return theValue;
  }
  FmiInterpolationMethod interp = parameter->InterpolationMethod();
  FmiParameterName param = static_cast<FmiParameterName>(parameter->GetIdent());

  bool wcParam = IsSubParamUsed() && ::IsWeatherSubParam(*this);

  if (IsGrid() && ((interp != kNoneInterpolation && interp != kNearestPoint) || wcParam ||
                   param == kFmiWindVectorMS))
  {
    NFmiPoint gpoint;
    if (Location(theLatLonPoint, &gpoint))
    {
      // optimointia, jos ollaan tarpeeksi l�hell� hilaa, palautetaan suoraan hilan arvo
      if (fabs(gpoint.X() - round(gpoint.X())) < 0.00001 &&
          fabs(gpoint.Y() - round(gpoint.Y())) < 0.00001)
        return InterpolatedValue(theTime, theMaxMinuteRange);

      // Location(theLatLonPoint)-metodi asettaa l�himp��n hilapisteeseen, josta seuraa se,
      // ett� PeekLocationValue:lle pit�� laskea mahdollinen siirtym�
      int xShift = static_cast<int>(round(gpoint.X())) - static_cast<int>(gpoint.X());
      int yShift = static_cast<int>(round(gpoint.Y())) - static_cast<int>(gpoint.Y());

      if (wcParam)  // interp == kByCombinedParam) //16.9.99 t�m� haara/Lasse // 1999.09.21/Marko
                    // Miten n�ist� p��see eroon?
      {
        SetIsSubParamUsed(false);
        NFmiWeatherAndCloudiness tmpWeather(itsInfoVersion);
        tmpWeather.TransformFromFloatValue(InterpolatedValueForWeatherAndCloudiness(
            theTime, theMaxMinuteRange, gpoint, xShift, yShift));
        SetIsSubParamUsed(true);
        return static_cast<float>(tmpWeather.SubValue(param));

        //				return InterpolatedValueForCombinedParam(gpoint, xShift,
        //yShift);
      }
      else if (interp == kByCombinedParam)
      {
        if (param == kFmiWeatherAndCloudiness)
          return InterpolatedValueForWeatherAndCloudiness(
              theTime, theMaxMinuteRange, gpoint, xShift, yShift);
        else if (param == kFmiTotalWindMS)
          return InterpolatedValueForTotalWind(theTime, theMaxMinuteRange, gpoint, xShift, yShift);

        return theValue;
      }
      else
      {
        // t�m� liikkuminen on melkoista virityst�
        MoveLeft(xShift);
        MoveDown(yShift);
        float bottomLeftValue = InterpolatedValue(theTime, theMaxMinuteRange);
        MoveRight(1);
        float bottomRightValue = InterpolatedValue(theTime, theMaxMinuteRange);
        MoveUp(1);
        float topRightValue = InterpolatedValue(theTime, theMaxMinuteRange);
        MoveLeft(1);
        float topLeftValue = InterpolatedValue(theTime, theMaxMinuteRange);

        if (param == kFmiWindDirection)
        {  // tehd��n wind-dir laskut WD ja WS avulla jolloin interpolointi on parempaa
          Param(kFmiWindSpeedMS);  // asetetaan parametriksi v�liaikaisesti tuulennopeus
          float tlWS = InterpolatedValue(theTime, theMaxMinuteRange);  // aloitetaan top-leftist� ja
                                                                       // haetaan kaikki nurkat
                                                                       // move-komennoilla
          MoveRight(1);
          float trWS = InterpolatedValue(theTime, theMaxMinuteRange);  // top-right
          MoveDown(1);
          float brWS = InterpolatedValue(theTime, theMaxMinuteRange);  // bottom-right
          MoveLeft(1);
          float blWS = InterpolatedValue(theTime, theMaxMinuteRange);  // bottom-left
          Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
          double dx = gpoint.X() - floor(gpoint.X());
          double dy = gpoint.Y() - floor(gpoint.Y());
          NFmiInterpolation::WindInterpolator windInterpolator;
          windInterpolator.operator()(blWS, bottomLeftValue, (1 - dx) * (1 - dy));
          windInterpolator.operator()(brWS, bottomRightValue, dx * (1 - dy));
          windInterpolator.operator()(trWS, topRightValue, dx * dy);
          windInterpolator.operator()(tlWS, topLeftValue, (1 - dx) * dy);

          theValue = static_cast<float>(windInterpolator.Direction());  // meit� kiinnostaa vain
                                                                        // tuulen suunta (t�m� pit��
                                                                        // tehd� jotenkin fiksummin)
        }
        else if (param == kFmiWaveDirection)
        {
          theValue =
              static_cast<float>(NFmiInterpolation::ModBiLinear(gpoint.X() - floor(gpoint.X()),
                                                                gpoint.Y() - floor(gpoint.Y()),
                                                                topLeftValue,
                                                                topRightValue,
                                                                bottomLeftValue,
                                                                bottomRightValue));
        }
        else if (param == kFmiWindVectorMS)
        {
          theValue =
              static_cast<float>(NFmiInterpolation::WindVector(gpoint.X() - floor(gpoint.X()),
                                                               gpoint.Y() - floor(gpoint.Y()),
                                                               topLeftValue,
                                                               topRightValue,
                                                               bottomLeftValue,
                                                               bottomRightValue));
        }
        else
        {
          theValue = static_cast<float>(NFmiInterpolation::BiLinear(gpoint.X() - floor(gpoint.X()),
                                                                    gpoint.Y() - floor(gpoint.Y()),
                                                                    topLeftValue,
                                                                    topRightValue,
                                                                    bottomLeftValue,
                                                                    bottomRightValue));
        }
        if (theValue != kFloatMissing) return theValue;
      }
    }
  }
  Location(theLatLonPoint);
  return InterpolatedValue(theTime, theMaxMinuteRange);
}

// ----------------------------------------------------------------------
/*!
 * \param theTimeOffset Undocumented
 * \param theXOffset Undocumented
 * \param theYOffset Undocumented
 * \return Undocumented
 * \bug Does not work correctly in all situations
 */
// ----------------------------------------------------------------------

// ei toimi kaikissa tilanteissa oikein, parempi versio fastqueryinfossa

float NFmiQueryInfo::PeekValue(int theTimeOffset, int theXOffset, int theYOffset)
{
  unsigned long paramIndex = itsParamDescriptor->Index();
  unsigned long timeIndex = itsTimeDescriptor->Index() + theTimeOffset;
  unsigned long theVPlaceIndex = itsVPlaceDescriptor->Index();
  unsigned long theHPlaceIndex = itsHPlaceDescriptor->Index();

  if (IsGrid())
    theHPlaceIndex = theHPlaceIndex + theYOffset * Grid()->XNumber() + theXOffset;
  else
    theHPlaceIndex = theHPlaceIndex + theXOffset;

  unsigned long idx =
      ((paramIndex) * (itsHPlaceDescriptor->Size()) * (itsVPlaceDescriptor->Size()) *
           (itsTimeDescriptor->Size()) +
       (theHPlaceIndex) * (itsVPlaceDescriptor->Size()) * (itsTimeDescriptor->Size()) +
       theVPlaceIndex * itsTimeDescriptor->Size() + timeIndex);
  return PeekValue(idx);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiQueryInfo::CalcAreaUnCertainty(void)
{
  double tmpValue = itsAreaUnCertaintyStart +
                    (itsAreaUnCertaintyEnd - itsAreaUnCertaintyStart) * TimeIndex() /
                        static_cast<double>(SizeTimes());
  return static_cast<long>(round(tmpValue));  // k�yt� oikeasti n�it�
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiQueryInfo::CalcTimeUnCertainty(void)
{
  return itsTimeUnCertaintyStart;  // k�yt� oikeasti n�it�
}

// ----------------------------------------------------------------------
/*!
 * \param newMask Undocumented
 * \param useNewMask Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiQueryInfo::StaticDataMask(const NFmiBitMask & newMask, bool useNewMask)
{
  delete itsStaticDataMask;
  itsStaticDataMask = new NFmiBitMask(newMask);
  fUseStaticDataMask = useNewMask;
}
*/
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
const NFmiBitMask & NFmiQueryInfo::StaticDataMask(void)
{
  return *itsStaticDataMask;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiQueryInfo::UseStaticMask(bool newValue)
{
  fUseStaticDataMask = newValue;
}
*/
// ----------------------------------------------------------------------
/*!
 * \param newProducer Undocumented
 */
// ----------------------------------------------------------------------

// vaihtaa tuottajan kaikkiin parametreihin niin infossa kuin querydatassa

void NFmiQueryInfo::SetProducer(const NFmiProducer &newProducer)
{
  if (itsParamDescriptor) itsParamDescriptor->SetProducer(newProducer);
  if (RefQueryData()) RefQueryData()->Info()->SetProducer(newProducer);
}

// ----------------------------------------------------------------------
/*!
 * \param theReplaceData Undocumented
 * \param theTimeBag Undocumented
 */
// ----------------------------------------------------------------------

// 12.6.2000/Pena Funktio laittaa this-infoon parametrina annetun datan sille aikav�lille,
// mik� on annettu parametrina. Jos timeBagia ei anneta, t�ytet��n this-info niin paljon kuin
// parametrina annettua dataa riitt��.

bool NFmiQueryInfo::ReplaceData(NFmiQueryInfo *theReplaceData, NFmiTimeBag *theTimeBag)
{
  if (!theReplaceData) return false;
  NFmiMetTime TheFirstTime;
  NFmiMetTime TheLastTime;

  First();
  theReplaceData->First();

  if (!theTimeBag)
  {
    TheFirstTime = theReplaceData->TimeDescriptor().FirstTime();
    TheLastTime = theReplaceData->TimeDescriptor().LastTime();
  }
  else
  {
    TheFirstTime = theTimeBag->FirstTime();
    TheLastTime = theTimeBag->LastTime();
  }

  float Float2Value;

  First();
  theReplaceData->First();

  theReplaceData->ResetLevel();
  ResetLevel();

  while (NextLevel())
  {
    if (!theReplaceData->Level(*Level())) continue;
    ResetTime();
    while (NextTime() && TheLastTime.DifferenceInHours(ValidTime()) >= 0)
    {
      if (TheFirstTime.DifferenceInHours(ValidTime()) <= 0)
      {
        if (!theReplaceData->Time(Time())) continue;

        ResetParam();

        while (NextParam())
        {
          if (!theReplaceData->Param(Param()))
            if (!theReplaceData->Param(*Param().GetParam()))  // 18.8.2000/Marko lis�si, koska
                                                              // tuottaja ei saa vaikuttaa t�ss�
              continue;

          ResetLocation();

          while (NextLocation())
          {
            if (theReplaceData->IsGrid())
            {
              Float2Value = theReplaceData->InterpolatedValue(LatLon());
              if (Float2Value != kFloatMissing)
              {
                FloatValue(Float2Value);
              }
            }
            else
            {
              if (theReplaceData->Location(*Location()))
              {
                if (theReplaceData->FloatValue() != kFloatMissing)
                {
                  FloatValue(theReplaceData->FloatValue());
                }
              }
            }
          }
        }
      }  //  if TheFirstTime
    }    // next Time
  }      // next Level
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
/*
bool NFmiQueryInfo::IsLocationMasked(unsigned long theLocationIndex) const
{
  return !fUseStaticDataMask ? true : itsStaticDataMask->IsMasked(theLocationIndex);
}
*/
// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ParamIndex(unsigned long theIndex)
{
  return itsParamDescriptor->Index(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::LocationIndex(unsigned long theIndex)
{
  return itsHPlaceDescriptor->Index(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::LevelIndex(unsigned long theIndex)
{
  return itsVPlaceDescriptor->Index(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 * \todo Const reference to long as argument does not make sense
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::TimeIndex(unsigned long theIndex) { return itsTimeDescriptor->Time(theIndex); }
// ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************
// *** HUOM!!! N�M� EIV�T TOIMI VIEL� NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
// ****************
// HUOM!! liikkumista ei voi sallia laatikon ulkopuolelle kuten esim. PeekValue-metodissa voi!!!
// Move*** ovat liikkumista varten

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveUp(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveDown(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveLeft(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param moveBy Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::MoveRight(int /* moveBy */)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// n�ill� asetetaan paikka suoraan johonkin laitaan (ei 'laatikon' ulkopuolelle!)

bool NFmiQueryInfo::Top(void)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Bottom(void)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Left(void)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Right(void)  // toimii vain gridi datalle oikein!!!
{
  return false;
}

// *** HUOM!!! N�M� EIV�T TOIMI VIEL� NFmiQueryInfo:ssa, vain NFmiFastQueryInfo:ssa!!!!
// ****************
// ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************

// ----------------------------------------------------------------------
/*!
 * Etsii 1. halutun avaimen esim.  AVAIN (ei saa laittaa FMI_ etuliitett�!).
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindFirstKey(const NFmiString &theKey)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->FindFirstKey(theKey);

  itsHeaderText->Reset();
  do
  {
    if (IsCurrentKey(theKey)) return true;
  } while (itsHeaderText->Next());
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindNextKey(const NFmiString &theKey)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->FindNextKey(theKey);

  for (; itsHeaderText->Next();)
  {
    if (IsCurrentKey(theKey)) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 *  Lis�� uuden avaimen theKey=AVAIN ja theValue=ARVO -> FMI_AVAIN=ARVO laitetaan talteen.
 * \param theKey Undocumented
 * \param theValue Undocumented
 * \param fClearDuplicatesFirst Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::AddKey(const NFmiString &theKey,
                           const NFmiString &theValue,
                           bool fClearDuplicatesFirst)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->AddKey(theKey, theValue, fClearDuplicatesFirst);

  if (fClearDuplicatesFirst) RemoveAllKeys(theKey);
  NFmiString *keyValueStr = new NFmiString(MakeKeyValueString(theKey, theValue));
  itsHeaderText->Add(keyValueStr);  // stringlist tuhoaa keyValueStr:in
  return true;
}

// ----------------------------------------------------------------------
/*!
 * Jos on l�ytynyt jokin avain, voi sen arvon asettaa t�ss�.
 *
 * \param newValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::SetCurrentKeyValue(const NFmiString &newValue)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->SetCurrentKeyValue(newValue);

  if (IsCurrentAnyKey())  // pit�� tarkistaa ollaako oikeasti mink��n avaimen kohdalla ensin
    return ReplaceCurrentKeyValue(newValue);
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Pyyd� etsityn avaimen arvo.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::GetCurrentKeyValue(void)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->GetCurrentKeyValue();

  if (IsCurrentAnyKey()) return ExtractCurrentKeyValue();
  NFmiString returnVal;  // palautetaan tyhj��, jos kyseess� ei ole avain
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * Poista etsityn avain ja sen arvo, eli poista listasta koko stringi.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveCurrentKey(void)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->RemoveCurrentKey();

  if (IsCurrentAnyKey()) return itsHeaderText->Remove();
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Etsii itsHeaderText:ist� kaikki avaimet ja poistaa ne. HUOM! muita ei
 * itsHeaderText otuksesta poisteta kuin avaimet (eli FMI_*= alkuiset stringit).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveAllKeys(void)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->RemoveAllKeys();

  if (FindFirstAnyKey())
  {
    do
    {
      // T�m� toinen sisempi luuppi piti laittaa, koska NFmiStringList k�ytt�� voidptrlistaa.
      // En saa voidptrlistasta pois tavaraa, niin ett� osoitin siirtyisi edelliseen stringiin.
      // Lis�ksi jos saisin , ei string-osoitin liikkua alun ohi, eli jos poistettava
      // stringi olisi 1., remove:n j�lkeen next heitt�isi ohi seuraavan mahd. poistettavan
      // stringin.
      // TEE NFmiStringList k�ytt�en STL vector:ia!!!!!!!!!!!!!!!!!!!!!!!!!!!
      do
      {
      } while (RemoveCurrentKey());
    } while (FindNextAnyKey());
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * Poistaa kaikki halutut avaimet.
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::RemoveAllKeys(const NFmiString &theKey)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->RemoveAllKeys(theKey);

  if (FindFirstKey(theKey))
  {
    do
    {
      // T�m� toinen sisempi luuppi piti laittaa, koska NFmiStringList k�ytt�� voidptrlistaa.
      // En saa voidptrlistasta pois tavaraa, niin ett� osoitin siirtyisi edelliseen stringiin.
      // Lis�ksi jos saisin , ei string-osoitin liikkua alun ohi, eli jos poistettava
      // stringi olisi 1., remove:n j�lkeen next heitt�isi ohi seuraavan mahd. poistettavan
      // stringin.
      // TEE NFmiStringList k�ytt�en STL vector:ia!!!!!!!!!!!!!!!!!!!!!!!!!!!
      do
      {
      } while (RemoveCurrentKey());
    } while (FindNextKey(theKey));
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa listan, joka sis�lt�� kaikki itsHeaderText:in avaimet.
 *
 * \param fRemoveDuplicates Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiStringList NFmiQueryInfo::GetAllKeys(bool fRemoveDuplicates)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->GetAllKeys(fRemoveDuplicates);

  NFmiStringList list;
  if (FindFirstAnyKey())
  {
    NFmiString *key = 0;
    do
    {
      key = new NFmiString(ExtractCurrentKey());
      if (fRemoveDuplicates && list.Find(*key))
        delete key;
      else
        list.Add(key);

    } while (FindNextAnyKey());
  }
  return list;
}

// ----------------------------------------------------------------------
/*!
 * Etsinn�n j�lkeen voi kysy� mik� avain olikaan kyseess�.
 * Palauttaa "" jos ei ole kyseess� avainta.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::GetCurrentKey(void)
{
  if (itsRefQueryData)  // jos this-info on jonkin datan iteraattori, t�ll�in v�lit� k�sky datalle
    return itsRefQueryData->GetCurrentKey();

  if (IsCurrentAnyKey()) return ExtractCurrentKey();
  NFmiString returnVal;  // jos current ei ole avain, palauta tyhj� stringi
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * Sis�inen apufunktio, joka tekee AVAIN + ARVO -> FMI_AVAIN=ARVO tekstin.
 *
 * \param theKey Undocumented
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::MakeKeyValueString(const NFmiString &theKey,
                                                   const NFmiString &theValue)
{
  NFmiString returnVal(gInfoHeaderKeyWord);
  returnVal += theKey;
  returnVal += "=";
  returnVal += theValue;
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * Etsii ensimm�isen avaimen itsHeaderText:ist�. Hakee jokaisen listassa
 * olevan stringin alusta FMI_*= -kuviota (* tarkoittaa mit� tahansa
 * teksti�, saa olla whitespaceja).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindFirstAnyKey(void)
{
  itsHeaderText->Reset();
  do
  {
    if (IsCurrentAnyKey()) return true;
  } while (itsHeaderText->Next());
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Etsii seuraavaa avainta.
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::FindNextAnyKey(void)
{
  for (; itsHeaderText->Next();)
  {
    if (IsCurrentAnyKey()) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Tarkistaa onko itsHeaderText:in osoittama stringi avain (jotta sille
 * voidaan tehd� operaatioita). Eli onko se jokin avain (FMI_*= ,
 * miss� * on sarja merkkej�, saa olla whitespaceja).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsCurrentAnyKey(void)
{
  NFmiString *currentStr = itsHeaderText->Current();
  if (currentStr)
  {
    NFmiString searchStr(gInfoHeaderKeyWord);
    int pos = 0;
    if ((pos = currentStr->Search(searchStr)) ==
        1)  // tuloksen pit�� olla 1 (= etsitty teksti l�ytyy heti stringin alusta)
    {
      unsigned char test = currentStr->operator[](5);
      if (!isspace(test))  // FMI_ j�lkeen ei saa olla whitespacea
      {
        NFmiString searchStr2("=");
        if (currentStr->Search(searchStr2, pos + 2) >
            0)  // +2 tarkoittaa, ett� etsit��n '='-merkki� FMI_ + yksi merkki j�lkeen
          return true;
      }
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Tarkistaa sis�lt��k� itsHeaderText:in currentti stringi halutun avaimen
 *
 * \param theKey Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsCurrentKey(const NFmiString &theKey)
{
  NFmiString *currentStr = itsHeaderText->Current();
  if (currentStr)
  {
    NFmiString searchStr(gInfoHeaderKeyWord);
    searchStr += theKey;
    searchStr += "=";
    if (currentStr->Search(searchStr) ==
        1)  // tuloksen pit�� olla 1 (= etsitty teksti l�ytyy heti stringin alusta)
      return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Vaihda currentin stringin avaimen arvo (se onko kyseess� avain,
 * on jo tarkastettu!).
 * \param newValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::ReplaceCurrentKeyValue(const NFmiString &newValue)
{
  NFmiString *currentStr = itsHeaderText->Current();
  if (currentStr)  // t�m�n pit�isi l�yty�, koska se on jo tarkistettu!!
  {
    NFmiString replaceStr(gInfoHeaderKeyWord);
    replaceStr += ExtractCurrentKey();
    replaceStr += "=";
    replaceStr += newValue;
    *currentStr = replaceStr;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin stringin avaimen arvon (se onko kyseess� avain,
 * on jo tarkastettu!).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::ExtractCurrentKeyValue(void)
{
  NFmiString returnVal;
  NFmiString *currentStr = itsHeaderText->Current();
  if (currentStr)  // t�m�n pit�isi l�yty�, koska se on jo tarkistettu!!
  {
    NFmiString searchStr("=");
    int pos = 0;
    if ((pos = currentStr->Search(searchStr)) > 0)
    {
      int valueStrLength = currentStr->GetLen() - pos;
      returnVal = currentStr->GetChars(pos + 1, valueStrLength);
    }
  }
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin stringin avaimen (se onko kyseess� avain, on
 * jo tarkastettu!).
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiString NFmiQueryInfo::ExtractCurrentKey(void)
{
  NFmiString returnVal;
  NFmiString *currentStr = itsHeaderText->Current();
  if (currentStr)  // t�m�n pit�isi l�yty�, koska se on jo tarkistettu!!
  {
    NFmiString searchStr("=");
    int pos = 0;                                     // '='-merkin paikka
    int startPos = gInfoHeaderKeyWord.GetLen() + 1;  // mist� l�hdet��n etsim��n =-merkki�
    if ((pos = currentStr->Search(searchStr, startPos)) > 0)
    {
      int valueStrLength = pos - startPos;
      returnVal = currentStr->GetChars(startPos, valueStrLength);
    }
  }
  return returnVal;
}

// ----------------------------------------------------------------------
/*!
 * Infon dataa t�ytet��n annetun gridin arvoilla, jos tietyt ehdot
 * t�yttyv�t: Info hilamuotoista, Hilat samat, area sama.
 * Datan t�ytt� tapahtuu currenttiin aikaan, parametriin ja leveliin.
 *
 * \param theSource Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::Grid2Info(NFmiGrid &theSource)
{
  if (IsGrid() && Grid()->AreGridsIdentical(theSource))
  {
    if (IsParamUsable() && IsLevelUsable() &&
        IsTimeUsable())  // onko info kelattu paikkaan, mihin voi yleens� laittaa dataa
    {
      for (ResetLocation(), theSource.Reset(); NextLocation() && theSource.Next();)
        FloatValue(static_cast<float>(theSource.FloatValue()));
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Hakee listan paikkaindeksi/et�isyys metrein� pareja.
 * Listaan haetaan annettua paikkaa l�himmat datapisteet j�rjestyksess�
 * l�himm�st� kauimpaan. Listaan haetaan joko haluttu m��r� l�himpi� pisteit�
 * tai hakua voi my�s rajoittaa maksimi et�isyydell�. Jos maksimi m��r�ksi
 * laitetaan -1, haetaan paikkoja niin paljon kuin l�ytyy (maxEt�isyys rajoitus
 * huomiooon ottaen).
 */
// ----------------------------------------------------------------------

checkedVector<std::pair<int, double> > NFmiQueryInfo::NearestLocations(
    const NFmiLocation &theLocation, int theMaxWantedLocations, double theMaxDistance) const
{
  return itsHPlaceDescriptor->NearestLocations(theLocation, theMaxWantedLocations, theMaxDistance);
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \param theMaxWantedLocations Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

checkedVector<std::pair<int, double> > NFmiQueryInfo::NearestLocations(
    const NFmiPoint &theLatLonPoint, int theMaxWantedLocations, double theMaxDistance) const
{
  NFmiLocation location(theLatLonPoint);
  return NearestLocations(location, theMaxWantedLocations, theMaxDistance);
}

// Laitoin uudet descriptorien muutos palikat.
// Huom!!! Muutokset sallitaan ainoastaan saman kokoisille descriptoreille.
void NFmiQueryInfo::SetHPlaceDescriptor(const NFmiHPlaceDescriptor &newDesc)
{
  if (!itsHPlaceDescriptor)
    throw runtime_error(
        "NFmiQueryInfo::SetHPlaceDescriptor - HPlaceDescriptoria ei oltu alustettu.");
  if (newDesc.Size() != Size())
    throw runtime_error(
        "NFmiQueryInfo::SetHPlaceDescriptor - ei voi asettaa erikokoista descriptoria.");
  else
    *itsHPlaceDescriptor = newDesc;

  if (itsRefQueryData)  // pit�� laittaa sisuskalutkin kuntoon!!!
    itsRefQueryData->SetHPlaceDescriptor(newDesc);
}

void NFmiQueryInfo::SetVPlaceDescriptor(const NFmiVPlaceDescriptor &newDesc)
{
  if (!itsVPlaceDescriptor)
    throw runtime_error(
        "NFmiQueryInfo::SetVPlaceDescriptor - VPlaceDescriptoria ei oltu alustettu.");
  if (newDesc.Size() == SizeLevels())
    *itsVPlaceDescriptor = newDesc;
  else
    throw runtime_error(
        "NFmiQueryInfo::SetVPlaceDescriptor - ei voi asettaa erikokoista descriptoria.");

  if (itsRefQueryData)  // pit�� laittaa sisuskalutkin kuntoon!!!
    itsRefQueryData->Info()->SetVPlaceDescriptor(newDesc);
}

void NFmiQueryInfo::SetTimeDescriptor(const NFmiTimeDescriptor &newDesc)
{
  if (!itsTimeDescriptor)
    throw runtime_error("NFmiQueryInfo::SetTimeDescriptor - TimeDescriptoria ei oltu alustettu.");
  if (newDesc.Size() == SizeTimes())
    *itsTimeDescriptor = newDesc;
  else
    throw runtime_error(
        "NFmiQueryInfo::SetTimeDescriptor - ei voi asettaa erikokoista descriptoria.");

  if (itsRefQueryData)  // pit�� laittaa sisuskalutkin kuntoon!!!
    itsRefQueryData->Info()->SetTimeDescriptor(newDesc);
}

void NFmiQueryInfo::SetParamDescriptor(const NFmiParamDescriptor &newDesc)
{
  if (!itsParamDescriptor)
    throw runtime_error("NFmiQueryInfo::SetParamDescriptor - ParamDescriptoria ei oltu alustettu.");
  if (newDesc.Size() == SizeParams())
    *itsParamDescriptor = newDesc;
  else
    throw runtime_error(
        "NFmiQueryInfo::SetParamDescriptor - ei voi asettaa erikokoista descriptoria.");

  if (itsRefQueryData)  // pit�� laittaa sisuskalutkin kuntoon!!!
    itsRefQueryData->Info()->SetParamDescriptor(newDesc);
}

bool NFmiQueryInfo::FindNearestTime(const NFmiMetTime &theTime,
                                    FmiDirection theDirection,
                                    unsigned long theTimeRangeInMinutes)
{
  return itsTimeDescriptor->FindNearestTime(theTime, theDirection, theTimeRangeInMinutes);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if the given coordinate is in the data
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsInside(const NFmiPoint &theLatLon, double theRadius) const
{
  return itsHPlaceDescriptor->IsInside(theLatLon, theRadius);
}

// ----------------------------------------------------------------------
/*!
 * \brief Retrun true if the given time is inside the data
 */
// ----------------------------------------------------------------------

bool NFmiQueryInfo::IsInside(const NFmiMetTime &theTime) const
{
  return TimeDescriptor().IsInside(theTime);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiTimeBag &NFmiQueryInfo::ValidTimes() const
{
  NFmiTimeBag *bag = itsTimeDescriptor->ValidTimeBag();
  if (bag) return *bag;
  throw std::runtime_error("Attempt to access timebag from querydata without one would crash");
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiParamBag &NFmiQueryInfo::ParamBag() const
{
  NFmiParamBag *bag = itsParamDescriptor->ParamBag();
  if (bag) return *bag;
  throw std::runtime_error("Attempt to access parambag from querydata without one would crash");
}

// ======================================================================
