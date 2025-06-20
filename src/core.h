#pragma once

#define CLEO_ANDROID		"CLEO ANDROID"
#define COPYRIGHT			"(C) Alexander Blade"
#define WEBPAGE				"http://dev-c.com"
#define WWW_WEBPAGE			"www.dev-c.com"
#define VERSION_STR 		CLEO_ANDROID " (" VERSION_DATE ")"
#define VERSION_DATE	 	"04 Apr 2021"
#define VERSION_CODE 		201
#define VERSION_CODE_STR 	"2.0.1"

#include "common.h"

namespace core
{
	enum e_game
	{
		GTA3,
		GTAVC,
		GTASA,
		GTALCS,
		GTAVCS
	};

	e_game GetGame();
	int GetGameVersion();

	const char *GetPackageName();
	const char *GetPackageVersionStr();
	int GetPackageVersionCode();

	uint16_t *GetGxtEntry(LPCSTR name, bool useCustom);

	uint32_t *GetScriptParams();
	void CRunningScript__CollectParameters(ptr thiz, uint32_t count);
	ptr CRunningScript__GetPointerToScriptVariable(ptr thiz, uint8_t param);

	void initialize();

	ptr get_real_code_ptr(uint32_t ip);
	uint32_t get_ip_using_real_code_ptr(ptr realPtr);
	ptr get_real_label_ptr(ptr handle, uint32_t offset);
	bool read_str_8byte(ptr handle, std::string &str);
	bool read_str_long(ptr handle, std::string &str);
}