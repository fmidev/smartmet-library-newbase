// ======================================================================
/*!
 * \file NFmiEditParam.h
 * \brief Interface of class NFmiEditParam
 */
// ======================================================================

#pragma once

#include "NFmiParam.h"

enum FmiMaskMethod
{
  kFmiIsLessThan = 0,
  kFmiIsGreaterThan,
  kFmiIsEqual
};

//! Undocumented class, with stupid interface
class NFmiEditParam : public NFmiParam
{
 public:
  ~NFmiEditParam() override;
  NFmiEditParam();

 private:
};  // class NFmiEditParam

// ======================================================================
