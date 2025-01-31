#pragma once
#include <cstddef>

namespace cs2 {

    namespace schemas {

        namespace client_dll
        {
            namespace C_BaseEntity {
                inline std::ptrdiff_t m_pGameSceneNode;
                inline std::ptrdiff_t m_iTeamNum;
                inline std::ptrdiff_t m_iHealth;
            	;
            }

            namespace CGameSceneNode {
                inline std::ptrdiff_t m_vecOrigin;
                inline std::ptrdiff_t m_vecAbsOrigin;
                inline std::ptrdiff_t m_bDormant;
            }

            namespace C_BasePlayerWeapon {
                inline std::ptrdiff_t m_iClip1;
            }

            namespace C_PlantedC4 {
                inline std::ptrdiff_t m_bC4Activated;
                inline std::ptrdiff_t m_flC4Blow;
                inline std::ptrdiff_t m_nBombSite;
                inline std::ptrdiff_t m_bBeingDefused;
                inline std::ptrdiff_t m_flDefuseLength;
                inline std::ptrdiff_t m_flDefuseCountDown;
                inline std::ptrdiff_t m_bBombDefused;
                inline std::ptrdiff_t m_bHasExploded;
                inline std::ptrdiff_t m_flTimerLength;
            }

            namespace CCSPlayerController {
                inline std::ptrdiff_t m_hPlayerPawn;
                inline std::ptrdiff_t m_sSanitizedPlayerName;
                inline std::ptrdiff_t m_bPawnIsAlive;
                inline std::ptrdiff_t m_iPawnArmor;
                inline std::ptrdiff_t m_bPawnHasDefuser;
                inline std::ptrdiff_t m_bPawnHasHelmet;
            }

            namespace CBasePlayerController {
                inline std::ptrdiff_t m_bIsLocalPlayerController;
                inline std::ptrdiff_t m_steamID;
            }

            namespace C_EconEntity {
                inline std::ptrdiff_t m_AttributeManager;
            }

            namespace C_AttributeContainer {
                inline std::ptrdiff_t m_Item;
            }

            namespace C_EconItemView {
                inline std::ptrdiff_t m_iItemDefinitionIndex;
            }

            namespace CSkeletonInstance {
                inline std::ptrdiff_t m_modelState;
            }

            namespace C_CSPlayerPawn {
                inline std::ptrdiff_t m_iShotsFired;
                inline std::ptrdiff_t m_aimPunchCache;
            }

            namespace C_CSPlayerPawnBase {
                inline std::ptrdiff_t m_pClippingWeapon;
            }
        }
    }
    }
