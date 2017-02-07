//************** NFmiDataModifierClipSum ************************
// 19.9.2000/Lasse; puolittaa alku- ja loppuhetken arvot
//   soveltuu esim sadesumman llskemiseen kun l�hdet��n hetkellisist� intensiteeteist�
// AllValid= vaatii ett� kaikki arvot ovat k�ypi�

#include "NFmiDataModifierAllValidSum.h"

class _FMI_DLL NFmiDataModifierAllValidClipSum : public NFmiDataModifierAllValidSum
{
 public:
  using NFmiDataModifierAllValidSum::Calculate;

  virtual void Calculate(NFmiQueryInfo* theQI)
  {
    // PIT�IS TARKISTAA VOIKO TOIMIA N�IN ja t�ss�h�n ei ole arvov�lin tsekkausta
    float value = theQI->FloatValue();
    float halfValue;
    if (CheckMissingValues(value))
    {
      halfValue = value * .5;
      if (itsCounter == 0) fCalculationResultOk = true;
      itsSum += halfValue + itsHalfValue;
    }
    else
    {
      fCalculationResultOk = false;
    }
    itsHalfValue = halfValue;
    itsCounter++;
  };

  NFmiDataModifierAllValidClipSum(void) : NFmiDataModifierAllValidSum() { itsHalfValue = 0.; };
  virtual ~NFmiDataModifierAllValidClipSum(void){};

 private:
  float itsHalfValue;
};

//************** NFmiDataModifierAllValidsClipSum ************************
