/*!
 *
 *

> Olen etsim�ss� juuri editori kirjastoista mahdollisia pointteri virhe
> kaato mahdollisuuksia. Monissa luokissa mit� k�yt�n on pointtereit�
> toisiin olioihin ja n�m� pointterit alustetaan usein konstruktorissa.
> Niit� ei tarkasteta my�hemmin kun rakennettua oliota k�ytet��n
> my�hemmin. Tuli vain mieleen, pit�isik� tehd� joku 0-pointteri
> poikkeusluokka (vaikka newbase:een), jota voisi heitell� eri luokkien
> konstruktoreista t�ll�isiss� tapauksissa. Silloin tulisi tarkastettua
> pointterit ja ongelmat voisi voisi k�sitell� ylemp�n� ja luokkia voisi
> k�ytt�� turvallisemmin kun niiden dataosien pointterit olisi
> tarkastettu.

Miten olisi checked_ptr templaatti, joka olisi kuten auto_ptr, mutta
ei omistaisi dataa, ja nollapointterin dereferointi heitt�isi aina
poikkeuksen? Parempi saattaisi olla smart_ptr, jolla olisi vastaava
policy, mutta se taitaa vaatia enemm�n muutoksia kuin checked_ptr.

Ehk� voisi yritt�� ensin siirty� checked_ptr luokkaan, ja sitten siirty�
asteittain smart_ptr luokkaan?

K�ytin mallina g++:n auto_ptr luokkaa, ja yksinkertaistin hieman asioita,
alla on (k��nt�m�t�n) lopputulos.

  *ptr ja
  ptr->metodi

heitt�isiv�t veivin, jos pointteri on nolla. get() kuitenkin toimisi ihan
normaalisti, sit� kautta saa testattua sis�ll�n. My�skin delete lakkaa
toimimasta, koska objekti ei olekaan en�� pointteri, eli mahdolliset
deallokointi virheet l�ytyv�t t�t� kautta.
 */

#ifndef CHECKED_PTR_H
#define CHECKED_PTR_H

#include <stdexcept>

struct checked_ptr_err : public std::runtime_error
{
  checked_ptr_err(const std::string& s) : std::runtime_error(s) {}
};

struct checked_ptr_deref_err : public checked_ptr_err
{
  checked_ptr_deref_err(const std::string& s) : checked_ptr_err(s) {}
};

struct checked_ptr_get_err : public checked_ptr_err
{
  checked_ptr_get_err(const std::string& s) : checked_ptr_err(s) {}
};

template <typename T>
class checked_ptr
{
 private:
  T* p;

 public:
  typedef T element_type;
  ~checked_ptr() {}
  explicit checked_ptr(element_type* ptr = 0) : p(ptr) {}
  checked_ptr(const checked_ptr& P) : p(P.p) {}
  checked_ptr& operator=(element_type* ptr)
  {
    p = ptr;
    return *this;
  }
  checked_ptr& operator=(const checked_ptr& P)
  {
    p = P.p;
    return *this;
  }
  element_type* get() { return p; }
  element_type& operator*() const
  {
    if (p) return *p;
    throw checked_ptr_deref_err("Trying to dereference a null checked_ptr");
  }

  element_type* operator->() const
  {
    if (p) return p;
    throw checked_ptr_get_err("Trying to get a null checked_ptr");
  }

  void reset(element_type* ptr = 0) { p = ptr; }
};

#endif  // CHECKED_PTR_H
