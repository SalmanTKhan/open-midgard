#include "ClientInfoLocale.h"
#include "../DebugLog.h"
#include "Globals.h"
#include "File.h"
#include "SettingsIni.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

namespace {

std::vector<ClientInfoConnection> g_clientInfoConnections;
int g_selectedClientInfoIndex = 0;
int g_clientInfoStateGeneration = 0;

void BumpClientInfoStateGeneration()
{
    ++g_clientInfoStateGeneration;
}

std::string ToLowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string TrimAscii(std::string value)
{
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

struct FallbackClientInfoConfig {
    std::string host;
    int port = 0;
    int packetVersion = 0;
    bool configured = false;
};

void AddUniqueLoadingScreen(const std::string& path)
{
    if (path.empty()) {
        return;
    }

    const std::string lowered = ToLowerAscii(path);
    for (const std::string& existing : g_loadingScreenList) {
        if (ToLowerAscii(existing) == lowered) {
            return;
        }
    }
    g_loadingScreenList.push_back(path);
}

void AddDefaultLoadingScreensFromDirectory(const char* directory)
{
    if (!directory || !*directory) {
        return;
    }

    char path[260] = {};
    for (int index = 0; index <= 99; ++index) {
        std::snprintf(path, sizeof(path), "%sloading%02d.jpg", directory, index);
        if (g_fileMgr.IsDataExist(path)) {
            AddUniqueLoadingScreen(path);
        }
    }
}

void EnsureDefaultLoadingScreens()
{
    static const char* kUiKor =
        "data\\texture\\"
        "\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA"
        "\\";

    AddDefaultLoadingScreensFromDirectory("data\\texture\\basepic\\");
    AddDefaultLoadingScreensFromDirectory(kUiKor);
}

std::string GetChildContents(XMLElement* element, const char* childName)
{
    if (!element) {
        return {};
    }
    XMLElement* child = element->FindChild(childName);
    if (!child) {
        return {};
    }
    return child->GetContents();
}

std::string GetChildContentsAny(XMLElement* element, std::initializer_list<const char*> childNames)
{
    if (!element) {
        return {};
    }

    for (const char* childName : childNames) {
        const std::string value = GetChildContents(element, childName);
        if (!value.empty()) {
            return value;
        }
    }

    return {};
}

bool EqualsIgnoreCase(const std::string& left, const char* right)
{
    if (!right) {
        return false;
    }

    size_t rightLen = std::strlen(right);
    if (left.size() != rightLen) {
        return false;
    }

    for (size_t i = 0; i < rightLen; ++i) {
        if (std::tolower(static_cast<unsigned char>(left[i])) !=
            std::tolower(static_cast<unsigned char>(right[i]))) {
            return false;
        }
    }

    return true;
}

void ParseLoadingImages(XMLElement* element)
{
    if (!element) {
        return;
    }

    XMLElement* loading = element->FindChild("loading");
    if (!loading) {
        return;
    }

    g_loadingScreenList.clear();
    for (XMLElement* image = loading->FindChild("image"); image; image = image->FindNext("image")) {
        const std::string& contents = image->GetContents();
        if (contents.empty()) {
            continue;
        }
        AddUniqueLoadingScreen("texture\\loading\\" + contents);
    }

    EnsureDefaultLoadingScreens();
}

#if !RO_PLATFORM_WINDOWS
std::string NormalizePortablePath(const char* path)
{
    if (!path) {
        return {};
    }

    std::string normalized(path);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized;
}
#endif

bool LoadDiskFile(const char* fileName, std::vector<char>* outBuffer)
{
    if (!fileName || !*fileName || !outBuffer) {
        return false;
    }

#if RO_PLATFORM_WINDOWS
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
#else
    const std::string normalizedPath = NormalizePortablePath(fileName);
    std::ifstream file(normalizedPath, std::ios::binary | std::ios::ate);
#endif
    if (!file.is_open()) {
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
        return false;
    }

    outBuffer->resize(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    return file.read(outBuffer->data(), size).good();
}

bool BuildExecutableRelativePath(const char* relativePath, char* outPath, size_t outPathSize)
{
    if (!relativePath || !*relativePath || !outPath || outPathSize == 0) {
        return false;
    }

    char modulePath[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }

    char* lastBackslash = std::strrchr(modulePath, '\\');
    char* lastForwardSlash = std::strrchr(modulePath, '/');
    char* lastSlash = lastBackslash;
    if (!lastSlash || (lastForwardSlash && lastForwardSlash > lastSlash)) {
        lastSlash = lastForwardSlash;
    }
    if (!lastSlash) {
        return false;
    }
    lastSlash[1] = '\0';

#if RO_PLATFORM_WINDOWS
    const int written = std::snprintf(outPath, outPathSize, "%s%s", modulePath, relativePath);
#else
    std::string normalizedRelative(relativePath);
    std::replace(normalizedRelative.begin(), normalizedRelative.end(), '\\', '/');
    const int written = std::snprintf(outPath, outPathSize, "%s%s", modulePath, normalizedRelative.c_str());
#endif
    return written > 0 && static_cast<size_t>(written) < outPathSize;
}

bool TryLoadExecutableRelativeFile(const char* relativePath, std::vector<char>* outBuffer)
{
#if !RO_PLATFORM_WINDOWS
    if (LoadDiskFile(relativePath, outBuffer)) {
        return true;
    }
#endif

    char absolutePath[MAX_PATH] = {};
    if (!BuildExecutableRelativePath(relativePath, absolutePath, sizeof(absolutePath))) {
        return false;
    }
    return LoadDiskFile(absolutePath, outBuffer);
}

bool HasLocalDataDirectory()
{
#if !RO_PLATFORM_WINDOWS
    const DWORD runtimeAttrs = GetFileAttributesA("data");
    if (runtimeAttrs != INVALID_FILE_ATTRIBUTES && (runtimeAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return true;
    }
#endif

    char dataPath[MAX_PATH] = {};
    if (!BuildExecutableRelativePath("data", dataPath, sizeof(dataPath))) {
        return false;
    }

    const DWORD attrs = GetFileAttributesA(dataPath);
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool ShouldPreferLocalDataFile(const char* fileName)
{
    if (!fileName || !*fileName || !HasLocalDataDirectory()) {
        return false;
    }

    return _strnicmp(fileName, "data\\", 5) == 0 || _strnicmp(fileName, "data/", 5) == 0;
}

bool LoadClientInfoBytes(const char* fileName, std::vector<char>* outBuffer)
{
    if (!fileName || !*fileName || !outBuffer) {
        return false;
    }

    outBuffer->clear();

    if (ShouldPreferLocalDataFile(fileName)) {
        if (TryLoadExecutableRelativeFile(fileName, outBuffer) || LoadDiskFile(fileName, outBuffer)) {
            DbgLog("[ClientInfo] Loaded local file first: %s\n", fileName);
            return true;
        }
    }

    int size = 0;
    unsigned char* bytes = g_fileMgr.GetData(fileName, &size);
    if (bytes && size > 0) {
        outBuffer->assign(reinterpret_cast<const char*>(bytes), reinterpret_cast<const char*>(bytes) + size);
        delete[] bytes;
        DbgLog("[ClientInfo] Loaded from GRF/fallback: %s size=%d\n", fileName, size);
        return true;
    }
    delete[] bytes;

    if (LoadDiskFile(fileName, outBuffer)) {
        DbgLog("[ClientInfo] Loaded fallback disk path: %s\n", fileName);
        return true;
    }

    return false;
}

void ApplyFallbackClientInfoConfig(const FallbackClientInfoConfig& config)
{
    const std::string host = config.host.empty() ? "127.0.0.1" : config.host;
    const int port = config.port > 0 ? config.port : 6900;

    g_accountAddr = host;
    g_accountPort = std::to_string(port);
    if (config.packetVersion > 0) {
        g_version = config.packetVersion;
    }

    g_clientInfoConnections.clear();
    ClientInfoConnection info;
    info.display = host + ":" + std::to_string(port);
    info.desc = "Fallback account endpoint";
    info.address = host;
    info.port = std::to_string(port);
    info.version = g_version;
    g_clientInfoConnections.push_back(info);
    g_selectedClientInfoIndex = 0;

    EnsureDefaultLoadingScreens();
    s_dwAdminAID.clear();
    s_dwYellowAID.clear();
    BumpClientInfoStateGeneration();
}

void ParseFallbackClientInfoLine(const std::string& rawLine, FallbackClientInfoConfig* outConfig)
{
    if (!outConfig) {
        return;
    }

    const std::string line = TrimAscii(rawLine);
    if (line.empty()) {
        return;
    }
    if (line[0] == '#' || (line.size() >= 2 && line[0] == '/' && line[1] == '/')) {
        return;
    }

    const std::string::size_type equals = line.find('=');
    if (equals == std::string::npos || equals == 0) {
        return;
    }

    const std::string key = ToLowerAscii(TrimAscii(line.substr(0, equals)));
    const std::string value = TrimAscii(line.substr(equals + 1));
    if (value.empty()) {
        return;
    }

    if (key == "auth_host" || key == "server_host") {
        outConfig->host = value;
        outConfig->configured = true;
        return;
    }
    if (key == "auth_port" || key == "server_port") {
        const int parsed = std::atoi(value.c_str());
        if (parsed > 0) {
            outConfig->port = parsed;
            outConfig->configured = true;
        }
        return;
    }
    if (key == "packet_version") {
        const int parsed = std::atoi(value.c_str());
        if (parsed > 0) {
            outConfig->packetVersion = parsed;
            outConfig->configured = true;
        }
    }
}

bool LoadFallbackClientInfoConfigFromFile(FallbackClientInfoConfig* outConfig)
{
    if (!outConfig) {
        return false;
    }

    std::vector<char> buffer;
    if (!TryLoadExecutableRelativeFile("client.cfg", &buffer) && !LoadDiskFile("client.cfg", &buffer)) {
        return false;
    }

    std::istringstream stream(std::string(buffer.begin(), buffer.end()));
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        ParseFallbackClientInfoLine(line, outConfig);
    }
    return outConfig->configured;
}

void LoadFallbackClientInfoConfigFromSettings(FallbackClientInfoConfig* outConfig)
{
    if (!outConfig) {
        return;
    }

    const std::string host = LoadSettingsIniString("Network", "AuthHost", "");
    if (!host.empty()) {
        outConfig->host = host;
        outConfig->configured = true;
    }

    const int port = LoadSettingsIniInt("Network", "AuthPort", 0);
    if (port > 0) {
        outConfig->port = port;
        outConfig->configured = true;
    }

    const int packetVersion = LoadSettingsIniInt("Network", "PacketVersion", 0);
    if (packetVersion > 0) {
        outConfig->packetVersion = packetVersion;
        outConfig->configured = true;
    }
}

void LoadFallbackClientInfoConfigFromEnvironment(FallbackClientInfoConfig* outConfig)
{
    if (!outConfig) {
        return;
    }

    if (const char* host = std::getenv("OPEN_MIDGARD_AUTH_HOST")) {
        if (*host) {
            outConfig->host = host;
            outConfig->configured = true;
        }
    }

    if (const char* port = std::getenv("OPEN_MIDGARD_AUTH_PORT")) {
        if (*port) {
            const int parsed = std::atoi(port);
            if (parsed > 0) {
                outConfig->port = parsed;
                outConfig->configured = true;
            }
        }
    }

    if (const char* packetVersion = std::getenv("OPEN_MIDGARD_PACKET_VERSION")) {
        if (*packetVersion) {
            const int parsed = std::atoi(packetVersion);
            if (parsed > 0) {
                outConfig->packetVersion = parsed;
                outConfig->configured = true;
            }
        }
    }
}

void ParseAidList(XMLElement* parent, std::vector<u32>* outList)
{
    if (!outList) {
        return;
    }

    outList->clear();
    if (!parent) {
        return;
    }

    for (XMLElement* entry = parent->FindChild("admin"); entry; entry = entry->FindNext("admin")) {
        const std::string& contents = entry->GetContents();
        if (contents.empty()) {
            continue;
        }
        outList->push_back(static_cast<u32>(std::strtoul(contents.c_str(), nullptr, 10)));
    }

    std::sort(outList->begin(), outList->end());
    outList->erase(std::unique(outList->begin(), outList->end()), outList->end());
}

void StoreConnectionOverrideIfPresent(ClientInfoConnection* info,
                                      XMLElement* connection,
                                      const char* key,
                                      std::initializer_list<const char*> childNames)
{
    if (!info || !connection || !key) {
        return;
    }

    const std::string value = TrimAscii(GetChildContentsAny(connection, childNames));
    if (!value.empty()) {
        info->runtimeOverrides[ToLowerAscii(key)] = value;
    }
}

void ParseClientInfoConnections(XMLElement* clientInfo)
{
    g_clientInfoConnections.clear();
    if (!clientInfo) {
        return;
    }

    for (XMLElement* connection = clientInfo->FindChild("connection"); connection; connection = connection->FindNext("connection")) {
        ClientInfoConnection info;
        info.display = GetChildContents(connection, "display");
        info.desc = GetChildContents(connection, "desc");
        info.address = GetChildContents(connection, "address");
        info.port = GetChildContents(connection, "port");
        info.registrationWeb = GetChildContents(connection, "registrationweb");

        const std::string version = GetChildContents(connection, "version");
        if (!version.empty()) {
            info.version = std::atoi(version.c_str());
        }

        const std::string langType = GetChildContents(connection, "langtype");
        if (!langType.empty()) {
            info.langType = std::atoi(langType.c_str());
        }

        StoreConnectionOverrideIfPresent(&info, connection, "clientprofile",
            { "clientprofile", "ClientProfile", "profile", "Profile" });
        StoreConnectionOverrideIfPresent(&info, connection, "charlistreceivelayout",
            { "charlistreceivelayout", "CharListReceiveLayout" });
        StoreConnectionOverrideIfPresent(&info, connection, "mapsendprofile",
            { "mapsendprofile", "MapSendProfile" });
        StoreConnectionOverrideIfPresent(&info, connection, "mapgameplaysendprofile",
            { "mapgameplaysendprofile", "MapGameplaySendProfile" });
        StoreConnectionOverrideIfPresent(&info, connection, "zoneconnectprofile",
            { "zoneconnectprofile", "ZoneConnectProfile" });
        StoreConnectionOverrideIfPresent(&info, connection, "accountloginpacket",
            { "accountloginpacket", "AccountLoginPacket" });
        StoreConnectionOverrideIfPresent(&info, connection, "clientdateoverride",
            { "clientdateoverride", "ClientDateOverride" });
        StoreConnectionOverrideIfPresent(&info, connection, "clienthashoverridemd5",
            { "clienthashoverridemd5", "ClientHashOverrideMd5" });
        StoreConnectionOverrideIfPresent(&info, connection, "clienthashsourceexe",
            { "clienthashsourceexe", "ClientHashSourceExe" });
        StoreConnectionOverrideIfPresent(&info, connection, "clienttypeoverride",
            { "clienttypeoverride", "ClientTypeOverride" });
        StoreConnectionOverrideIfPresent(&info, connection, "clientversionoverride",
            { "clientversionoverride", "ClientVersionOverride" });

        g_clientInfoConnections.push_back(info);
    }
}

}

//===========================================================================
// Helpers
//===========================================================================
static void SetOption(XMLElement* element) {
    if (!element) {
        return;
    }

    const std::string serviceType = GetChildContents(element, "servicetype");
    if (EqualsIgnoreCase(serviceType, "korea")) g_serviceType = ServiceKorea;
    else if (EqualsIgnoreCase(serviceType, "america")) g_serviceType = ServiceAmerica;
    else if (EqualsIgnoreCase(serviceType, "japan")) g_serviceType = ServiceJapan;
    else if (EqualsIgnoreCase(serviceType, "china")) g_serviceType = ServiceChina;
    else if (EqualsIgnoreCase(serviceType, "taiwan")) g_serviceType = ServiceTaiwan;
    else if (EqualsIgnoreCase(serviceType, "thai")) g_serviceType = ServiceThai;
    else if (EqualsIgnoreCase(serviceType, "indonesia")) g_serviceType = ServiceIndonesia;
    else if (EqualsIgnoreCase(serviceType, "philippine")) g_serviceType = ServicePhilippine;
    else if (EqualsIgnoreCase(serviceType, "malaysia")) g_serviceType = ServiceMalaysia;
    else if (EqualsIgnoreCase(serviceType, "singapore")) g_serviceType = ServiceSingapore;
    else if (EqualsIgnoreCase(serviceType, "germany")) g_serviceType = ServiceGermany;
    else if (EqualsIgnoreCase(serviceType, "india")) g_serviceType = ServiceIndia;
    else if (EqualsIgnoreCase(serviceType, "brazil")) g_serviceType = ServiceBrazil;
    else if (EqualsIgnoreCase(serviceType, "australia")) g_serviceType = ServiceAustralia;
    else if (EqualsIgnoreCase(serviceType, "russia")) g_serviceType = ServiceRussia;
    else if (EqualsIgnoreCase(serviceType, "vietnam")) g_serviceType = ServiceVietnam;
    else if (EqualsIgnoreCase(serviceType, "chile")) g_serviceType = ServiceChile;
    else if (EqualsIgnoreCase(serviceType, "france")) g_serviceType = ServiceFrance;

    const std::string serverType = GetChildContents(element, "servertype");
    if (EqualsIgnoreCase(serverType, "primary")) g_serverType = ServerNormal;
    else if (EqualsIgnoreCase(serverType, "sakray")) g_serverType = ServerSakray;
    else if (EqualsIgnoreCase(serverType, "local")) g_serverType = ServerLocal;
    else if (EqualsIgnoreCase(serverType, "pk")) g_serverType = ServerPK;

    if (element->FindChild("passwordencrypt")) {
        g_passwordEncrypt = 1;
    }

    if (element->FindChild("passwordencrypt2")) {
        g_passwordEncrypt = 1;
        g_passwordEncrypt2 = 1;
    }

    if (element->FindChild("readfolder")) {
        g_readFolderFirst = 1;
    }

    ParseLoadingImages(element);
}

//===========================================================================
// Implementation
//===========================================================================
bool InitClientInfo(const char* fileName) {
    std::vector<char> buffer;
    if (!LoadClientInfoBytes(fileName, &buffer) || buffer.empty()) {
        DbgLog("[ClientInfo] Failed to load candidate: %s\n", fileName ? fileName : "(null)");
        return false;
    }

    if (!g_xmlDocument.ReadDocument(buffer.data(), buffer.data() + buffer.size())) {
        return false;
    }

    XMLElement* clientInfo = GetClientInfo();
    if (!clientInfo) {
        return false;
    }

    ParseClientInfoConnections(clientInfo);
    DbgLog("[ClientInfo] Parsed %d connection entries from %s\n", static_cast<int>(g_clientInfoConnections.size()), fileName ? fileName : "(null)");
    g_passwordEncrypt = 0;
    g_passwordEncrypt2 = 0;
    SetOption(clientInfo);
    g_selectedClientInfoIndex = 0;
    SelectClientInfo(0);
    DbgLog("[ClientInfo] Auth flags: passwordencrypt=%d passwordencrypt2=%d\n",
           g_passwordEncrypt,
           g_passwordEncrypt2);
    return true;
}

bool InitFallbackClientInfo()
{
    FallbackClientInfoConfig config;
    LoadFallbackClientInfoConfigFromSettings(&config);
    const bool loadedClientCfg = LoadFallbackClientInfoConfigFromFile(&config);
    LoadFallbackClientInfoConfigFromEnvironment(&config);
    ApplyFallbackClientInfoConfig(config);
    if (config.packetVersion <= 0) {
        DbgLog("[ClientInfo] PacketVersion unset; using the packet_ver 23 map-send profile.\n");
    }

    DbgLog("[ClientInfo] Using fallback account endpoint %s:%s version=%d source=%s%s%s\n",
        g_accountAddr.c_str(),
        g_accountPort.c_str(),
        g_version,
        loadedClientCfg ? "client.cfg " : "",
        config.configured ? "configured " : "",
        (!loadedClientCfg && !config.configured) ? "default" : "");
    return config.configured || loadedClientCfg;
}

XMLElement* GetClientInfo() {
    return g_xmlDocument.m_root.FindChild("clientinfo");
}

void SelectClientInfo(int connectionIndex) {
    XMLElement* clientInfo = GetClientInfo();
    if (!clientInfo) return;

    XMLElement* connection = clientInfo->FindChild("connection");
    int current = 0;
    while (connection && current < connectionIndex) {
        connection = connection->FindNext("connection");
        current++;
    }

    if (!connection) return;

    g_selectedClientInfoIndex = current;

    SetOption(connection);

    XMLElement* addr = connection->FindChild("address");
    if (addr) g_accountAddr = addr->GetContents();

    XMLElement* port = connection->FindChild("port");
    if (port) g_accountPort = port->GetContents();

    XMLElement* version = connection->FindChild("version");
    if (version) g_version = std::atoi(version->GetContents().c_str());

    XMLElement* lang = connection->FindChild("langtype");
    if (lang) g_serviceType = (ServiceType)std::atoi(lang->GetContents().c_str());

    XMLElement* registration = connection->FindChild("registrationweb");
    if (registration) g_regstrationWeb = registration->GetContents();

    ParseAidList(connection->FindChild("aid"), &s_dwAdminAID);
    XMLElement* yellow = connection->FindChild("yellow");
    if (yellow) {
        ParseAidList(yellow, &s_dwYellowAID);
    } else {
        s_dwYellowAID = s_dwAdminAID;
    }

    BumpClientInfoStateGeneration();
}

void SelectClientInfo2(int connectionIndex, int subConnectionIndex) {
    // Port of the original SelectClientInfo2 logic if needed for sub-connections
    (void)subConnectionIndex;
    SelectClientInfo(connectionIndex);
}

const std::vector<std::string>& GetLoadingScreenList() {
    EnsureDefaultLoadingScreens();
    return g_loadingScreenList;
}

void RefreshDefaultLoadingScreenList() {
    EnsureDefaultLoadingScreens();
}

const std::vector<ClientInfoConnection>& GetClientInfoConnections()
{
    return g_clientInfoConnections;
}

int GetClientInfoConnectionCount()
{
    return static_cast<int>(g_clientInfoConnections.size());
}

int GetSelectedClientInfoIndex()
{
    return g_selectedClientInfoIndex;
}

int GetClientInfoStateGeneration()
{
    return g_clientInfoStateGeneration;
}

const ClientInfoConnection* GetSelectedClientInfoConnection()
{
    if (g_selectedClientInfoIndex < 0
        || g_selectedClientInfoIndex >= static_cast<int>(g_clientInfoConnections.size())) {
        return nullptr;
    }

    return &g_clientInfoConnections[static_cast<size_t>(g_selectedClientInfoIndex)];
}

bool TryLoadSelectedClientInfoSettingString(const char* key, std::string* value)
{
    if (!key || !*key || !value) {
        return false;
    }

    const ClientInfoConnection* connection = GetSelectedClientInfoConnection();
    if (!connection) {
        return false;
    }

    const auto found = connection->runtimeOverrides.find(ToLowerAscii(key));
    if (found == connection->runtimeOverrides.end()) {
        return false;
    }

    *value = found->second;
    return true;
}

bool TryLoadSelectedClientInfoSettingInt(const char* key, int* value)
{
    if (!key || !*key || !value) {
        return false;
    }

    std::string rawValue;
    if (!TryLoadSelectedClientInfoSettingString(key, &rawValue)) {
        return false;
    }

    const std::string trimmedValue = TrimAscii(rawValue);
    if (trimmedValue.empty()) {
        return false;
    }

    char* end = nullptr;
    const long parsed = std::strtol(trimmedValue.c_str(), &end, 10);
    if (end == trimmedValue.c_str() || (end && *end != '\0')) {
        return false;
    }

    *value = static_cast<int>(parsed);
    return true;
}

bool IsGravityAid(unsigned int aid)
{
    return std::binary_search(s_dwAdminAID.begin(), s_dwAdminAID.end(), static_cast<u32>(aid));
}

bool IsNameYellow(unsigned int aid)
{
    return std::binary_search(s_dwYellowAID.begin(), s_dwYellowAID.end(), static_cast<u32>(aid));
}
