// ======================================================================
/*!
 * \file NFmiSuperSmartInfo.h
 * \brief Interface of class NFmiSuperSmartInfo
 */
// ======================================================================

#ifndef NFMISUPERSMARTINFO_H
#define NFMISUPERSMARTINFO_H

#ifdef _MSC_VER
#pragma warning(disable : 4251)  // poistaa VC++ k��nt�j�n varoituksen dll k��nn�ksiss�
#endif

#include "NFmiFastQueryInfo.h"
#include "NFmiRect.h"

class NFmiCalculator;
class NFmiAreaMask;
class NFmiDataModifierDescriptor;
class NFmiBitmapAreaMask;

//! Undocumented
class _FMI_DLL NFmiSuperSmartInfo : public NFmiFastQueryInfo
{
 public:
  // Accesslevel m��r�� mill� tasolla dataa haetaan kulloisellakin hetkell� SSInfosta.
  enum DataAccessLevel
  {
    kNormalAccess = 0,     // == 'raaka' yhteys dataan eli k�yt� suoraan fastinfon FloatValue:ta
    kVarianceAccess = 1,   // varianssi tason k�ytt�
    kVariationAccess = 2,  // ep�varmuus tason k�ytt�
    kTimeIntegrationAccess = 3,  // aikaintegraatio tason k�ytt� esim. sateen 12h yhteenlaskeminen
    kCalculatorAccess = 4        // korkeimman tason yhteys dataan (== laskee mukaan mahdollisesti
                                 // calculatorin ja todenn�k�isyyden ja varianssin, jos ne ovat
                                 // olemassa ja k�yt�ss�)
  };

  ~NFmiSuperSmartInfo(void);

  NFmiSuperSmartInfo(void);
  NFmiSuperSmartInfo(const NFmiQueryInfo& theInfo);
  NFmiSuperSmartInfo(const NFmiSuperSmartInfo& theInfo);

  NFmiSuperSmartInfo(const NFmiParamDescriptor& theParamDescriptor,
                     const NFmiTimeDescriptor& theTimeDescriptor,
                     const NFmiHPlaceDescriptor& theHPlaceDescriptor = NFmiHPlaceDescriptor(),
                     const NFmiVPlaceDescriptor& theVPlaceDescriptor = NFmiVPlaceDescriptor());

  NFmiSuperSmartInfo(NFmiQueryData* data,
                     NFmiParamDescriptor* theParamDescriptor = 0,
                     NFmiTimeDescriptor* theTimeDescriptor = 0,
                     NFmiHPlaceDescriptor* theHPlaceDescriptor = 0,
                     NFmiVPlaceDescriptor* theVPlaceDescriptor = 0);

  // paikan-asetus-funktiot pit�� kirjoittaa uudestaan areamaskien takia, koska
  // nyt liikutaan maskeja k�ytett�ess� vain maskattuihin kohtiin ja muut hyp�t��n yli!!!!

  bool FirstLocation(void);
  bool NextLocation(void);
  bool PreviousLocation(void);
  using NFmiFastQueryInfo::NearestLocation;
  bool NearestLocation(const NFmiLocation& theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);

  bool NearestPoint(const NFmiPoint& theLatLonPoint);

  // Parametrin-asetus-funktiot pit�� kirjoittaa uudestaan varianssi-calculaattorien takia!!!!!

  bool FirstParam(bool fIgnoreSubParam = true);
  bool NextParam(bool fIgnoreSubParam = true);
  bool PreviousParam(bool fIgnoreSubParam = true);
  bool LastParam(bool fIgnoreSubParam = true);

#ifndef UNIX
  using NFmiFastQueryInfo::Param;  // nyt voi k�ytt�� suoraan SSInfosta muitakin Param-funktioita
#else
  // oli pakko kirjoittaa, koska muuten en voi kutsua SSInfo:lta t�t�,
  // koska muitakin Param-metodeja on peitt�m�ss� t�t�
  NFmiDataIdent& Param(void) const { return NFmiFastQueryInfo::Param(); };
#endif

  bool Param(const NFmiParam& theParam);
  bool Param(const NFmiParam& theParam, const NFmiParam& theSubParam);
  bool Param(const NFmiDataIdent& theDataIdent);
  bool Param(FmiParameterName theParam);

  // ajan-asetus-funktiot pit�� kirjoittaa uudestaan areamaskin takia,
  // jos sattuu olemaan dynaamisia ajassa muuttuvia!!!!

  bool FirstTime(void);
  bool NextTime(void);
  bool LastTime(void);
  bool PreviousTime(void);

#ifndef UNIX
  using NFmiFastQueryInfo::Time;       // nyt voi k�ytt�� suoraan SSInfosta muitakin Time-funktioita
  using NFmiFastQueryInfo::TimeIndex;  // nyt voi k�ytt�� suoraan SSInfosta muitakin
                                       // TimeIndex-funktioita
#else
  const NFmiMetTime& Time(void) const { return NFmiFastQueryInfo::Time(); }
  unsigned long TimeIndex(void) const { return NFmiFastQueryInfo::TimeIndex(); }
#endif
  bool Time(const NFmiMetTime& theTime);
  bool TimeIndex(unsigned long theIndex);

  unsigned long SizeLocations(void) const;
  unsigned long SizeActiveLocations(void) const;

  // QueryData Read
  float FloatValue(void) const;

  using NFmiQueryInfo::FloatValue;

  float FloatValue(bool doVariationCalculation,
                   bool doCalculation,
                   bool doTimeIntegration,
                   bool doVarianceCalculation);

  NFmiCombinedParam* CombinedValue(void);
  NFmiCombinedParam* CombinedValue(bool doVariationCalculation,
                                   bool doCalculation,
                                   bool doTimeIntegration,
                                   bool doVarianceCalculation);

  using NFmiFastQueryInfo::PeekLocationValue;
  float PeekLocationValue(int theXOffset, int theYOffset) const;
  float PeekTimeValue(int theTimeOffset);
  using NFmiQueryInfo::PeekValue;
  float PeekValue(int theTimeOffset, int theXOffset, int theYOffset);

  // ClassIdent
  unsigned long ClassId(void) const;
  const char* ClassName(void) const;

  NFmiQueryInfo* Clone(void) const;
  std::ostream& Write(std::ostream& file) const;
  std::istream& Read(std::istream& file);

  // operator
  NFmiSuperSmartInfo& operator=(const NFmiSuperSmartInfo& theInfo);

  void FloatValueAccessLevel(DataAccessLevel value);
  DataAccessLevel FloatValueAccessLevel(void) const;
  void UseCalculator(bool value);
  bool UseCalculator(void) const;
  void UseVarianceCalculator(bool value);
  bool UseVarianceCalculator(void) const;
  void UseVariationCalculator(bool value);
  bool UseVariationCalculator(void) const;
  bool UseTimeIntegrationCalculator(void);
  void UseTimeIntegrationCalculator(bool value);
  void SetCalculator(NFmiCalculator* value, bool useCalculator = true);
  void SetDataModifierDescriptor(NFmiDataModifierDescriptor* value);
  NFmiDataModifierDescriptor* DataModifierDescriptor(void);
  NFmiCalculator* Calculator(void) const;
  NFmiCalculator* CurrentVarianceCalculator(void) const;
  void AreaMask(NFmiAreaMask* value);
  NFmiAreaMask* AreaMask(void) const;
  void UseAreaMask(bool value);
  bool UseAreaMask(void) const;
  bool UseHelperBinaryMasks(void) const;
  void UseHelperBinaryMasks(bool newStatus);
  const NFmiBitmapAreaMask* CurrentHelperBinaryMask(void) const;
  NFmiCalculator* VariationCalculator(void);
  void VariationCalculator(NFmiCalculator* newCalculator);
  NFmiCalculator* TimeIntegrationCalculator(void);
  void TimeIntegrationCalculator(NFmiCalculator* newCalculator);
  NFmiString HelperBinaryMaskTestString(int theUsedVariationFactor);  // testi funktio
  bool LocationIndex2XYPosition(
      unsigned long theIndex,
      int* theXOffset,
      int* theYOffset);                // t�m� kannattaa testien j�lkeen laittaa privateksi
  void UpdateHelperBinaryMasks(void);  // kutsu t�t� kun haluat p�ivitt�� apuharvennusmaskeja
  void InitEmptyAreaMask(void);
  bool SetNearestPointMask(const NFmiPoint& theLatLonPoint, bool newValue, bool fClearFirst);

 protected:
  void Destroy(void);

  virtual size_t Index(void) const;
  size_t Index(unsigned long theParamIndex,
               unsigned long theLocationIndex,
               unsigned long theLevelIndex,
               unsigned long theTimeIndex) const
  {
    return NFmiFastQueryInfo::Index(theParamIndex, theLocationIndex, theLevelIndex, theTimeIndex);
  }

  bool IsLocationMasked(unsigned long theLocationIndex) const;

  float VarianceFloatValue(void) const;
  float VariationFloatValue(void) const;
  float TimeIntegrationFloatValue(void) const;
  float RawFloatValue(void) const;
  float CalculationFloatValue(void) const;
  void UpdateVarianceCalculator(void);
  void UpdateAreaMaskTime(void);
  unsigned long CalcPeekLocationIndex(unsigned long currentIndex,
                                      int theXOffset,
                                      int theYOffset) const;
  NFmiCombinedParam* VariationCombinedValue(void);
  NFmiCombinedParam* CalculationCombinedValue(void);

 private:
  // kutsu t�t� aina kun aika muuttuu SSInfossa!!!!
  void TimeChanged(unsigned long theOldTimeIndex);

  // kopioi helperbinarymaskit omaan k�ytt��ns�
  void CopyHelperBinaryMasksAndInfo(const NFmiSuperSmartInfo& theSSInfo);

  // alustaa kaikki tarvittavat harvennusapu maskit(eli kaikille ep�varmuuksille 0-n)
  bool InitHelperBinaryMasks(void);

  // luo aina tietylle ep�varmuuskertoimelle harvennusmaskin
  NFmiBitmapAreaMask* CreateHelperBinaryMask(int theUsedVariationFactor);

  NFmiBitmapAreaMask* CreateZeroVariationHelperBinaryMask(void);

  // tuhoaa maskit
  void ClearHelperBinaryMasks(void);

  // laskee k�ytetyn maskin xy-indeksi-boundingboxin
  void CalcXYMaskBoundingBox(void);

  NFmiBitmapAreaMask* HelperBinaryMask(int theUsedVariationFactor);
  void SetCurrentHelperBinaryMask(void);

  // Vaikuttaa mm. FloatValue-metodin toimintaan. Mill� tasolla dataa haetaan:
  // raakana, varianssia, todenn�k�isyytt�, vai Calculaattori tasolla laskettua.
  // 0 = raaka data k�ytt� eli sama kuin FastInfo:n FloatValue.
  mutable DataAccessLevel itsFloatValueAccessLevel;

  // K�ytet��nk� calculaattoria laskuissa. Jos ei ole kyseist� calculaattoria, arvon pit�� olla
  // false.
  bool fUseCalculator;

  // K�ytet��nk� varianssi calculaattoria laskuissa. Jos ei ole kyseist� calculaattoria, arvon pit��
  // olla false.
  bool fUseVarianceCalculator;
  bool fUseVariationCalculator;
  bool fUseTimeIntegrationCalculator;

  // t�h�n laitetaan mm. integraatio calculaattorit // ei omista, ei tuhoa!!!
  NFmiCalculator* itsCalculator;

  // ep�varmuuslaatikko calculaattori // ei omista, ei tuhoaa!!!
  NFmiCalculator* itsVariationCalculator;

  // t�m� laskee vesiputouksen 3. portaassa halutunlaisen aikaintegroinnin, esim. 12h
  // sade summa. ei omista, ei tuhoa!!!
  NFmiCalculator* itsTimeIntegrationCalculator;

  // T�h�n SSInfo hakee aina kulloisenkin parametrin varianssi-modifierin
  // parametrikohtaisesta varianssi-modifier-listasta, jonka k�ytt�j� on
  // antanut. omistaa, tuhoaa!!!
  NFmiCalculator* itsCurrentVarianceCalculator;

  // parametrikohtaiset varianssi ja muut datamodifierit, jos jonkun parametrin
  // v-calculator puuttuu, ei lasketa varianssia! ei omista, ei tuhoa!!!
  NFmiDataModifierDescriptor* itsDataModifierDescriptor;

  // t�m� vaikuttaa SSInfon paikka juoksutukseen, vain maskatut paikat
  // k�yd��n l�pi!!! // ei omista, ei tuhoa!!!
  NFmiAreaMask* itsAreaMask;

  bool fUseAreaMask;

  // T�m� on optimointi vipu: Harvennetaan k�yt�vi� maskattuja pisteit� t�m�n
  // avulla, jotta ep�varmuusalue ei hidasta liikaa laskentoja.
  bool fUseHelperBinaryMasks;

  // Optimoinnissa k�ytetty bin��ri maski. vectorista haetaan aina t�h�n
  // sopiva maski(ei omista ei tuhoa)
  NFmiBitmapAreaMask* itsCurrentHelperBinaryMask;

  // t�t� k�ytet��n apuna laskettaessa harvennusmaskeja
  NFmiRect itsXYMaskBoundingBox;

  // t�h�n talletetaan eri ep�varmuus kertoimilla lasketut apumaskit (omistaa, tuhoaa)
  checkedVector<NFmiBitmapAreaMask*> itsHelperBinaryMaskList;

  // T�m� osaa luoda k�ytt�j�n pyyt�mi� speciaali maskeja ja osaa mm.
  // hoitaa optimoinnin binaarimaskin rakentamisen.

};  // class NFmiSuperSmartInfo

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::Param(FmiParameterName theParam)
{
  return Param(NFmiParam(theParam));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiSuperSmartInfo::ClassId(void) const { return kNFmiQueryInfo; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiSuperSmartInfo::ClassName(void) const { return "NFmiSuperSmartInfo"; }
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::FloatValueAccessLevel(DataAccessLevel value)
{
  itsFloatValueAccessLevel = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiSuperSmartInfo::DataAccessLevel NFmiSuperSmartInfo::FloatValueAccessLevel(void) const
{
  return itsFloatValueAccessLevel;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseCalculator(bool value) { fUseCalculator = value; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseCalculator(void) const { return fUseCalculator; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseVarianceCalculator(void) const { return fUseVarianceCalculator; }
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseVariationCalculator(bool value)
{
  fUseVariationCalculator = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseVariationCalculator(void) const
{
  return fUseVariationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseTimeIntegrationCalculator(void)
{
  return fUseTimeIntegrationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseTimeIntegrationCalculator(bool value)
{
  fUseTimeIntegrationCalculator = value;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiDataModifierDescriptor* NFmiSuperSmartInfo::DataModifierDescriptor(void)
{
  return itsDataModifierDescriptor;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::Calculator(void) const { return itsCalculator; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// huom! ei asetus funktiota, koska t�m� haetaan aina parametrikohtaisesta listasta!!!
inline NFmiCalculator* NFmiSuperSmartInfo::CurrentVarianceCalculator(void) const
{
  return itsCurrentVarianceCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::AreaMask(NFmiAreaMask* value) { itsAreaMask = value; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiAreaMask* NFmiSuperSmartInfo::AreaMask(void) const { return itsAreaMask; }
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::UseAreaMask(bool value) { fUseAreaMask = value; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseAreaMask(void) const { return fUseAreaMask; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiSuperSmartInfo::UseHelperBinaryMasks(void) const { return fUseHelperBinaryMasks; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiBitmapAreaMask* NFmiSuperSmartInfo::CurrentHelperBinaryMask(void) const
{
  return itsCurrentHelperBinaryMask;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::VariationCalculator(void)
{
  return itsVariationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param newCalculator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::VariationCalculator(NFmiCalculator* newCalculator)
{
  itsVariationCalculator = newCalculator;
  TimeChanged(static_cast<unsigned long>(-1));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiCalculator* NFmiSuperSmartInfo::TimeIntegrationCalculator(void)
{
  return itsTimeIntegrationCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \param newCalculator Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiSuperSmartInfo::TimeIntegrationCalculator(NFmiCalculator* newCalculator)
{
  itsTimeIntegrationCalculator = newCalculator;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiSuperSmartInfo::RawFloatValue(void) const
{
  return NFmiFastQueryInfo::FloatValue();
}

#endif  // class NFmiSuperSmartInfo

// ======================================================================
