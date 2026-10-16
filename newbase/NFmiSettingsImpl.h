// ======================================================================
/*!
 * \file NFmiSettingsImpl.h
 * \brief Interface of class NFmiSettingsImpl
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

//! A generic settings holder
class NFmiSettingsImpl
{
 public:
  static NFmiSettingsImpl& Instance();
  void Clear();
  bool Init(const std::string& theFile = "") const;
  bool Read(const std::string& theFile) const;

  std::string Value(const std::string& theName, const std::string& theAlternate = "") const;
  std::string Require(const std::string& theName) const;
  bool IsSet(const std::string& theName) const;

#undef SearchPath  // win32 macro pitää undefinata
  void SearchPath(const std::string& thePath) const;
  void Set(const std::string& theName, const std::string& theValue, bool modifyOnlyExisting) const;
  void Save() const;
  std::string ToString(const std::string& thePrefix = "") const;
  std::vector<std::string> ListChildren(const std::string& thePrefix) const;

 private:
  //! Destruction is private - this is a singleton
  ~NFmiSettingsImpl();

  void InternalSet(const std::string& theName, const std::string& theValue) const;
  void SetOrigin(const std::string& theVariable, const std::string& theFilename) const;
  void UseNamespace(std::istringstream& input, const std::string& filename) const;
  void AssignVariable(std::istringstream& input,
                      const std::string& token,
                      unsigned long words,
                      const std::string& var,
                      const std::string& filename) const;
  void BeginNamespace(unsigned long words, const std::string& var) const;
  void EndNamespace(unsigned long words, const std::string& var) const;

  //! Default constructor is private - this is a singleton
  NFmiSettingsImpl();

  //! Copy constructor is disabled - this is a singleton
  NFmiSettingsImpl(const NFmiSettingsImpl& theSettings) = delete;

  //! Assignment operator is disabled - this is a singleton
  NFmiSettingsImpl& operator=(const NFmiSettingsImpl& theSettings) = delete;

  static void Create();
  [[noreturn]] static void Die();
  static NFmiSettingsImpl* itsInstance;

  //! The type of the actual data
  using DataType = std::map<std::string, std::string>;

  static bool itIsDestroyed;        //!< Boolean to detect singleton coupling at exit
  mutable std::string itsFilename;  //!< The file containing the configuration
  mutable DataType itsData;         //!< The actual data
  mutable bool itIsInitialized = false;  //!< For detecting initialization

  mutable std::list<std::string> itsSearchPath;

  mutable std::list<std::string> itsNamespaces;

  mutable std::set<std::string> itsExpandedVariables;
  mutable std::set<std::string> itsChangedVariables;

  using FileMap = std::map<std::string, std::string>;
  mutable FileMap itsFilenames;

};  // class NFmiSettingsImpl

// ======================================================================
