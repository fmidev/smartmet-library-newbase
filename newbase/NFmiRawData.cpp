// ======================================================================
/*!
 * \file NFmiRawData.cpp
 * \brief Implementation of class NFmiRawData
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4503)  // VC++ 2012 kääntäjällä tulee paljon ruman näköistä varoitusta
                                 // ilman tätä (decorated name length exceeded (4096 bytes), name
                                 // was truncated)
#pragma warning(disable : 4101)  // VC++ 2012 kääntäjällä tulee paljon 'lock' unreferenced
                                 // -varoitusta ilman tätä.  Scoped variableen ei ole
                                 // tarkoitustakaan viitata, sen tarkoitus on purkaa jotain scopen
                                 // loppuessa.
#endif

#include "NFmiRawData.h"
#include "NFmiVersion.h"

#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

using MappedFileParams = boost::iostreams::mapped_file_params;
using MappedFile = boost::iostreams::mapped_file;

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Pimple class
 */
// ----------------------------------------------------------------------

class NFmiRawData::Pimple
{
 public:
  ~Pimple();
  Pimple();
  Pimple(const Pimple &other);
  Pimple(istream &file, size_t size, bool endianswap);
  Pimple(const string &filename, istream &file, size_t size);
  bool Init(size_t size);
  bool Init(size_t size,
            const std::string &theHeader,
            const std::string &theFilename,
            bool fInitialize);
  size_t Size() const;
  float GetValue(size_t index) const;
  bool SetValue(size_t index, float value);
  void SetBinaryStorage(bool flag) const;
  bool IsBinaryStorageUsed() const;
  ostream &Write(ostream &file) const;
  void Backup(char *ptr) const;
  void Undo(char *ptr);
  bool Advise(FmiAdvice advice);

 private:
#if NFMIRAWDATA_ENABLE_UNDO_REDO
  void Unmap() const;
#endif

  mutable float *itsData{nullptr};                      // non-memory mapped data
  mutable boost::scoped_ptr<MappedFile> itsMappedFile;  // memory mapped data
  size_t itsOffset{0};                                  // offset to raw data

  size_t itsSize{0};
  mutable bool itsSaveAsBinaryFlag{true};
  bool itsEndianSwapFlag{false};
};

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::~Pimple() { delete[] itsData; }
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple() : itsMappedFile(nullptr) {}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(const Pimple &other)
    : itsData(nullptr),
      itsMappedFile(nullptr),
      itsOffset(0),
      itsSize(other.itsSize),
      itsSaveAsBinaryFlag(other.itsSaveAsBinaryFlag)

{
  // We assume copied data will be changed so that copying mmapping would
  // be a wasted effort

  itsData = new float[itsSize];

  if (other.itsData != nullptr)
  {
    memcpy(itsData, other.itsData, itsSize * sizeof(float));
  }
  else
  {
    auto *dst = reinterpret_cast<char *>(itsData);
    const char *src = other.itsMappedFile->const_data() + other.itsOffset;
    memcpy(dst, src, itsSize * sizeof(float));
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief File constructor (memory mapping)
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(const string &filename, istream &file, size_t size)
    : itsData(nullptr), itsMappedFile(nullptr), itsOffset(0), itsSize(size)

{
  // Backward compatibility:
  long datatype;
  file >> datatype;

  file >> itsSaveAsBinaryFlag;

  size_t poolsize = 0;
  file >> poolsize;

  if (poolsize != itsSize * sizeof(float))
    throw runtime_error("Invalid datapool size in querydata");

  // Skip last newline charater after poolsize
  char ch;
  file.get(ch);

  // Sanity check on the size of the data. Note that old querydata
  // has an extra endl at the end, hence we must permit 2 extra
  // characters at the end for legacy data to work

  std::size_t filesize = boost::filesystem::file_size(filename);
  itsOffset = file.tellg();

  if (itsOffset + poolsize > filesize)
    throw std::runtime_error("Querydata file " +
                             boost::lexical_cast<std::string>(itsOffset + poolsize - filesize) +
                             " bytes too short: '" + filename + "'");

  else if (filesize - itsOffset - poolsize > 2)
    throw std::runtime_error("Querydata file " +
                             boost::lexical_cast<std::string>(filesize - itsOffset - poolsize) +
                             " bytes too long: '" + filename + "'");

  // memory map starting from this file position
  // only if itsSaveAsBinaryFlag is true

  if (itsSaveAsBinaryFlag)
  {
    MappedFileParams params(filename.c_str());
    params.flags = boost::iostreams::mapped_file::readonly;
    params.length = filesize;
    itsMappedFile.reset(new MappedFile(params));
    if (!itsMappedFile->is_open())
      throw std::runtime_error("Failed to memory map '" + filename + "' in read only mode");
  }
  else
  {
    itsData = new float[itsSize];

    for (size_t idx = 0; idx < itsSize; ++idx)
      file >> itsData[idx];
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Stream constructor (no memory mapping)
 */
// ----------------------------------------------------------------------

NFmiRawData::Pimple::Pimple(istream &file, size_t size, bool endianswap)
    : itsData(nullptr),
      itsMappedFile(nullptr),
      itsOffset(0),
      itsSize(size),

      itsEndianSwapFlag(endianswap)
{
  delete itsData;
  itsData = new float[itsSize];

  // Backward compatibility:
  long datatype;
  file >> datatype;

  if (FmiInfoVersion >= 6)
    file >> itsSaveAsBinaryFlag;
  else
    itsSaveAsBinaryFlag = false;

  size_t poolsize = 0;
  file >> poolsize;

  if (!itsSaveAsBinaryFlag)
  {
    for (size_t idx = 0; idx < itsSize; ++idx)
      file >> itsData[idx];
  }
  else
  {
    if (poolsize != itsSize * sizeof(float))
      throw runtime_error("Invalid datapool size in querydata");

    // Skip last newline charater after poolsize
    char ch;
    file.get(ch);

    auto *ptr = reinterpret_cast<char *>(itsData);
    file.read(ptr, poolsize);
  }

  if (file.fail()) throw runtime_error("Failed to read rawdata from input stream");

  if (itsEndianSwapFlag)
  {
    char tmp1, tmp2, tmp3, tmp4;
    auto *ptr = reinterpret_cast<char *>(itsData);
    for (size_t i = 3; i < itsSize * sizeof(float); i += 4)
    {
      tmp1 = ptr[i - 3];
      tmp2 = ptr[i - 2];
      tmp3 = ptr[i - 1];
      tmp4 = ptr[i - 0];

      ptr[i - 3] = tmp4;
      ptr[i - 2] = tmp3;
      ptr[i - 1] = tmp2;
      ptr[i - 0] = tmp1;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize memory mapped file for writing
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::Init(size_t size,
                               const std::string &theHeader,
                               const std::string &theFilename,
                               bool fInitialize)
{
  // Update number of elements in the data

  itsSize = size;

  // Create info header fully up to start of raw data

  long datatype = 6;  // float??
  bool saveasbinary = true;
  std::ostringstream headerstream;
  headerstream << theHeader << '\n'
               << datatype << '\n'
               << saveasbinary << '\n'
               << itsSize * sizeof(float) << '\n';
  std::string fullheader = headerstream.str();

  // Now we know the offset to the start of raw data

  itsOffset = fullheader.size();

  // Create memory mapped file for writing

  MappedFileParams params(theFilename.c_str());
  params.flags = boost::iostreams::mapped_file::readwrite;
  params.new_file_size = itsOffset + itsSize * sizeof(float);
  itsMappedFile.reset(new MappedFile(params));

  // Initialize the header

  char *headerdata = itsMappedFile->data();
  memcpy(headerdata, fullheader.c_str(), fullheader.size());

  // Initialize the data section to kFloatMissing if so requested.
  // It seems as if this is not requested, the mapped region
  // will be initialized to zero bytes from start to finish.

  if (fInitialize)
  {
    auto *data = reinterpret_cast<float *>(itsMappedFile->data() + itsOffset);
    for (std::size_t i = 0; i < itsSize; i++)
      data[i] = kFloatMissing;
  }

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Init
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::Init(size_t size)
{
  itsData = nullptr;
  itsSize = size;
  if (itsSize > 0)
  {
    itsData = new float[itsSize];
    for (size_t i = 0; i < itsSize; i++)
      itsData[i] = kFloatMissing;
  }
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Get size
 */
// ----------------------------------------------------------------------

size_t NFmiRawData::Pimple::Size() const { return itsSize; }
// ----------------------------------------------------------------------
/*!
 * \brief Set storage mode to binary/ascii
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::SetBinaryStorage(bool flag) const { itsSaveAsBinaryFlag = flag; }

// ----------------------------------------------------------------------
/*!
 * \brief Return true if binary output is used
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::IsBinaryStorageUsed() const { return itsSaveAsBinaryFlag; }

// ----------------------------------------------------------------------
/*!
 * \brief Switch from mmapped data to dynamically allocated array
 */
// ----------------------------------------------------------------------

#if NFMIRAWDATA_ENABLE_UNDO_REDO
void NFmiRawData::Pimple::Unmap() const
{
  if (itsData) return;  // oli jo alustettu

  itsData = new float[itsSize];
  char *dst = reinterpret_cast<char *>(itsData);

  char *src = reinterpret_cast<char *>(itsMappedRegion->get_address());
  memcpy(dst, src, itsSize * sizeof(float));

  itsMappedFile.reset();
  itsOffset = 0;
}
#endif

// ----------------------------------------------------------------------
/*!
 * \brief Get value
 */
// ----------------------------------------------------------------------

float NFmiRawData::Pimple::GetValue(size_t index) const
{
  if (index >= itsSize) return kFloatMissing;

  if (itsData) return itsData[index];

  const auto *ptr = reinterpret_cast<const float *>(itsMappedFile->const_data() + itsOffset);
  return ptr[index];
}

// ----------------------------------------------------------------------
/*!
 * \brief Set value
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::SetValue(size_t index, float value)
{
  if (index >= itsSize) return false;

  if (itsData)
  {
    itsData[index] = value;
    return true;
  }
  if (itsOffset > 0)
  {
    // We have mmapped output data
    auto *ptr = reinterpret_cast<float *>(itsMappedFile->data() + itsOffset);
    ptr[index] = value;
    return true;
  }

// copy-on-write semantics: switch to memory buffer on a write

#if 0
  Unmap();
  itsData[index] = value;
#endif

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Write to output
 */
// ----------------------------------------------------------------------

ostream &NFmiRawData::Pimple::Write(ostream &file) const
{
  // Backward compatibility when other than floats were supported
  const int kFloat = 6;
  file << kFloat << endl;

  if (FmiInfoVersion >= 6) file << itsSaveAsBinaryFlag << endl;

  file << itsSize * sizeof(float) << endl;

  if (itsSaveAsBinaryFlag && FmiInfoVersion >= 6)
  {
    if (itsData != nullptr)
    {
      auto *ptr = reinterpret_cast<char *>(itsData);
      file.write(ptr, itsSize * sizeof(float));
    }
    else
    {
      const char *ptr = itsMappedFile->const_data() + itsOffset;
      file.write(ptr, itsSize * sizeof(float));
    }

    // Backward compatibility - not sure if needed:
    file << endl;
  }
  else
  {
    for (size_t idx = 0; idx < itsSize; ++idx)
      file << GetValue(idx) << ' ';
  }
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief Backup the raw data
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::Backup(char *ptr) const
{
// we assume data which is backed up is edited, so might as well unmap
#if NFMIRAWDATA_ENABLE_UNDO_REDO
  Unmap();
#endif

  auto *src = reinterpret_cast<char *>(itsData);
  memcpy(ptr, src, itsSize * sizeof(float));
}

// ----------------------------------------------------------------------
/*!
 * \brief Undo changes from backup
 */
// ----------------------------------------------------------------------

void NFmiRawData::Pimple::Undo(char *ptr)
{
// This may be slower than necessary when mmapped, but since Backup
// unmaps this really should never actually unmap

#if NFMIRAWDATA_ENABLE_UNDO_REDO
  Unmap();
#endif
  auto *src = reinterpret_cast<char *>(itsData);
  memcpy(src, ptr, itsSize * sizeof(float));
}

// ----------------------------------------------------------------------
/*!
 * \brief Advice the memory mapping
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Pimple::Advise(FmiAdvice advice)
{
  // was supported with boost::interprocess, not with boost::iostreams
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

NFmiRawData::~NFmiRawData() { delete itsPimple; }
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData() : itsPimple(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(const NFmiRawData &other) : itsPimple(new Pimple(*other.itsPimple)) {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor anticipating a memory mapped file
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(const string &filename, istream &file, size_t size)
    : itsPimple(new Pimple(filename, file, size))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor not anticipating a memory mapped file
 */
// ----------------------------------------------------------------------

NFmiRawData::NFmiRawData(istream &file, size_t size, bool endianswap)
    : itsPimple(new Pimple(file, size, endianswap))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Init
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Init(size_t size) { return itsPimple->Init(size); }
// ----------------------------------------------------------------------
/*!
 * \brief Init memory mapped file for writing
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Init(size_t size,
                       const std::string &theHeader,
                       const std::string &theFilename,
                       bool fInitialize)
{
  return itsPimple->Init(size, theHeader, theFilename, fInitialize);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return size of raw data
 */
// ----------------------------------------------------------------------

size_t NFmiRawData::Size() const { return itsPimple->Size(); }
// ----------------------------------------------------------------------
/*!
 * \brief Get value at given index
 */
// ----------------------------------------------------------------------

float NFmiRawData::GetValue(size_t index) const { return itsPimple->GetValue(index); }
// ----------------------------------------------------------------------
/*!
 * \brief Set value at given index
 */
// ----------------------------------------------------------------------

bool NFmiRawData::SetValue(size_t index, float value) { return itsPimple->SetValue(index, value); }
// ----------------------------------------------------------------------
/*!
 * \brief Set binary storage mode based on given flag
 */
// ----------------------------------------------------------------------

void NFmiRawData::SetBinaryStorage(bool flag) const { itsPimple->SetBinaryStorage(flag); }
// ----------------------------------------------------------------------
/*!
 * \brief Is binary storage mode on?
 */
// ----------------------------------------------------------------------

bool NFmiRawData::IsBinaryStorageUsed() const { return itsPimple->IsBinaryStorageUsed(); }
// ----------------------------------------------------------------------
/*!
 * \brief Write raw data
 */
// ----------------------------------------------------------------------

ostream &NFmiRawData::Write(ostream &file) const { return itsPimple->Write(file); }
// ----------------------------------------------------------------------
/*!
 * \brief Backup data to given pointer
 */
// ----------------------------------------------------------------------

void NFmiRawData::Backup(char *ptr) const { itsPimple->Backup(ptr); }
// ----------------------------------------------------------------------
/*!
 * \brief Restore data from given pointer
 */
// ----------------------------------------------------------------------

void NFmiRawData::Undo(char *ptr) { itsPimple->Undo(ptr); }
// ----------------------------------------------------------------------
/*!
 * \brief Advise memory mapped region
 *
 * Returns false on failure.
 */
// ----------------------------------------------------------------------

bool NFmiRawData::Advise(FmiAdvice theAdvice) { return itsPimple->Advise(theAdvice); }
// ======================================================================
