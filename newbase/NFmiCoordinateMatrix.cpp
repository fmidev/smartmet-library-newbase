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

void NFmiCoordinateMatrix::Swap(NFmiCoordinateMatrix& other)
{
  std::swap(itsNX, other.itsNX);
  std::swap(itsNX, other.itsNY);
  std::swap(itsX, other.itsX);
  std::swap(itsY, other.itsY);
}

bool NFmiCoordinateMatrix::Transform(OGRCoordinateTransformation& transformation)
{
  if (transformation.GetSourceCS() == nullptr || transformation.GetTargetCS() == nullptr)
    throw std::runtime_error("Projecting coordinate matrix without valid spatial references");

  const auto swap_input = transformation.GetSourceCS()->EPSGTreatsAsLatLong();
  const auto swap_output = transformation.GetTargetCS()->EPSGTreatsAsLatLong();

  if (swap_input) std::swap(itsX, itsY);

  const auto flag = transformation.Transform(itsNX * itsNY, &itsX[0], &itsY[0]);

  if (swap_output) std::swap(itsX, itsY);

  return flag;
}
