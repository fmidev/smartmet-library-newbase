// ======================================================================
/*!
 * \file NFmiQueryInfoSatel.h
 * \brief Interface of class NFmiQueryInfoSatel
 */
// ======================================================================

#pragma once

#include "NFmiQueryInfo.h"

class NFmiQueryData;

//! Undocumented

class _FMI_DLL NFmiQueryInfoSatel : public NFmiQueryInfo
{
 public:
  ~NFmiQueryInfoSatel() override;
  NFmiQueryInfoSatel();
  NFmiQueryInfoSatel(const NFmiQueryInfoSatel& theInfo);

  NFmiQueryInfoSatel(const NFmiParamDescriptor& theParamDescriptor,
                     const NFmiTimeDescriptor& theTimeDescriptor,
                     const NFmiHPlaceDescriptor& theHPlaceDescriptor,
                     const NFmiVPlaceDescriptor& theVPlaceDescriptor = NFmiVPlaceDescriptor());

  NFmiQueryInfoSatel(NFmiQueryData* theInfo,
                     NFmiParamDescriptor* theParamDescriptor = 0,
                     NFmiTimeDescriptor* theTimeDescriptor = 0,
                     NFmiHPlaceDescriptor* theHPlaceDescriptor = 0,
                     NFmiVPlaceDescriptor* theVPlaceDescriptor = 0);

  virtual NFmiQueryInfo& operator=(const NFmiQueryInfo& theInfo);

  NFmiQueryInfo* Clone() const override;

  std::ostream& Write(std::ostream& file) const override;
  std::istream& Read(std::istream& file) override;

  unsigned long ClassId() const override;
  const char* ClassName() const override;

 private:
  NFmiQueryInfoSatel& operator=(const NFmiQueryInfoSatel& theInfo);

  void Destroy();
  NFmiString* itsSatelName;

};  // class NFMiQueryInfoSatel

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiQueryInfoSatel::ClassId() const { return kNFmiSatelQueryInfo; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char* NFmiQueryInfoSatel::ClassName() const { return "NFmiQueryInfoSatel"; }

// ======================================================================
