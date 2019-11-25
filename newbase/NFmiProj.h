#pragma once
#include "NFmiGlobals.h"
#include <boost/optional.hpp>
#include <map>
#include <set>
#include <string>

class _FMI_DLL NFmiProj
{
 public:
  NFmiProj() = default;
  NFmiProj(const std::string& theProj);

  const std::string& ProjStr() const { return itsProjStr; }
  boost::optional<double> GetDouble(const std::string& theName) const;
  boost::optional<std::string> GetString(const std::string& theName) const;
  bool GetBool(const std::string& theName) const;
  int DetectClassId() const;

  std::string InverseProjStr() const;

  void Dump(std::ostream& theOutput) const;

 private:
  std::string itsProjStr;
  std::map<std::string, double> itsDoubles;       // +R=radius etc
  std::map<std::string, std::string> itsStrings;  // +ellps=intl etc
  std::set<std::string> itsOptions;               // +over etc
};