// ======================================================================
/*!
 * \file NFmiPoint.h
 * \brief Interface of class NFmiPoint
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include <cmath>
#include <iosfwd>
#include <tuple>

class _FMI_DLL NFmiPoint
{
 public:
  static const NFmiPoint gMissingLatlon;

  NFmiPoint() = default;
  NFmiPoint(double theX, double theY) : itsX(theX), itsY(theY) {}

  void Set(double newX, double newY)
  {
    itsX = newX;
    itsY = newY;
  }

  void X(double newValue) { itsX = newValue; }
  void Y(double newValue) { itsY = newValue; }

  double X() const { return itsX; }
  double Y() const { return itsY; }

  friend bool operator==(const NFmiPoint &lhs, const NFmiPoint &rhs)
  {
    return lhs.itsX == rhs.itsX && lhs.itsY == rhs.itsY;
  }

  friend bool operator!=(const NFmiPoint &lhs, const NFmiPoint &rhs)
  {
    return lhs.itsX != rhs.itsX || lhs.itsY != rhs.itsY;
  }

  friend bool operator<(const NFmiPoint &lhs, const NFmiPoint &rhs)
  {
    return std::tie(lhs.itsX, lhs.itsY) < std::tie(rhs.itsX, rhs.itsY);
  }

  NFmiPoint &operator+=(const NFmiPoint &thePoint);
  NFmiPoint &operator-=(const NFmiPoint &thePoint);
  NFmiPoint &operator*=(const NFmiPoint &thePoint);
  NFmiPoint &operator/=(const NFmiPoint &thePoint);

  // Euclidian surface (!) distance
  double Distance(const NFmiPoint &thePoint) const
  {
    return std::hypot(itsX - thePoint.X(), itsY - thePoint.Y());
  }

  FmiDirection DirectionOfDifference(const NFmiPoint &thePoint) const;

  std::ostream &Write(std::ostream &file) const;
  std::istream &Read(std::istream &file);

  const char *ClassName() const { return "NFmiPoint"; }

  std::size_t HashValue() const;

 private:
  double itsX = 0;
  double itsY = 0;
};  // class NFmiPoint

//! Undocumented, should be removed
using PNFmiPoint = NFmiPoint *;

inline NFmiPoint operator+(const NFmiPoint &leftPoint, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftPoint.X() + rightPoint.X(), leftPoint.Y() + rightPoint.Y());
}

inline NFmiPoint operator-(const NFmiPoint &leftPoint, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftPoint.X() - rightPoint.X(), leftPoint.Y() - rightPoint.Y());
}

inline NFmiPoint operator*(const NFmiPoint &leftPoint, double rightScale)
{
  return NFmiPoint(leftPoint.X() * rightScale, leftPoint.Y() * rightScale);
}

inline NFmiPoint operator*(double leftScale, const NFmiPoint &rightPoint)
{
  return NFmiPoint(leftScale * rightPoint.X(), leftScale * rightPoint.Y());
}

inline std::ostream &operator<<(std::ostream &os, const NFmiPoint &item)
{
  return (*const_cast<NFmiPoint *>(&item)).Write(os);
}

inline std::istream &operator>>(std::istream &is, NFmiPoint &item) { return item.Read(is); }
