#pragma once
#include <cstddef>

namespace cs2 {

    namespace offsets {

        namespace client_dll {
            inline std::ptrdiff_t dwEntityList;
            inline std::ptrdiff_t dwGameEntitySystem;
            inline std::ptrdiff_t dwGameEntitySystem_highestEntityIndex;
            inline std::ptrdiff_t dwGlobalVars;
            inline std::ptrdiff_t dwLocalPlayerController = 0x1A79C58;
            inline std::ptrdiff_t dwLocalPlayerPawn = 0x187CEF0;
            inline std::ptrdiff_t dwPlantedC4 = 0x1A986D0;
            inline std::ptrdiff_t dwSensitivity = 0x1A908F8;
            inline std::ptrdiff_t dwSensitivity_sensitivity = 0x40;
            inline std::ptrdiff_t dwViewAngles;
            inline std::ptrdiff_t dwViewMatrix;
            inline std::ptrdiff_t dwWeaponC4 = 0x1A2C420;
        }

        namespace engine2_dll {
            inline std::ptrdiff_t dwBuildNumber = 0x540BE4;
            inline std::ptrdiff_t dwWindowHeight = 0x62354C;
            inline std::ptrdiff_t dwWindowWidth = 0x623548;
        }

        namespace matchmaking_dll {
            inline std::ptrdiff_t dwGameTypes = 0x1A41B0;
            inline std::ptrdiff_t dwGameTypes_mapName = 0x120;
        }

    }

}
