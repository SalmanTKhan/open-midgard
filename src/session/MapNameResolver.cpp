#include "MapNameResolver.h"

#include "core/File.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace mapname {
namespace {

// Lowercase ASCII; non-ASCII bytes pass through unchanged so Korean/Japanese
// map IDs (rare in 2008) still hash deterministically.
std::string ToLowerAscii(std::string s)
{
    for (char& ch : s) {
        const unsigned char u = static_cast<unsigned char>(ch);
        if (u >= 'A' && u <= 'Z') {
            ch = static_cast<char>(u - 'A' + 'a');
        }
    }
    return s;
}

std::string TrimExtensionAndPath(const std::string& s)
{
    // Strip directory.
    size_t slash = s.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? s : s.substr(slash + 1);

    // Strip the last extension (.gat / .rsw / .gnd / anything).
    size_t dot = base.find_last_of('.');
    if (dot != std::string::npos) {
        base.resize(dot);
    }
    return base;
}

std::string TrimWhitespace(const std::string& s)
{
    size_t a = 0;
    size_t b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

class Table {
public:
    static Table& Instance()
    {
        static Table s_inst;
        return s_inst;
    }

    const std::string* Lookup(const std::string& trimmedLowercaseId)
    {
        EnsureLoaded();
        auto it = m_table.find(trimmedLowercaseId);
        return (it == m_table.end()) ? nullptr : &it->second;
    }

private:
    void EnsureLoaded()
    {
        if (m_loaded) return;
        m_loaded = true;

        int size = 0;
        unsigned char* bytes = g_fileMgr.GetData("data\\mapnametable.txt", &size);
        if (!bytes || size <= 0) {
            if (bytes) delete[] bytes;
            return;
        }

        // Format: <mapfilename>#<DisplayName>#  (one per line)
        // Examples:
        //   prontera.rsw#Prontera#
        //   prt_fild08.rsw#Prontera Field 08#
        // Some lines also use .gat extension. Comments / blanks are tolerated.
        std::string line;
        auto commit = [&]() {
            std::string trimmed = TrimWhitespace(line);
            line.clear();
            if (trimmed.empty() || trimmed[0] == '/') return;
            const size_t firstHash = trimmed.find('#');
            if (firstHash == std::string::npos) return;
            const size_t secondHash = trimmed.find('#', firstHash + 1);
            if (secondHash == std::string::npos) return;
            std::string idPart = trimmed.substr(0, firstHash);
            std::string displayPart = trimmed.substr(firstHash + 1, secondHash - firstHash - 1);
            const std::string key = ToLowerAscii(TrimExtensionAndPath(idPart));
            if (key.empty() || displayPart.empty()) return;
            m_table.emplace(key, std::move(displayPart));
        };

        for (int i = 0; i < size; ++i) {
            const char ch = static_cast<char>(bytes[i]);
            if (ch == '\n' || ch == '\r') {
                commit();
            } else {
                line.push_back(ch);
            }
        }
        commit();

        delete[] bytes;
    }

    bool m_loaded = false;
    std::unordered_map<std::string, std::string> m_table;
};

}  // namespace

std::string ResolveMapDisplayName(const std::string& mapIdOrFilename)
{
    const std::string trimmed = TrimExtensionAndPath(mapIdOrFilename);
    if (trimmed.empty()) return trimmed;
    const std::string lookupKey = ToLowerAscii(trimmed);
    if (const std::string* hit = Table::Instance().Lookup(lookupKey)) {
        return *hit;
    }
    return trimmed;
}

}  // namespace mapname
