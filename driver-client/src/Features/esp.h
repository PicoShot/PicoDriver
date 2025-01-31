#pragma once
#include <algorithm>

#include "../imgui/imgui.h"
#include "../Utils/vars.h"
#include "../overlay/overlay.h"
#undef max

struct ScreenBounds {
    ImVec2 min;
    ImVec2 max;
    ImVec2 center;
    bool isBehind;
};

ScreenBounds CalculateScreenSpaceBounds(const structures::Vector3& basePos) {
    std::vector<structures::Vector3> bounds = {
        {basePos.x - 16, basePos.y - 16, basePos.z},
        {basePos.x + 16, basePos.y - 16, basePos.z},
        {basePos.x + 16, basePos.y + 16, basePos.z},
        {basePos.x - 16, basePos.y + 16, basePos.z},
        {basePos.x - 16, basePos.y - 16, basePos.z + 72},
        {basePos.x + 16, basePos.y - 16, basePos.z + 72},
        {basePos.x + 16, basePos.y + 16, basePos.z + 72},
        {basePos.x - 16, basePos.y + 16, basePos.z + 72}
    };

    std::vector<structures::Vector3> screenPoints;
    screenPoints.reserve(bounds.size());
    bool anyInFront = false;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    for (const auto& point : bounds) {
        structures::Vector3 screen = overlay::WorldToScreen(point);
        screenPoints.push_back(screen);

        if (screen.z >= 0.001f &&
            screen.x >= 0 && screen.x <= displaySize.x &&
            screen.y >= 0 && screen.y <= displaySize.y) {
            anyInFront = true;
        }
    }

    bool isBehind = !anyInFront;

    ImVec2 min = ImVec2(FLT_MAX, FLT_MAX);
    ImVec2 max = ImVec2(-FLT_MAX, -FLT_MAX);

    for (const auto& point : screenPoints) {
        if (point.z < 0.001f) continue;

        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
    }

    if (isBehind) {
        return { ImVec2(0,0), ImVec2(0,0), ImVec2(0,0), isBehind };
    }

    ImVec2 center = ImVec2(
        (max.x + min.x) * 0.5f,
        (max.y + min.y) * 0.5f
    );

    min.x = std::clamp(min.x, 0.0f, displaySize.x);
    min.y = std::clamp(min.y, 0.0f, displaySize.y);
    max.x = std::clamp(max.x, 0.0f, displaySize.x);
    max.y = std::clamp(max.y, 0.0f, displaySize.y);

    return { min, max, center, isBehind };
}

float CalculateDistance(const structures::Vector3& pos1, const structures::Vector3& pos2)
{
    return sqrt(
        pow(pos1.x - pos2.x, 2) +
        pow(pos1.y - pos2.y, 2) +
        pow(pos1.z - pos2.z, 2)
    ) / 100.0f; 
}

ImVec2 GetTextSizeCustom(ImFont* font, const char* text) {
    if (!font || !text) return ImVec2(0, 0);
    return font->CalcTextSizeA(
        font->FontSize,
        FLT_MAX,
        0.0f,
        text
    );
}

const char* GetWeaponName(enums::ItemDefinitionIndex weaponIndex)
{
    static char weaponStr[2] = { 0 };
    auto it = enums::ItemsNames.find(weaponIndex);
    if (it != enums::ItemsNames.end()) {
        weaponStr[0] = it->second;
        weaponStr[1] = '\0';
        return weaponStr;
    }
    return "s";
}

void RenderBombTimer() {

    if (vars::C4.Exploded || vars::C4.Defused || !vars::C4.Activated || !vars::bombTimer) return;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    float timeRemaining = vars::C4.TimeToExplode - vars::gameTime;
    if (timeRemaining < 0) return;

    float defuseTimeRequired = vars::localPlayer.HasDefuser ? 5.f : 10.f;
    bool canDefuse = timeRemaining > defuseTimeRequired;

    float barWidth = 300.0f;
    float barHeight = 6.0f;
    float barY = 120.0f;
    ImVec2 barPos = ImVec2((displaySize.x - barWidth) * 0.5f, barY);

    drawList->AddRectFilled(barPos, ImVec2(barPos.x + barWidth, barPos.y + barHeight),
        ImColor(0, 0, 0, 180), 4.0f);

    float progress = std::clamp(timeRemaining / vars::C4.MaxTimeToExplode, 0.0f, 1.0f);
    ImColor barColor = timeRemaining < defuseTimeRequired ?
        ImColor(255, 0, 0, 255) : ImColor(255, 165, 0, 255);

    drawList->AddRectFilled(barPos, ImVec2(barPos.x + (barWidth * progress), barPos.y + barHeight),
        barColor, 4.0f);

    static float defuseStartTime = 0.0f;
    if (vars::C4.BeingDefused) {
       
        if (defuseStartTime == 0.0f) {
            defuseStartTime = vars::gameTime;
        }

        float defuseTimeElapsed = vars::gameTime - defuseStartTime;
        float defuseProgress = std::clamp(defuseTimeElapsed / vars::C4.MaxTimeToDefuse, 0.0f, 1.0f);

        drawList->AddRectFilled(
            ImVec2(barPos.x, barPos.y + barHeight + 2),
            ImVec2(barPos.x + barWidth, barPos.y + barHeight + 8),
            ImColor(0, 0, 0, 180),
            4.0f
        );

        drawList->AddRectFilled(
            ImVec2(barPos.x, barPos.y + barHeight + 2),
            ImVec2(barPos.x + (barWidth * defuseProgress), barPos.y + barHeight + 8),
            ImColor(0, 255, 0, 255),
            4.0f
        );
    }
    if (!vars::C4.BeingDefused) {
        defuseStartTime = 0.0f;
    }

    char timerText[32];
    sprintf_s(timerText, "%.1fs", timeRemaining);
    ImVec2 textSize = ImGui::CalcTextSize(timerText);
    drawList->AddText(
        ImVec2(barPos.x + (barWidth - textSize.x) * 0.5f, barPos.y - textSize.y - 5),
        barColor, timerText
    );

    char siteText[32];
    sprintf_s(siteText, "SITE %c", vars::C4.Site == 0 ? 'A' : 'B');
    drawList->AddText(
        ImVec2(barPos.x + barWidth + 10, barPos.y - 5),
        barColor, siteText
    );

    if (vars::localPlayer.Team == 3) {
        const char* defuseText = canDefuse ? "Can Defuse" : "! RUN !";
        ImColor defuseColor = canDefuse ? ImColor(0, 255, 0, 255) : ImColor(255, 0, 0, 255);
        ImVec2 defuseSize = ImGui::CalcTextSize(defuseText);
        drawList->AddText(
            ImVec2(barPos.x + (barWidth - defuseSize.x) * 0.5f, barPos.y + barHeight + 10),
            defuseColor, defuseText
        );
    }


    structures::Vector3 screenPos = overlay::WorldToScreen(vars::C4.Position);
    if (screenPos.z > 0.001f) 
    {
        ImVec2 textSize = ImGui::CalcTextSize("O");
        drawList->AddText(
            fonts::icons,
            25.f,
            ImVec2(screenPos.x - textSize.x / 2, screenPos.y),
            ImColor(255, 0, 0, 255),
            "o"
        );
    }
}

void RenderItemESP() {
    if (!vars::esp || !vars::showItems) return;
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    for (const auto& item : lists::Items) {
        if (!item.IsValid) continue;

        structures::Vector3 screenPos = overlay::WorldToScreen(item.Position);
        if (screenPos.z < 0.001f) continue;

        // Calculate size based on distance
        float distance = CalculateDistance(vars::localPlayer.Position, item.Position);
        if (distance >= 20.f) continue;
    	float scale = std::max(0.3f, 1.0f - (distance / 100.0f));
        float fontSize = 20.0f * scale;

        // Get text sizes with scaled font
        ImVec2 textSize = GetTextSizeCustom(fonts::icons, item.Name.c_str());
        textSize.x *= scale;
        textSize.y *= scale;

        // Center text properly
        drawList->AddText(
            fonts::icons,
            fontSize,
            ImVec2(screenPos.x - (textSize.x / 2), screenPos.y - (textSize.y / 2)),
            item.IsProjectile ? ImColor(255, 0, 0, 255) : ImColor(255, 255, 255, 255),
            item.Name.c_str()
        );

        if (vars::esp) {
            char distText[32];
            sprintf_s(distText, "%.1fm", distance);
            ImVec2 distTextSize = ImGui::CalcTextSize(distText);
            distTextSize.x *= scale;

            drawList->AddText(
                ImGui::GetFont(),
                fontSize * 0.8f,
                ImVec2(screenPos.x - (distTextSize.x / 2), screenPos.y + textSize.y * 0.7f),
                item.IsProjectile ? ImColor(255, 0, 0, 255) : ImColor(255, 255, 255, 255),
                distText
            );
        }

        if (!item.IsProjectile && item.MaxClipAmmo > 0) {
            const float barWidth = 40.0f * scale;
            const float barHeight = 4.0f * scale;
            const float barY = screenPos.y + textSize.y + (15.0f * scale);

            drawList->AddRectFilled(
                ImVec2(screenPos.x - barWidth / 2, barY),
                ImVec2(screenPos.x + barWidth / 2, barY + barHeight),
                ImColor(0, 0, 0, 180)
            );

            float ammoRatio = (float)item.Clip / item.MaxClipAmmo;
            drawList->AddRectFilled(
                ImVec2(screenPos.x - barWidth / 2, barY),
                ImVec2(screenPos.x - barWidth / 2 + barWidth * ammoRatio, barY + barHeight),
                ImColor(0, 128, 255, 255)
            );
        }
    }
}

std::unordered_map<std::string, structures::Vector3> GetBonePositions(uintptr_t sceneNodePtr)
{
    std::unordered_map<std::string, structures::Vector3> bonePositions;

    uintptr_t boneArray = driver::Read<uintptr_t>(
        sceneNodePtr +
        cs2::schemas::client_dll::CSkeletonInstance::m_modelState +
        128
    );

    if (!boneArray)
        return bonePositions;

    for (const auto& bone : lists::BONES)
    {
        uintptr_t boneAddress = boneArray + (bone.index * 32);

        structures::Vector3 bonePosition = driver::Read<structures::Vector3>(boneAddress);

        bonePositions[bone.name] = bonePosition;
    }

    return bonePositions;
}

void DrawPlayerSkeleton(ImDrawList* drawList, const std::unordered_map<std::string, structures::Vector3>& bonePositions, ImColor color)
{
    for (const auto& connection : lists::BONE_CONNECTIONS)
    {
        const auto& startBone = bonePositions.find(connection.first);
        const auto& endBone = bonePositions.find(connection.second);

        if (startBone == bonePositions.end() || endBone == bonePositions.end())
            continue;

        structures::Vector3 startScreen = overlay::WorldToScreen(startBone->second);
        structures::Vector3 endScreen = overlay::WorldToScreen(endBone->second);

        if (startScreen.z < 0.001f || endScreen.z < 0.001f)
            continue;

        drawList->AddLine(
            ImVec2(startScreen.x, startScreen.y),
            ImVec2(endScreen.x, endScreen.y),
            color,
            1.5f
        );
    }
}

void RenderEsp()
{
    if (!vars::esp)
        return;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    RenderItemESP();
    RenderBombTimer();

    for (const auto& player : lists::Players)
    {
        if (player.Dormant || !player.IsAlive)
            continue;

        if (!vars::showTeammates && player.Team == vars::localPlayer.Team)
            continue;

        ScreenBounds bounds = CalculateScreenSpaceBounds(player.Position);

        if (bounds.isBehind)
            continue;

        ImColor playerColor = (player.Team == enums::Team::Terrorists) ?
            ImColor(255, 64, 64) : ImColor(64, 64, 255);

        if (vars::showPlayerBox)
        {
            drawList->AddRect(
                ImVec2(bounds.min.x, bounds.min.y),
                ImVec2(bounds.max.x, bounds.max.y),
                playerColor,
                0.0f,
                0,
                2.0f
            );
        }

        if (vars::showPlayerName)
        {
            ImVec2 textSize = ImGui::CalcTextSize(player.Name.c_str());
            drawList->AddText(
                ImVec2(bounds.center.x - textSize.x / 2, bounds.min.y - textSize.y - 2),
                ImColor(255, 255, 255),
                player.Name.c_str()
            );
        }

        if (vars::showHealthBar)
        {
            const float barWidth = 3.0f;
            const float barOffset = 8.0f;
            float healthPercent = static_cast<float>(player.Health) / 100.0f;

            drawList->AddRectFilled(
                ImVec2(bounds.min.x - barOffset, bounds.min.y),
                ImVec2(bounds.min.x - barOffset + barWidth, bounds.max.y),
                ImColor(60, 60, 60)
            );

            drawList->AddRectFilled(
                ImVec2(bounds.min.x - barOffset, bounds.min.y + (bounds.max.y - bounds.min.y) * (1.0f - healthPercent)),
                ImVec2(bounds.min.x - barOffset + barWidth, bounds.max.y),
                ImColor(0.f, 255 * healthPercent, 0.f)
            );
        }

        if (vars::showDistance)
        {
            float distance = CalculateDistance(vars::localPlayer.Position, player.Position);
            char distText[32];
            sprintf_s(distText, "%.1fm", distance);

            ImVec2 textSize = ImGui::CalcTextSize(distText);
            drawList->AddText(
                ImVec2(bounds.max.x + 5, bounds.center.y - textSize.y / 2),
                ImColor(255, 255, 255),
                distText
            );
        }

        if (vars::Armor)
        {
            std::string info;
            if (player.HasHelmet)
                info += "HK ";
            else if (player.Armor > 0)
                info += "K ";

            if (player.HasDefuser)
                info += "D";

            if (!info.empty())
            {
                ImVec2 textSize = ImGui::CalcTextSize(info.c_str());
                drawList->AddText(
                    ImVec2(bounds.max.x + 5, bounds.min.y),
                    ImColor(0, 191, 255),
                    info.c_str()
                );
            }
        }

        if (vars::showWeapon)
        {
            const char* weaponStr = GetWeaponName(player.HoldingWeapon);
            float distance = CalculateDistance(vars::localPlayer.Position, player.Position);

            const float baseSize = 18.f;
            const float minSize = 12.f;
            const float maxDistance = 2000.f;

            float scaledSize = std::max(
                minSize,
                baseSize * (1.0f - (distance / maxDistance))
            );

            ImVec2 textSize = ImGui::CalcTextSize(weaponStr);
            drawList->AddText(
                fonts::icons,
                scaledSize,
                ImVec2(bounds.center.x - textSize.x / 2, bounds.max.y),
                ImColor(255, 255, 255),
                weaponStr
            );
        }
    }
}
