#include "NFmiSimpleCondition.h"
#include <macgyver/Exception.h>

namespace
{
bool CheckForStationaryData(const boost::shared_ptr<NFmiAreaMask> &mask)
{
  try
  {
    if (mask)
    {
      if (mask->GetDataType() == NFmiInfoData::kStationary)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool UseTimeInterpolation(bool maskIsStationaryData, bool normalInterpolationCondition)
{
  try
  {
    if (maskIsStationaryData)
      return false;

    return normalInterpolationCondition;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double CalculateValue(double value1,
                      double value2,
                      NFmiAreaMask::CalculationOperator calculationOperator)
{
  try
  {
    if (value1 == kFloatMissing || value2 == kFloatMissing)
      return kFloatMissing;

    switch (calculationOperator)
    {
      case NFmiAreaMask::Add:
        return value1 + value2;
      case NFmiAreaMask::Sub:
        return value1 - value2;
      case NFmiAreaMask::Mul:
        return value1 * value2;
      case NFmiAreaMask::Div:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return value1 / value2;
      }
      case NFmiAreaMask::Mod:
      {
        if (value2 == 0)
          return kFloatMissing;
        else
          return std::fmod(value1, value2);
      }
      case NFmiAreaMask::Pow:
        return std::pow(value1, value2);
      default:
        throw Fmi::Exception(
            BCP,
            "Internal program error with SimpleCondition's calculation operator - unknown "
            "operator");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

NFmiSimpleConditionPart::~NFmiSimpleConditionPart(void) = default;

NFmiSimpleConditionPart::NFmiSimpleConditionPart(
    boost::shared_ptr<NFmiAreaMask> &mask1,
    NFmiAreaMask::CalculationOperator calculationOperator,
    boost::shared_ptr<NFmiAreaMask> &mask2)
    : itsMask1(mask1), itsCalculationOperator(calculationOperator), itsMask2(mask2)
{
}

NFmiSimpleConditionPart::NFmiSimpleConditionPart(const NFmiSimpleConditionPart &theOther)
    : itsMask1(theOther.itsMask1 ? theOther.itsMask1->Clone() : nullptr),
      isMask1StationaryData(theOther.isMask1StationaryData),
      itsCalculationOperator(theOther.itsCalculationOperator),
      itsMask2(theOther.itsMask2 ? theOther.itsMask2->Clone() : nullptr),
      isMask2StationaryData(theOther.isMask2StationaryData)
{
}

void NFmiSimpleConditionPart::Initialize(void)
{
  try
  {
    isMask1StationaryData = ::CheckForStationaryData(itsMask1);
    isMask2StationaryData = ::CheckForStationaryData(itsMask2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSimpleConditionPart *NFmiSimpleConditionPart::Clone(void) const
{
  try
  {
    return new NFmiSimpleConditionPart(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::Value(const NFmiCalculationParams &theCalculationParams,
                                      bool fUseTimeInterpolationAlways)
{
  try
  {
    double value1 =
        itsMask1->Value(theCalculationParams,
                        ::UseTimeInterpolation(isMask1StationaryData, fUseTimeInterpolationAlways));
    if (!itsMask2)
      return value1;

    double value2 =
        itsMask2->Value(theCalculationParams,
                        ::UseTimeInterpolation(isMask2StationaryData, fUseTimeInterpolationAlways));
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::PressureValue(double thePressure,
                                              const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = itsMask1->PressureValue(thePressure, theCalculationParams);
    if (!itsMask2)
      return value1;

    double value2 = itsMask2->PressureValue(thePressure, theCalculationParams);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

double NFmiSimpleConditionPart::HeightValue(double theHeight,
                                            const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = itsMask1->HeightValue(theHeight, theCalculationParams);
    if (!itsMask2)
      return value1;

    double value2 = itsMask2->HeightValue(theHeight, theCalculationParams);
    return ::CalculateValue(value1, value2, itsCalculationOperator);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSingleCondition::~NFmiSingleCondition(void) = default;

NFmiSingleCondition::NFmiSingleCondition(const boost::shared_ptr<NFmiSimpleConditionPart> &thePart1,
                                         FmiMaskOperation theConditionOperand1,
                                         const boost::shared_ptr<NFmiSimpleConditionPart> &thePart2,
                                         FmiMaskOperation theConditionOperand2,
                                         const boost::shared_ptr<NFmiSimpleConditionPart> &thePart3)
    : part1(thePart1),
      conditionOperand1(theConditionOperand1),
      part2(thePart2),
      conditionOperand2(theConditionOperand2),
      part3(thePart3)
{
}

NFmiSingleCondition::NFmiSingleCondition(const NFmiSingleCondition &theOther)
    : part1(theOther.part1 ? theOther.part1->Clone() : nullptr),
      conditionOperand1(theOther.conditionOperand1),
      part2(theOther.part2 ? theOther.part2->Clone() : nullptr),
      conditionOperand2(theOther.conditionOperand2),
      part3(theOther.part3 ? theOther.part3->Clone() : nullptr)
{
}

static void InitializePart(boost::shared_ptr<NFmiSimpleConditionPart> &part)
{
  try
  {
    if (part)
      part->Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiSingleCondition::Initialize(void)
{
  try
  {
    InitializePart(part1);
    InitializePart(part2);
    InitializePart(part3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSingleCondition *NFmiSingleCondition::Clone(void) const
{
  try
  {
    return new NFmiSingleCondition(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool EvaluateCondition(double value1, FmiMaskOperation operand, double value2)
{
  try
  {
    if (value1 == kFloatMissing || value2 == kFloatMissing)
      return false;

    if (operand == kFmiNoMaskOperation)
      return false;

    switch (operand)
    {
      case kFmiMaskEqual:
        return value1 == value2;
      case kFmiMaskGreaterThan:
        return value1 > value2;
      case kFmiMaskLessThan:
        return value1 < value2;
      case kFmiMaskGreaterOrEqualThan:
        return value1 >= value2;
      case kFmiMaskLessOrEqualThan:
        return value1 <= value2;
      case kFmiMaskNotEqual:
        return value1 != value2;
      default:
        throw Fmi::Exception(
            BCP,
            "Error in SimpleCondition's EvaluateCondition function: unsupported condition operand");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool IsGreaterOperand(FmiMaskOperation operand)
{
  try
  {
    return (operand == kFmiMaskGreaterThan || operand == kFmiMaskGreaterOrEqualThan);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Note: only range option accepted are:
// 1. In between case e.g. "limit1 < x < limit2", where operands can be < or <=
// 2. Outside case e.g. "limit1 > x > limit2", where operands can be > or >=
static bool EvaluateRangeCondition(double value1,
                                   FmiMaskOperation operand1,
                                   double value2,
                                   FmiMaskOperation operand2,
                                   double value3)
{
  try
  {
    bool condition1 = ::EvaluateCondition(value1, operand1, value2);
    bool condition2 = ::EvaluateCondition(value2, operand2, value3);
    if (::IsGreaterOperand(operand1))
      // Outside limits case e.g. "-5 > T > 0", only other condition is true
      return condition1 ^ condition2;

    // In between case e.g. "-5 < T < 0", both conditions must be true
    return condition1 && condition2;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSingleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  try
  {
    double value1 = part1->Value(theCalculationParams, fUseTimeInterpolationAlways);
    double value2 = part2->Value(theCalculationParams, fUseTimeInterpolationAlways);
    if (!part3)
      return ::EvaluateCondition(value1, conditionOperand1, value2);

    double value3 = part3->Value(theCalculationParams, fUseTimeInterpolationAlways);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSingleCondition::CheckPressureCondition(double thePressure,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = part1->PressureValue(thePressure, theCalculationParams);
    double value2 = part2->PressureValue(thePressure, theCalculationParams);
    if (!part3)
      return ::EvaluateCondition(value1, conditionOperand1, value2);

    double value3 = part3->PressureValue(thePressure, theCalculationParams);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSingleCondition::CheckHeightCondition(double theHeight,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    double value1 = part1->HeightValue(theHeight, theCalculationParams);
    double value2 = part2->HeightValue(theHeight, theCalculationParams);
    if (!part3)
      return ::EvaluateCondition(value1, conditionOperand1, value2);

    double value3 = part3->HeightValue(theHeight, theCalculationParams);
    return ::EvaluateRangeCondition(value1, conditionOperand1, value2, conditionOperand2, value3);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSimpleCondition::~NFmiSimpleCondition(void) = default;

NFmiSimpleCondition::NFmiSimpleCondition(
    const boost::shared_ptr<NFmiSingleCondition> &theCondition1,
    NFmiAreaMask::BinaryOperator theConditionOperator,
    const boost::shared_ptr<NFmiSingleCondition> &theCondition2)
    : condition1(theCondition1), conditionOperator(theConditionOperator), condition2(theCondition2)
{
}

NFmiSimpleCondition::NFmiSimpleCondition(const NFmiSimpleCondition &theOther)
    : condition1(theOther.condition1 ? theOther.condition1->Clone() : nullptr),
      conditionOperator(theOther.conditionOperator),
      condition2(theOther.condition2 ? theOther.condition2->Clone() : nullptr)
{
}

static void InitializePart(boost::shared_ptr<NFmiSingleCondition> &condition)
{
  try
  {
    if (condition)
      condition->Initialize();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// T�t� kutsutaan konstruktorin j�lkeen, t�ss� alustetaan ainakin tieto siit� onko maski ns.
// stationaarist� dataa
void NFmiSimpleCondition::Initialize(void)
{
  try
  {
    ::InitializePart(condition1);
    ::InitializePart(condition2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

NFmiSimpleCondition *NFmiSimpleCondition::Clone(void) const
{
  try
  {
    return new NFmiSimpleCondition(*this);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

static bool EvaluateBinaryCondition(bool condition1,
                                    NFmiAreaMask::BinaryOperator conditionOperator,
                                    bool condition2)
{
  try
  {
    switch (conditionOperator)
    {
      case NFmiAreaMask::kAnd:
        return condition1 && condition2;
      case NFmiAreaMask::kOr:
        return condition1 || condition2;
      case NFmiAreaMask::kXor:
        return condition1 ^ condition2;
      default:
        throw Fmi::Exception(
            BCP,
            "Error in SimpleCondition's EvaluateCondition function: unsupported binary operator");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// Eri tapauksia varten omat tarkastelu funktiot
bool NFmiSimpleCondition::CheckCondition(const NFmiCalculationParams &theCalculationParams,
                                         bool fUseTimeInterpolationAlways)
{
  try
  {
    bool conditionValue1 =
        condition1->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 =
        condition2->CheckCondition(theCalculationParams, fUseTimeInterpolationAlways);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSimpleCondition::CheckPressureCondition(double thePressure,
                                                 const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool conditionValue1 = condition1->CheckPressureCondition(thePressure, theCalculationParams);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 = condition2->CheckPressureCondition(thePressure, theCalculationParams);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool NFmiSimpleCondition::CheckHeightCondition(double theHeight,
                                               const NFmiCalculationParams &theCalculationParams)
{
  try
  {
    bool conditionValue1 = condition1->CheckHeightCondition(theHeight, theCalculationParams);
    if (!condition2)
      return conditionValue1;

    bool conditionValue2 = condition2->CheckHeightCondition(theHeight, theCalculationParams);
    return ::EvaluateBinaryCondition(conditionValue1, conditionOperator, conditionValue2);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}
