// ======================================================================
/*!
 * \file NFmiRegressionModifier.cpp
 * \brief Implementation of class NFmiRegressionModifier
 */
// ======================================================================
/*!
 * \class NFmiRegressionModifier
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRegressionModifier.h"
#include "NFmiDataIdent.h"
#include "NFmiDataModifierConstant.h"
#include "NFmiDataModifierList.h"
#include "NFmiDataModifierLogical.h"
#include "NFmiRegressionItem.h"
#include <macgyver/Exception.h>
#include <algorithm>
#include <cmath>

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a regression item for the given parameter
 *
 * The modifiers store copies of the given identity and level, so the
 * identity can live on the stack.
 */
// ----------------------------------------------------------------------

NFmiRegressionItem *NewRegressionItem(double theCoefficient,
                                      FmiParameterName theParam,
                                      NFmiLevel *theLevel,
                                      NFmiQueryInfo *theData)
{
  NFmiDataIdent ident{NFmiParam(theParam)};
  return new NFmiRegressionItem(theCoefficient, &ident, theLevel, theData);
}

// ----------------------------------------------------------------------
/*!
 * \brief Create an info modifier for the given parameter
 */
// ----------------------------------------------------------------------

NFmiInfoModifier *NewInfoModifier(FmiParameterName theParam,
                                  NFmiLevel *theLevel,
                                  NFmiQueryInfo *theData)
{
  NFmiDataIdent ident{NFmiParam(theParam)};
  return new NFmiInfoModifier(&ident, theLevel, theData);
}
}  // namespace

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiRegressionModifier::~NFmiRegressionModifier()
{
  try
  {
    delete itsRegressionItems;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Destructor failed", nullptr);
    exception.printError();
  }
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theParam Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 */
// ----------------------------------------------------------------------

NFmiRegressionModifier::NFmiRegressionModifier(NFmiDataIdent *theParam,
                                               NFmiLevel *theLevel,
                                               NFmiQueryInfo *theData)
    : NFmiInfoModifier(theParam, theLevel, theData), itsRegressionItems(new NFmiDataModifierList)
{
  try
  {
    if (*itsParam == NFmiDataIdent(NFmiParam(kFmiTemperature)))
    {
      auto *firstAlternative = new NFmiDataModifierList;
      firstAlternative->Add(new NFmiDataModifierConstant(3.0, kFmiAdd));
      firstAlternative->Add(NewRegressionItem(-0.02, kFmiTotalCloudCover, itsLevel, itsData));
      firstAlternative->Add(NewRegressionItem(-0.8, kFmiWindSpeedMS, itsLevel, itsData));

      auto *secondAlternative = new NFmiDataModifierList;
      secondAlternative->Add(new NFmiRegressionItem(5.0));
      secondAlternative->Add(NewRegressionItem(-0.03, kFmiTotalCloudCover, itsLevel, itsData));
      secondAlternative->Add(NewRegressionItem(-0.5, kFmiWindSpeedMS, itsLevel, itsData));

      auto *conditionData = NewInfoModifier(kFmiTemperature, itsLevel, itsData);
      auto *conditionLimit = new NFmiRegressionItem(14.0);
      auto *condition =
          new NFmiDataModifierBoolean(kFmiModifierValueGreaterThan, conditionData, conditionLimit);
      auto *alternatives =
          new NFmiDataModifierLogical(condition, firstAlternative, secondAlternative);
      itsRegressionItems->Add(alternatives);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiRegressionModifier::FloatValue()
{
  try
  {
    if (!itsData)
      return kFloatMissing;

    return std::max(static_cast<double>(*itsRegressionItems), 0.0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiRegressionModifier::WriteOperand(std::ostream &file) const
{
  try
  {
    file << "<operand>";
    itsRegressionItems->Write(file);
    file << "</operator>";
    return file;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
