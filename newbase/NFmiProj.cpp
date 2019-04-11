#include "NFmiProj.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <vector>

// ----------------------------------------------------------------------
/*!
 * \brief Parse PROJ.4 settings
 *
 * Sample input: +to_meter=.0174532925199433 +proj=ob_tran +o_proj=latlon
 *               +o_lon_p=0 +o_lat_p=30 +a=6371220 +b=6371220
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
        auto value = std::stod(string_value);
        itsDoubles[name] = value;
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
    theOutput << name_double.first << " = " << name_double.second << " (double)\n";

  for (const auto& name_string : itsStrings)
    theOutput << name_string.first << " = " << name_string.second << " (string)\n";

  for (const auto& name : itsOptions)
    theOutput << name << " (option)\n";
}
