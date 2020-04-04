#pragma once

#include "NFmiDef.h"
#include <memory>
#include <string>

class OGRSpatialReference;

/*
 * A proxy class for OGRSpatialReference
 */

class _FMI_DLL NFmiSpatialReference
{
 public:
  ~NFmiSpatialReference();
  NFmiSpatialReference() = delete;

  NFmiSpatialReference(const NFmiSpatialReference &other);
  NFmiSpatialReference(const OGRSpatialReference &other);  // since GDAL is not const correct
  NFmiSpatialReference(OGRSpatialReference &other);
  NFmiSpatialReference(const char *theDesc);
  NFmiSpatialReference(const std::string &theDesc);

  NFmiSpatialReference &operator=(const NFmiSpatialReference &other) = delete;

  // Excplicit and implicit accessors

  const OGRSpatialReference &operator*() const { return *itsSR; }
  OGRSpatialReference *get() const { return itsSR; }

  operator OGRSpatialReference &() const { return *itsSR; }
  operator OGRSpatialReference *() const { return itsSR; }

  // Common accessors
  bool IsAxisSwapped() const;
  bool IsGeographic() const;

  // This is mostly for debugging
  const std::string &ProjStr() const { return itsProjStr; }

 private:
  std::string itsProjStr;  // set only if initialized from a string
  void init(const std::string &theSR);

  OGRSpatialReference *itsSR = nullptr;
};
