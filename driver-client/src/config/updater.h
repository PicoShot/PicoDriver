#pragma once
#include "../offsets/offsets.hpp"
#include "../utils/json.hpp"
#include <string>
#include "../utils/xorstr.hpp"
#include <wininet.h>

#include "../offsets/client_dll.hpp"
#pragma comment(lib, "wininet.lib")

namespace updater
{
    class OffsetUpdater {
    private:
        static std::string GetClientUrl() {
            return xorstr_("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json");
        }

        static std::string GetOffsetsUrl() {
            return xorstr_("https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.json");
        }
        static std::string DownloadJSON(const char* url) {
            std::string result;
            HINTERNET hInternet = InternetOpenA(xorstr_("Mozilla/5.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

            if (hInternet) {
                HINTERNET hConnection = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);

                if (hConnection) {
                    char buffer[2048];
                    DWORD bytesRead;

                    while (InternetReadFile(hConnection, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                        result.append(buffer, bytesRead);
                    }

                    InternetCloseHandle(hConnection);
                }
                InternetCloseHandle(hInternet);
            }
            return result;
        }

        static nlohmann::json ParseJSON(const std::string& jsonStr) {
            try {
                return nlohmann::json::parse(jsonStr);
            }
            catch (const nlohmann::json::parse_error& e) {
                return nlohmann::json();
            }
        }

    public:
        static bool UpdateOffsets()
    	{
            try {
                auto clientJson = ParseJSON(DownloadJSON(GetClientUrl().c_str()));
                auto offsetsJson = ParseJSON(DownloadJSON(GetOffsetsUrl().c_str()));

                if (clientJson.empty() || offsetsJson.empty()) {
                    return false;
                }

                if (!clientJson.contains(xorstr_("client.dll"))) return false;
                auto clientDll = clientJson[xorstr_("client.dll")];
                if (!clientDll.contains(xorstr_("classes"))) return false;
                auto clientClasses = clientDll[xorstr_("classes")];

                if (offsetsJson.contains(xorstr_("client.dll")) && offsetsJson[xorstr_("client.dll")].is_object()) {
                    auto& client = offsetsJson[xorstr_("client.dll")];
                    cs2::offsets::client_dll::dwGlobalVars = client[xorstr_("dwGlobalVars")];
                    cs2::offsets::client_dll::dwGameEntitySystem_highestEntityIndex = client[xorstr_("dwGameEntitySystem_highestEntityIndex")];
                    cs2::offsets::client_dll::dwLocalPlayerController = client[xorstr_("dwLocalPlayerController")];
                    cs2::offsets::client_dll::dwLocalPlayerPawn = client[xorstr_("dwLocalPlayerPawn")];
                    cs2::offsets::client_dll::dwPlantedC4 = client[xorstr_("dwPlantedC4")];
                    cs2::offsets::client_dll::dwSensitivity = client[xorstr_("dwSensitivity")];
                    cs2::offsets::client_dll::dwSensitivity_sensitivity = client[xorstr_("dwSensitivity_sensitivity")];
                    cs2::offsets::client_dll::dwViewAngles = client[xorstr_("dwViewAngles")];
                    cs2::offsets::client_dll::dwViewMatrix = client[xorstr_("dwViewMatrix")];
                    cs2::offsets::client_dll::dwEntityList = client[xorstr_("dwEntityList")];
                    cs2::offsets::client_dll::dwGameEntitySystem = client[xorstr_("dwGameEntitySystem")];
                    cs2::offsets::client_dll::dwWeaponC4 = client[xorstr_("dwWeaponC4")];
                }

                if (offsetsJson.contains(xorstr_("engine2.dll")) && offsetsJson[xorstr_("engine2.dll")].is_object()) {
                    auto& engine2 = offsetsJson[xorstr_("engine2.dll")];
                    cs2::offsets::engine2_dll::dwBuildNumber = engine2[xorstr_("dwBuildNumber")];
                    cs2::offsets::engine2_dll::dwWindowHeight = engine2[xorstr_("dwWindowHeight")];
                    cs2::offsets::engine2_dll::dwWindowWidth = engine2[xorstr_("dwWindowWidth")];
                }

                if (offsetsJson.contains(xorstr_("matchmaking.dll")) && offsetsJson[xorstr_("matchmaking.dll")].is_object()) {
                    auto& matchmaking = offsetsJson[xorstr_("matchmaking.dll")];
                    cs2::offsets::matchmaking_dll::dwGameTypes = matchmaking[xorstr_("dwGameTypes")];
                    cs2::offsets::matchmaking_dll::dwGameTypes_mapName = matchmaking[xorstr_("dwGameTypes_mapName")];
                }

                if (clientClasses.contains(xorstr_("C_BaseEntity")) && clientClasses[xorstr_("C_BaseEntity")].is_object()) {
                    auto& C_BaseEntity = clientClasses[xorstr_("C_BaseEntity")];
                    auto& fields = C_BaseEntity[xorstr_("fields")];
                    cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode = fields[xorstr_("m_pGameSceneNode")];
                    cs2::schemas::client_dll::C_BaseEntity::m_iTeamNum = fields[xorstr_("m_iTeamNum")];
                    cs2::schemas::client_dll::C_BaseEntity::m_iHealth = fields[xorstr_("m_iHealth")];
                }

                if (clientClasses.contains(xorstr_("CGameSceneNode")) && clientClasses[xorstr_("CGameSceneNode")].is_object()) {
                    auto& CGameSceneNode = clientClasses[xorstr_("CGameSceneNode")];
                    auto& fields = CGameSceneNode[xorstr_("fields")];
                    cs2::schemas::client_dll::CGameSceneNode::m_vecOrigin = fields[xorstr_("m_vecOrigin")];
                    cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin = fields[xorstr_("m_vecAbsOrigin")];
                    cs2::schemas::client_dll::CGameSceneNode::m_bDormant = fields[xorstr_("m_bDormant")];
                }

                if (clientClasses.contains(xorstr_("C_BasePlayerWeapon")) && clientClasses[xorstr_("C_BasePlayerWeapon")].is_object()) {
                    auto& C_BasePlayerWeapon = clientClasses[xorstr_("C_BasePlayerWeapon")];
                    auto& fields = C_BasePlayerWeapon[xorstr_("fields")];
                    cs2::schemas::client_dll::C_BasePlayerWeapon::m_iClip1 = fields[xorstr_("m_iClip1")];
                }

                if (clientClasses.contains(xorstr_("C_PlantedC4")) && clientClasses[xorstr_("C_PlantedC4")].is_object()) {
                    auto& C_PlantedC4 = clientClasses[xorstr_("C_PlantedC4")];
                    auto& fields = C_PlantedC4[xorstr_("fields")];
                    cs2::schemas::client_dll::C_PlantedC4::m_bC4Activated = fields[xorstr_("m_bC4Activated")];
                    cs2::schemas::client_dll::C_PlantedC4::m_nBombSite = fields[xorstr_("m_nBombSite")];
                    cs2::schemas::client_dll::C_PlantedC4::m_flC4Blow = fields[xorstr_("m_flC4Blow")];
                    cs2::schemas::client_dll::C_PlantedC4::m_bBeingDefused = fields[xorstr_("m_bBeingDefused")];
                    cs2::schemas::client_dll::C_PlantedC4::m_flDefuseLength = fields[xorstr_("m_flDefuseLength")];
                    cs2::schemas::client_dll::C_PlantedC4::m_flDefuseCountDown = fields[xorstr_("m_flDefuseCountDown")];
                    cs2::schemas::client_dll::C_PlantedC4::m_bBombDefused = fields[xorstr_("m_bBombDefused")];
                    cs2::schemas::client_dll::C_PlantedC4::m_bHasExploded = fields[xorstr_("m_bHasExploded")];
                    cs2::schemas::client_dll::C_PlantedC4::m_flTimerLength = fields[xorstr_("m_flTimerLength")];
                }

                if (clientClasses.contains(xorstr_("CCSPlayerController")) && clientClasses[xorstr_("CCSPlayerController")].is_object()) {
                    auto& CCSPlayerController = clientClasses[xorstr_("CCSPlayerController")];
                    auto& fields = CCSPlayerController[xorstr_("fields")];
                    cs2::schemas::client_dll::CCSPlayerController::m_hPlayerPawn = fields[xorstr_("m_hPlayerPawn")];
                    cs2::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName = fields[xorstr_("m_sSanitizedPlayerName")];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive = fields[xorstr_("m_bPawnIsAlive")];
                    cs2::schemas::client_dll::CCSPlayerController::m_iPawnArmor = fields[xorstr_("m_iPawnArmor")];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasDefuser = fields[xorstr_("m_bPawnHasDefuser")];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasHelmet = fields[xorstr_("m_bPawnHasHelmet")];
                }

                if (clientClasses.contains(xorstr_("CBasePlayerController")) && clientClasses[xorstr_("CBasePlayerController")].is_object()) {
                    auto& CBasePlayerController = clientClasses[xorstr_("CBasePlayerController")];
                    auto& fields = CBasePlayerController[xorstr_("fields")];
                    cs2::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController = fields[xorstr_("m_bIsLocalPlayerController")];
                    cs2::schemas::client_dll::CBasePlayerController::m_steamID = fields[xorstr_("m_steamID")];
                }

                if (clientClasses.contains(xorstr_("C_EconEntity")) && clientClasses[xorstr_("C_EconEntity")].is_object()) {
                    auto& C_EconEntity = clientClasses[xorstr_("C_EconEntity")];
                    auto& fields = C_EconEntity[xorstr_("fields")];
                    cs2::schemas::client_dll::C_EconEntity::m_AttributeManager = fields[xorstr_("m_AttributeManager")];
                }

                if (clientClasses.contains(xorstr_("C_AttributeContainer")) && clientClasses[xorstr_("C_AttributeContainer")].is_object()) {
                    auto& C_AttributeContainer = clientClasses[xorstr_("C_AttributeContainer")];
                    auto& fields = C_AttributeContainer[xorstr_("fields")];
                    cs2::schemas::client_dll::C_AttributeContainer::m_Item = fields[xorstr_("m_Item")];
                }

                if (clientClasses.contains(xorstr_("C_EconItemView")) && clientClasses[xorstr_("C_EconItemView")].is_object()) {
                    auto& C_EconItemView = clientClasses[xorstr_("C_EconItemView")];
                    auto& fields = C_EconItemView[xorstr_("fields")];
                    cs2::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex = fields[xorstr_("m_iItemDefinitionIndex")];
                }

                if (clientClasses.contains(xorstr_("CSkeletonInstance")) && clientClasses[xorstr_("CSkeletonInstance")].is_object()) {
                    auto& CSkeletonInstance = clientClasses[xorstr_("CSkeletonInstance")];
                    auto& fields = CSkeletonInstance[xorstr_("fields")];
                    cs2::schemas::client_dll::CSkeletonInstance::m_modelState = fields[xorstr_("m_modelState")];
                }

                if (clientClasses.contains(xorstr_("C_CSPlayerPawn")) && clientClasses[xorstr_("C_CSPlayerPawn")].is_object()) {
                    auto& C_CSPlayerPawn = clientClasses[xorstr_("C_CSPlayerPawn")];
                    auto& fields = C_CSPlayerPawn[xorstr_("fields")];
                    cs2::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired = fields[xorstr_("m_iShotsFired")];
                    cs2::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache = fields[xorstr_("m_aimPunchCache")];
                }

                if (clientClasses.contains(xorstr_("C_CSPlayerPawnBase")) && clientClasses[xorstr_("C_CSPlayerPawnBase")].is_object()) {
                    auto& C_CSPlayerPawnBase = clientClasses[xorstr_("C_CSPlayerPawnBase")];
                    auto& fields = C_CSPlayerPawnBase[xorstr_("fields")];
                    cs2::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon = fields[xorstr_("m_pClippingWeapon")];
                }

                return true;
            }
            catch (const std::exception& e) {
                return false;
            }
        }
    };
}