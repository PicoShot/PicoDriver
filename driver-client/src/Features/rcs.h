#pragma once
#include <algorithm>
#include "../config/vars.h"

namespace Feature
{
    struct C_UTL_VECTOR
    {
        DWORD64 Count = 0;
        DWORD64 Data = 0;
    };

    static ImVec2 rcsDotPosition = { 0.0f, 0.0f };

    DirectX::XMFLOAT2 rcsOldPunch;
    DirectX::XMFLOAT2 currentPunchAngle;

    void standalone_rcs_mouse(int numShots, DirectX::XMFLOAT2 aimPunch, float sensitivity)
    {
        if (numShots > 1)
        {
            const float smoothing = 0.f;

            float x = (aimPunch.x - rcsOldPunch.x) * -1.0f * 0.5f;
            float y = (aimPunch.y - rcsOldPunch.y) * -1.0f * 0.5f;

            x = x * (1.0f - smoothing) + (currentPunchAngle.x * smoothing);
            y = y * (1.0f - smoothing) + (currentPunchAngle.y * smoothing);

            currentPunchAngle = { x, y };

            int mouse_angle_x = static_cast<int>(y * 2.0f) / sensitivity / -0.022f;
            int mouse_angle_y = static_cast<int>(x * 2.0f) / sensitivity / 0.022f;

            mouse_angle_x = std::clamp(mouse_angle_x, -75, 75);
            mouse_angle_y = std::clamp(mouse_angle_y, -75, 75);

            mouse_event(MOUSEEVENTF_MOVE, mouse_angle_x, mouse_angle_y, NULL, NULL);
            driver::MoveMouse(mouse_angle_x, mouse_angle_y, 0);
        }
        rcsOldPunch = aimPunch;
    }

    void RecoilControlSystem()
    {
        if (!vars::localPlayer.IsAlive)
        {
            vars::showRcsDot = false;
            return;
        }

        int numShots = driver::Read<int>(vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);

        if (numShots <= 1)
        {
            rcsOldPunch = { 0, 0 };
            currentPunchAngle = { 0, 0 };
            vars::showRcsDot = false;
            return;
        }

        C_UTL_VECTOR punchCache = driver::Read<C_UTL_VECTOR>(vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache);

        if (punchCache.Count == 0 || punchCache.Count > 0xFFFF || punchCache.Data == 0)
        {
            currentPunchAngle = { 0, 0 };
            vars::showRcsDot = false;
            return;
        }

        DirectX::XMFLOAT2 punchAngle = driver::Read<DirectX::XMFLOAT2>(punchCache.Data + (punchCache.Count - 1) * sizeof(DirectX::XMFLOAT3));

        if (std::isfinite(punchAngle.x) && std::isfinite(punchAngle.y))
        {
            float compensationMultiplier = 1.f;
            punchAngle.x *= compensationMultiplier;
            punchAngle.y *= compensationMultiplier;

            float mouseX = (punchAngle.y * 2.0f / vars::localSensitivity) / -0.022f;
            float mouseY = (punchAngle.x * 2.0f / vars::localSensitivity) / 0.022f;

            ImVec2 screenSize = ImGui::GetIO().DisplaySize;
            float screenCenterX = screenSize.x * 0.5f;
            float screenCenterY = screenSize.y * 0.5f;

            rcsDotPosition = ImVec2(
                screenCenterX + mouseX * 0.15,
                screenCenterY + mouseY * 0.15
            );

            vars::showRcsDot = true;
            if (vars::rcs && vars::aim)
            {
                standalone_rcs_mouse(numShots, punchAngle, vars::localSensitivity);
            }

        }
        else
        {
            vars::showRcsDot = false;
        }
    }

    void DrawRCSDot()
    {
        if (vars::showRcsDot)
        {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            drawList->AddCircleFilled(rcsDotPosition, 3.0f, IM_COL32(255, 0, 0, 255));
        }
    }
}