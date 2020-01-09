#include "NFmiCoordinateMatrix.h"
#include <cmath>
#include <ogr_spatialref.h>

// PROJ uses HUGE_VAL as a missing value, hence we do too to avoid unnecessary modifications to data

NFmiCoordinateMatrix::NFmiCoordinateMatrix(std::size_t nx, std::size_t ny)
    : itsNX{nx},
      itsNY{ny},
      itsX{std::vector<double>(nx * ny, HUGE_VAL)},
      itsY{std::vector<double>(nx * ny, HUGE_VAL)}
{
}

void NFmiCoordinateMatrix::Swap(NFmiCoordinateMatrix& other)
{
  std::swap(itsNX, other.itsNX);
  std::swap(itsNX, other.itsNY);
  std::swap(itsX, other.itsX);
  std::swap(itsY, other.itsY);
}

bool NFmiCoordinateMatrix::Transform(OGRCoordinateTransformation& transformation)
{
  return transformation.Transform(itsNX * itsNY, &itsX[0], &itsY[0]);
}
