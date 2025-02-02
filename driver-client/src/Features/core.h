#pragma once
#include <iostream>
#include <unordered_map>

#include "../offsets/client_dll.hpp"
#include "../offsets/offsets.hpp"
#include "../Utils/driver.h"
#include "../config/vars.h"
#include "esp.h"
#include "rcs.h"
#include "aim.h"

namespace PicoDriver
{
    namespace ItemCache {
        static std::unordered_map<uintptr_t, structures::Item> itemCache;
    }

    void ProcessItems(const uintptr_t Entity, int highestEntityIndex) {
        lists::Items.clear();
		ItemCache::itemCache.clear();
      

        for (int i = 64; i < highestEntityIndex; i++) {
            const uintptr_t listEntity = driver::Read<uintptr_t>(Entity + 8LL * ((i & 0x7FFF) >> 9) + 16);
            if (!listEntity) continue;

            const uintptr_t entityController = driver::Read<uintptr_t>(listEntity + 120LL * (i & 0x1FF));
            if (!entityController) continue;

            auto cachedItem = ItemCache::itemCache.find(entityController);
            if (cachedItem != ItemCache::itemCache.end()) {
                structures::Item& item = cachedItem->second;
                const uintptr_t sceneNode = driver::Read<uintptr_t>(entityController + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
                if (!sceneNode) continue;

                item.Position = driver::Read<structures::Vector3>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_vecOrigin);
                if (vars::localPlayer.IsAlive) {
                    item.Distance = (item.Position - vars::localPlayer.Position).Length() / 100.0f;
                }
                lists::Items.add(item);
                continue;
            }

            const uintptr_t sceneNode = driver::Read<uintptr_t>(entityController + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!sceneNode) continue;

            structures::Vector3 entityPosition = driver::Read<structures::Vector3>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_vecOrigin);
            if (entityPosition.x == 0 && entityPosition.y == 0 && entityPosition.z == 0) continue;

            const uintptr_t itemInfo = driver::Read<uintptr_t>(entityController + 0x10);
            if (!itemInfo) continue;

            const uintptr_t itemTypePtr = driver::Read<uintptr_t>(itemInfo + 0x20);
            if (!itemTypePtr) continue;

            const std::string entityName = driver::ReadString(itemTypePtr);
            structures::Item item;
            bool itemFound = false;

            if (entityName.find(xorstr_("weapon_")) != std::string::npos) {
                const auto weaponIndex = driver::Read<enums::ItemDefinitionIndex>(
                    entityController + cs2::schemas::client_dll::C_EconEntity::m_AttributeManager +
                    cs2::schemas::client_dll::C_AttributeContainer::m_Item +
                    cs2::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);

                const auto nameIt = enums::ItemsNames.find(weaponIndex);
                if (nameIt != enums::ItemsNames.end()) {
                    item.Name = std::string(1, nameIt->second);
                    item.MaxClipAmmo = enums::MaxClipAmmo[weaponIndex];
                    item.Clip = driver::Read<int>(entityController + cs2::schemas::client_dll::C_BasePlayerWeapon::m_iClip1);
                    item.IsProjectile = false;
                    itemFound = true;
                }
            }
            else if (entityName.find(xorstr_("projectile")) != std::string::npos) {
                const auto it = enums::ItemsProjectile.find(entityName);
                if (it != enums::ItemsProjectile.end()) {
                    item.Name = it->second;
                    item.IsProjectile = true;
                    itemFound = true;
                }
            }
            else if (entityName.find(xorstr_("baseanimgraph")) != std::string::npos) {
                const auto it = enums::ItemsNames.find(enums::ItemDefinitionIndex::ItemDefuser);
                if (it != enums::ItemsNames.end()) {
                    item.Name = it->second;
                    item.IsProjectile = false;
                    itemFound = true;
                }
            }

            if (itemFound) {
                item.Position = entityPosition;
                item.Distance = vars::localPlayer.IsAlive ?
                    (item.Position - vars::localPlayer.Position).Length() / 100.0f : 0.0f;
                item.IsValid = true;

                ItemCache::itemCache[entityController] = item;
                lists::Items.add(std::move(item));
            }
        }
    }
    void Main()
    {

        const auto clientBase = vars::clientBase;
        const auto Entity = driver::Read<uintptr_t>(clientBase + cs2::offsets::client_dll::dwEntityList);
        const auto gameEntitySystem = driver::Read<uintptr_t>(clientBase + cs2::offsets::client_dll::dwGameEntitySystem);

        struct GlobalData {
            structures::Matrix4x4 viewMatrix;
            float localSensitivity;
            structures::Vector3 viewAngles;
            int highestEntityIndex;
           float gameTime;
        } globalData;

        const auto sensitivityPtr = driver::Read<uintptr_t>(clientBase + cs2::offsets::client_dll::dwSensitivity);
        globalData = {
        driver::Read<structures::Matrix4x4>(clientBase + cs2::offsets::client_dll::dwViewMatrix),
        driver::Read<float>(sensitivityPtr + cs2::offsets::client_dll::dwSensitivity_sensitivity),
        driver::Read<structures::Vector3>(clientBase + cs2::offsets::client_dll::dwViewAngles),
        driver::Read<int>(gameEntitySystem + cs2::offsets::client_dll::dwGameEntitySystem_highestEntityIndex),
        driver::Read<float>(driver::Read<uintptr_t>(clientBase + cs2::offsets::client_dll::dwGlobalVars) + 0x34)
        };

        vars::viewMatrix = globalData.viewMatrix;
        vars::localSensitivity = globalData.localSensitivity;
        vars::localViewAngel = globalData.viewAngles;
        vars::gameTime = globalData.gameTime;

        const auto dwPlantedC4 = driver::Read<uintptr_t>(driver::Read<uintptr_t>(clientBase + cs2::offsets::client_dll::dwPlantedC4));

        if (dwPlantedC4) {
            const auto PlantedC4Node = driver::Read<uintptr_t>(dwPlantedC4 + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

            struct C4Data {
                structures::Vector3 position;
                bool activated;
                int site;
                float timeToExplode;
                bool beingDefused;
                float maxTimeToDefuse;
                float timeToDefuse;
                bool defused;
                bool exploded;
                float maxTimeToExplode;
            } c4Data;

            c4Data = {
                driver::Read<structures::Vector3>(PlantedC4Node + cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bC4Activated),
                driver::Read<int>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_nBombSite),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flC4Blow),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bBeingDefused),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flDefuseLength),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flDefuseCountDown),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bBombDefused),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bHasExploded),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flTimerLength)
            };
            vars::C4 = { c4Data.activated, false, c4Data.exploded, c4Data.defused, c4Data.position,
                    c4Data.beingDefused, c4Data.timeToExplode, c4Data.timeToDefuse,
                    c4Data.maxTimeToExplode, c4Data.maxTimeToDefuse, c4Data.site };
        }

        lists::Players.clear();

        structures::Player player;


       

        for (int i = 0; i < 64; i++) {
            uintptr_t listEntity = driver::Read<uintptr_t>(Entity + ((8 * (i & 0x7FFF) >> 9) + 16));
            if (listEntity == 0)
                continue;

            uintptr_t entityController = driver::Read<uintptr_t>(listEntity + (120) * (i & 0x1FF));
            if (entityController == 0)
                continue;

            uintptr_t entityControllerPawn = driver::Read<uintptr_t>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
            if (entityControllerPawn == 0)
                continue;

            listEntity = driver::Read<uintptr_t>(Entity + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16));
            if (listEntity == 0)
                continue;

            uintptr_t entityPawn = driver::Read<uintptr_t>(listEntity + (120) * (entityControllerPawn & 0x1FF));
            if (entityPawn == 0)
                continue;



            struct PlayerData {
                uintptr_t pawn;
                uintptr_t sceneNode;
                std::string name;
                enums::Team team;
                int health;
                structures::Vector3 position;
                bool dormant;
                bool isAlive;
                int armor;
                bool hasDefuser;
                bool hasHelmet;
                bool isLocalPlayer;
                int steamId;
                enums::ItemDefinitionIndex HoldingWeapon;
            } playerData;


            const auto sceneNode = driver::Read<uintptr_t>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

        	const auto clippingWeapon = driver::Read<uintptr_t>(entityPawn + cs2::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon);

            playerData = {
                entityPawn,
                sceneNode,
                driver::ReadString(driver::Read<uintptr_t>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName), 128),
                driver::Read<enums::Team>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_iTeamNum),
                driver::Read<int>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_iHealth),
                driver::Read<structures::Vector3>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin),
                driver::Read<bool>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_bDormant),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive),
                driver::Read<int>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_iPawnArmor),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasDefuser),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasHelmet),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController),
                driver::Read<int>(entityController + cs2::schemas::client_dll::CBasePlayerController::m_steamID),
                driver::Read<enums::ItemDefinitionIndex>(clippingWeapon + cs2::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2::schemas::client_dll::C_AttributeContainer::m_Item + cs2::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex)
            };

            if (playerData.steamId == 0)
            {
                playerData.steamId = i;
            }

            structures::Player player{
                playerData.position,
                playerData.pawn,
                playerData.health,
                playerData.armor,
                playerData.HoldingWeapon,
                playerData.steamId,
                playerData.team,
                playerData.dormant,
                playerData.isAlive,
                playerData.hasDefuser,
                playerData.hasHelmet, 
                playerData.name
            };

            if (playerData.isLocalPlayer) {
                vars::localPlayer = player;
            }
            else {
                lists::Players.add(std::move(player));
            }
        }

        lists::Items.clear();

        ProcessItems(Entity, globalData.highestEntityIndex);

        aim_assist::RunAimbot();
        esp::RenderEsp();
        RecoilControlSystem();
    }
}

