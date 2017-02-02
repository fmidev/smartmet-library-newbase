// ======================================================================
/*!
 * \file NFmiQueryDataUtil.h
 * \brief Interface of class NFmiQueryDataUtil
 */
// ======================================================================

#ifndef NFMIQUERYDATAUTIL_H
#define NFMIQUERYDATAUTIL_H

#include "NFmiDef.h"
#include "NFmiGlobals.h"
#include "NFmiMetTime.h"
#include "NFmiDataIdent.h"
#include "NFmiLevel.h"
#include "NFmiDataMatrix.h"
#include "NFmiArea.h"
#include "NFmiGrid.h"
#include <boost/shared_ptr.hpp>

#ifndef BOOST_DISABLE_THREADS

#ifdef _MSC_VER
#pragma warning( \
    disable : 4244 4267 4512)  // boost:in thread kirjastosta tulee ik�v�sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244 4267 4512)  // laitetaan 4244 takaisin p��lle, koska se on t�rke�
                                           // (esim. double -> int auto castaus varoitus)
#endif

#endif  // BOOST_DISABLE_THREADS

#include <vector>
#include <set>
#include <string>

class NFmiFastQueryInfo;
class NFmiQueryInfo;
class NFmiGrid;
class NFmiQueryData;
class NFmiTimeDescriptor;
class NFmiTimeBag;
class NFmiParamBag;
class NFmiPoint;
class NFmiString;
class NFmiRect;
class NFmiDataModifier;
class NFmiCalculationCondition;
class NFmiLocationBag;
class NFmiArea;
class NFmiLogger;

/*!
 * \class MyGrid
 * T�ss� pari apu luokkaa useiden hilojen yhdist�mist� varten.
 * En k�ytt�nyt NFmiGrid-luokkaa, koska sill� ja sen emolla ei
 * ollut kopi-konstruktoria ei sijoitus operaattoreita (enk� jaksanut tehd�).
 * K�ytet��n apuna mm. kun grib:eist� tehd��n qdataa ja kun yhdistell��n
 * monista qdatoista yksi uusi qdata.
 */
class MyGrid
{
 public:
  MyGrid(void) : itsArea(0), itsNX(0), itsNY(0) {}
  MyGrid(const NFmiArea *theArea, int theNX, int theNY)
      : itsArea(theArea ? theArea->Clone() : 0), itsNX(theNX), itsNY(theNY)
  {
  }

  MyGrid(const MyGrid &theGrid)
      : itsArea(theGrid.itsArea ? theGrid.itsArea->Clone() : 0),
        itsNX(theGrid.itsNX),
        itsNY(theGrid.itsNY)
  {
  }

  MyGrid(const NFmiGrid &theGrid)
      : itsArea(theGrid.Area() ? theGrid.Area()->Clone() : 0),
        itsNX(theGrid.XNumber()),
        itsNY(theGrid.YNumber())
  {
  }

  ~MyGrid(void) { delete itsArea; }
  MyGrid &operator=(const MyGrid &theGrid)
  {
    if (this != &theGrid)
    {
      delete itsArea;
      itsArea = theGrid.itsArea ? theGrid.itsArea->Clone() : 0;
      itsNX = theGrid.itsNX;
      itsNY = theGrid.itsNY;
    }
    return *this;
  }

  MyGrid &operator=(const NFmiGrid &theGrid)
  {
    delete itsArea;
    itsArea = theGrid.Area() ? theGrid.Area()->Clone() : 0;
    itsNX = theGrid.XNumber();
    itsNY = theGrid.YNumber();
    return *this;
  }
  bool operator==(const MyGrid &theGrid)
  {
    if (itsNX == theGrid.itsNX)
      if (itsNY == theGrid.itsNY)
      {
        if (itsArea == 0 && theGrid.itsArea == 0)
          return true;
        else if (itsArea && theGrid.itsArea)
        {
          if (itsArea->ClassId() == theGrid.itsArea->ClassId())
            if (*itsArea == *(theGrid.itsArea)) return true;
        }
      }
    return false;
  }

  bool operator<(const MyGrid &theGrid) const;

  bool operator==(const NFmiGrid &theGrid)
  {
    MyGrid tmpGrid(theGrid);
    return (*this == tmpGrid);
  }

  // private:
  NFmiArea *itsArea;
  unsigned long itsNX;
  unsigned long itsNY;
};

struct CombinedParamStruct
{  // t�m� structi piti tehd�, ett� sain v�hennetty� boost::in Thread:ille annettujen parametrin
  // m��r�� (max 9 parametria)
  CombinedParamStruct(void)
      : weather1(false), weather2(false), wind1(false), wind2(false), wind3(false)
  {
  }

  bool weather1;  // tehd��n weatherandcloudiness seuraavista: kFmiWeatherSymbol1,
                  // kFmiTotalCloudCover, kFmiCloudSymbol ja kFmiFogSymbol
  bool weather2;  // tehd��n weatherandcloudiness seuraavista: kFmiWeatherSymbol3
  bool wind1;     // tehd��n totalwind seuraavista: kFmiWindVectorMS
  bool wind2;     // tehd��n totalwind seuraavista: kFmiWindDirection ja kFmiWindSpeedMS
  bool wind3;     // tehd��n totalwind seuraavista: kFmiWindUMS ja kFmiWindVMS
};

static const NFmiRect gMissingCropRect(kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing);

// T�h�n rakenteeseen saadaan kaikki tieto mist� tahansa hilasta, ett�
// voidaan tehd� yhdistelyj�.
// K�ytet��n apuna mm. kun grib:eist� tehd��n qdataa ja kun yhdistell��n
// monista qdatoista yksi uusi qdata.
struct GridRecordData
{
  GridRecordData(void);
  void ChangeParam(const NFmiParam &theWantedParam);

  NFmiMetTime itsOrigTime;
  NFmiMetTime itsValidTime;
  NFmiDataIdent itsParam;
  NFmiLevel itsLevel;
  MyGrid itsOrigGrid;  // t�m� on datan originaali gridi
  MyGrid itsGrid;  // t�m� on haluttu datan gridi, joka yleensa sama kuin orig, mutta jos on haluttu
                   // cropata dataa, t�ss� croppi hila
  bool fDoProjectionConversion;
  NFmiDataMatrix<float> itsGridData;
  NFmiRect itsLatlonCropRect;        // t�h�n talletetaan mahdollinen croppi recti
  NFmiPoint itsGridPointCropOffset;  // t�h�n talletetaan mahdollinen cropin offset originaali
                                     // gridin hila yksik�ss�
  double itsMissingValue;
  bool fParamChanged;
  NFmiParam itsOrigParam;
};

class NFmiStopFunctor
{
 public:
  NFmiStopFunctor() : fStop(false) {}
  ~NFmiStopFunctor() {}
  bool Stop(void) const { return fStop; }
  void Stop(bool newValue) { fStop = newValue; }
 private:
  bool fStop;
};

class NFmiOperationProgress
{
 public:
  virtual ~NFmiOperationProgress() {}
  virtual void StepIt(void) = 0;
  virtual void SetRange(int low, int high, int stepCount) = 0;
  virtual void AddRange(int value) = 0;
  virtual bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0) = 0;
  virtual bool WaitUntilInitialized(void) = 0;
};

class NFmiThreadCallBacks
{
 public:
  NFmiThreadCallBacks(NFmiStopFunctor *theStopper = 0, NFmiOperationProgress *theProgress = 0)
      : itsStopper(theStopper), itsProgress(theProgress)
  {
  }

  bool Stop(void) const;
  void Stop(bool newValue);
  void StepIt(void);
  void SetRange(int low, int high, int stepCount);
  void AddRange(int value);
  bool DoPostMessage(unsigned int message, unsigned int wParam = 0, long lParam = 0);
  void CheckIfStopped(void);  // heitt�� poikkeuksen, jos lopetetaan
  bool WaitUntilInitialized(void);

 private:
  NFmiStopFunctor *itsStopper;
  NFmiOperationProgress *itsProgress;
};

class NFmiStopThreadException
{
 public:
  NFmiStopThreadException(void) {}
  ~NFmiStopThreadException(void) {}
};

// T�m� luokka laskee worker-threadi parvelle aina kulloisenkin 'vapaan' laskettavan timeIndeksin.
// T�lt� siis pyydet��n seuraav ty�stett�v� aika-indeksi mm.
// FillSingleTimeGridDataInThread-funktiossa.
class NFmiTimeIndexCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen t�h�n, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  NFmiTimeIndexCalculator(unsigned long theTimeSize);
  NFmiTimeIndexCalculator(unsigned long theStartTimeIndex, unsigned long theEndTimeIndex);
  bool GetCurrentTimeIndex(unsigned long &theTimeIndexOut);

 private:
  unsigned long itsStartTimeIndex;
  unsigned long itsEndTimeIndex;
  unsigned long itsCurrentTimeIndex;
  MutexType itsMutex;
  bool fNoMoreWork;

  NFmiTimeIndexCalculator(const NFmiTimeIndexCalculator &);  // ei toteuteta kopio konstruktoria
  NFmiTimeIndexCalculator &operator=(
      NFmiTimeIndexCalculator const &);  // ei toteuteta sijoitus operaattoria
};

// T�m� luokka laskee worker-threadi parvelle aina sopivan locationIndex-v�lin laskettavaksi.
// Idea on siis se ett� hila laskut jaetaan sopivan mitt�isiin p�tkiin esim. 50 pituisiin
// ja kukin worker threadi aina vuorollaan kysisi, mink� p�tk�n nyt voisi tehd�.
class NFmiLocationIndexRangeCalculator
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen t�h�n, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  NFmiLocationIndexRangeCalculator(unsigned long theLocationSize, unsigned long theChunkSize);
  NFmiLocationIndexRangeCalculator(unsigned long theStartLocationIndex,
                                   unsigned long theEndLocationIndex,
                                   unsigned long theChunkSize);

  bool GetCurrentLocationRange(unsigned long &theStartIndexOut, unsigned long &theEndIndexOut);

 private:
  unsigned long itsStartLocationIndex;
  unsigned long itsEndLocationIndex;
  unsigned long itsCurrentLocationIndex;
  unsigned long itsChunkSize;
  MutexType itsMutex;
  bool fNoMoreWork;

  NFmiLocationIndexRangeCalculator(
      const NFmiLocationIndexRangeCalculator &);  // ei toteuteta kopio konstruktoria
  NFmiLocationIndexRangeCalculator &operator=(
      NFmiLocationIndexRangeCalculator const &);  // ei toteuteta sijoitus operaattoria
};

//! Undocumented
class _FMI_DLL NFmiQueryDataUtil
{
 public:
  typedef checkedVector<GridRecordData *> GridDataVector;
  static void DestroyGridRecordData(GridDataVector &theGridRecordDatas);  // apufunktio, joka tuhoaa
                                                                          // dynaamisen datan ja
                                                                          // tyhjent�� vektorin

  /*!
   *  \class LimitChecker
   *  Tarkistaa ettei jokin laskettu arvo mene yli alle s��dettyjen rajojen.
   *  Tarpeen mm. Lagrange interpoloinnissa, jossa voi synty� uusia minimi ja maksimi
   *  arvoja. T�ll�in esim. tuulen suunnaksi voi tulla negatiivinen arvo tai yli 360 astetta.
   *  Halutessa my�s py�r�ytt�� ymp�ri eli jos arvoksi olisi tuulen suunnalle tulossa
   *  370, osaa laittaa arvoksi 10 eik� leikkaa 360:een. Jos taas kyseess� vaikka
   *  kosteusprosentti, pit�� yli sadan mene arvo leikata 100:n.
   */
  class _FMI_DLL LimitChecker
  {
   public:
    LimitChecker(float theLowerLimit, float theUpperLimit, bool theCircularValue = false);
    float GetInsideLimitsValue(float theValue) const;

    inline float LowerLimit(void) const { return itsLowerLimit; }
    inline void LowerLimit(float newValue)
    {
      itsLowerLimit = newValue;
      Update();
    }
    inline float UpperLimit(void) const { return itsUpperLimit; }
    inline void UpperLimit(float newValue)
    {
      itsUpperLimit = newValue;
      Update();
    }
    inline bool CircularValue(void) const { return fCircularValue; }
    inline void CircularValue(bool newValue)
    {
      fCircularValue = newValue;
      Update();
    }

   private:
    void Update(void);

    float itsLowerLimit;
    float itsUpperLimit;
    bool fCircularValue;       // esim tuulen suunta menee ymp�ri rajojen
    float itsLimitDifference;  // ympyr� laskuja varten
  };                           // class LimitChecker

 public:
  static void CheckIfStopped(NFmiStopFunctor *theStopFunctor);
  static void CheckIfStopped(NFmiThreadCallBacks *theThreadCallBacks);
  static void DoStepIt(NFmiThreadCallBacks *theThreadCallBacks);
  static void SetRange(NFmiThreadCallBacks *theThreadCallBacks, int low, int high, int stepCount);
  static void AddRange(NFmiThreadCallBacks *theThreadCallBacks, int value);

  template <typename T>
  static bool IsEqualEnough(T value1, T value2, T usedEpsilon)
  {
    if (::fabs(static_cast<double>(value1 - value2)) < usedEpsilon) return true;
    return false;
  }

  static NFmiQueryData *ReadNewestData(const std::string &theFileFilter);
  static void FillGridData(NFmiQueryData *theSource,
                           NFmiQueryData *theTarget,
                           unsigned long theStartTimeIndex = gMissingIndex,
                           unsigned long theEndTimeIndex = gMissingIndex,
                           NFmiLogger *theLogger = 0,
                           bool fUseOnlyOneThread = true);
  static void FillGridDataFullMT(NFmiQueryData *theSource,
                                 NFmiQueryData *theTarget,
                                 unsigned long theStartTimeIndex = gMissingIndex,
                                 unsigned long theEndTimeIndex = gMissingIndex,
                                 NFmiLogger *theDebugLogger = 0);
  static bool AreAreasEqual(const NFmiArea *theArea1, const NFmiArea *theArea2);
  static bool AreGridsEqual(const NFmiGrid *theGrid1, const NFmiGrid *theGrid2);

  static NFmiQueryData *GridQD2NewGridQD(NFmiQueryData *theSourceData,
                                         NFmiGrid *theWantedGridFormat);

  static NFmiQueryData *ExtractParams(NFmiQueryData *theSourceData,
                                      const NFmiParamBag &theWantedParams);

  static NFmiQueryData *ExtractLocations(NFmiQueryData *theSourceData,
                                         const NFmiLocationBag &theWantedLocations,
                                         FmiInterpolationMethod theInterpolationMethod);

  static NFmiQueryData *CombineParams(NFmiQueryData *theSourceData1, NFmiQueryData *theSourceData2);

  static NFmiQueryData *CombineLocations(NFmiQueryData *theSourceData1,
                                         NFmiQueryData *theSourceData2,
                                         int thePriorisedDataNumber);

  static NFmiQueryData *Interpolate2OtherGrid(NFmiQueryData *theSourceData,
                                              const NFmiGrid *theWantedGrid,
                                              NFmiLogger *theLogger = 0);

  static NFmiQueryData *ExtractTimes(const NFmiQueryData *theSourceData,
                                     const NFmiTimeDescriptor &theWantedTimeDesc);

  static NFmiQueryData *InterpolateTimes(NFmiQueryData *theSourceData,
                                         int theTimeResolutionInMinutes,
                                         int theStartTimeResolutionInMinutes,
                                         NFmiTimeBag *thePossibleReferenceTimeBag = 0,
                                         int theMaxTimeSearchRangeInMinutes = 360,
                                         FmiInterpolationMethod theInterpolationMethod = kLinearly);

  static NFmiQueryData *Area1QDOverArea2QD(NFmiQueryData *areaData1, NFmiQueryData *areaData2);

  static NFmiTimeDescriptor NewTimeDescriptor(NFmiQueryData *theSourceData,
                                              int theTimeResolutionInMinutes);

  //! Aikaresoluution pit�� olla sama
  static bool OverWriteTimesFromFile(NFmiFastQueryInfo &theLongerInfo,
                                     NFmiFastQueryInfo &theShorterInfo);

  static NFmiQueryData *SpaceOutData(NFmiFastQueryInfo &theSourceInfo,
                                     const NFmiPoint &newGridSize,
                                     NFmiDataModifier *theModifier,
                                     int theModifierId,
                                     double theAreaFactor,
                                     double theRange,
                                     const NFmiRect &theDataGatheringRange,
                                     const NFmiCalculationCondition &theMaskCondition);

  static NFmiQueryData *CombineTimes(NFmiFastQueryInfo &theSourceInfo1,
                                     NFmiFastQueryInfo &theSourceInfo2,
                                     int theMaxTimeBagSizeInNewData,
                                     int theStartTimeFunction,
                                     int theEndTimeFunction,
                                     int theOriginTimeFunction,
                                     bool fMakeChangingTimeResolution);

  static NFmiQueryData *CombineTimes(checkedVector<std::string> &theFileNames,
                                     int theMaxTimesInNewData);

  static NFmiQueryData *MakeCombineParams(NFmiFastQueryInfo &theSourceInfo,
                                          double theWantedInfoVersion,
                                          bool fKeepCloudSymbolParameter,
                                          bool fDoTotalWind,
                                          bool fDoWeatherAndCloudiness,
                                          FmiParameterName theWindGustParId,
                                          bool fAllowLessParamsWhenCreatingWeather,
                                          int theMaxUsedThreadCount,
                                          bool fDoaccuratePrecip,
                                          bool fForceTimeBag = false);

  static NFmiQueryData *MakeCombineParams(NFmiFastQueryInfo &theSourceInfo,
                                          double theWantedInfoVersion,
                                          bool fKeepCloudSymbolParameter,
                                          bool fDoTotalWind,
                                          bool fDoWeatherAndCloudiness,
                                          FmiParameterName theWindGustParId,
                                          const std::vector<int> &thePrecipFormParIds,
                                          bool fAllowLessParamsWhenCreatingWeather,
                                          int theMaxUsedThreadCount,
                                          bool fDoaccuratePrecip,
                                          bool fForceTimeBag = false);

  static NFmiQueryData *CreateEmptyData(NFmiQueryInfo &srcInfo);
  static NFmiQueryData *CreateEmptyData(NFmiQueryInfo &srcInfo,
                                        const std::string &theFilename,
                                        bool fInitialize);

  //@{ \name datan konversiot 6:sta 7:aan tai p�invastoin -funktiot
  static NFmiQueryData *FqdV6ToV7(NFmiFastQueryInfo &theSourceInfo);
  static NFmiQueryData *FqdV7ToV6(NFmiFastQueryInfo &theSourceInfo);
  //@}

  static NFmiQueryData *DoTimeFiltering(NFmiQueryData *theSourceData,
                                        int theFilterId,
                                        int theTimeOffsetStartInMinutes,
                                        int theTimeOffsetEndInMinutes,
                                        double theAdditionalParam1 = 1,
                                        double theAdditionalParam2 = 1);

  static NFmiQueryData *DoAreaFiltering(NFmiQueryData *theSourceData,
                                        int theFilterId,
                                        int theAreaType,
                                        double theAdditionalParam1 = 1,
                                        double theAdditionalParam2 = 1);

  // ********************************************************************************
  // ***************  NowcastFilter osuus on nyt t��ll�!  ***************************
  // ********************************************************************************
  static bool DoNowCastFiltering(const NFmiString &theData1FileName,
                                 const NFmiString &theData2FileName,
                                 const NFmiString &theOutputFileName,
                                 int theTimeRangeForPureData);

  static NFmiQueryData *DoNowCastFiltering(NFmiFastQueryInfo &theInfo1,
                                           NFmiFastQueryInfo &theInfo2,
                                           int theTimeRangeForPureData);

  // ********************************************************************************
  // ***************  NowcastFilter osuus on nyt t��ll�!  ***************************
  // ********************************************************************************

  static const NFmiString &GetOfficialQueryDataProdIdsKey(void);

  static NFmiQueryData *QDCrop(
      NFmiFastQueryInfo &theInfo, int theLeft, int theTop, int theRight, int theBottom);

  static std::string GetFileFilterDirectory(const std::string &theFileFilter);
  static NFmiQueryData *CombineQueryDatas(
      bool fDoRebuild,
      boost::shared_ptr<NFmiQueryData> &theBaseQData,
      std::vector<boost::shared_ptr<NFmiQueryData> > &theQDataVector,
      bool fDoTimeStepCombine,
      int theMaxTimeStepsInData = 0,
      NFmiStopFunctor *theStopFunctor = 0);
  static NFmiQueryData *CombineQueryDatas(bool fDoRebuildCheck,
                                          const std::string &theBaseDataFileFilter,
                                          const std::string &theFileFilter,
                                          bool fDoTimeStepCombine,
                                          int theMaxTimeStepsInData = 0,
                                          NFmiStopFunctor *theStopFunctor = 0);
  static int CalcOptimalThreadCount(int maxAvailableThreads, int separateTaskCount);

};  // class NFmiQueryDataUtil

#endif  // NFMIQUERYDATAUTIL_H

// ======================================================================
