// ======================================================================
/*!
 * \file NFmiVoidPtrList.cpp
 * \brief Implementation of class NFmiVoidPtrList
 * \todo This file should be annihilated
 */
// ======================================================================

#include "NFmiVoidPtrList.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiVoidPtrList::NFmiVoidPtrList()
    : itsFirstItem(nullptr), itsCurrentItem(nullptr), itsPreviousItem(nullptr), itsNumberOffItems(0)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param listItem The other list being copied
 */
// ----------------------------------------------------------------------

NFmiVoidPtrList::NFmiVoidPtrList(const NFmiVoidPtrList &listItem)
    : itsFirstItem(nullptr), itsCurrentItem(nullptr), itsPreviousItem(nullptr), itsNumberOffItems(0)
{
  CopyList(listItem);
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Add(NFmiVoidPtrData *value) { AddEnd(value); }
// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddStart(NFmiVoidPtrData *value)
{
  if (value != nullptr)
  {
    if (itsFirstItem != nullptr)
      itsFirstItem = new NFmiVoidPtrItem(value, itsFirstItem);
    else
      itsFirstItem = itsCurrentItem = new NFmiVoidPtrItem(value, itsFirstItem);

    ++itsNumberOffItems;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddEnd(NFmiVoidPtrData *value)
{
  if (value != nullptr)
  {
    ++itsNumberOffItems;
    if (itsFirstItem == nullptr)
    {
      itsFirstItem = itsCurrentItem = new NFmiVoidPtrItem(value);
    }
    else
    {
      while (itsCurrentItem->itsNextItem != nullptr)
        itsCurrentItem = itsCurrentItem->itsNextItem;
      itsCurrentItem->itsNextItem = new NFmiVoidPtrItem(value);
      itsPreviousItem = itsCurrentItem;
      itsCurrentItem = itsCurrentItem->itsNextItem;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::AddBefore(NFmiVoidPtrData *value)
{
  if (value != nullptr)
  {
    if (itsCurrentItem == nullptr)  // empty list
      AddEnd(value);
    else if (itsPreviousItem == nullptr)  // added before first item
    {
      itsFirstItem = itsPreviousItem = new NFmiVoidPtrItem(value);
      itsFirstItem->itsNextItem = itsCurrentItem;
      ++itsNumberOffItems;
    }
    else
    {
      itsPreviousItem = itsPreviousItem->itsNextItem = new NFmiVoidPtrItem(value);
      itsPreviousItem->itsNextItem = itsCurrentItem;
      ++itsNumberOffItems;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \param removeValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Remove(NFmiVoidPtrData *removeValue)
{
  NFmiVoidPtrItem *temp = itsFirstItem;
  if (temp->itsValue == removeValue)
  {
    DeleteItem();
    itsNumberOffItems--;
    return;
  }
  NFmiVoidPtrItem *previousItem = nullptr;

  for (; temp != nullptr; previousItem = temp, temp = temp->itsNextItem)
    if (temp->itsValue == removeValue)
    {
      previousItem->itsNextItem = temp->itsNextItem;
      delete temp;
      itsNumberOffItems--;
      return;
    }
}

// ----------------------------------------------------------------------
/*!
 * \param doDelete Undocumented, unused
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::Clear(
    bool /* doDelete */)  // miksi ei tarkisteta tätä ja jätetä deletoimatta tarvittaessa?
{
  while (itsFirstItem != nullptr)
  {
    DeleteItem();
  }
  itsNumberOffItems = 0;
  itsCurrentItem = nullptr;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::DeleteItem()
{
  NFmiVoidPtrItem *deleteItem;
  deleteItem = itsFirstItem;
  itsFirstItem = itsFirstItem->itsNextItem;
  if (deleteItem == itsCurrentItem) itsCurrentItem = itsFirstItem;
  delete deleteItem;
}

// ----------------------------------------------------------------------
/*!
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::CopyList(const NFmiVoidPtrList &listItem)
{
  NFmiVoidPtrItem *temp = listItem.itsFirstItem;
  for (; temp != nullptr; temp = temp->itsNextItem)
  {
    AddEnd(temp->itsValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrList::operator+=(const NFmiVoidPtrList &listItem) { CopyList(listItem); }
// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

//                            Iterator                          II

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrIterator::NFmiVoidPtrIterator(NFmiVoidPtrList *listItem)
    : itsIndex(0),
      itsCurrentItem(listItem->itsFirstItem),
      itsPreviousItem(nullptr),
      itsListItem(listItem)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param listItem Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrIterator::NFmiVoidPtrIterator(NFmiVoidPtrList &listItem)
    : itsIndex(0),
      itsCurrentItem(listItem.itsFirstItem),
      itsPreviousItem(nullptr),
      itsListItem(&listItem)
{
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrIterator::Reset()
{
  itsListItem->itsPreviousItem = itsPreviousItem = nullptr;
  itsCurrentItem = itsListItem->itsFirstItem;
  itsIndex = 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

void *NFmiVoidPtrIterator::Next()
{
  NFmiVoidPtrItem *temp = itsCurrentItem;
  if (temp != nullptr)
  {
    itsListItem->itsPreviousItem = itsPreviousItem;
    itsListItem->itsCurrentItem = temp;
    itsPreviousItem = itsCurrentItem;
    itsCurrentItem = itsCurrentItem->itsNextItem;
    itsIndex++;
    return temp->itsValue->itsDataValue;
  }
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiVoidPtrIterator::NextPreviousPtr()
{
  if (itsPreviousItem != nullptr)
    itsPreviousItem = itsPreviousItem->itsNextItem;
  else
    itsPreviousItem = itsListItem->itsFirstItem;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrData *NFmiVoidPtrIterator::NextPtr()
{
  NFmiVoidPtrItem *temp = itsCurrentItem;
  if (temp != nullptr)
  {
    itsPreviousItem = itsCurrentItem;
    itsCurrentItem = itsCurrentItem->itsNextItem;
    itsIndex++;
    return temp->itsValue;
  }

  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiVoidPtrData *NFmiVoidPtrIterator::CurrentPtr() { return itsCurrentItem->itsValue; }
// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::Next(void *&theItem)
{
  theItem = Next();
  return *&theItem != nullptr ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * \param theItem Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::NextPtr(NFmiVoidPtrData *&theItem)
{
  theItem = NextPtr();
  return *&theItem != nullptr ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------
void *NFmiVoidPtrIterator::Current()
{
  return itsCurrentItem != nullptr ? itsCurrentItem->itsValue->itsDataValue : nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

long NFmiVoidPtrIterator::Index() const { return CheckIndex(itsIndex) ? itsIndex : kNotInList; }
// ----------------------------------------------------------------------
/*!
 * \param theNewValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::Index(long theNewValue)
{
  if (!CheckIndex(theNewValue)) return false;
  Reset();
  do
  {
    if (theNewValue == itsIndex)
    {
      return true;
    }
  } while (Next() != nullptr);
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiVoidPtrIterator::CheckIndex(long theValue) const
{
  return theValue >= 0 && theValue < itsListItem->itsNumberOffItems;
}

// ======================================================================
