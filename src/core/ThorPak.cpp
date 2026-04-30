//===========================================================================
// ThorPak.cpp  –  THOR patch archive reader
//
// File layout (mode 0x30, the only mode this reader handles):
//   [24] ASCII magic "ASSF (C) 2007 Aeomin DEV"
//   [ 1] useGrfMerging flag
//   [ 4] numberOfFiles (informational)
//   [ 2] mode (0x30 multi-file zlib table, 0x21 single-file exe-style)
//   [ 1] targetGrfNameLen
//   [ N] targetGrfName
//   [ 4] fileTableCompressedLength
//   [ 4] fileTableOffset (absolute)
//   [..] data section
//   ...
//   [fileTableOffset]
//   [zlib-compressed table]
//
// Compressed table contents:
//   For each entry:
//     [ 1] nameLen
//     [ N] relativePath
//     [ 1] flags  (bit 0 = remove file, marker only — no offset/sizes follow)
//     [ 4] offset (u32)
//     [ 4] sizeCompressed
//     [ 4] sizeDecompressed
//
// Reference: GRFEditor ThorHeader.cs / ThorEntry.cs (single-file mode 0x21
// not implemented — that's used for distributing exe updates, not assets).
//===========================================================================
#include "ThorPak.h"

#include "File.h"
#include "../DebugLog.h"

#include <algorithm>
#include <cstring>
#include <zlib.h>

namespace {

constexpr const char* kThorMagic = "ASSF (C) 2007 Aeomin DEV";
constexpr size_t kThorMagicLen = 24;

uint16_t ReadLe16(const unsigned char* bytes)
{
    return static_cast<uint16_t>(static_cast<uint16_t>(bytes[0])
        | (static_cast<uint16_t>(bytes[1]) << 8));
}

uint32_t ReadLe32(const unsigned char* bytes)
{
    return static_cast<uint32_t>(bytes[0])
        | (static_cast<uint32_t>(bytes[1]) << 8)
        | (static_cast<uint32_t>(bytes[2]) << 16)
        | (static_cast<uint32_t>(bytes[3]) << 24);
}

}  // namespace

CThorPak::~CThorPak() = default;

bool CThorPak::Open(CMemFile* memFile)
{
    if (!memFile) return false;
    m_memFile = memFile;

    const u32 fileSize = memFile->size();
    if (fileSize < kThorMagicLen + 1 + 4 + 2) {
        DbgLog("[ThorPak::Open] file too small (%u bytes)\n", fileSize);
        return false;
    }

    const unsigned char* magic = memFile->read(0, static_cast<u32>(kThorMagicLen));
    if (!magic) return false;
    if (std::memcmp(magic, kThorMagic, kThorMagicLen) != 0) {
        DbgLog("[ThorPak::Open] magic mismatch\n");
        return false;
    }

    u32 cursor = static_cast<u32>(kThorMagicLen);
    const unsigned char* useMerge = memFile->read(cursor, 1);
    if (!useMerge) return false;
    m_useGrfMerging = (useMerge[0] != 0);
    cursor += 1;

    const unsigned char* fileCountBytes = memFile->read(cursor, 4);
    if (!fileCountBytes) return false;
    const uint32_t numberOfFiles = ReadLe32(fileCountBytes);
    cursor += 4;

    const unsigned char* modeBytes = memFile->read(cursor, 2);
    if (!modeBytes) return false;
    m_mode = static_cast<int16_t>(ReadLe16(modeBytes));
    cursor += 2;

    if (m_mode != 0x30) {
        DbgLog("[ThorPak::Open] unsupported mode 0x%X (only 0x30 multi-file is supported)\n",
            static_cast<unsigned>(m_mode));
        return false;
    }

    const unsigned char* nameLenBytes = memFile->read(cursor, 1);
    if (!nameLenBytes) return false;
    const uint8_t targetNameLen = nameLenBytes[0];
    cursor += 1;

    if (targetNameLen > 0) {
        const unsigned char* nameBytes = memFile->read(cursor, targetNameLen);
        if (!nameBytes) return false;
        m_targetGrf.assign(reinterpret_cast<const char*>(nameBytes), targetNameLen);
        cursor += targetNameLen;
    } else {
        m_targetGrf.clear();
    }

    const unsigned char* compLenBytes = memFile->read(cursor, 4);
    if (!compLenBytes) return false;
    const uint32_t tableCompressedLength = ReadLe32(compLenBytes);
    cursor += 4;

    const unsigned char* offsetBytes = memFile->read(cursor, 4);
    if (!offsetBytes) return false;
    const uint32_t fileTableOffset = ReadLe32(offsetBytes);

    if (fileTableOffset > fileSize || tableCompressedLength == 0
        || fileTableOffset + tableCompressedLength > fileSize) {
        DbgLog("[ThorPak::Open] file table bounds invalid (off=%u len=%u size=%u)\n",
            fileTableOffset, tableCompressedLength, fileSize);
        return false;
    }

    const unsigned char* compressed = memFile->read(fileTableOffset, tableCompressedLength);
    if (!compressed) return false;

    // Inflate the table. We don't know the uncompressed size up front; grow
    // the buffer until inflate succeeds. Cap at 64 MB to avoid runaway.
    constexpr uLongf kMaxTableSize = 64ull * 1024ull * 1024ull;
    uLongf decompressedSize = static_cast<uLongf>((std::max)(uint32_t{1024}, tableCompressedLength * 8u));
    std::vector<uint8_t> table;
    while (true) {
        if (decompressedSize > kMaxTableSize) {
            DbgLog("[ThorPak::Open] table inflate exceeded cap (%lu bytes)\n",
                static_cast<unsigned long>(decompressedSize));
            return false;
        }
        table.resize(decompressedSize);
        uLongf actual = decompressedSize;
        const int rc = uncompress(table.data(), &actual, compressed,
            static_cast<uLong>(tableCompressedLength));
        if (rc == Z_OK) {
            table.resize(actual);
            break;
        }
        if (rc != Z_BUF_ERROR) {
            DbgLog("[ThorPak::Open] uncompress failed rc=%d\n", rc);
            return false;
        }
        decompressedSize *= 2;
    }

    if (!ParseMode30Table(table.data(), static_cast<uint32_t>(table.size()))) {
        DbgLog("[ThorPak::Open] ParseMode30Table failed (entries=%u)\n", numberOfFiles);
        return false;
    }

    DbgLog("[ThorPak::Open] OK target='%s' merge=%d entries=%zu\n",
        m_targetGrf.c_str(),
        m_useGrfMerging ? 1 : 0,
        m_PakPack.size());
    return true;
}

bool CThorPak::ParseMode30Table(const unsigned char* table, uint32_t tableSize)
{
    if (!table) return false;

    m_PakPack.clear();
    m_removedFiles.clear();
    uint32_t off = 0;
    uint32_t maxScratch = 0;

    while (off < tableSize) {
        if (off + 1 > tableSize) return false;
        const uint8_t nameLen = table[off];
        off += 1;
        if (nameLen == 0 || off + nameLen + 1 > tableSize) {
            // Empty name or out-of-bounds — table ended.
            break;
        }

        const char* nameBytes = reinterpret_cast<const char*>(table + off);
        std::string name(nameBytes, nameLen);
        off += nameLen;

        const uint8_t flags = table[off];
        off += 1;

        PakPack entry{};
        entry.m_fName.SetString(name.c_str());
        if ((flags & 0x01u) != 0u) {
            // Remove-file tombstone — hide the file from any base GRF behind
            // this patch. The hash goes into a sorted list that lookup
            // consults before falling through to GRFs (see CFileMgr).
            m_removedFiles.push_back(entry.m_fName);
            continue;
        }

        if (off + 12 > tableSize) return false;
        entry.m_Offset = ReadLe32(table + off);
        entry.m_compressSize = ReadLe32(table + off + 4);
        entry.m_dataSize = entry.m_compressSize;
        entry.m_size = ReadLe32(table + off + 8);
        entry.m_type = 0u;
        off += 12;

        if (entry.m_compressSize > maxScratch) {
            maxScratch = entry.m_compressSize;
        }

        m_PakPack.push_back(entry);
    }

    std::sort(m_PakPack.begin(), m_PakPack.end(), PakPrtLess{});
    std::sort(m_removedFiles.begin(), m_removedFiles.end());
    m_decompressScratch.resize(maxScratch);
    return true;
}

bool CThorPak::IsRemoved(const CHash& key) const
{
    return std::binary_search(m_removedFiles.begin(), m_removedFiles.end(), key);
}

bool CThorPak::GetInfo(const CHash& key, PakPack* out) const
{
    PakPack probe{};
    probe.m_fName = key;
    auto it = std::lower_bound(m_PakPack.begin(), m_PakPack.end(), probe, PakPrtLess{});
    if (it == m_PakPack.end() || it->m_fName != key) {
        return false;
    }
    if (out) *out = *it;
    return true;
}

bool CThorPak::GetData(const PakPack& pack, void* buffer)
{
    if (!buffer || !m_memFile) return false;
    if (pack.m_Offset > 0xFFFFFFFFull) return false;

    const unsigned char* raw = m_memFile->read(static_cast<u32>(pack.m_Offset),
        pack.m_compressSize);
    if (!raw) return false;

    // Mode 0x30 entries are zlib-compressed. The "raw data file" / LZMA / custom
    // compression flavors GRFEditor handles for write-time tooling don't appear
    // in patch THORs we encounter at runtime; if one shows up uncompress will
    // simply fail and the lookup will fall through to the base GRF.
    uLongf actual = pack.m_size;
    const int rc = uncompress(static_cast<unsigned char*>(buffer), &actual,
        raw, static_cast<uLong>(pack.m_compressSize));
    if (rc != Z_OK) {
        DbgLog("[ThorPak::GetData] uncompress rc=%d for '%s'\n", rc, pack.m_fName.m_String);
        return false;
    }
    return actual == pack.m_size;
}

void CThorPak::CollectFileNamesByExtension(const char* ext, std::vector<std::string>& out) const
{
    if (!ext || !*ext) return;

    char normalizedExt[32] = {};
    if (ext[0] == '.') {
        std::strncpy(normalizedExt, ext, sizeof(normalizedExt) - 1);
    } else {
        normalizedExt[0] = '.';
        std::strncpy(normalizedExt + 1, ext, sizeof(normalizedExt) - 2);
    }

    for (const PakPack& pack : m_PakPack) {
        const char* name = pack.m_fName.m_String;
        if (!name || !*name) continue;
        const char* dot = std::strrchr(name, '.');
        if (!dot) continue;
        if (_stricmp(dot, normalizedExt) == 0) {
            out.emplace_back(name);
        }
    }
}
