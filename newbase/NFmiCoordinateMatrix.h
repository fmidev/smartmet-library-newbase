#pragma once
#include "NFmiPoint.h"
#include <vector>

class NFmiCoordinateTransformation;

class _FMI_DLL NFmiCoordinateMatrix
{
 public:
  // init to missing values
  NFmiCoordinateMatrix(std::size_t nx, std::size_t ny);
  // init to a rectilinear grid
  NFmiCoordinateMatrix(std::size_t nx, std::size_t ny, double x1, double y1, double x2, double y2);

  // size accessors
  std::size_t Width() const { return itsNX; }
  std::size_t Height() const { return itsNY; }

  // data accessors
  double X(std::size_t i, std::size_t j) const { return itsX[i + j * itsNX]; }
  double Y(std::size_t i, std::size_t j) const { return itsY[i + j * itsNX]; }

  NFmiPoint operator()(std::size_t i, std::size_t j) const
  {
    const auto pos = i + j * itsNX;
    return {itsX[pos], itsY[pos]};
  }

  // data setters are normally not needed, constructing a 1D array of station coordinates
  // is likely the only exception
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

  // occasionally needed for speed
  void swap(NFmiCoordinateMatrix& other);

  // Always uses lon/lat x/y ordering.
  bool Transform(NFmiCoordinateTransformation& transformation);

 private:
  std::size_t itsNX = 0;
  std::size_t itsNY = 0;
  std::vector<double> itsX;
  std::vector<double> itsY;

};  // class NFmiCoordinateMatrix
