#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include "imgui.h"
#include <iostream>
#include "Helper.h"
#include "externs.h"

// Make the UI compact because there are so many fields
// static void PushStyleCompact();
// static void PopStyleCompact();

class ConsoleSettings
{
public:
    ConsoleSettings();
    void loadToml(toml::table &tbl);
    void saveToml(toml::table &tbl);
    bool isEqual(const ConsoleSettings &other) const;
    void drawImGui();
    ImVec4 WarningColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 LogColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 DebugColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 MainConsoleBgColor = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
};

class KeyBindSettings
{
public:
    KeyBindSettings();
    void loadToml(toml::table &tbl);
    void saveToml(toml::table &tbl);
    bool isEqual(const KeyBindSettings &other) const;
    void drawImGui();
    std::string toggleUiKeyName = "F2";
    std::string toggleStatsKeyName = "F3";
    std::string toggleConsoleKeyName = "GraveAccent";
    std::string toggleSettingsKeyName = "Escape";
    ImGuiKey toggleUiKey = ImGuiKey_F2;
    ImGuiKey toggleStatsKey = ImGuiKey_F3;
    ImGuiKey toggleConsoleKey = ImGuiKey_GraveAccent; // the ` key
    ImGuiKey toggleSettingsKey = ImGuiKey_Escape;
};

class GraphicsSettings
{
public:
    GraphicsSettings();
    void loadToml(toml::table &tbl);
    void saveToml(toml::table &tbl);
    bool isEqual(const GraphicsSettings &other) const;
    void drawImGui();
    bool vsync = true;
    bool fullscreenPrimary = false;
    bool borderlessWindow = false;

private:
};

class GameSettings
{
public:
    GameSettings();
    void loadDefaults(std::string filename = "main.toml");
    void saveChanges(std::string filename = "main.toml");
    bool isEqual(const GameSettings &other) const;
    ConsoleSettings consoleSettings;
    KeyBindSettings keyBindSettings;
    GraphicsSettings graphicsSettings;

private:
    toml::table settingsTable;
};
