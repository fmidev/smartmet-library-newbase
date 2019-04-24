#include "NFmiProj.h"
#include "NFmiSaveBaseFactory.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <vector>

// ----------------------------------------------------------------------
/*!
 * \brief Parse PROJ.4 settings
 *
 * Sample input: +to_meter=.0174532925199433 +proj=ob_tran +o_proj=eqc
 *               +o_lon_p=0 +o_lat_p=30 +R=6371220
 *               +wktext +over +towgs84=0,0,0 +no_defs
 */
// ----------------------------------------------------------------------

NFmiProj::NFmiProj(const std::string& theProj)
{
  // Split options by whitespace
  auto proj = boost::algorithm::trim_copy(theProj);
  std::vector<std::string> options;
  boost::algorithm::split(
      options, proj, boost::algorithm::is_any_of(" "), boost::token_compress_on);

  // Process the options one by one

  for (const auto& option : options)
  {
    if (option.empty()) continue;  // safety check

    if (option[0] != '+')
      throw std::runtime_error("Only PROJ options starting with '+' are allowed");

    std::vector<std::string> parts;
    boost::algorithm::split(parts, option, boost::algorithm::is_any_of("="));

    // Extract option name
    auto name = parts[0].substr(1, std::string::npos);

    if (parts.size() == 1)
      itsOptions.insert(name);
    else if (parts.size() == 2)
    {
      auto string_value = parts[1];

      // Store value as double or string

      try
      {
        std::size_t pos = 0;
        auto value = std::stod(string_value, &pos);
        if (pos == string_value.size())
          itsDoubles[name] = value;
        else
          itsStrings[name] = string_value;
      }
      catch (...)
      {
        itsStrings[name] = string_value;
      }
    }
    else
      throw std::runtime_error("PROJ option '" + option + "' contains too many '=' characters");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return given PROJ.4 setting
 */
// ----------------------------------------------------------------------

boost::optional<double> NFmiProj::GetDouble(const std::string& theName) const
{
  auto pos = itsDoubles.find(theName);
  if (pos == itsDoubles.end()) return {};
  return pos->second;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return given PROJ.4 setting
 */
// ----------------------------------------------------------------------

boost::optional<std::string> NFmiProj::GetString(const std::string& theName) const
{
  auto pos = itsStrings.find(theName);
  if (pos == itsStrings.end()) return {};
  return pos->second;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return given PROJ.4 setting
 */
// ----------------------------------------------------------------------

bool NFmiProj::GetBool(const std::string& theName) const
{
  auto pos = itsOptions.find(theName);
  return (pos != itsOptions.end());
}

//----------------------------------------------------------------------
/*!
 * \brief Dump the settings mostly for debugging purposes

 */
// ----------------------------------------------------------------------

void NFmiProj::Dump(std::ostream& theOutput) const
{
  for (const auto& name_double : itsDoubles)
    theOutput << '+' << name_double.first << " = " << name_double.second << "\n";

  for (const auto& name_string : itsStrings)
    theOutput << '+' << name_string.first << " = \"" << name_string.second << "\"\n";

  for (const auto& name : itsOptions)
    theOutput << '+' << name << "\n";
}

// ----------------------------------------------------------------------
/*!
 * \brief Guess legacy ClassID from the settings
 */
// ----------------------------------------------------------------------

int NFmiProj::DetectClassId() const
{
  auto name = GetString("proj");
  if (!name) throw std::runtime_error("Projection name not set, should be impossible");

  if (GetDouble("R") == kRearth)
  {
    if (*name == "eqc") return kNFmiLatLonArea;
    if (*name == "merc") return kNFmiMercatorArea;
    if (*name == "stere") return kNFmiStereographicArea;
    if (*name == "aeqd") return kNFmiEquiDistArea;
    if (*name == "lcc") return kNFmiLambertConformalConicArea;
    if (*name == "ob_tran" && GetString("o_proj") == std::string("eqc") &&
        GetString("towgs84") == std::string("0,0,0"))
      return kNFmiRotatedLatLonArea;
  }
  else if (*name == "tmerc" && GetString("ellps") == std::string("intl") &&
           GetDouble("x_0") == 3500000.0 && GetDouble("lat_0") == 0.0 &&
           GetDouble("lon_0") == 27.0 &&
           GetString("towgs84") ==
               std::string("-96.0617,-82.4278,-121.7535,4.80107,0.34543,-1.37646,1.4964"))
    return kNFmiYKJArea;

  // Not a legacy projection, use PROJ.4
  return kNFmiProjArea;
}
