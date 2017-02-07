// ======================================================================
/*!
 * \file NFmiAreaMaskList.cpp
 * \brief Implementation of class NFmiAreaMaskList
 */
// ======================================================================
/*!
 * \class NFmiAreaMaskList
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiAreaMaskList.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::~NFmiAreaMaskList(void) {}
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiAreaMaskList::NFmiAreaMaskList(void) : itsMaskVector(), itsCurrentIndex(-1), fMaskInUse(false)
{
}

NFmiAreaMaskList::NFmiAreaMaskList(const NFmiAreaMaskList &theOther)
    : itsMaskVector(theOther.itsMaskVector),
      itsCurrentIndex(theOther.itsCurrentIndex),
      fMaskInUse(theOther.fMaskInUse)
{
}

boost::shared_ptr<NFmiAreaMaskList> NFmiAreaMaskList::CreateShallowCopy(
    const boost::shared_ptr<NFmiAreaMaskList> &theOther)
{
  if (theOther)
  {
    boost::shared_ptr<NFmiAreaMaskList> copyMaskList(new NFmiAreaMaskList());
    for (theOther->Reset(); theOther->Next();)
    {
      boost::shared_ptr<NFmiAreaMask> copyedMask(theOther->Current()->Clone());
      copyMaskList->Add(copyedMask);
    }
    copyMaskList->CheckIfMaskUsed();
    return copyMaskList;
  }
  else
    return boost::shared_ptr<NFmiAreaMaskList>();
}

unsigned long NFmiAreaMaskList::NumberOfItems(void)
{
  return static_cast<unsigned long>(itsMaskVector.size());
}

// ----------------------------------------------------------------------
/*!
 * \param theMask Undocumented, unused parameter
 * \return Undocumented
 *
 * \todo Investigate whether theMask parameter can be removed
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Add(boost::shared_ptr<NFmiAreaMask> &theMask)
{
  itsMaskVector.push_back(theMask);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 *�\todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Reset(void)
{
  itsCurrentIndex = -1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Next(void)
{
  itsCurrentIndex++;
  if (static_cast<std::size_t>(itsCurrentIndex) < itsMaskVector.size())
    return true;
  else
    return false;
}

// T�m� tarkistaa vektori-taulukkoon osoittavia indeksej�, jotka alkavat 0:sta.
bool NFmiAreaMaskList::IsValidIndex(int theIndex)
{
  if (theIndex >= 0 && static_cast<std::size_t>(theIndex) < itsMaskVector.size())
    return true;
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented, always zero pointer
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

boost::shared_ptr<NFmiAreaMask> NFmiAreaMaskList::Current(void)
{
  if (IsValidIndex(itsCurrentIndex))
    return itsMaskVector[itsCurrentIndex];
  else
  {
    // jos indeksi on vektorin ulkopuolella, palautetaan 0-pointteri (shared_ptr tyhj�n�)
    static boost::shared_ptr<NFmiAreaMask> dummy;
    return dummy;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Remove(void)
{
  if (IsValidIndex(itsCurrentIndex))
  {
    itsMaskVector.erase(itsMaskVector.begin() + itsCurrentIndex);
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 *  K�y l�pi listassa olevat maski-oliot ja kysyy
 *  niilt� IsMasked ja tekee tulosta AND operaation kanssa.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::IsMasked(const NFmiPoint &theLatLon)
{
  if (fMaskInUse)  // 1999.09.24/Marko Muutin t�m�n katsomaan ensin onko maski k�yt�ss�
  {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                   // n�in p��see eroon muutamasta ik�v�st� if-lause testeist�
    for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
    {
      if (itsMaskVector[index]->IsEnabled())
      {
        if (!(itsMaskVector[index]->IsMasked(theLatLon)))
        {
          return false;
        }
      }
    }
    return true;
  }
  else
    return true;
}

// ----------------------------------------------------------------------
/*!
 * k�y kaikki listan maskit l�pi ja katsoo, ettei yhdenk��n maskin kerroin ole 0,
 * t�ll�in palautetaan 0. Jos ei ole yht��n 0-kerrointa, lasketaan ramppimaskien
 * keskiarvo, joka palautetaan (tavallisia maskeja ei lasketa t�ss� koska ne
 * mukaan lukien tulisi liian rankkoja muutoksia). Jos vain tavallisia maskeja, palautetaan arvo 1.
 * \param theLatLon Undocumented, unused
 * \return Undocumented, always kFloatMissing
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

double NFmiAreaMaskList::MaskValue(const NFmiPoint &theLatLon)
{
  if (fMaskInUse)  // 1999.09.24/Marko Muutin t�m�n katsomaan ensin onko maski k�yt�ss�
  {                // jos on, katsotaan onko maskattu, muuten on aina maskattu
                   // n�in p��see eroon muutamasta ik�v�st� if-lause testeist�
    double sum = 0, tempValue = 0;
    int count = 0;
    for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
    {
      if (itsMaskVector[index]->IsEnabled())
      {
        tempValue = itsMaskVector[index]->MaskValue(theLatLon);
        if (!tempValue)
        {
          return 0.;
        }
        else
        {
          if (itsMaskVector[index]->IsRampMask())
          {
            sum += tempValue;
            count++;
          }
        }
      }
    }
    if (count) return sum / count;
  }
  return 1.;
}

// ----------------------------------------------------------------------
/*!
 *  Etsii annetun indeksin mukaisen otuksen listasta ja asettaa itsCurrentIndex:in osoittamaan
 * siihen.
 *  HUOM! theIndex on 1:sta alkava indeksi, sis�inen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex 1:ll� alkava indeksi jota etsit��n maski vektorista.
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(unsigned long theIndex)
{
  int vectorIndex = static_cast<int>(theIndex) - 1;
  if (IsValidIndex(vectorIndex))
  {
    itsCurrentIndex = vectorIndex;
    return true;
  }
  else
  {
    itsCurrentIndex = -1;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param fDeleteData Undocumented, unused
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

void NFmiAreaMaskList::Clear(void) { itsMaskVector.clear(); }
// ----------------------------------------------------------------------
/*!
 *  Tarkistaa listassa olevilta maskeilta, onko
 *  yht��n maskia k�yt�ss� (enabled). Jos on, asetetaan
 *  fMaskInUse attribuutti true:ksi jos ei asetetaan
 *  se false:ksi. Funktio palauttaa fMaskInUse:in arvon.
 *  Jatkossa listalta voidaan kysy� metodilla UseMask
 *  fMaskInUse:n tilaa. (t�m� on optimointia varten tehty
 *  viritelm�)
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::CheckIfMaskUsed(void)
{
  fMaskInUse = false;
  for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
  {
    if (itsMaskVector[index]->IsEnabled())
    {
      fMaskInUse = true;
    }
  }
  return fMaskInUse;
}

// ----------------------------------------------------------------------
/*!
 *  K�y enabloidut maskit l�pi antaa theTime:n
 *  niiden Time-metodille.
 * \param theTime Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::SyncronizeMaskTime(const NFmiMetTime &theTime)
{
  for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
  {
    if (itsMaskVector[index]->IsEnabled()) itsMaskVector[index]->Time(theTime);
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 *  HUOM! theIndex on 1:sta alkava indeksi, sis�inen muuttuja itsCurrentIndex on taas 0:sta alkava
 * \param theIndex theIndex alkaa 1:st�
 * \return jos l�ytyi halutulla indeksill� oleva maski, palauttaa true
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Index(unsigned long theIndex) { return Find(theIndex); }
// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam)
{
  for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
  {
    if (itsMaskVector[index]->IsWantedParam(theParam)) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented, unused
 * \param theLevel Undocumented, unused
 * \return Undocumented, always false
 * \todo Make this an abstract method?
 */
// ----------------------------------------------------------------------

bool NFmiAreaMaskList::Find(const NFmiDataIdent &theParam, const NFmiLevel *theLevel)
{
  for (int index = 0; index < static_cast<int>(itsMaskVector.size()); index++)
  {
    if (itsMaskVector[index]->IsWantedParam(theParam, theLevel)) return true;
  }
  return false;
}

// ======================================================================
