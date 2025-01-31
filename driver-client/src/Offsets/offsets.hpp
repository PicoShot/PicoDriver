#pragma once
#include <cstddef>

namespace cs2 {

    namespace offsets {

        namespace client_dll {
            inline std::ptrdiff_t dwEntityList;
            inline std::ptrdiff_t dwGameEntitySystem;
            inline std::ptrdiff_t dwGameEntitySystem_highestEntityIndex;
            inline std::ptrdiff_t dwGlobalVars;
            inline std::ptrdiff_t dwLocalPlayerController;
            inline std::ptrdiff_t dwLocalPlayerPawn;
            inline std::ptrdiff_t dwPlantedC4;
            inline std::ptrdiff_t dwSensitivity;
            inline std::ptrdiff_t dwSensitivity_sensitivity;
            inline std::ptrdiff_t dwViewAngles;
            inline std::ptrdiff_t dwViewMatrix;
            inline std::ptrdiff_t dwWeaponC4;
        }

        namespace engine2_dll {
            inline std::ptrdiff_t dwBuildNumber;
            inline std::ptrdiff_t dwWindowHeight;
            inline std::ptrdiff_t dwWindowWidth;
        }

        namespace matchmaking_dll {
            inline std::ptrdiff_t dwGameTypes;
            inline std::ptrdiff_t dwGameTypes_mapName;
        }

    }

}
