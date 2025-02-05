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

namespace Feature
{
    namespace ItemCache {
        static std::unordered_map<uintptr_t, structures::Item> itemCache;
    }

    void ProcessItems(const uintptr_t Entity) {
        lists::Items.clear();
		ItemCache::itemCache.clear();

        const uintptr_t gameEntitySystem = driver::Read<uintptr_t>(vars::clientBase + cs2::offsets::client_dll::dwGameEntitySystem);
        const int highestEntityIndex = driver::Read<int>(gameEntitySystem + cs2::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
      

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
        const uintptr_t Entity = driver::Read<uintptr_t>(vars::clientBase + cs2::offsets::client_dll::dwEntityList);
        const uintptr_t sensitivityPtr = driver::Read<uintptr_t>(vars::clientBase + cs2::offsets::client_dll::dwSensitivity);
      
        vars::viewMatrix = driver::Read<structures::Matrix4x4>(vars::clientBase + cs2::offsets::client_dll::dwViewMatrix);
        vars::localSensitivity = driver::Read<float>(sensitivityPtr + cs2::offsets::client_dll::dwSensitivity_sensitivity);
        vars::localViewAngel = driver::Read<structures::Vector3>(vars::clientBase + cs2::offsets::client_dll::dwViewAngles);
        vars::gameTime = driver::Read<float>(driver::Read<uintptr_t>(vars::clientBase + cs2::offsets::client_dll::dwGlobalVars) + 0x34);

        const uintptr_t dwPlantedC4 = driver::Read<uintptr_t>(driver::Read<uintptr_t>(vars::clientBase + cs2::offsets::client_dll::dwPlantedC4));

        if (dwPlantedC4) {
            const uintptr_t PlantedC4Node = driver::Read<uintptr_t>(dwPlantedC4 + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

            vars::C4 = {
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bC4Activated),
            	false,
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bHasExploded),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bBombDefused),
                driver::Read<structures::Vector3>(PlantedC4Node + cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin),
                driver::Read<bool>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_bBeingDefused),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flC4Blow),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flDefuseCountDown),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flTimerLength),
                driver::Read<float>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_flDefuseLength),
                driver::Read<int>(dwPlantedC4 + cs2::schemas::client_dll::C_PlantedC4::m_nBombSite),
            };
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

            const uintptr_t sceneNode = driver::Read<uintptr_t>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

        	const uintptr_t clippingWeapon = driver::Read<uintptr_t>(entityPawn + cs2::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon);

            const bool isLocalPlayer = driver::Read<bool>(entityController + cs2::schemas::client_dll::CBasePlayerController::m_bIsLocalPlayerController);

            structures::Player player{
                driver::Read<structures::Vector3>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin),
                entityPawn,
                driver::Read<int>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_iHealth),
                driver::Read<int>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_iPawnArmor),
                driver::Read<enums::ItemDefinitionIndex>(clippingWeapon + cs2::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2::schemas::client_dll::C_AttributeContainer::m_Item + cs2::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex),
                driver::Read<int>(entityController + cs2::schemas::client_dll::CBasePlayerController::m_steamID),
                driver::Read<enums::Team>(entityPawn + cs2::schemas::client_dll::C_BaseEntity::m_iTeamNum),
                driver::Read<bool>(sceneNode + cs2::schemas::client_dll::CGameSceneNode::m_bDormant),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasDefuser),
                driver::Read<bool>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_bPawnHasHelmet),
                driver::ReadString(driver::Read<uintptr_t>(entityController + cs2::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName), 128),
            };

            if (player.SteamId == 0)
            {
                player.SteamId = i;
            }

            if (isLocalPlayer) {
                vars::localPlayer = player;
            }
            else {
                lists::Players.add(std::move(player));
            }
        }

        ProcessItems(Entity);

        RunAimbot();
        RenderEsp();
        RecoilControlSystem();
        DrawRCSDot();
    }
}

