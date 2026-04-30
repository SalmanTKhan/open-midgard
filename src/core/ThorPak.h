#pragma once
//===========================================================================
// ThorPak.h  –  THOR patch archive reader
//
// THOR is a patch container distributed by RO patchers. A THOR layered on top
// of the base GRF lets that patch override files without rewriting the GRF.
//
// Reference: GRFEditor/GRF/FileFormats/ThorFormat (Aeomin DEV, 2007).
//===========================================================================
#include "GPak.h"
#include "Hash.h"
#include <string>
#include <vector>
#include <cstdint>

class CMemFile;

class CThorPak
{
public:
    CThorPak() = default;
    virtual ~CThorPak();

    // Attach a CMemFile data source and populate the file table.
    bool Open(CMemFile* memFile);

    // Populate `out` with metadata for the file named by `key`.
    bool GetInfo(const CHash& key, PakPack* out) const;

    // Returns true if the THOR carries a "remove file" tombstone for `key`.
    // Tombstones hide the file from any base GRF behind this patch, so a
    // patch can both override and delete entries.
    bool IsRemoved(const CHash& key) const;

    // Decompress the entry described by `pack` into `buffer`. `buffer` must
    // be >= pack.m_size bytes.
    bool GetData(const PakPack& pack, void* buffer);

    // Collect every archived file path that ends with `ext`.
    void CollectFileNamesByExtension(const char* ext, std::vector<std::string>& out) const;

    // Whether this THOR is intended to be merged into the target GRF (vs an
    // exe-style standalone patch). Currently informational only.
    bool UseGrfMerging() const { return m_useGrfMerging; }
    const std::string& TargetGrf() const { return m_targetGrf; }

protected:
    bool ParseMode30Table(const unsigned char* table, uint32_t tableSize);

    CMemFile*            m_memFile = nullptr;
    bool                 m_useGrfMerging = false;
    int16_t              m_mode = 0;
    std::string          m_targetGrf;
    std::vector<PakPack> m_PakPack;        // sorted by CHash for binary search
    std::vector<CHash>   m_removedFiles;   // sorted tombstone hashes
    std::vector<uint8_t> m_decompressScratch;
};
