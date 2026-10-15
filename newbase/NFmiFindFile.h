// ======================================================================
/*!
 * \file NFmiFindFile.h
 * \brief Interface of class NFmiFindFile
 */
// ======================================================================

#pragma once

#include "NFmiFileString.h"
#include "NFmiGlobals.h"
#include <string>
#ifdef UNIX
#include <glob.h>
#endif

#define ONLY_PATH "*."

//! Undocumented
class NFmiFindFile
{
 public:
  ~NFmiFindFile();
  NFmiFindFile();

  void Path(const NFmiString& thePathName);
  bool Find(const NFmiString& theFileMask = "*.*");
  bool Next();

  NFmiString FileName();
  NFmiString PathName();
  NFmiString PathAndFileName();

 private:
  NFmiFindFile(const NFmiFindFile& theFile) = delete;
  NFmiFindFile& operator=(const NFmiFindFile& theFile) = delete;

  std::string itsFileName;
  std::string itsFilePath;

  long itsFileFile = 0;

#ifdef UNIX
  unsigned int itsGlobIndex = 0;
  glob_t* itsGlobBuffer = nullptr;
#endif

};  // class NFmiFindFile

// ======================================================================
