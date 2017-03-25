// ======================================================================
/*!
 * \file NFmiInfoModifier.h
 * \brief Interface of class NFmiInfoModifier
 */
// ======================================================================

#pragma once

#include "NFmiParamDataModifier.h"

class NFmiQueryInfo;

//! Undocumented
class _FMI_DLL NFmiInfoModifier : public NFmiParamDataModifier
{
 public:
  ~NFmiInfoModifier() override;
  NFmiInfoModifier(NFmiDataIdent* theParam,
                   NFmiLevel* theLevel,
                   NFmiQueryInfo* theData = 0,
                   FmiJoinOperator theJoinOperator = kFmiAdd);
  NFmiQueryInfo* Info();
  double FloatValue() override;
  virtual double PeekValue(long dx, long dy, long dt);

 protected:
  NFmiQueryInfo* itsData;

 private:
  NFmiInfoModifier(const NFmiInfoModifier& theMod);
  NFmiInfoModifier& operator=(const NFmiInfoModifier& theMod);

};  // class NFmiInfoModifier

// ======================================================================
