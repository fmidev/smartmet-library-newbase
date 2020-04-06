#include "NFmiCoordinateMatrix.h"
#include <gis/CoordinateTransformation.h>
#include <cmath>

// PROJ uses HUGE_VAL as a missing value, hence we do too to avoid unnecessary modifications to data

NFmiCoordinateMatrix::NFmiCoordinateMatrix(std::size_t nx, std::size_t ny)
    : itsNX{nx},
      itsNY{ny},
      itsX{std::vector<double>(nx * ny, HUGE_VAL)},
      itsY{std::vector<double>(nx * ny, HUGE_VAL)}
{
}

// Initialize X coordinates to x1...x2 and Y coordinates to y1..y2 with constant step sizes
NFmiCoordinateMatrix::NFmiCoordinateMatrix(
    std::size_t nx, std::size_t ny, double x1, double y1, double x2, double y2)
    : itsNX{nx},
      itsNY{ny},
      itsX{std::vector<double>(nx * ny, HUGE_VAL)},
      itsY{std::vector<double>(nx * ny, HUGE_VAL)}
{
  const auto dx = nx > 1 ? (x2 - x1) / (nx - 1) : 0;
  const auto dy = ny > 1 ? (y2 - y1) / (ny - 1) : 0;

  std::size_t pos = 0;
  for (std::size_t j = 0; j < ny; j++)
  {
    const auto y = y1 + j * dy;
    for (std::size_t i = 0; i < nx; i++)
    {
      itsX[pos] = x1 + i * dx;
      itsY[pos] = y;
      ++pos;
    }
  }
}

// Swap contents
void NFmiCoordinateMatrix::swap(NFmiCoordinateMatrix& other)
{
  std::swap(itsNX, other.itsNX);
  std::swap(itsNX, other.itsNY);
  std::swap(itsX, other.itsX);
  std::swap(itsY, other.itsY);
}

// Project the coordinates. User takes responsibility on making sure the input coordinates
// are in the correct spatial reference. Input/output order is always lon/lat or x/y,
// EPSG rules are followed only temporarily to make the projection work.

bool NFmiCoordinateMatrix::Transform(Fmi::CoordinateTransformation& transformation)
{
  return transformation.Transform(itsX, itsY);
}
