#pragma once

#include "NFmiGlobals.h"

class OGRSpatialReference;

class _FMI_DLL NFmiWGS84
{
 public:
  static OGRSpatialReference* SpatialReference();

 private:
  NFmiWGS84() = delete;
  NFmiWGS84(const NFmiWGS84& other) = delete;
  NFmiWGS84& operator=(const NFmiWGS84& other) = delete;
};
