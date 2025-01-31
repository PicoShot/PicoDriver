#pragma once
#include "../offsets/offsets.hpp"
#include "../utils/json.hpp"
#include <string>
#include "../utils/xorstr.hpp"
#include <wininet.h>

#include "../offsets/client_dll.hpp"
#pragma comment(lib, "wininet.lib")

#define nameof(Variable) (#Variable)

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
            HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

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
                if (!clientJson.contains("client.dll")) return false;
                auto clientDll = clientJson["client.dll"];
                if (!clientDll.contains("classes")) return false;
                auto clientClasses = clientDll["classes"];

                if (offsetsJson.contains("client.dll") && offsetsJson["client.dll"].is_object()) {
                    auto& client = offsetsJson["client.dll"];
                    cs2::offsets::client_dll::dwGlobalVars = client["dwGlobalVars"];
                    cs2::offsets::client_dll::dwGameEntitySystem_highestEntityIndex = client["dwGameEntitySystem_highestEntityIndex"];
                    cs2::offsets::client_dll::dwLocalPlayerController = client["dwLocalPlayerController"];
                    cs2::offsets::client_dll::dwLocalPlayerPawn = client["dwLocalPlayerPawn"];
                    cs2::offsets::client_dll::dwPlantedC4 = client["dwPlantedC4"];
                    cs2::offsets::client_dll::dwSensitivity = client["dwSensitivity"];
                    cs2::offsets::client_dll::dwSensitivity_sensitivity = client["dwSensitivity_sensitivity"];
                    cs2::offsets::client_dll::dwViewAngles = client["dwViewAngles"];
                    cs2::offsets::client_dll::dwViewMatrix = client["dwViewMatrix"];
                    cs2::offsets::client_dll::dwEntityList = client["dwEntityList"];
                    cs2::offsets::client_dll::dwGameEntitySystem = client["dwGameEntitySystem"];
                    cs2::offsets::client_dll::dwWeaponC4 = client["dwWeaponC4"];

                    printf("\n=== Client DLL Offsets ===\n");
                    printf("dwEntityList: 0x%llX\n", cs2::offsets::client_dll::dwEntityList);
                    printf("dwGameEntitySystem: 0x%llX\n", cs2::offsets::client_dll::dwGameEntitySystem);
                    printf("dwViewMatrix: 0x%llX\n", cs2::offsets::client_dll::dwViewMatrix);
                }

                if (offsetsJson.contains("engine2.dll") && offsetsJson["engine2.dll"].is_object()) {
                    auto& engine2 = offsetsJson["engine2.dll"];
                    cs2::offsets::engine2_dll::dwBuildNumber = engine2["dwBuildNumber"];
                    cs2::offsets::engine2_dll::dwWindowHeight = engine2["dwWindowHeight"];
                    cs2::offsets::engine2_dll::dwWindowWidth = engine2["dwWindowWidth"];
                }

                if (offsetsJson.contains("matchmaking.dll") && offsetsJson["matchmaking.dll"].is_object()) {
                    auto& matchmaking = offsetsJson["matchmaking.dll"];
                    cs2::offsets::matchmaking_dll::dwGameTypes = matchmaking["dwGameTypes"];
                    cs2::offsets::matchmaking_dll::dwGameTypes_mapName = matchmaking["dwGameTypes_mapName"];
                }

                if (clientClasses.contains("C_BaseEntity") && clientClasses["C_BaseEntity"].is_object()) {
                    auto& C_BaseEntity = clientClasses["C_BaseEntity"];
                    auto& fields = C_BaseEntity["fields"];
                    cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode = fields["m_pGameSceneNode"];
                    cs2::schemas::client_dll::C_BaseEntity::m_iTeamNum = fields["m_iTeamNum"];
                    cs2::schemas::client_dll::C_BaseEntity::m_iHealth = fields["m_iHealth"];
                }

                if (clientClasses.contains("CGameSceneNode") && clientClasses["CGameSceneNode"].is_object()) {
                    auto& CGameSceneNode = clientClasses["CGameSceneNode"];
                    auto& fields = CGameSceneNode["fields"];
                    cs2::schemas::client_dll::CGameSceneNode::m_vecOrigin = fields["m_vecOrigin"];
                    cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin = fields["m_vecAbsOrigin"];
                    cs2::schemas::client_dll::CGameSceneNode::m_bDormant = fields["m_bDormant"];

                    printf("\n=== Schema Offsets ===\n");
                    printf("m_pGameSceneNode: 0x%llX\n", cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
                    printf("m_vecAbsOrigin: 0x%llX\n", cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
                    printf("m_bDormant: 0x%llX\n", cs2::schemas::client_dll::CGameSceneNode::m_bDormant);
                }

                if (clientClasses.contains("C_BasePlayerWeapon") && clientClasses["C_BasePlayerWeapon"].is_object()) {
                    auto& C_BasePlayerWeapon = clientClasses["C_BasePlayerWeapon"];
                    auto& fields = C_BasePlayerWeapon["fields"];
                    cs2::schemas::client_dll::C_BasePlayerWeapon::m_iClip1 = fields["m_iClip1"];
                }

                if (clientClasses.contains("C_PlantedC4") && clientClasses["C_PlantedC4"].is_object()) {
                    auto& C_PlantedC4 = clientClasses["C_PlantedC4"];
                    auto& fields = C_PlantedC4["fields"];
                    cs2::schemas::client_dll::C_PlantedC4::m_bC4Activated = fields["m_bC4Activated"];
                    cs2::schemas::client_dll::C_PlantedC4::m_nBombSite = fields["m_nBombSite"];
                    cs2::schemas::client_dll::C_PlantedC4::m_flC4Blow = fields["m_flC4Blow"];
                    cs2::schemas::client_dll::C_PlantedC4::m_bBeingDefused = fields["m_bBeingDefused"];
                    cs2::schemas::client_dll::C_PlantedC4::m_flDefuseLength = fields["m_flDefuseLength"];
                    cs2::schemas::client_dll::C_PlantedC4::m_flDefuseCountDown = fields["m_flDefuseCountDown"];
                    cs2::schemas::client_dll::C_PlantedC4::m_bBombDefused = fields["m_bBombDefused"];
                    cs2::schemas::client_dll::C_PlantedC4::m_bHasExploded = fields["m_bHasExploded"];
                    cs2::schemas::client_dll::C_PlantedC4::m_flTimerLength = fields["m_flTimerLength"];
                }

                if (clientClasses.contains("CCSPlayerController") && clientClasses["CCSPlayerController"].is_object()) {
                    auto& CCSPlayerController = clientClasses["CCSPlayerController"];
                    auto& fields = CCSPlayerController["fields"];
                    cs2::schemas::client_dll::CCSPlayerController::m_hPlayerPawn = fields["m_hPlayerPawn"];
                    cs2::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName = fields["m_sSanitizedPlayerName"];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive = fields["m_bPawnIsAlive"];
                    cs2::schemas::client_dll::CCSPlayerController::m_iPawnArmor = fields["m_iPawnArmor"];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasDefuser = fields["m_bPawnHasDefuser"];
                    cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasHelmet = fields["m_bPawnHasHelmet"];
                }

                if (clientJson.contains("CBasePlayerController") && clientJson["CBasePlayerController"].is_object()) {
                    auto& CBasePlayerController = clientJson["CBasePlayerController"];
                    auto& fields = CBasePlayerController["fields"];
                    cs2::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController = fields["m_bIsLocalPlayerController"];
                }

                if (clientClasses.contains("C_EconEntity") && clientClasses["C_EconEntity"].is_object()) {
                    auto& C_EconEntity = clientClasses["C_EconEntity"];
                    auto& fields = C_EconEntity["fields"];
                    cs2::schemas::client_dll::C_EconEntity::m_AttributeManager = fields["m_AttributeManager"];
                }

                if (clientClasses.contains("C_AttributeContainer") && clientClasses["C_AttributeContainer"].is_object()) {
                    auto& C_AttributeContainer = clientClasses["C_AttributeContainer"];
                    auto& fields = C_AttributeContainer["fields"];
                    cs2::schemas::client_dll::C_AttributeContainer::m_Item = fields["m_Item"];
                }

                if (clientClasses.contains("C_EconItemView") && clientClasses["C_EconItemView"].is_object()) {
                    auto& C_EconItemView = clientClasses["C_EconItemView"];
                    auto& fields = C_EconItemView["fields"];
                	cs2::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex = fields["m_iItemDefinitionIndex"];
                }

                if (clientClasses.contains("CSkeletonInstance") && clientClasses["CSkeletonInstance"].is_object()) {
                    auto& CSkeletonInstance = clientClasses["CSkeletonInstance"];
                    auto& fields = CSkeletonInstance["fields"];
                    cs2::schemas::client_dll::CSkeletonInstance::m_modelState = fields["m_modelState"];
                }

                if (clientClasses.contains("C_CSPlayerPawn") && clientClasses["C_CSPlayerPawn"].is_object()) {
                    auto& C_CSPlayerPawn = clientClasses["C_CSPlayerPawn"];
                    auto& fields = C_CSPlayerPawn["fields"];
                    cs2::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired = fields["m_iShotsFired"];
                    cs2::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache = fields["m_aimPunchCache"];
                }

                if (clientClasses.contains("C_CSPlayerPawnBase") && clientClasses["C_CSPlayerPawnBase"].is_object()) {
                    auto& C_CSPlayerPawnBase = clientClasses["C_CSPlayerPawnBase"];
                    auto& fields = C_CSPlayerPawnBase["fields"];
                    cs2::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon = fields["m_pClippingWeapon"];
                }

                return true;
            }
            catch (const std::exception& e) {
                return false;
            }
        }
    };
}