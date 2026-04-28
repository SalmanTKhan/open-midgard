#pragma once

#include <string>

namespace mapname {

// Returns the human-friendly map name for a map ID or filename.
//
// Accepts inputs like "prontera", "prontera.gat", "prt_fild08.rsw", etc.
// and returns "Prontera" / "Prontera Field 08" if known. Falls back to the
// trimmed map ID (no extension) if the table doesn't list the map or the
// table couldn't be loaded.
//
// The map name table is read lazily on first call from
// `data/mapnametable.txt` via `g_fileMgr` (so GRF + extracted data dir +
// `OPEN_MIDGARD_DATA_DIR` overrides all work). Lookup is case-insensitive on
// the trimmed ID. Thread-safety: not required — UI is single-threaded.
std::string ResolveMapDisplayName(const std::string& mapIdOrFilename);

}  // namespace mapname
