// ======================================================================
/*!
 * \file NFmiMetTime.h
 * \brief Interface of class NFmiMetTime
 */
// ======================================================================

#pragma once

#include "NFmiTime.h"
#include "NFmiTimePeriod.h"

class NFmiLocation;

//! Undocumented
class _FMI_DLL NFmiMetTime : public NFmiTime
{
 public:
  static const NFmiMetTime gMissingTime;  // Tätä missing aikaa voi käyttää, jos haluaa ilmaista
                                          // että jokin aika ei mahdollisesti ole initialisoitu

  explicit NFmiMetTime(long timeStepInMinutes = 60);
  NFmiMetTime(const NFmiMetTime& aMetTime);
  NFmiMetTime(long datePart, long timePart);
  NFmiMetTime(const NFmiMetTime& aMetTime, short negRange, short posRange);
  NFmiMetTime(const NFmiTime& aTime,
              long theConstructMetTime = 60,
              short negRange = 0,
              short posRange = 0);
  NFmiMetTime(short year, short month, short day);
  NFmiMetTime(short year,
              short month,
              short day,
              short hour,
              short minute = 0,
              short sec = 0,
              long timeStep = 60,
              short negRange = 0,
              short posRange = 0);

  // NfmiMetTime timestamp with seconds and minutes
  static NFmiMetTime now();

  // Explicit conversion to boost::posix_time::ptime
  boost::posix_time::ptime PosixTime() const;

  // Implicit conversion to and from boost::posix_time::ptime
  NFmiMetTime(const boost::posix_time::ptime& thePosixTime);
  operator boost::posix_time::ptime() const { return PosixTime(); }
  // And local_date_time
  NFmiMetTime(const boost::local_time::local_date_time& theLocalTime);

  bool IsEqual(const NFmiSortable& aFmiTest) const override;

  NFmiMetTime& operator=(const NFmiMetTime& /*aTime*/);
  NFmiMetTime& operator++();          // prefix
  NFmiMetTime& operator--();          // prefix
  const NFmiMetTime operator++(int);  // postfix
  const NFmiMetTime operator--(int);  // postfix

  void NextMetTime();
  void NextMetTime(long deltaInMinutes);
  void NextMetTime(const NFmiTimePerioid& perioid);
  void PreviousMetTime();
  void PreviousMetTime(long deltaInMinutes);
  void PreviousMetTime(const NFmiTimePerioid& perioid);
  void NearestMetTime(FmiDirection theDirect = kNoDirection);
  void NearestMetTime(long deltaInMinutes, FmiDirection theDirect = kNoDirection);
  void NearestMetTime(const NFmiTimePerioid& perioid, FmiDirection theDirect = kNoDirection);

  // Näiden pitäisi mielestäni muuttaa olion sisäistä arvoa, vaan nämäpä
  // palauttavatkin uuden arvon jättäen olion arvot entiselleen!
  // Eli selkeyden vuoksi pitäisi olla GetNextObservation(), Get...
  //    NFmiMetTime NextObservation( const short deltaInMinutes ) const;
  //    NFmiMetTime PreviousObservation( const short deltaInMinutes ) const;
  //    NFmiMetTime NearestObservation( const short deltaInMinutes ) const;

  void SetTimeStep(long timeStepInMinutes,
                   bool fSetTime = true,
                   FmiDirection theDirect = kNoDirection);

  long GetTimeStep() const;

  short GetLocalHour() const;

  const NFmiTime UTCTime(float theLongitude = kFloatMissing) const override;
  const NFmiTime UTCTime(const NFmiLocation& theLocation) const override;
  const NFmiTime LocalTime(float theLongitude = kFloatMissing) const override;
  const NFmiTime LocalTime(const NFmiLocation& theLocation) const override;

  virtual const NFmiTime CorrectLocalTime() const;

  const char* ClassName() const override;

  std::ostream& Write(std::ostream& file) const override;
  virtual std::istream& Read(std::istream& file);

 protected:
  struct std::tm GetSystemTime() override;

 private:
  void ConstructMetTime(long timeStepInMinutes, FmiDirection theDirect = kNoDirection);

 private:
  NFmiTimePerioid fTimeStepInMinutes;
  short itsNegativeRange;
  short itsPositiveRange;

};  // class NFmiMetTime

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param datePart Undocumented
 * \param timePart Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const long datePart, const long timePart)
    : NFmiTime(datePart, timePart), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  SetTimeStep(60);
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param aMetTime The object being copied
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const NFmiMetTime& aMetTime)
    : NFmiTime(aMetTime),
      fTimeStepInMinutes(aMetTime.fTimeStepInMinutes),
      itsNegativeRange(aMetTime.itsNegativeRange),
      itsPositiveRange(aMetTime.itsPositiveRange)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aMetTime The time to copy
 * \param negRange The negative range
 * \param posRange The positive range
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const NFmiMetTime& aMetTime, short negRange, short posRange)
    : NFmiTime(aMetTime),
      fTimeStepInMinutes(aMetTime.fTimeStepInMinutes),
      itsNegativeRange(negRange),
      itsPositiveRange(posRange)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline long NFmiMetTime::GetTimeStep() const { return fTimeStepInMinutes; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiMetTime::NearestMetTime(FmiDirection theDirect)
{
  NearestMetTime(GetTimeStep(), theDirect);
  SetSec(0);  // tämä korjaa sekunti bugin NerarestMetTime(step)-metodista. Olisin laittanut
              // korjauksen kyseiseen metodiin, mutta MSVC++ 7.1 kääntäjä ei ota jostain syystä
              // koodi muutoksia huomioon!!!
}

// ----------------------------------------------------------------------
/*!
 * \param timeStepInMinutes Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiMetTime::ConstructMetTime(const long timeStepInMinutes, FmiDirection theDirect)
{
  fTimeStepInMinutes = timeStepInMinutes;
  NearestMetTime(theDirect);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param aTime Undocumented
 * \param theConstructMetTime Undocumented
 * \param negRange Undocumented
 * \param posRange Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const NFmiTime& aTime,
                                long theConstructMetTime,
                                short negRange,
                                short posRange)
    : NFmiTime(aTime), fTimeStepInMinutes(), itsNegativeRange(negRange), itsPositiveRange(posRange)
{
  ConstructMetTime(theConstructMetTime);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 * \todo There is no need to declare the parameters const
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const short year, const short month, const short day)
    : NFmiTime(year, month, day), fTimeStepInMinutes(), itsNegativeRange(0), itsPositiveRange(0)
{
  ConstructMetTime(60);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param year Undocumented
 * \param month Undocumented
 * \param day Undocumented
 * \param hour Undocumented
 * \param minute Undocumented
 * \param sec Undocumented
 * \param timeStep Undocumented
 * \param negRange Undocumented
 * \param posRange Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiMetTime::NFmiMetTime(const short year,
                                const short month,
                                const short day,
                                const short hour,
                                const short minute,
                                const short sec,
                                long timeStep,
                                short negRange,
                                short posRange)
    : NFmiTime(year, month, day, hour, minute, sec),
      fTimeStepInMinutes(),
      itsNegativeRange(negRange),
      itsPositiveRange(posRange)
{
  ConstructMetTime(timeStep);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiMetTime::PreviousMetTime() { PreviousMetTime(GetTimeStep()); }

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline short NFmiMetTime::GetLocalHour() const
{
  return static_cast<short>((GetHour() - GetZoneDifferenceHour()) % 24);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiMetTime::ClassName() const { return "NFmiMetTime"; }
// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiMetTime
 *
 * \param os The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const NFmiMetTime& ob) { return ob.Write(os); }
// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiMetTime
 *
 * \param is The input stream to read from
 * \param ob The object to read
 * \return The input stream readen from
 */
// ----------------------------------------------------------------------

inline std::istream& operator>>(std::istream& is, NFmiMetTime& ob) { return ob.Read(is); }

// ======================================================================
