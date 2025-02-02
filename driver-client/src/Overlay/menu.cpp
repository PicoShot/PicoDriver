#include "overlay.h"
#include "../config/config.h"
#include "../imgui/imgui.h"
#include "../utils/xorstr.hpp"
#include "../config/vars.h"

enum Tabs {
    Visual,
    AimBot,
    Misc
};

static int currentTab = Visual;
static ImVec4 accentColor = ImVec4(0.49f, 0.15f, 0.64f, 1.00f);

void overlay::drawMenu() {
    ImGui::SetNextWindowSize(ImVec2(650, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin(xorstr_("PicoDriver"), nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::BeginChild(xorstr_("##sidebar"), ImVec2(120, 0), true);
    {
        if (ImGui::Button(xorstr_("Visuals##tab"), ImVec2(104, 40))) {
            currentTab = Visual;
        }
        ImGui::Dummy(ImVec2(0, 5));

        if (ImGui::Button(xorstr_("Aim Assist##tab"), ImVec2(104, 40))) {
            currentTab = AimBot;
        }
        ImGui::Dummy(ImVec2(0, 5));

        if (ImGui::Button(xorstr_("Misc##tab"), ImVec2(104, 40))) {
            currentTab = Misc;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(xorstr_("##content"), ImVec2(0, 0), true);
    {
        switch (currentTab) {
        case Visual: {
            ImGui::TextColored(accentColor, xorstr_("Visual Settings"));
            ImGui::Separator();
            ImGui::BeginChild(xorstr_("##visuals_col1"), ImVec2(ImGui::GetWindowWidth() * 0.5f, 0), false);
            {
                ImGui::Checkbox(xorstr_("Player ESP"), &vars::esp);
                if (vars::esp)
                {
                    ImGui::Checkbox(xorstr_("Teammate ESP"), &vars::showTeammates);
                    ImGui::Checkbox(xorstr_("Box ESP"), &vars::showPlayerBox);
                    ImGui::Checkbox(xorstr_("Name ESP"), &vars::showPlayerName);
                    ImGui::Checkbox(xorstr_("Distance"), &vars::showDistance);
                    ImGui::Checkbox(xorstr_("Health Bar"), &vars::showHealthBar);
                    ImGui::Checkbox(xorstr_("Skeleton"), &vars::showPlayerBone);
                    ImGui::Checkbox(xorstr_("showArmor"), &vars::showArmor);
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild(xorstr_("##visuals_col2"), ImVec2(0, 0), false);
            {
                ImGui::Combo(xorstr_("Box Type"), &vars::boxStyle, xorstr_("2D Box\0Corners\0"));
                ImGui::Checkbox(xorstr_("Items ESP"), &vars::showItems);
                ImGui::Checkbox(xorstr_("Weapons ESP"), &vars::showItems);
            }
            ImGui::EndChild();
            break;
        }
        case AimBot: {
            ImGui::TextColored(accentColor, xorstr_("Aim Assistance"));
            ImGui::Separator();
            ImGui::Checkbox(xorstr_("Enable Aim Assist"), &vars::aim);
            if (vars::aim)
            {
                ImGui::Checkbox(xorstr_("Aimbot"), &vars::aimbot);
                if (vars::aimbot)
                {
                    ImGui::Checkbox(xorstr_("Draw Fov"), &vars::showAimbotFov);
                    ImGui::Combo(xorstr_("Aimbot Mode"), &vars::aimbotMode, xorstr_("Always\0Mouse5\0Firing\0"));
                    ImGui::Combo(xorstr_("Aimbot Target"), &vars::aimbotTarget, xorstr_("Head\0Neck\0Chest\0Stomach\0Pelvis\0"));
                    ImGui::SliderFloat(xorstr_("Aimbot smooth"), &vars::aimSmooth, 1.f, 50.f);
                    ImGui::SliderFloat(xorstr_("Aimbot Fov"), &vars::aimFov, 1.f, 90.f);
                }
                ImGui::Spacing();
                ImGui::Checkbox(xorstr_("Recoil Control"), &vars::rcs);
            }
            break;
        }
        case Misc: {
            ImGui::TextColored(accentColor, xorstr_("Miscellaneous"));
            ImGui::Separator();

            ImGui::SliderFloat(xorstr_("Text Sizes"), &vars::textSize, 4.f, 24.f);
            ImGui::SliderFloat(xorstr_("Overlay Max FPS"), &vars::maxFps, 15.f, 165.f);

            if (ImGui::Button(xorstr_("Save Config"), ImVec2(100, 30)))
            {
                config::SaveConfig();
            }

            ImGui::SameLine();

            if (ImGui::Button(xorstr_("Load Config"), ImVec2(100, 30)))
            {
                config::LoadConfig();
            }

            ImGui::SameLine();

            if (ImGui::Button(xorstr_("Unload (Exit)"), ImVec2(100, 30)))
            {
                std::exit(0);
            }
            
            break;
        }
        }
    }
    ImGui::EndChild();

    // Footer
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), xorstr_("Status:"));
    ImGui::SameLine(ImGui::GetWindowWidth() - 600);
    ImGui::TextColored(ImVec4(0.f, 0.8f, 0.f, 1.0f), xorstr_("Undetected"));
    ImGui::SameLine(ImGui::GetWindowWidth() - 50);
    ImGui::Text(xorstr_("v1.0.0"));

    ImGui::End();
}