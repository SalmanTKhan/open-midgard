#pragma once
#include "Xml.h"
#include <map>
#include <string>
#include <vector>

struct ClientInfoConnection {
	std::string display;
	std::string desc;
	std::string address;
	std::string port;
	std::string registrationWeb;
	int version = 0;
	int langType = -1;
    std::map<std::string, std::string> runtimeOverrides;
};

//===========================================================================
// Client info locale functions
//===========================================================================
bool InitClientInfo(const char* fileName);
bool InitFallbackClientInfo();
XMLElement* GetClientInfo();

void SelectClientInfo(int connectionIndex);
void SelectClientInfo2(int connectionIndex, int subConnectionIndex);
int GetClientInfoStateGeneration();
const std::vector<std::string>& GetLoadingScreenList();
void RefreshDefaultLoadingScreenList();
const std::vector<ClientInfoConnection>& GetClientInfoConnections();
int GetClientInfoConnectionCount();
int GetSelectedClientInfoIndex();
const ClientInfoConnection* GetSelectedClientInfoConnection();
bool TryLoadSelectedClientInfoSettingString(const char* key, std::string* value);
bool TryLoadSelectedClientInfoSettingInt(const char* key, int* value);
bool IsGravityAid(unsigned int aid);
bool IsNameYellow(unsigned int aid);
