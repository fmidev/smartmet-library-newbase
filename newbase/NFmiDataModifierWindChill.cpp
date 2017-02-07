// ======================================================================
/*!
 * \file NFmiDataModifierWindChill.cpp
 * \brief Implementation of class NFmiDataModifierWindChill
 */
// ======================================================================
/*!
 * \class NFmiDataModifierWindChill
 *
 * Luokka laskee wind chill arvon siten ett� pyyt�� info:lta l�p�tilan
 * ja tuulen nopeuden arvon ja laskee wind chilin niiden avulla.
 * K�ytet��n esim. queryInfo->CalcTimeData:n yhteydess�.
 *
 * Modifier ottaa parametrina viel� toisen modifierin jonka avulla voidaan
 * laskea esim. max tai min arvo jonkin p�iv�n ajalta.
 *
 */
// ======================================================================

#include "NFmiDataModifierWindChill.h"
#include "NFmiQueryInfo.h"
#include "NFmiMetMath.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierWindChill::~NFmiDataModifierWindChill(void) {}
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theQueryInfo Undocumented
 * \param theDataModifier Undocumented
 * \param theLatLonPoint Undocumented
 * \param theTeperatureParamId Undocumented
 * \param theWindSpeedParamId Undocumented
 *
 * \todo Spell the theTeperatureParamId argument correctly.
 */
// ----------------------------------------------------------------------

NFmiDataModifierWindChill::NFmiDataModifierWindChill(NFmiQueryInfo* theQueryInfo,
                                                     NFmiDataModifier* theDataModifier,
                                                     const NFmiPoint& theLatLonPoint,
                                                     int theTeperatureParamId,
                                                     int theWindSpeedParamId)
    : NFmiDataModifier(),
      itsTemperatureParamId(theTeperatureParamId),
      itsWindSpeedParamId(theWindSpeedParamId),
      itsLatLonPoint(theLatLonPoint),
      itsQueryInfo(theQueryInfo),
      itsDataModifier(theDataModifier)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \param theInfo Undocumented, unused
 *
 * \todo Take new newbase into use
 */
// ----------------------------------------------------------------------

void NFmiDataModifierWindChill::Calculate(float /* theValue */)
{
  if (itsQueryInfo)
  {
    FmiParameterName oldParam = FmiParameterName(itsQueryInfo->Param().GetParamIdent());

    // unsigned long oldPlaceIndex = itsQueryInfo->LocationIndex(); // t�m� toimii vain uudessa
    // newbase:ssa (parempi!)
    NFmiPoint oldLatlon(
        itsQueryInfo->LatLon());  // kommentoi t�m� kun uusi versio k�yt�ss�!!!!!!!!!

    float T = kFloatMissing;
    if (itsQueryInfo->Param(FmiParameterName(itsTemperatureParamId)))
      T = itsQueryInfo->InterpolatedValue(itsLatLonPoint);
    float WS = kFloatMissing;
    if (itsQueryInfo->Param(FmiParameterName(itsWindSpeedParamId)))
      WS = itsQueryInfo->InterpolatedValue(itsLatLonPoint);  // HUOM 2 on Njurmen tulkinta

    itsDataModifier->Calculate(FmiWindChill(WS, T));
    itsQueryInfo->Param(oldParam);  // palauta parametri alkutilaan!!

    // itsQueryInfo->LocationIndex(oldPlaceIndex); // t�m� toimii vain uudessa newbase:ssa
    // (parempi!) // palauta vanha paikka my�s (pit�isik�h�n
    // itsQueryInfo->InterpolatedValue(itsLatLonPoint); -metodi muuttaa niin ett� paikka ei
    // muutu???)
    itsQueryInfo->Location(oldLatlon);  // kommentoi t�m� kun uusi versio k�yt�ss�!!!!!!!!!
  }
  return;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierWindChill::CalculationResult(void)
{
  return itsDataModifier->CalculationResult();
}

// ----------------------------------------------------------------------
/*!
 * Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierWindChill::Clear(void) { itsDataModifier->Clear(); }
// ======================================================================
