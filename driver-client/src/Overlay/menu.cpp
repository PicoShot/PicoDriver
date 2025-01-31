#include "overlay.h"
#include "../imgui/imgui.h"
#include "../Utils/xorstr.hpp"

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
                    ImGui::Checkbox(xorstr_("Armor"), &vars::Armor);
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild(xorstr_("##visuals_col2"), ImVec2(0, 0), false);
            {
                ImGui::Combo(xorstr_("Box Type"), &vars::boxStyle, xorstr_("2D Box\0Corners\0"));
                ImGui::Checkbox(xorstr_("Items ESP"), &vars::showItems);
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
                ImGui::Checkbox(xorstr_("Recoil Control"), &vars::rcs);
            }
            break;
        }
        case Misc: {
            ImGui::TextColored(accentColor, xorstr_("Miscellaneous"));
            ImGui::Separator();
            ImGui::Checkbox(xorstr_("Spectator List"), &vars::spectatorList);
            ImGui::SliderFloat(xorstr_("Overlay Max FPS"), &vars::maxFps, 15.f, 165.f);
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