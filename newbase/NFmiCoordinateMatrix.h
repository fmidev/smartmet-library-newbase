#pragma once
#include "NFmiPoint.h"
#include <vector>

class OGRCoordinateTransformation;

class NFmiCoordinateMatrix
{
 public:
  // init to missing values
  NFmiCoordinateMatrix(std::size_t nx, std::size_t ny);

  std::size_t Width() const { return itsNX; }
  std::size_t Height() const { return itsNY; }

  double X(std::size_t i, std::size_t j) const { return itsX[i + j * itsNX]; }
  double Y(std::size_t i, std::size_t j) const { return itsY[i + j * itsNX]; }

  NFmiPoint operator()(std::size_t i, std::size_t j) const
  {
    const auto pos = i + j * itsNX;
    return {itsX[pos], itsY[pos]};
  }

  void Set(std::size_t i, std::size_t j, double x, double y)
  {
    const auto pos = i + j * itsNX;
    itsX[pos] = x;
    itsY[pos] = y;
  }

  void Set(std::size_t i, std::size_t j, const NFmiPoint& xy)
  {
    const auto pos = i + j * itsNX;
    itsX[pos] = xy.X();
    itsY[pos] = xy.Y();
  }

  void Swap(NFmiCoordinateMatrix& other);

  // returns false if no coordinates could be projected
  bool Transform(OGRCoordinateTransformation& transformation);

 private:
  std::size_t itsNX = 0;
  std::size_t itsNY = 0;
  std::vector<double> itsX;
  std::vector<double> itsY;

};  // class NFmiCoordinateMatrix
