// ======================================================================
/*!
 * \file NFmiFileString.h
 * \brief Interface of class NFmiFileString
 */
// ======================================================================

#pragma once

#include "NFmiString.h"

//! Undocumented
class _FMI_DLL NFmiFileString : public NFmiString
{
 public:
  ~NFmiFileString() override;
  NFmiFileString();
  NFmiFileString(const NFmiFileString& theFileStr);
  NFmiFileString(const NFmiString& theStr);

  const NFmiString Extension() const;
  const NFmiString FileName() const;
  const NFmiString Header() const;
  const NFmiString Path() const;
  const NFmiString Device() const;
  const NFmiString Directory() const;

  bool HasExtension() const;
  bool IsAbsolutePath() const;

  void Extension(const NFmiString& theExtension);
  void FileName(const NFmiString& theExtension);
  void Header(const NFmiString& theExtension);
  void Path(const NFmiString& theExtension);
  void Device(const NFmiString& theExtension);

  void AddDirectory(const NFmiString& theDirectory);
  void DeleteDirectory();
  void ReplaceDirectory(const NFmiString& theDirectory);

  void Extension(const char* theExt);
  void FileName(const char* theFileName);
  void Header(const char* theHeader);
  void Path(const char* thePath);
  void Device(const char* theDevice);

  void NormalizeDelimiter();
  void ChangeScandinavian();

  const NFmiString PathWithoutLastDirectory();

};  // class NFmiFileString

// ======================================================================
