#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <ShlObj.h>
#include "../utils/json.hpp"
#include "vars.h"
#include "../utils/xorstr.hpp"

namespace config
{
    inline std::string GetConfigPath()
    {
        char documentsPath[MAX_PATH];
        HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documentsPath);

        if (SUCCEEDED(result))
        {
            std::string configFolder = std::string(documentsPath) + xorstr_("\\PicoDriver");
            return configFolder + xorstr_("\\default.json");
        }
        return "";
    }

    inline void LoadConfig()
    {
        std::string configPath = GetConfigPath();
        if (configPath.empty()) return;

        std::ifstream file(configPath);
        if (!file.is_open()) return;

        try
        {
            nlohmann::json j;
            file >> j;
            file.close();

            if (j.contains(xorstr_("esp")))
            {
                auto& esp = j[xorstr_("esp")];
                if (esp.contains(xorstr_("esp"))) vars::esp = esp[xorstr_("esp")];
                if (esp.contains(xorstr_("showPlayerBox"))) vars::showPlayerBox = esp[xorstr_("showPlayerBox")];
                if (esp.contains(xorstr_("showPlayerBone"))) vars::showPlayerBone = esp[xorstr_("showPlayerBone")];
                if (esp.contains(xorstr_("showPlayerName"))) vars::showPlayerName = esp[xorstr_("showPlayerName")];
                if (esp.contains(xorstr_("showTeammates"))) vars::showTeammates = esp[xorstr_("showTeammates")];
                if (esp.contains(xorstr_("showHealthBar"))) vars::showHealthBar = esp[xorstr_("showHealthBar")];
                if (esp.contains(xorstr_("showWeapon"))) vars::showWeapon = esp[xorstr_("showWeapon")];
                if (esp.contains(xorstr_("showDistance"))) vars::showDistance = esp[xorstr_("showDistance")];
                if (esp.contains(xorstr_("showArmor"))) vars::showArmor = esp[xorstr_("showArmor")];
            }

            if (j.contains(xorstr_("aim")))
            {
                auto& aimbot = j[xorstr_("aim")];
                if (aimbot.contains(xorstr_("aim"))) vars::aim = aimbot[xorstr_("aim")];
                if (aimbot.contains(xorstr_("rcs"))) vars::rcs = aimbot[xorstr_("rcs")];
            }

            if (j.contains(xorstr_("misc")))
            {
                auto& aimbot = j[xorstr_("misc")];
                if (aimbot.contains(xorstr_("bombTimer"))) vars::bombTimer = aimbot[xorstr_("bombTimer")];
                if (aimbot.contains(xorstr_("showItems"))) vars::showItems = aimbot[xorstr_("showItems")];
                if (aimbot.contains(xorstr_("textSize"))) vars::textSize = aimbot[xorstr_("textSize")];
                if (aimbot.contains(xorstr_("maxFps"))) vars::maxFps = aimbot[xorstr_("maxFps")];
            }

        }
        catch (const std::exception& e)
        {
            return;
        }
    }

    inline void SaveConfig()
    {
        std::string configPath = GetConfigPath();
        if (configPath.empty()) return;

        std::filesystem::path configDir = std::filesystem::path(configPath).parent_path();
        create_directories(configDir);

        nlohmann::json j;

        j[xorstr_("esp")] = {
            {xorstr_("showPlayerBox"), vars::showPlayerBox},
            {xorstr_("showPlayerBone"), vars::showPlayerBone},
            {xorstr_("showPlayerName"), vars::showPlayerName},
            {xorstr_("showTeammates"), vars::showTeammates},
            {xorstr_("showHealthBar"), vars::showHealthBar},
            {xorstr_("showWeapon"), vars::showWeapon},
            {xorstr_("ShowDistance"), vars::showDistance},
            {xorstr_("showArmor"), vars::showArmor},
        };

        j[xorstr_("aim")] = {
            {xorstr_("aim"), vars::aim},
            {xorstr_("rcs"), vars::rcs},
        };

        j[xorstr_("misc")] = {
            {xorstr_("bombTimer"), vars::bombTimer},
            {xorstr_("showItems"), vars::showItems},
            {xorstr_("textSize"), vars::textSize},
            {xorstr_("maxFps"), vars::maxFps},
        };


        try
        {
            std::ofstream file(configPath);
            file << j.dump(2);
            file.close();
        }
        catch (const std::exception& e)
        {
            return;
        }
    }
}