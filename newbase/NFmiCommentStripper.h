// ======================================================================
/*!
 * \file NFmiCommentStripper.h
 * \brief Interface of class NFmiCommentStripper
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiDef.h"
#include <set>
#include <string>

//! Undocumented
class _FMI_DLL NFmiCommentStripper
{
 public:
  virtual ~NFmiCommentStripper() {}
  NFmiCommentStripper(bool stripPound = true,
                      bool stripDoubleSlash = true,
                      bool stripSlashAst = true,
                      bool stripSlashAstNested = false,
                      bool stripEndOfLineSpaces = true);

  NFmiCommentStripper(const std::string& theFileName,
                      bool stripPound = false,
                      bool stripDoubleSlash = true,
                      bool stripSlashAst = true,
                      bool stripSlashAstNested = true,
                      bool stripEndOfLineSpaces = true);

  NFmiCommentStripper(const NFmiCommentStripper& theStripper);

  bool ReadFile(const std::string& theFileName);
  bool ReadFileCheckingOptions(const std::string& theFileName,
                               const std::string& theOptionText,
                               std::set<std::string>& theOptionTexts);
  bool ReadAndStripFile(const std::string& theFileName);
  const std::string GetMessage() const { return itsMessage; };
  const std::string& GetString() const { return itsString; };
  void SetString(const std::string& theString) { itsString = theString; };
  virtual bool Strip();
  virtual bool Strip(const std::string& theString);
  bool StripBlocks(const std::string& theBeginDirective = "/*",
                   const std::string& theEndDirective = "*/");
  bool StripDoubleSlashes();  // to endline
  bool StripPounds();         // to endline
  bool StripSubStrings(const std::string& theString);

 private:
  bool CollectAndStripNested(const std::string& theBeginDirective,
                             const std::string& theEndDirective);
  bool StripNested(checkedVector<unsigned long> theBeginPositions,
                   checkedVector<unsigned long> theEndPositions);
  bool CollectStringPositions(const std::string& theSearchString,
                              checkedVector<unsigned long>& theResVector);

 protected:
  std::string itsMessage;
  std::string itsString;
  std::string itsFileName;  // for error messages
  bool fStripPound;
  bool fStripNested;
  bool fStripDoubleSlash;
  bool fStripSlashAst;
  bool fStripEndOfLineSpaces;  // Marko Lisäsin falgin, koska oli ongelmia joissain tapauksissa, jos
                               // spacet poistetaan.

};  // class NFmiCommentStripper

// ======================================================================
