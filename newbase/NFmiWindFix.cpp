#include "NFmiWindFix.h"
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFastInfoUtils.h"

using namespace std;

namespace
{

bool RecalculateWindParameters(NFmiFastQueryInfo& sourceInfo, NFmiFastQueryInfo& destInfo, const NFmiFastInfoUtils::MetaWindParamUsage& metaWindParamUsage)
{
    sourceInfo.Param(kFmiWindSpeedMS);
    auto wsParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindDirection);
    auto wdParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindUMS);
    auto uParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindVMS);
    auto vParamIndex = sourceInfo.ParamIndex();
    sourceInfo.Param(kFmiWindVectorMS);
    auto windVectorParamIndex = sourceInfo.ParamIndex();
    for(sourceInfo.ResetLevel(), destInfo.ResetLevel(); sourceInfo.NextLevel() && destInfo.NextLevel(); )
    {
        for(sourceInfo.ResetLocation(), destInfo.ResetLocation(); sourceInfo.NextLocation() && destInfo.NextLocation(); )
        {
            for(sourceInfo.ResetTime(), destInfo.ResetTime(); sourceInfo.NextTime() && destInfo.NextTime(); )
            {
                sourceInfo.ParamIndex(wsParamIndex);
                auto WS = sourceInfo.FloatValue();
                sourceInfo.ParamIndex(wdParamIndex);
                auto WD = sourceInfo.FloatValue();
                if(destInfo.ParamIndex(uParamIndex))
                    destInfo.FloatValue(NFmiFastInfoUtils::CalcU(WS, WD));
                if(destInfo.ParamIndex(vParamIndex))
                    destInfo.FloatValue(NFmiFastInfoUtils::CalcV(WS, WD));
                if(destInfo.ParamIndex(windVectorParamIndex))
                    destInfo.FloatValue(NFmiFastInfoUtils::CalcWindVectorFromSpeedAndDirection(WS, WD));
            }
        }
    }
    return true;
}

}


namespace NFmiWindFix
{

bool FixWinds(NFmiQueryData& sourceData)
{
    NFmiFastQueryInfo sourceInfo(&sourceData);
    return FixWinds(sourceInfo);
}

bool FixWinds(NFmiFastQueryInfo& sourceInfo)
{
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(sourceInfo);
    if(metaWindParamUsage.HasTotalWind())
        return false;
    if(!metaWindParamUsage.HasWsAndWd())
        return false;

    if(metaWindParamUsage.HasWindComponents() || metaWindParamUsage.HasWindVectorParam())
    {
        NFmiFastQueryInfo destInfo(sourceInfo);
        return ::RecalculateWindParameters(sourceInfo, destInfo, metaWindParamUsage);
    }
    return false;
}

} // NFmiWindFix
