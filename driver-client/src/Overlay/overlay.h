#pragma once
#include <atomic>
#include "../imgui/imgui.h"
#include "../config/vars.h"

namespace fonts {
    static ImFont* normal = nullptr;
    static ImFont* Title = nullptr;
    static ImFont* icons = nullptr;
}

namespace overlay {
    void drawMenu();
    void render();
    __forceinline structures::Vector3 WorldToScreen(const structures::Vector3& point) noexcept;
    static std::atomic<bool> menuEnabled{ false };
}