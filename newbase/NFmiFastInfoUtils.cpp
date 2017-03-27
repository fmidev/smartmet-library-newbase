#include "NFmiFastInfoUtils.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFileString.h"
#include "NFmiProducerName.h"

namespace NFmiFastInfoUtils
{
bool IsInfoShipTypeData(NFmiFastQueryInfo &theInfo)
{
  if (theInfo.IsGrid() == false)
  {
    FmiProducerName prodId = static_cast<FmiProducerName>(theInfo.Producer()->GetIdent());
    if (prodId == kFmiSHIP || prodId == kFmiBUOY) return true;
  }
  return false;
}

// synop-dataa ei tarvitsee asettaa mihink‰‰n, mutta
// mm. luotaus plotin yhteydess‰ pit‰‰ etsi‰ oikea korkeus/level
void SetSoundingDataLevel(const NFmiLevel &theWantedSoundingPressureLevel, NFmiFastQueryInfo &info)
{
  if (theWantedSoundingPressureLevel.LevelType() == kFmiSoundingLevel)
  {  // sounding plotin yhteydess‰ pit‰‰ asettaa level haluttuun korkeuteen
    float levelValue = theWantedSoundingPressureLevel.LevelValue();
    FmiParameterName parName = static_cast<FmiParameterName>(info.Param().GetParamIdent());
    unsigned long parIndex = info.ParamIndex();
    bool subParaUsed = info.IsSubParamUsed();
    if (info.Param(kFmiPressure))
    {
      for (info.ResetLevel(); info.NextLevel();)
        if (info.FloatValue() == levelValue) break;
    }
    if (subParaUsed)
      info.Param(parName);  // pakko vet‰‰ t‰m‰ hitaalla tavalla jostain syyst‰
    else
      info.ParamIndex(parIndex);
  }
}

std::string GetTotalDataFilePath(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  NFmiFileString totalFilePath(theInfo->DataFilePattern());
  totalFilePath.FileName(theInfo->DataFileName());
  return std::string(totalFilePath);
}

bool IsYearLongData(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  const auto &timeDescriptor = info->TimeDescriptor();
  auto timeDiffInDays = timeDescriptor.LastTime().DifferenceInDays(timeDescriptor.FirstTime());
  if (timeDiffInDays >= 364 && timeDiffInDays <= 366)
    return true;
  else
    return false;
}

bool IsModelClimatologyData(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (info && info->DataType() == NFmiInfoData::kClimatologyData)
  {
    if (info->IsGrid())
    {
      if (IsYearLongData(info))
      {
        return true;
      }
    }
  }
  return false;
}

NFmiMetTime GetUsedTimeIfModelClimatologyData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                              const NFmiMetTime &theTime)
{
  if (NFmiFastInfoUtils::IsModelClimatologyData(theInfo))
  {
    // For year long climatology data, used time must be fixed to data's own year
    auto usedTime(theTime);
    usedTime.SetYear(theInfo->TimeDescriptor().FirstTime().GetYear());
    return usedTime;
  }
  else
    return theTime;
}
}
