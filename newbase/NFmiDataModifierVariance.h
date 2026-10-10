// ======================================================================
/*!
 * \file NFmiDataModifierVariance.h
 * \brief Interface of class NFmiDataModifierVariance
 */
// ======================================================================

#pragma once

#include "NFmiRegressionModifier.h"

#include <boost/thread.hpp>
using MutexType = boost::shared_mutex;

//! Undocumented
class NFmiDataModifierVariance : public NFmiRegressionModifier
{
 public:
  ~NFmiDataModifierVariance() override;
  NFmiDataModifierVariance(NFmiDataIdent* theDataIdent,
                           NFmiLevel* theLevel,
                           NFmiQueryInfo* theData = nullptr,
                           float theVarianceLevel = 1.0,
                           bool onlyVariance = false);

  double FloatValue() override;

 protected:
  double GaussianRandom();
  bool fVarianceOnly;
  double itsVarianceLevel;
  mutable MutexType itsMutex;

};  // class NFmiDataModifierVariance

// ======================================================================
