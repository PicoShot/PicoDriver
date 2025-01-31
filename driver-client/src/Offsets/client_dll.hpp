#pragma once
#include <cstddef>

namespace cs2 {

    namespace schemas {

        namespace client_dll
        {
            namespace C_BaseEntity {
                inline std::ptrdiff_t m_pGameSceneNode = 0x328;
                inline std::ptrdiff_t m_iTeamNum = 0x3E3;
                inline std::ptrdiff_t m_iHealth = 0x344
            	;
            }



            namespace CGameSceneNode {
                inline std::ptrdiff_t m_vecOrigin = 0x88;
                inline std::ptrdiff_t m_vecAbsOrigin = 0xD0;
                inline std::ptrdiff_t m_bDormant = 0xEF;
            }



            namespace C_BasePlayerWeapon {
                inline std::ptrdiff_t m_iClip1 = 0x1678;
            }


            namespace C_PlantedC4 {
                inline std::ptrdiff_t m_bC4Activated = 0xFD8;
                inline std::ptrdiff_t m_flC4Blow = 0xFC0;
                inline std::ptrdiff_t m_nBombSite = 0xF94;
                inline std::ptrdiff_t m_bBeingDefused = 0xFCC;
                inline std::ptrdiff_t m_flDefuseLength = 0xFDC;
                inline std::ptrdiff_t m_flDefuseCountDown;
                inline std::ptrdiff_t m_bBombDefused = 0xFE0;
                inline std::ptrdiff_t m_bHasExploded = 0xFC5;
                inline std::ptrdiff_t m_flTimerLength = 0xFC8;
            }



            namespace CCSPlayerController {
                inline std::ptrdiff_t m_hPlayerPawn = 0x80C;
                inline std::ptrdiff_t m_sSanitizedPlayerName = 0x770;
                inline std::ptrdiff_t m_bPawnIsAlive = 0x814;
                inline std::ptrdiff_t m_iPawnArmor = 0x81C;
                inline std::ptrdiff_t m_bPawnHasDefuser = 0x820;
                inline std::ptrdiff_t m_bPawnHasHelmet = 0x821;
            }



            namespace CBasePlayerController {
                inline std::ptrdiff_t m_bIsLocalPlayerController = 0x6F0;
            }



            namespace C_EconEntity {
                inline std::ptrdiff_t m_AttributeManager = 0x1148;
            }


            namespace C_AttributeContainer {
                inline std::ptrdiff_t m_Item = 0x50;
            }


            namespace C_EconItemView {
                inline std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
            }



            namespace CSkeletonInstance {
                inline std::ptrdiff_t m_modelState = 0x170;
            }

            namespace C_CSPlayerPawn {
                inline std::ptrdiff_t m_iShotsFired = 0x23FC;
                inline std::ptrdiff_t m_aimPunchCache = 0x15A8;
            }

            namespace C_CSPlayerPawnBase {
                inline std::ptrdiff_t m_pClippingWeapon = 0x13A0;
            }
        }
    }
    }
