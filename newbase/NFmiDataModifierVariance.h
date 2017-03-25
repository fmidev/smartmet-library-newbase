// ======================================================================
/*!
 * \file NFmiDataModifierVariance.h
 * \brief Interface of class NFmiDataModifierVariance
 */
// ======================================================================

#pragma once

#include "NFmiRegressionModifier.h"

#include <boost/thread.hpp>
typedef boost::shared_mutex MutexType;

//! Undocumented
class _FMI_DLL NFmiDataModifierVariance : public NFmiRegressionModifier
{
 public:
  ~NFmiDataModifierVariance() override;
  NFmiDataModifierVariance(NFmiDataIdent* theDataIdent,
                           NFmiLevel* theLevel,
                           NFmiQueryInfo* theData = 0,
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
