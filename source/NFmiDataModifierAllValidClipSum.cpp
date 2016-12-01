// ======================================================================
/*!
 * \file NFmiDataModifierAllValidClipSum.cpp
 * \brief Implementation of class NFmiDataModifierAllValidClipSum
 */
// ======================================================================
/*!
 * \class NFmiDataModifierAllValidClipSum
 *
 * Undocumented
 *
 *
 * 19.9.2000/Lasse; puolittaa alku- ja loppuhetken arvot
 * soveltuu esim sadesumman llskemiseen kun l�hdet��n hetkellisist� intensiteeteist�
 * AllValid= vaatii ett� kaikki arvot ovat k�ypi�
 */
// ======================================================================

#include "NFmiDataModifierAllValidClipSum.h"
#include "NFmiQueryInfo.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidClipSum::~NFmiDataModifierAllValidClipSum(void) {}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierAllValidClipSum::NFmiDataModifierAllValidClipSum(void)
    : NFmiDataModifierAllValidSum(), itsHalfValue(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theQI Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierAllValidClipSum::Calculate(NFmiQueryInfo* theQI)
{
  // PIT�IS TARKISTAA VOIKO TOIMIA N�IN ja t�ss�h�n ei ole arvov�lin tsekkausta
  float value = theQI->FloatValue();
  float halfValue = 0;
  if (CheckMissingValues(value))
  {
    halfValue = static_cast<float>(value * .5);
    if (itsCounter == 0) fCalculationResultOk = true;
    itsSum += halfValue + itsHalfValue;
  }
  else
  {
    fCalculationResultOk = false;
  }
  itsHalfValue = halfValue;
  itsCounter++;
}

// ======================================================================
