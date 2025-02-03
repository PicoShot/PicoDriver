#pragma once
#include <cmath>
#include <map>
#include "esp.h"
#include "../config/vars.h"
#include "../offsets/client_dll.hpp"
#include "../Utils/driver.h"

namespace aim_assist {
    enum AimPosition {
        Head = 0,
        Neck = Head + 1,
        Chest = Neck + 1,
        Stomach = Chest + 1,
        Pelvis = Stomach + 1
    };

    const std::map<AimPosition, std::string> boneNames = {
        {Head, "head"},
        {Neck, "neck_0"},
        {Chest, "spine_1"},
        {Stomach, "spine_2"},
        {Pelvis, "pelvis"}
    };

    struct TargetWeights {
        float distanceWeight = 5.0f;
        float crosshairWeight = 4.0f;
        float healthWeight = 0.3f;
    };

    struct AimTarget {
        structures::Vector3 position;
        float totalScore;
        float distance;
        float fov;
        int playerIndex;
        bool isValid;
        int health;
    };

    static TargetWeights weights;

    bool shouldAim() {
        bool shouldAim;
	    switch (vars::aimbotMode)
	    {
        case 0:
            shouldAim = true;
            break;
        case 1:
            shouldAim = GetAsyncKeyState(VK_XBUTTON2) & 0x8000;
            break;
        case 2:
            shouldAim = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
            break;
	    }
        return shouldAim;
    }

    void DrawFOVCircle() {
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        ImVec2 screenCenter((io.DisplaySize.x + 2) * 0.5f, io.DisplaySize.y * 0.5f);
        float radius = tanf(vars::aimFov * (M_PI / 180.0f)) * (io.DisplaySize.y * 0.5f);

        drawList->AddCircle(
            screenCenter,
            radius,
            ImColor(255, 0, 0, 255),
            100,
            1.0f
        );
    }

    structures::Vector3 GetAimPosition(const structures::Player& player) {
        auto sceneNode = driver::Read<uintptr_t>(player.EntityPawn + cs2::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
        auto bonePositions = esp::GetBonePositions(sceneNode);

        structures::Vector3 targetPos = bonePositions[boneNames.at(static_cast<AimPosition>(vars::aimbotTarget))];

        structures::Vector3 velocity = driver::Read<structures::Vector3>(player.EntityPawn + cs2::schemas::client_dll::C_BaseEntity::m_vecVelocity);
        float predictionTime = 0.07f;
        targetPos += velocity * predictionTime;

        return targetPos;
    }

    structures::Vector3 CalculateAngle(const structures::Vector3& source, const structures::Vector3& destination) {
        structures::Vector3 delta = destination - source;
        float hypotenuse = delta.Length();

        structures::Vector3 angles;
        angles.x = -asinf(delta.z / hypotenuse) * (180.0f / M_PI);
        angles.y = atan2f(delta.y, delta.x) * (180.0f / M_PI);
        angles.z = 0.0f;

        return angles;
    }

    structures::Vector3 NormalizeAngles(structures::Vector3 angles) {
        while (angles.y > 180.0f) angles.y -= 360.0f;
        while (angles.y < -180.0f) angles.y += 360.0f;
        angles.x = std::clamp(angles.x, -89.0f, 89.0f);
    	return angles;
    }

    float CalculateFOV(const structures::Vector3& viewAngles, const structures::Vector3& aimAngles) {
        structures::Vector3 delta = aimAngles - viewAngles;
        delta = NormalizeAngles(delta);
        return sqrt(pow(delta.x, 2) + pow(delta.y, 2));
    }

    float NormalizeValue(float value, float min, float max) {
        return (value - min) / (max - min);
    }

    AimTarget GetBestTarget() {
        AimTarget bestTarget = { {0,0,0}, 0, FLT_MAX, FLT_MAX, -1, false, 0 };
        float maxDistance = 0;
        float maxHealth = 0;

        if (!vars::localPlayer.IsAlive)
            return bestTarget;

        structures::Vector3 localViewPos = vars::localPlayer.Position;
        structures::Vector3 localViewOffset = driver::Read<structures::Vector3>(
            vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);
        localViewPos += localViewOffset;

        for (int i = 0; i < lists::Players.size(); i++) {
            const auto& player = lists::Players[i];
            if (!player.IsAlive || player.Dormant || player.Team == vars::localPlayer.Team)
                continue;

            float distance = (player.Position - localViewPos).Length();
            maxDistance = std::max(maxDistance, distance);
            maxHealth = std::max(maxHealth, (float)player.Health);
        }

        for (int i = 0; i < lists::Players.size(); i++) {
            const auto& player = lists::Players[i];
            if (!player.IsAlive || player.Dormant || player.Team == vars::localPlayer.Team)
                continue;

            auto targetPos = GetAimPosition(player);
            structures::Vector3 aimAngles = CalculateAngle(localViewPos, targetPos);
            float fov = CalculateFOV(vars::localViewAngel, aimAngles);

            if (fov > vars::aimFov)
                continue;

            float distance = (player.Position - localViewPos).Length();
            float normalizedDistance = NormalizeValue(distance, 0, maxDistance);
            float distanceScore = pow(1 - normalizedDistance, 2) * weights.distanceWeight;

            float normalizedFov = NormalizeValue(fov, 0, vars::aimFov);
            float fovScore = pow(1 - normalizedFov, 2) * weights.crosshairWeight;

            float normalizedHealth = NormalizeValue(player.Health, 0, maxHealth);
            float healthScore = (1 - normalizedHealth) * weights.healthWeight;

            float totalScore = distanceScore + fovScore + healthScore;

            if (totalScore > bestTarget.totalScore) {
                bestTarget.position = targetPos;
                bestTarget.totalScore = totalScore;
                bestTarget.distance = distance;
                bestTarget.fov = fov;
                bestTarget.playerIndex = i;
                bestTarget.isValid = true;
                bestTarget.health = player.Health;
            }
        }

        return bestTarget;
    }

    void AimAtTarget(const AimTarget& target) {
        if (!target.isValid)
            return;

        structures::Vector3 localViewPos = vars::localPlayer.Position + driver::Read<structures::Vector3>(
            vars::localPlayer.EntityPawn + cs2::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);

        structures::Vector3 aimAngles = CalculateAngle(localViewPos, target.position);
        structures::Vector3 delta = NormalizeAngles(aimAngles - vars::localViewAngel);

        float distanceFromCenter = delta.Length();
        float smoothFactor = vars::aimSmooth * (1.0f + pow(distanceFromCenter / 10.0f, 2));

        float deadzone = 0.1f;
        if (distanceFromCenter < deadzone)
            return;

        delta.x /= smoothFactor;
        delta.y /= smoothFactor;

        int mouseX = static_cast<int>((delta.y / vars::localSensitivity) / 0.022f);
        int mouseY = static_cast<int>(-((delta.x / vars::localSensitivity) / 0.022f));

        mouseX = std::clamp(mouseX, -25, 25);
        mouseY = std::clamp(mouseY, -25, 25);

        if (abs(mouseX) < 1 && abs(mouseY) < 1)
            return;

        driver::MoveMouse(-mouseX, -mouseY, MOUSE_MOVE_RELATIVE);
    }

    void RunAimbot()
	{

        if (!vars::aim || !vars::aimbot)
            return;

        if (vars::showAimbotFov || vars::aimbot || vars::aim) {
            DrawFOVCircle();
        }

        if (!shouldAim())
            return;

        AimTarget bestTarget = GetBestTarget();
        AimAtTarget(bestTarget);
    }
}