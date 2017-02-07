// ======================================================================
/*!
 * \file NFmiCombinedParam.h
 * \brief Interface of class NFmiCombinedParam
 */
// ======================================================================

#ifndef NFMICOMBINEDPARAM_H
#define NFMICOMBINEDPARAM_H

#include "NFmiGlobals.h"
#include "NFmiParameterName.h"
#include "NFmiProducer.h"
#include "NFmiMetTime.h"

//! Undocumented
enum FmiWeatherParamType
{
  kFmiPackedWeather,
  kFmiHessaaWeather
};

//! Undocumented
enum FmiWindParamType
{
  kFmiPackedWind,
  kFmiWindVector,
  kFmiDirectionAndSpeed,
  kFmiUVComponents
};

// T�RKE��!!! T�ss� selontekoa virheest�, jota ei voi en�� inhimillisin ponnistuksin korjata.
// Aluperin tarkoitus olisi ollut ett� kTCombinedWeatherMissing olisi 0xFFFFFFFF, eli kaikki
// bitit olisivat 1:i�. T�st� seurasi ongelma, koska 0xFFFFFFFF on my�s NaN. Tarkoitus oli
// korjata se niin ett� puuttuvaksi arvoksi tulisi luku miss� kaikki muut olisivat 1:i� paitsi
// merkitsevin olisi 0. Virheellisesti t�t� lukua luultiin 0xEFFFFFFF:ksi. Vasta my�hemmin paljastui
// ett� ykk�seksi menikin 3. merkitsevin bitti. Oikea haluttu puuttuva arvo olisi ollut 0x7FFFFFFF.
// T�t� ei voi en�� j�lkik�teen korjata, koska siit� seuraisi vanhojen ohjelmien ja arkisto datojen
// kanssa potentiaalisesti vaikka mink�laisia ongelmia. Siksi on k�yt�ss� kWindGustV7Missing
// joka yritt�� hanskata ongelma bitteja oikein, mutta sit� pit�� k�ytt�� yhdess� kT12BitMissing
// kanssa.

const unsigned long kTCombinedWeatherMissing = 0xefffffff;

// g++ herjaa tuosta castista. Laskin vastaavan desimaaliluvun k�ytt�en
// osoitetta http://babbage.cs.qc.edu/IEEE-754/32bit.html
// monissa paikoissa pit�� tarkistaa, onko queryinfolta saatu arvo t�m� puuttuva arvo
#if 0
  const float kTCombinedWeatherFloatMissing = *reinterpret_cast<const float*>(&kTCombinedWeatherMissing);
#else
const float kTCombinedWeatherFloatMissing = -1.5845631558379571e+29;
#endif

const unsigned long kT2BitMissing = 3;
const unsigned long kT3BitMissing = 7;
const unsigned long kT4BitMissing = 15;
const unsigned long kT5BitMissing = 31;
const unsigned long kT6BitMissing = 63;
const unsigned long kT9BitMissing = 511;
const unsigned long kWindGustV7Missing = 3071;  // t�m� ei voinut olla 12-bit missing, koska
// ylimm�isen hexan viimeinen arvo ei voi olla f vaan se pit�� laittaa e:ksi
// ja t�ll�in windgust missingiksi pit�� laittaa 3071. Yst. Terv. Marko
const unsigned long kT12BitMissing = 4095;
const unsigned long kTThunderProbabilityMissing = 14;

const unsigned long kTNoPrecipitation = 0;
const unsigned long kTSomePrecipitation = 1;
const unsigned long kTModeratePrecipitation = 2;
const unsigned long kTMuchPrecipitation = 3;

const unsigned long kTSkyClear = 0;
const unsigned long kTPartlyCloudy = 3;
const unsigned long kTHalfCloudy = 5;
const unsigned long kTNearlyCloudy = 7;
const unsigned long kTOverCast = 10;

const unsigned long FmiNumberOfPrecipitationForms = 7;
const unsigned long kTHail = 6;
const unsigned long kTFreezingRain = 5;
const unsigned long kTFreezingDrizzle = 4;
const unsigned long kTSnow = 3;
const unsigned long kTSleet = 2;
const unsigned long kTRain = 1;
const unsigned long kTDrizzle = 0;

const unsigned long kTLargeScalePrecipitation = 1;
const unsigned long kTConvectivePrecipitation = 2;

const unsigned long kTVariableWind = 62;

const unsigned long kTNoFog = 0;
const unsigned long kTModerateFog = 1;
const unsigned long kTDenseFog = 2;

class NFmiDataIdent;
class NFmiParamBag;
class NFmiQueryInfo;
class NFmiPoint;
class NFmiParamDataModifierList;
class NFmiDataModifier;
class NFmiDataModifierCombi;
class NFmiIntegrationSelector;

//! Undocumented
class _FMI_DLL NFmiCombinedParam
{
 public:
  virtual ~NFmiCombinedParam(void);
  NFmiCombinedParam(double theInfoVersion = 7.);
  NFmiCombinedParam(const NFmiCombinedParam& theParam);
  NFmiCombinedParam& operator=(const NFmiCombinedParam& theParam);

  unsigned long ConvertFloatToLong(float theValue);
  virtual NFmiCombinedParam* CreateNew(float theInitValue);

  virtual bool TransformFromFloatValue(float theValue);
  virtual float TransformedFloatValue(void);
  virtual bool LongValue(unsigned long theValue) = 0;
  virtual unsigned long LongValue(void) const = 0;
  virtual bool SubValue(double theValue, FmiParameterName theParam) = 0;
  virtual double SubValue(FmiParameterName theParam, NFmiIntegrationSelector* theSelector = 0) = 0;
  virtual double RawSubValue(FmiParameterName theParam) = 0;
  virtual NFmiCombinedParam* Clone(void) const = 0;
  virtual bool IsMemberParam(FmiParameterName type) const = 0;
  virtual bool SetToWeightedMean(NFmiCombinedParam* theWeatherAndCloudiness1,
                                 float fac1,
                                 NFmiCombinedParam* theWeatherAndCloudiness2,
                                 float fac2,
                                 NFmiCombinedParam* theWeatherAndCloudiness3 = 0,
                                 float fac3 = kFloatMissing,
                                 NFmiCombinedParam* theWeatherAndCloudiness4 = 0,
                                 float fac4 = kFloatMissing) = 0;

  virtual bool SetToWeightedPeriod(NFmiQueryInfo* info,
                                   const NFmiPoint& theLonLat,
                                   unsigned long period,
                                   bool considerPrecipitationMax,
                                   float factor1 = 1.,
                                   float factor2 = 1.,
                                   float factor3 = 1.) = 0;

  virtual bool SetToWeightedPeriod(NFmiQueryInfo* info,
                                   const NFmiPoint& theLonLat,
                                   const NFmiMetTime& startTime,
                                   const NFmiMetTime& endTime,
                                   bool considerPrecipitationMax,
                                   float factor1 = 1.,
                                   float factor2 = 1.,
                                   float factor3 = 1.) = 0;

  virtual void InitIntegration(void);
  virtual void ClearIntegration(void);
  virtual void Integrate(float theValue);
  void EndIntegration(void);

  bool SetSubIntegrator(FmiParameterName theSubParamName, NFmiDataModifierCombi* theModifier);
  NFmiDataModifierCombi* SubIntegrator(FmiParameterName theSubParamName);

  double InfoVersion(void) const;
  void InfoVersion(double newValue);

 protected:
  int FindSubParamIntegratorIndex(FmiParameterName theName);
  virtual void DeleteIntegrators(void);
  virtual void CreateIntegrators(void);

 protected:
  virtual void CrossCheck(FmiParameterName theParam);
  NFmiDataModifierCombi* GetSubIntegrator(unsigned long theIndex);
  virtual void CreateSubParams(void);
  NFmiDataModifierCombi* FindSubParamIntegrator(FmiParameterName theName);
  NFmiParamBag* itsSubParams;
  NFmiDataModifierCombi** itsIntegrators;

  bool fIntegrationMode;
  bool fIntegrationReady;
  bool fIntegrationStarted;
  bool fDataOk;
  bool fAutoUpdate;

  // info version pit�� ottaa my�s t�nne, koska totalwind ja weatherandcloudiness
  // tarvitsevat tietoa kun kaavat muuttuivat
  // talletettiin varmuuden vuoksi double:na, jos haluaa k�ytt�� desimaaleja joskus

  double itsInfoVersion;

};  // class NFmiCombinedParam

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline double NFmiCombinedParam::InfoVersion(void) const { return itsInfoVersion; }
// ----------------------------------------------------------------------
/*!
 * \param newValue Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiCombinedParam::InfoVersion(double newValue) { itsInfoVersion = newValue; }
#endif  // NFMICOMBINEDPARAM_H

// ======================================================================
