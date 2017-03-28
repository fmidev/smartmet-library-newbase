// ======================================================================
/*!
 * \file NFmiInfoAreaMask.h
 * \brief Interface of class NFmiInfoAreaMask
 */
// ======================================================================

#pragma once

#include "NFmiAreaMaskImpl.h"
#include "NFmiDataIdent.h"
#include "NFmiGrid.h"  // täältä tulee NFmiTimeCache-luokka (tiedän, pitäisi jakaa newbase:a osiin)
#include "NFmiLevel.h"
#include "NFmiMetTime.h"

class NFmiDataModifier;

// Perus areamask-luokka.
// Sisältää myös listan mahdollisiin 'ali'-maskeihin. Niihin yhteys vain
// indeksin kautta. Ylimaski on indeksillä 1 ja muut seuraavat numerojärjestyksessä.

//! Undocumented

class _FMI_DLL NFmiInfoAreaMask : public NFmiAreaMaskImpl
{
 public:
  ~NFmiInfoAreaMask() override;
  NFmiInfoAreaMask();
  NFmiInfoAreaMask(const NFmiCalculationCondition &theOperation,
                   Type theMaskType,
                   NFmiInfoData::Type theDataType,
                   const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                   BinaryOperator thePostBinaryOperator = kNoValue);

  NFmiInfoAreaMask(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                   BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMask(const NFmiInfoAreaMask &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double HeightValue(double theHeight, const NFmiCalculationParams &theCalculationParams) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;
  // Pitää olla ei virtuaalinen versio PressureValue-metodista, jotta tietyissä tilanteissa estetään
  // mahdollisen lapsiluokan metodin virtuaalinen kutsu
  double PressureValueStatic(double thePressure, const NFmiCalculationParams &theCalculationParams);
  double HeightValueStatic(double theHeight, const NFmiCalculationParams &theCalculationParams);

  // erikoistapaus optimoituun käyttöön (ei voi käyttää kaikille luokille!!!!)
  bool IsMasked(int theIndex) const override;
  bool IsMasked(const NFmiPoint &theLatLon) const override
  {
    return NFmiAreaMaskImpl::IsMasked(theLatLon);
  }
  bool Time(const NFmiMetTime &theTime) override;
  bool IsWantedParam(const NFmiDataIdent &theParam, const NFmiLevel *theLevel = 0) const override;
  boost::shared_ptr<NFmiFastQueryInfo> Info() override { return itsInfo; };
  void Info(const boost::shared_ptr<NFmiFastQueryInfo> &newInfo);
  void UpdateInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo) override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiDataIdent *DataIdent() const override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiParam *Param() const override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  const NFmiLevel *Level() const override;
  void Level(const NFmiLevel &theLevel) override;

  // tehty virtuaaliseksi, koska perusluokassa ei ole valmiuksia paaluttaa arvoa tähän
  bool UseLevelInfo() const override;
  bool UsePressureLevelInterpolation() const override { return fUsePressureLevelInterpolation; }
  void UsePressureLevelInterpolation(bool newValue) override
  {
    fUsePressureLevelInterpolation = newValue;
  }
  double UsedPressureLevelValue() const override { return itsUsedPressureLevelValue; }
  void UsedPressureLevelValue(double newValue) override { itsUsedPressureLevelValue = newValue; }

 protected:
  double CalcValueFromLocation(const NFmiPoint &theLatLon) const override;
  const NFmiString MakeSubMaskString() const override;

 protected:
  boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
  NFmiDataIdent itsDataIdent;
  NFmiLevel itsLevel;
  NFmiMetTime itsTime;  // jos vanhassa maski jutussa tarvitaan aikainterpolointia, tässä on
                        // interpoloitava aika
  bool fIsTimeIntepolationNeededInValue;  // erikois optimointia Value-metodin ja Time-metodin
                                          // käytössä
  bool fUsePressureLevelInterpolation;    // tämä asetetaan trueksi jos tarvitaan tehdä lennossa
                                          // painepinta interpolointeja
  double itsUsedPressureLevelValue;  // jos fUsePressureLevelInterpolation on true, käytetään
                                     // laskuissa tätä painepintaa

 private:
  NFmiInfoAreaMask &operator=(const NFmiInfoAreaMask &theMask);

};  // class NFmiInfoAreaMask

//! Tämä luokka toimii kuten NFmiInfoAreaMask mutta kurkkaa halutun x-y hila pisteen yli arvoa
class _FMI_DLL NFmiInfoAreaMaskPeekXY : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskPeekXY() override;
  NFmiInfoAreaMaskPeekXY();
  NFmiInfoAreaMaskPeekXY(const NFmiCalculationCondition &theOperation,
                         Type theMaskType,
                         NFmiInfoData::Type theDataType,
                         const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         int theXOffset,
                         int theYOffset,
                         BinaryOperator thePostBinaryOperator = kNoValue);

  NFmiInfoAreaMaskPeekXY(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         int theXOffset,
                         int theYOffset,
                         BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskPeekXY(const NFmiInfoAreaMaskPeekXY &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  int itsXOffset;  // kuinka monen hilapisteen yli kurkataan x-suunnassa
  int itsYOffset;  // kuinka monen hilapisteen yli kurkataan y-suunnassa
 private:
  void CalcGridDiffs();

  double itsGridXDiff;  // mikä on x-hilaväli xy-koordinaatistossa
  double itsGridYDiff;  // mikä on y-hilaväli xy-koordinaatistossa

 private:
  NFmiInfoAreaMaskPeekXY &operator=(const NFmiInfoAreaMaskPeekXY &theMask);

};  // class NFmiInfoAreaMaskPeekXY

//! Tämä luokka toimii kuten NFmiInfoAreaMaskPeekXY käyttää kurkkaukseen editoritavan-datan tai
//! macroParam-datan hilaväliä, tällöin voidaan käyttää uutta GridSizeX ja Y funktioita
class _FMI_DLL NFmiInfoAreaMaskPeekXY2 : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskPeekXY2() override;
  NFmiInfoAreaMaskPeekXY2();
  NFmiInfoAreaMaskPeekXY2(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          boost::shared_ptr<NFmiFastQueryInfo> theEditedInfo,
                          int theXOffset,
                          int theYOffset,
                          BinaryOperator thePostBinaryOperator = kNoValue);

  NFmiInfoAreaMaskPeekXY2(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          boost::shared_ptr<NFmiFastQueryInfo> theEditedInfo,
                          int theXOffset,
                          int theYOffset,
                          BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskPeekXY2(const NFmiInfoAreaMaskPeekXY2 &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  int itsXOffset;  // kuinka monen hilapisteen yli kurkataan x-suunnassa
  int itsYOffset;  // kuinka monen hilapisteen yli kurkataan y-suunnassa
 private:
  boost::shared_ptr<NFmiFastQueryInfo>
      itsEditedInfo;  // Tästä katsotaan vain haluttu siirtymä matka

  NFmiInfoAreaMaskPeekXY2 &operator=(const NFmiInfoAreaMaskPeekXY2 &theMask);

};  // class NFmiInfoAreaMaskPeekXY2

//! Tämä luokka toimii kuten NFmiInfoAreaMaskPeekXY ja  käyttää kurkkaukseen haluttuja kilometri
//! etäisyyksiä
class _FMI_DLL NFmiInfoAreaMaskPeekXY3 : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskPeekXY3() override;
  NFmiInfoAreaMaskPeekXY3();
  NFmiInfoAreaMaskPeekXY3(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          boost::shared_ptr<NFmiFastQueryInfo> theEditedInfo,
                          double theXOffsetInKM,
                          double theYOffsetInKM,
                          BinaryOperator thePostBinaryOperator = kNoValue);

  NFmiInfoAreaMaskPeekXY3(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          boost::shared_ptr<NFmiFastQueryInfo> theEditedInfo,
                          double theXOffsetInKM,
                          double theYOffsetInKM,
                          BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskPeekXY3(const NFmiInfoAreaMaskPeekXY3 &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;

 protected:
  NFmiCalculationParams MakeModifiedCalculationParams(
      const NFmiCalculationParams &theCalculationParams);

  double itsXOffsetInKM;  // kuinka monen kilometrin yli kurkataan x-suunnassa
  double itsYOffsetInKM;  // kuinka monen kilometrin yli kurkataan y-suunnassa
 private:
  boost::shared_ptr<NFmiFastQueryInfo>
      itsEditedInfo;  // Tästä katsotaan vain haluttu siirtymä matka

  NFmiInfoAreaMaskPeekXY3 &operator=(const NFmiInfoAreaMaskPeekXY2 &theMask);

};  // class NFmiInfoAreaMaskPeekXY3

class _FMI_DLL NFmiInfoAreaMaskMetFuncBase : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskMetFuncBase() override;
  NFmiInfoAreaMaskMetFuncBase(const NFmiCalculationCondition &theOperation,
                              Type theMaskType,
                              NFmiInfoData::Type theDataType,
                              const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                              bool thePeekAlongTudes,
                              MetFunctionDirection theMetFuncDirection,
                              BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskMetFuncBase(const NFmiInfoAreaMaskMetFuncBase &theOther);
  void Initialize() override;

  bool Time(const NFmiMetTime &theTime) override;

 protected:
  typedef std::vector<std::pair<int, float> > CalcFactorVector;

  virtual void SetDividers() = 0;
  virtual void InitCalcFactorVectors() = 0;
  virtual const CalcFactorVector &LowerEdgeFactors() const = 0;
  virtual const CalcFactorVector &UpperEdgeFactors() const = 0;
  virtual const CalcFactorVector &MiddleAreaFactors() const = 0;

  NFmiLocationCache CalcLocationCache(const NFmiPoint &theLatlon);
  void SetGridSizeVariables();
  bool IsDataOperatable(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo) const;
  float Peek(const NFmiLocationCache &theLocationCachePoint, int offsetX, int offsetY);
  NFmiLocationCache CalcPeekedLocation(const NFmiLocationCache &theLocationCachePoint,
                                       int theOffsetX,
                                       int theOffsetY);
  float CalcMetFuncWithFactors(const NFmiLocationCache &theLocationCachePoint,
                               bool fDoX,
                               const CalcFactorVector &theCalcFactors);
  float CalcUpperEdgeMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcLowerEdgeMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcMiddleAreaMetFunc(const NFmiLocationCache &theLocationCachePoint, bool fDoX);
  float CalcMetFuncComponent(const NFmiLocationCache &theLocationCachePoint,
                             bool fDoX,
                             double theRealGridPointComponent,
                             int theGridSizeComponent);

  NFmiTimeCache
      itsTimeCache;  // tähän lasketaan time-metodissa optimoinnissa käytetty aika-cache laskuri
  int itsGridSizeX;
  int itsGridSizeY;
  float itsGridPointWidthInMeters;
  float itsGridPointHeightInMeters;
  float itsMetFuncDividerX;  // tähän lasketaan luku, millä jaetaan laskut X-suunnassa
  float itsMetFuncDividerY;  // tähän lasketaan luku, millä jaetaan laskut Y-suunnassa
  bool fTotalWindParam;  // jos parametrina on total wind- itää se ottaa tietyissä tapauksissa
                         // erikseen huomioon laskuissa
  bool fPeekAlongTudes;  // tehdäänkö xy-suuntaiset kurkkaukset datan oman hilan suunnassa (false)
                         // vaiko pitkin latitude ja longitude linjoja (true)
 private:
  NFmiInfoAreaMaskMetFuncBase &operator=(const NFmiInfoAreaMaskMetFuncBase &theMask);
};

//! Tämä luokka laskee gradientin tai divergenssin.
class _FMI_DLL NFmiInfoAreaMaskGrad : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskGrad() override;
  NFmiInfoAreaMaskGrad(const NFmiCalculationCondition &theOperation,
                       Type theMaskType,
                       NFmiInfoData::Type theDataType,
                       const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       bool thePeekAlongTudes,
                       MetFunctionDirection theMetFuncDirection,
                       BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskGrad(const NFmiInfoAreaMaskGrad &theOther);
  NFmiAreaMask *Clone() const override;

  bool CalculateDivergence() const { return fCalculateDivergence; }
  void CalculateDivergence(bool newValue) { fCalculateDivergence = newValue; }
  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  double PressureValue(double thePressure,
                       const NFmiCalculationParams &theCalculationParams) override;
  void InitCalcFactorVectors() override;

 protected:
  void SetDividers() override;
  const CalcFactorVector &LowerEdgeFactors() const override { return itsLowerEdgeFactors; }
  const CalcFactorVector &UpperEdgeFactors() const override { return itsUpperEdgeFactors; }
  const CalcFactorVector &MiddleAreaFactors() const override { return itsMiddleAreaFactors; }

 private:
  static CalcFactorVector itsLowerEdgeFactors;
  static CalcFactorVector itsUpperEdgeFactors;
  static CalcFactorVector itsMiddleAreaFactors;
  bool fCalculateDivergence;  // tämä muuttuja määrää lasketaanko gradientti (oletus) vai
                              // divergenssi. Laskenta on muuten sama,
                              // mutta x- ja y-componentit lasketaan vain erilailla lopuksi yhteen

  static bool fCalcFactorVectorsInitialized;

  NFmiInfoAreaMaskGrad &operator=(const NFmiInfoAreaMaskGrad &theMask);

};  // class NFmiInfoAreaMaskGrad

class _FMI_DLL NFmiInfoAreaMaskAdvection : public NFmiInfoAreaMaskGrad
{
 public:
  ~NFmiInfoAreaMaskAdvection() override;
  NFmiInfoAreaMaskAdvection(const NFmiCalculationCondition &theOperation,
                            Type theMaskType,
                            NFmiInfoData::Type theDataType,
                            const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                            boost::shared_ptr<NFmiFastQueryInfo> theInfoUwind,
                            boost::shared_ptr<NFmiFastQueryInfo> theInfoVwind,
                            bool thePeekAlongTudes,
                            MetFunctionDirection theMetFuncDirection,
                            BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskAdvection(const NFmiInfoAreaMaskAdvection &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 private:
  void SetupWindComponents();

  boost::shared_ptr<NFmiFastQueryInfo> itsInfoUwind;  // Tästä katsotaan tuulen u-komponentti
  boost::shared_ptr<NFmiFastQueryInfo> itsInfoVwind;  // Tästä katsotaan tuulen v-komponentti
};

//! Tämä luokka laskee laplacen (eli nablan?).
class _FMI_DLL NFmiInfoAreaMaskLaplace : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskLaplace() override;
  NFmiInfoAreaMaskLaplace(const NFmiCalculationCondition &theOperation,
                          Type theMaskType,
                          NFmiInfoData::Type theDataType,
                          const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          bool thePeekAlongTudes,
                          MetFunctionDirection theMetFuncDirection,
                          BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskLaplace(const NFmiInfoAreaMaskLaplace &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  void InitCalcFactorVectors() override;

 protected:
  void SetDividers() override;
  float CalcLaplaceX(const NFmiLocationCache &theLocationCachePoint);
  float CalcLaplaceY(const NFmiLocationCache &theLocationCachePoint);

  const CalcFactorVector &LowerEdgeFactors() const override { return itsLowerEdgeFactors; }
  const CalcFactorVector &UpperEdgeFactors() const override { return itsUpperEdgeFactors; }
  const CalcFactorVector &MiddleAreaFactors() const override { return itsMiddleAreaFactors; }

 private:
  static CalcFactorVector itsLowerEdgeFactors;
  static CalcFactorVector itsUpperEdgeFactors;
  static CalcFactorVector itsMiddleAreaFactors;
  static bool fCalcFactorVectorsInitialized;

  NFmiInfoAreaMaskLaplace &operator=(const NFmiInfoAreaMaskLaplace &theMask);

};  // class NFmiInfoAreaMaskLaplace

class _FMI_DLL NFmiInfoAreaMaskRotor : public NFmiInfoAreaMaskGrad
{
 public:
  ~NFmiInfoAreaMaskRotor() override;
  NFmiInfoAreaMaskRotor(const NFmiCalculationCondition &theOperation,
                        Type theMaskType,
                        NFmiInfoData::Type theDataType,
                        const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        bool thePeekAlongTudes,
                        MetFunctionDirection theMetFuncDirection,
                        BinaryOperator thePostBinaryOperator = kNoValue);
  NFmiInfoAreaMaskRotor(const NFmiInfoAreaMaskRotor &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
};

class _FMI_DLL NFmiInfoAreaMaskVertFunc : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoAreaMaskVertFunc() override;
  NFmiInfoAreaMaskVertFunc(const NFmiCalculationCondition &theOperation,
                           Type theMaskType,
                           NFmiInfoData::Type theDataType,
                           const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                           NFmiAreaMask::FunctionType thePrimaryFunc,
                           NFmiAreaMask::FunctionType theSecondaryFunc,
                           int theArgumentCount);
  NFmiInfoAreaMaskVertFunc(const NFmiInfoAreaMaskVertFunc &theOther);
  NFmiAreaMask *Clone() const override;
  void Initialize() override;
  void SetArguments(std::vector<float> &theArgumentVector) override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  void FindCalculatedLeves(const NFmiLocationCache &theLocationCache);
  void SearchLevels(const NFmiLocationCache &theLocationCache);
  void SetLevelValues();
  float DoGetFunction(const NFmiLocationCache &theLocationCache,
                      const NFmiCalculationParams &theCalculationParams,
                      float theLevelValue);
  float DoVerticalGrad(const NFmiLocationCache &theLocationCache,
                       const NFmiCalculationParams &theCalculationParams);
  float DoFindFunction(const NFmiLocationCache &theLocationCache);
  void SetLevelIndexies(float theHeightValue);
  unsigned long GetNonMissingStartLevelIndex(const NFmiLocationCache &theLocationCache);
  float GetLevelHeightValue(const NFmiLocationCache &theLocationCache);
  float DoNormalFunction(const NFmiLocationCache &theLocationCache);

  NFmiAreaMask::FunctionType itsPrimaryFunc;    // esim. Avg, Max, Get, Find, jne.
  NFmiAreaMask::FunctionType itsSecondaryFunc;  // esim. VertP, VertZ, jne.
  std::vector<float> itsArgumentVector;

  boost::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;   // tämä luodaan itsPrimaryFunc-dataosan mukaan
  float itsStartLevelValue;  // tähän otetaan annetusta argumentti listasta aloitus korkeus (missä
                             // yksikössä onkaan)
  float itsEndLevelValue;  // tähän otetaan annetusta argumentti listasta lopetus korkeus (missä
  // yksikössä onkaan), PAITSI, jos kyse on get-funktiosta, jolloin tämä on
  // puuttuva
  unsigned long itsStartLevelIndex;  // dataa käydään läpi alkaen tästä levelistä
  unsigned long itsEndLevelIndex;    // dataa käydään läpi tähän leveliin asti

  FmiParameterName itsUsedHeightParId;
  bool fReturnHeightValue;
  int itsLevelIncrement;  // kun ollaan päätelty mihin suuntaan leveldata menee (ylös maanpinnasta
                          // vai alas avaruudesta)
  // tähän on tarkoitus laskea for-looppeja varten level incrementti (joko 1 tai -1)
  // Jos datan levelien suunta on maanpinnasta ylöspäin, on incrementti 1 ja käydään levelit
  // normaali järjestyksessä läpi.
  // Jos datan levelien suuntä on avaruudesta maanpintaa kohden, on incrementti -1 ja levelit
  // käydään käänteisessä järjestyksessä.
  bool fReverseLevels;  // Jos itsLevelIncrement on -1, tämä on true, jolloin for-loopitus tehdään
                        // käänteisessä järjestyksessä

 private:
  NFmiInfoAreaMaskVertFunc &operator=(const NFmiInfoAreaMaskVertFunc &theMask);
};

class _FMI_DLL NFmiInfoAreaMaskVertConditionalFunc : public NFmiInfoAreaMaskVertFunc
{
 public:
  ~NFmiInfoAreaMaskVertConditionalFunc() override;
  NFmiInfoAreaMaskVertConditionalFunc(const NFmiCalculationCondition &theOperation,
                                      Type theMaskType,
                                      NFmiInfoData::Type theDataType,
                                      const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                      NFmiAreaMask::FunctionType thePrimaryFunc,
                                      NFmiAreaMask::FunctionType theSecondaryFunc,
                                      int theArgumentCount);
  NFmiInfoAreaMaskVertConditionalFunc(const NFmiInfoAreaMaskVertConditionalFunc &theOther);
  NFmiAreaMask *Clone() const override;
  void Initialize() override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  bool InitializeFromArguments();
  bool CheckProbabilityCondition(double value);
  float DoFindConditionalFunction(const NFmiLocationCache &theLocationCache);

  double itsLimit1;
  double itsLimit2;

 private:
  NFmiInfoAreaMaskVertConditionalFunc &operator=(
      const NFmiInfoAreaMaskVertConditionalFunc &theMask);
};

class NFmiInfoAreaMaskTimeVertFunc : public NFmiInfoAreaMaskVertFunc
{
 public:
  ~NFmiInfoAreaMaskTimeVertFunc() override;
  NFmiInfoAreaMaskTimeVertFunc(const NFmiCalculationCondition &theOperation,
                               Type theMaskType,
                               NFmiInfoData::Type theDataType,
                               const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                               NFmiAreaMask::FunctionType thePrimaryFunc,
                               NFmiAreaMask::FunctionType theSecondaryFunc,
                               int theArgumentCount);
  NFmiInfoAreaMaskTimeVertFunc(const NFmiInfoAreaMaskTimeVertFunc &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  void SetRangeValuesFromArguments();

  double itsStartTimeOffsetInHours;  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa
                                     // current-timesta
  double itsEndTimeOffsetInHours;    // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa
                                     // current-timesta

 private:
  NFmiInfoAreaMaskTimeVertFunc &operator=(const NFmiInfoAreaMaskTimeVertFunc &theMask);
};

class _FMI_DLL NFmiInfoAreaMaskProbFunc : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskProbFunc() override;
  NFmiInfoAreaMaskProbFunc(const NFmiCalculationCondition &theOperation,
                           Type theMaskType,
                           NFmiInfoData::Type theDataType,
                           const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                           NFmiAreaMask::FunctionType thePrimaryFunc,
                           NFmiAreaMask::FunctionType theSecondaryFunc,
                           int theArgumentCount);
  NFmiInfoAreaMaskProbFunc(const NFmiInfoAreaMaskProbFunc &theOther);
  NFmiAreaMask *Clone() const override;
  void Initialize() override;
  void SetArguments(std::vector<float> &theArgumentVector) override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;
  static NFmiMetTime CalcTimeLoopLimits(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                        const NFmiCalculationParams &theCalculationParams,
                                        double theStartTimeOffsetInHours,
                                        double theEndTimeOffsetInHours,
                                        unsigned long *theStartTimeIndexOut,
                                        unsigned long *theEndTimeIndexOut,
                                        bool *doSpecialCalculation,
                                        bool allowInterpolation);

 protected:
  void InitializeFromArguments();
  bool CheckProbabilityCondition(double value);

  NFmiAreaMask::FunctionType itsPrimaryFunc;    // esim. Over, Under, Between, Equal
  NFmiAreaMask::FunctionType itsSecondaryFunc;  // esim. ProbRect, ProbCircle
  std::vector<float> itsArgumentVector;         // tähän lasketaan lennossa laskuissa tarvittavat
                                                // argumentit (alueen säde ja raja(t))

  double itsSearchRangeInKM;  // minkä säteisen alueen halutaan käyvän läpi originaali datassa
  double itsLimit1;
  double itsLimit2;
  double itsStartTimeOffsetInHours;  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa
                                     // current-timesta
  double itsEndTimeOffsetInHours;    // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa
                                     // current-timesta
  int itsGridPointRectSizeX;  // käydään läpi ali hila, joka on tämän kokoinen x-suunnassa (min 2)
  int itsGridPointRectSizeY;  // käydään läpi ali hila, joka on tämän kokoinen y-suunnassa (min 2)
  int itsConditionFullfilledGridPointCount;  // Kaikki hilapisteet, missä todennäköisyys ehto piti
                                             // paikkaansa (tämä otetaan talteen Value -metodissa,
                                             // jotta NFmiInfoAreaMaskOccurrance -lapsiluokka voi
  // käyttää tämän luokan laskuja hyväkseen hiladatan
  // tapauksessa)

 private:
  NFmiInfoAreaMaskProbFunc &operator=(const NFmiInfoAreaMaskProbFunc &theMask);
};

class _FMI_DLL NFmiInfoAreaMaskTimeRange : public NFmiInfoAreaMask
{
 public:
  ~NFmiInfoAreaMaskTimeRange() override;
  NFmiInfoAreaMaskTimeRange(const NFmiCalculationCondition &theOperation,
                            Type theMaskType,
                            NFmiInfoData::Type theDataType,
                            const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                            NFmiAreaMask::FunctionType theIntegrationFunc,
                            int theArgumentCount);
  NFmiInfoAreaMaskTimeRange(const NFmiInfoAreaMaskTimeRange &theOther);
  NFmiAreaMask *Clone() const override;
  void Initialize() override;
  void SetArguments(std::vector<float> &theArgumentVector) override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  void InitializeFromArguments();

  NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. max, min, avg, sum
  boost::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;               // tämä luodaan itsIntegrationFunc-dataosan mukaan
  std::vector<float> itsArgumentVector;  // tähän lasketaan lennossa laskuissa tarvittavat
                                         // argumentit (alueen säde ja raja(t))

  double itsStartTimeOffsetInHours;  // kuinka monta tuntia aikaiteroinnin alkuaika poikkeaa
                                     // current-timesta
  double itsEndTimeOffsetInHours;    // kuinka monta tuntia aikaiteroinnin loppuaika poikkeaa
                                     // current-timesta

 private:
  NFmiInfoAreaMaskTimeRange &operator=(const NFmiInfoAreaMaskTimeRange &theMask);
};

class _FMI_DLL NFmiInfoTimeIntegrator : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoTimeIntegrator() override;
  NFmiInfoTimeIntegrator(const NFmiCalculationCondition &theOperation,
                         Type theMaskType,
                         NFmiInfoData::Type theDataType,
                         const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         NFmiAreaMask::FunctionType theIntegrationFunc,
                         int theStartTimeOffset,
                         int theEndTimeOffset);
  NFmiInfoTimeIntegrator(const NFmiInfoTimeIntegrator &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. Avg, Max, Min, Sum, jne.

  boost::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;  // tämä luodaan itsIntegrationFunc-dataosan mukaan
  int itsStartTimeOffset;   // dataa käydään läpi alkaen tästä suhteellisesta ajasta
  int itsEndTimeOffset;     // dataa käydään läpi päätyen tähän suhteelliseen aikaan
};

class _FMI_DLL NFmiInfoRectAreaIntegrator : public NFmiInfoAreaMaskMetFuncBase
{
 public:
  ~NFmiInfoRectAreaIntegrator() override;
  NFmiInfoRectAreaIntegrator(const NFmiCalculationCondition &theOperation,
                             Type theMaskType,
                             NFmiInfoData::Type theDataType,
                             const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             NFmiAreaMask::FunctionType theIntegrationFunc,
                             int theStartXOffset,
                             int theEndXOffset,
                             int theStartYOffset,
                             int theEndYOffset);
  NFmiInfoRectAreaIntegrator(const NFmiInfoRectAreaIntegrator &theOther);
  NFmiAreaMask *Clone() const override;

  // tätä kaytetaan smarttool-modifierin yhteydessä
  double Value(const NFmiCalculationParams &theCalculationParams,
               bool fUseTimeInterpolationAlways) override;

 protected:
  // Nämä virtuaali funktiot on toteutettava, vaikka niillä ei ole taas käyttöä täällä
  void SetDividers() override {}
  void InitCalcFactorVectors() override {}
  const CalcFactorVector &LowerEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &UpperEdgeFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }
  const CalcFactorVector &MiddleAreaFactors() const override
  {
    static CalcFactorVector dummy;
    return dummy;
  }

  NFmiAreaMask::FunctionType itsIntegrationFunc;  // esim. Avg, Max, Min, Sum, jne.

  boost::shared_ptr<NFmiDataModifier>
      itsFunctionModifier;  // tämä luodaan itsIntegrationFunc-dataosan mukaan
  int itsStartXOffset;
  int itsEndXOffset;
  int itsStartYOffset;
  int itsEndYOffset;
};

// ======================================================================
