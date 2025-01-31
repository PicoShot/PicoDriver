#pragma once
#include <algorithm>
#include "../Utils/vars.h"

struct C_UTL_VECTOR
{
    DWORD64 Count = 0;
    DWORD64 Data = 0;
};

structures::Vector2 rcsOldPunch;
structures::Vector2 currentPunchAngle;

void standalone_rcs_mouse(int numShots, structures::Vector2 aimPunch, float sensitivity)
{
    if (numShots > 1)
    {
        const float smoothing = 0.f;

        float x = (aimPunch.x - rcsOldPunch.x) * -1.0f * 0.5f;
        float y = (aimPunch.y - rcsOldPunch.y) * -1.0f * 0.5f;

        x = x * (1.0f - smoothing) + (currentPunchAngle.x * smoothing);
        y = y * (1.0f - smoothing) + (currentPunchAngle.y * smoothing);

        currentPunchAngle = { x, y };

        int mouse_angle_x = (int)(((y * 2.0f) / sensitivity) / -0.022f);
        int mouse_angle_y = (int)(((x * 2.0f) / sensitivity) / 0.022f);

        mouse_angle_x = std::clamp(mouse_angle_x, -75, 75);
        mouse_angle_y = std::clamp(mouse_angle_y, -75, 75);

        mouse_event(MOUSEEVENTF_MOVE, mouse_angle_x, mouse_angle_y, NULL, NULL);
        driver::MoveMouse(mouse_angle_x, mouse_angle_y, 0);
    }
    rcsOldPunch = aimPunch;
}

void RecoilControlSystem()
{
	if (!vars::rcs || !vars::aim) return;
    if (!vars::localPlayer.IsAlive)
        return;

    static float lastUpdateTime = 0.0f;
    const float updateRate = 1.0f / 60.0f;

    float currentTime = GetTickCount() * 0.001f;
    if (currentTime - lastUpdateTime < updateRate)
        return;

    lastUpdateTime = currentTime;

    int numShots = driver::Read<int>(vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);

    if (numShots <= 1)
    {
        rcsOldPunch = { 0, 0 };
        currentPunchAngle = { 0, 0 };
        return;
    }

    C_UTL_VECTOR punchCache = driver::Read<C_UTL_VECTOR>(vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache);

    if (punchCache.Count == 0 || punchCache.Count > 0xFFFF || punchCache.Data == 0)
    {
        currentPunchAngle = { 0, 0 };
        return;
    }

    structures::Vector2 punchAngle = driver::Read<structures::Vector2>(punchCache.Data + (punchCache.Count - 1) * sizeof(structures::Vector3));

    if (std::isfinite(punchAngle.x) && std::isfinite(punchAngle.y))
    {
        float compensationMultiplier = 1.f;

        punchAngle.x *= compensationMultiplier;
        punchAngle.y *= compensationMultiplier;

        standalone_rcs_mouse(numShots, punchAngle, vars::localSensitivity);
    }
}
