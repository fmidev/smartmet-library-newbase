// ======================================================================
/*!
 * \file NFmiSvgPath.cpp
 * \brief Implementation of class NFmiSvgPath
 */
// ======================================================================
/*!
 * \class NFmiSvgPath
 *
 * \brief Yksinkertaistetti SVG polku ilman k�yri� polkuelementtej�.
 *
 *  Lukee ja kirjoittaa SVG-formaatilla talletetun polku-tiedot.
 *
 * Huom! Ainoat sallitut elementit ovat absoluuttinen moveto ja lineto
 * sek� closepath. Relatiiviset moveto ja lineto k�skyt muutetaan
 * luettaessa absoluuttisiksi. Bezier-elementtej� ei tueta ollenkaan.
 */
// ======================================================================

#include "NFmiSvgPath.h"
#include "NFmiPoint.h"

#include <iterator>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

// ----------------------------------------------------------------------
//				LOCAL HIDDEN FUNCTIONS
// ----------------------------------------------------------------------

namespace
{
// ----------------------------------------------------------------------
/*!
 *  sy� spacet pois streamista ja palauttaa true:n jos ei olla lopussa
 *
 * \param theInput The input stream
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool EatWhiteSpaces(istream& theInput)
{
  char ch = '\0';
  do
  {
    ch = static_cast<char>(theInput.get());
  } while (isspace(ch));
  if (theInput.fail())
    return false;  // jos stremin lopussa, ep�onnistuu
  else
    theInput.unget();
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Muunna SVG elementti merkiksi
 *
 * \param theType Elementtityyppi
 * \return Merkki, tai '?'
 */
// ----------------------------------------------------------------------

char SvgElementToChar(NFmiSvgPath::ElementType theType)
{
  switch (theType)
  {
    case NFmiSvgPath::kElementMoveto:
      return 'M';
    case NFmiSvgPath::kElementLineto:
      return 'L';
    case NFmiSvgPath::kElementClosePath:
      return 'Z';
    default:
      return '?';
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Testaa onko kirjain SVG elementin k�sky
 *
 * \param theChar Testattava kirjain
 * \return Elementtityyppi, mahdollisesti kElementNotValid
 */
// ----------------------------------------------------------------------

NFmiSvgPath::ElementType CharToSvgElement(char theChar)
{
  switch (theChar)
  {
    case 'M':
    case 'm':
      return NFmiSvgPath::kElementMoveto;
    case 'L':
    case 'l':
      return NFmiSvgPath::kElementLineto;
    case 'Z':
    case 'z':
      return NFmiSvgPath::kElementClosePath;
    default:
      return NFmiSvgPath::kElementNotValid;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Testaa onko kirjain suhteellinen SVG elementin k�sky
 *
 * \param theChar Testattava kirjain
 * \return True, jos kirjain viittaa suhteelliseen k�skyyn
 */
// ----------------------------------------------------------------------

bool IsRelativeSvgElement(char theChar) { return (theChar >= 'a' && theChar <= 'z'); }
// ----------------------------------------------------------------------
/*!
 *  Irroittaa streamista aina stringin, joka on seuraavan ""-blokin sis�ll�.
 *
 * \param theInput Undocumented
 * \param thePolygonDataStr Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool GetPolygonStringFromStream(istream& theInput, string& thePolygonDataStr)
{
  char ch = '\0';
  do  // luetaan streamia kunnes loppuu tai tulee "-merkki
  {
    theInput.get(ch);
  } while ((!theInput.fail()) && ch != '\"');

  // luetaan streamia kunnes loppuu tai tulee "-merkki ja
  // lis�t��n merkit stringiin
  do
  {
    theInput.get(ch);
    if ((!theInput.fail()) && ch != '\"')
      thePolygonDataStr += ch;
    else if ((!theInput.fail()) && ch == '\"')
      return true;  // saatiin "" -blokin v�list� merkit kunnialla talteen, palautetaan true
  } while ((!theInput.fail()) && ch != '\"');  // tupla tarkistus fail-bittiin (t�m� on turha
                                               // periaateessa koska loopissa on tarkistus)

  return false;
}

// ----------------------------------------------------------------------
/*!
 *  K�ytet��n SVG-formaattia talletuksessa.
 *  Bezieriereit� ja ympyr�it� ei ole toteutettu.
 *  Polygonin tiedosto formaatti on seuraava:
 *
 * \code
 *  "
 *  type1 type1data1 type1data2 jne.
 *  type2 type2data1 type2data2 jne.
 *  type3 type3data1 type3data2 jne.
 *  jne.
 *  "
 * \endcode
 *  T�m� on esim. simppelin polygonin yhteydess� seuraava (0,0 10,10 laatikko):
 * \code
 *  "
 *  M 0 0
 *  L 0 10
 *  L 10 10
 *  L 10 0
 *  L 0 0
 *  "
 * \endcode
 *  eli vaikka yhdell� rivill�:
 * \code
 *  "M 0 0 L 0 10 L 10 10 L 10 0 L 0 0"
 * \endcode
 *
 * Huom! Closepath k�skyn loppukoordinaatiksi tallennetaan
 * se koordinaatti, mihin closepath siirtyy.
 *
 * \param thePath The path into which the data is appended
 * \param theSvgPathString The path to parse
 * \return True, if everything parsed fine
 */
// ----------------------------------------------------------------------

bool ExtractSvgPath(NFmiSvgPath& thePath, const string& theSvgPathString)
{
  stringstream strStream(theSvgPathString);
  char elementTypeChar = '\0';
  double x = 0, y = 0;
  NFmiSvgPath::ElementType eType = NFmiSvgPath::kElementNotValid;

  double firstX = 0, firstY = 0;
  double lastX = 0, lastY = 0;

  bool isRelative = false;
  do
  {
    if (!EatWhiteSpaces(strStream)) break;

    // katsotaan peek:illa ensin onko merkki, koska k�skyt
    // voivat olla yhdess� 1. koordinaatin kanssa.

    elementTypeChar = static_cast<char>(strStream.peek());

    // Jos l�ytyy merkki, saadaan uusi k�sky, muuten vanha
    // k�sky j�� voimaan ja luetaan vain koordinaatteja.
    // Kuitenkin jos vanha k�sky oli moveto, muutetaan
    // se automaattisesti lineto k�skyksi.

    if (isalpha(elementTypeChar))
    {
      strStream.get(elementTypeChar);
      eType = CharToSvgElement(elementTypeChar);
      isRelative = IsRelativeSvgElement(elementTypeChar);
    }
    else if (elementTypeChar == ',')
    {
      strStream.get();
      continue;
    }
    else if (eType == NFmiSvgPath::kElementMoveto)
      eType = NFmiSvgPath::kElementLineto;

    // Tarkistetaan luvun onnistuminen

    if (strStream.fail()) return false;

    switch (eType)
    {
      case NFmiSvgPath::kElementMoveto:
      case NFmiSvgPath::kElementLineto:
      {
        strStream >> x;
        if (strStream.fail()) return false;

        if (!EatWhiteSpaces(strStream)) return false;

        if (strStream.peek() == ',') strStream.get();
        strStream >> y;
        if (strStream.fail()) return false;

        if (isRelative)
        {
          lastX += x;
          lastY += y;
        }
        else
        {
          lastX = x;
          lastY = y;
        }
        thePath.push_back(NFmiSvgPath::Element(eType, lastX, lastY));

        if (eType == NFmiSvgPath::kElementMoveto)
        {
          firstX = lastX;
          firstY = lastY;
        }

        break;
      }
      case NFmiSvgPath::kElementClosePath:
      {
        lastX = firstX;
        lastY = firstY;
        thePath.push_back(NFmiSvgPath::Element(eType, lastX, lastY));
        break;
      }
      case NFmiSvgPath::kElementNotValid:
        return false;
    }
  } while (!strStream.fail());
  return (!strStream.fail());
}

}  // namespace anonymous

// ----------------------------------------------------------------------
//			CLASS METHODS
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/*!
 * \brief Konstruktori
 */
// ----------------------------------------------------------------------

NFmiSvgPath::NFmiSvgPath()
    : itsData(), itsBBoxValid(false), itsMinX(0), itsMinY(0), itsMaxX(0), itsMaxY(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Palauta polun koko
 *
 * \return Polun koko
 */
// ----------------------------------------------------------------------

NFmiSvgPath::size_type NFmiSvgPath::size(void) const { return itsData.size(); }
// ----------------------------------------------------------------------
/*!
 * \brief Testaa onko polku tyhj�
 *
 * \return True jos polku on tyhj�
 */
// ----------------------------------------------------------------------

bool NFmiSvgPath::empty(void) const { return itsData.empty(); }
// ----------------------------------------------------------------------
/*!
 * \brief Tyhjenn� polku
 */
// ----------------------------------------------------------------------

void NFmiSvgPath::clear(void)
{
  itsBBoxValid = false;
  itsData.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Lis�� uusi polkuelementti polkuun
 *
 * \param theElement Lis�tt�v� elementti
 */
// ----------------------------------------------------------------------

void NFmiSvgPath::push_back(const Element& theElement)
{
  itsBBoxValid = false;
  itsData.push_back(theElement);
}

// ----------------------------------------------------------------------
/*!
 * Write tekee aina talletuksen 'siistiin' muotoon, joten
 * alkuper�inen tiedostosta luettu voi muuttua kun se kirjoitetaan
 * takaisin tiedostoon. Lis�ksi muutoksia tulee koska relatiiviset
 * siirtym�t on muutettu absoluuttisiin paikkoihin.
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream& NFmiSvgPath::Write(std::ostream& file) const
{
  if (itsData.size() > 0)
  {
    file << '"';
    for (const_iterator it = begin(); it != end(); ++it)
    {
      if (it != begin()) file << ' ';

      file << SvgElementToChar(it->itsType);

      if (it->itsType != kElementClosePath) file << ' ' << it->itsX << ' ' << it->itsY;
    }
    file << '"';
  }
  return file;
}

// ----------------------------------------------------------------------
/*!
 *  Luku on jaettu kahteen osaan. Ensin luetaan inputista stringin p�tk�
 *  joka on 1. ""-blokin sis�ll� oleva merkkijono. Tuosta merkkijonosta
 *  tulkitaan sitten polku. Joten tiedostossa voi olla kommentteja ja
 *  muuta roskaa ilman ett� tulee ongelmia.
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream& NFmiSvgPath::Read(std::istream& file)
{
  itsBBoxValid = false;

  if (file)
  {
    clear();  // Pit��k� tyhjent�� olemassa oleva polku ennen lukua?!?! Nyt tyhjenee.
    string polygonDataStr;
    if (GetPolygonStringFromStream(file, polygonDataStr)) ExtractSvgPath(*this, polygonDataStr);
  }
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief Testaa onko annettu piste polun sis�ll�
 *
 *  Perus Isinside algoritmi on haettu osoitteesta
 *  http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/ kohdasta
 *  Solution 1 (2D). Ratkaisua on muutettu hieman, koska alkuper�inen ei
 *  hanskannut kaikkea kunnolla.
 *
 * \param thePath Polku
 * \param thePoint Testattava piste
 * \return True, jos piste on polun sis�ll�
 */
// ----------------------------------------------------------------------

bool NFmiSvgPath::IsInside(const NFmiPoint& thePoint) const
{
  if (empty()) return false;

  // lyhyt nimi koodin luettavuuden kannalta
  const NFmiPoint& p = thePoint;

  // Ensin pit�� varmistaa, ett� bounding box on ajan tasalla

  if (!itsBBoxValid)
  {
    for (const_iterator it = itsData.begin(); it != itsData.end(); ++it)
    {
      switch (it->itsType)
      {
        case kElementNotValid:
        case kElementClosePath:
          break;
        case kElementMoveto:
        case kElementLineto:
        {
          if (!itsBBoxValid)
          {
            itsMinX = itsMaxX = it->itsX;
            itsMinY = itsMaxY = it->itsY;
            itsBBoxValid = true;
          }
          else
          {
            itsMinX = FmiMin(itsMinX, it->itsX);
            itsMinY = FmiMin(itsMinY, it->itsY);
            itsMaxX = FmiMax(itsMaxX, it->itsX);
            itsMaxY = FmiMax(itsMaxY, it->itsY);
          }
        }
      }
    }
  }

  // K�ytet��n bounding boksia hyv�ksi

  if (itsBBoxValid)
  {
    if (p.X() < itsMinX || p.X() > itsMaxX || p.Y() < itsMinY || p.Y() > itsMaxY) return false;
  }

  // Joudutaan laskemaan suoraan

  NFmiSvgPath::const_iterator firstPoint = begin();
  NFmiSvgPath::const_iterator p1 = begin();
  NFmiSvgPath::const_iterator p2 = begin();

  int counter = 0;

  // hyp�t��n ensimm�isen k�skyn yli, se on aina moveto
  NFmiSvgPath::const_iterator it = begin();
  for (++it; it != end(); ++it)
  {
    p2 = it;
    if (p2->itsType == NFmiSvgPath::kElementMoveto)
      firstPoint = p2;
    else
    {
      if (p2->itsType == NFmiSvgPath::kElementClosePath) p2 = firstPoint;
      if (p.Y() > FmiMin(p1->itsY, p2->itsY) && p.Y() <= FmiMax(p1->itsY, p2->itsY) &&
          p.X() <= FmiMax(p1->itsX, p2->itsX) && p1->itsY != p2->itsY)
      {
        const double xinters =
            ((p.Y() - p1->itsY) * (p2->itsX - p1->itsX) / (p2->itsY - p1->itsY) + p1->itsX);
        if (p1->itsX == p2->itsX || p.X() <= xinters) counter++;
      }
    }
    p1 = p2;
  }

  return (counter % 2 != 0);
}

// ======================================================================
