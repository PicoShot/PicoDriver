#pragma once
#include <atomic>
#include "../imgui/imgui.h"
#include "../Utils/vars.h"
#define d_toggle_bind 0x2D

namespace fonts {
    static ImFont* normal = nullptr;
    static ImFont* Title = nullptr;
    static ImFont* icons = nullptr;
}

namespace overlay {
    void drawMenu();
    void render();
    __forceinline structures::Vector3 WorldToScreen(const structures::Vector3& point) noexcept;
    static std::atomic<bool> enabled{ true };
}