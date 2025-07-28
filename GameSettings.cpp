#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include "imgui.h"
#include "Helper.h"
#include "externs.h"
#include "GameSettings.h"
#include "Console.h"

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, (float)(int)(style.FramePadding.y * 0.60f));
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, (float)(int)(style.ItemSpacing.y * 0.60f));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

GraphicsSettings::GraphicsSettings() {}
void GraphicsSettings::loadToml(toml::table &tbl)
{
    if (tbl["Graphics"].as_table() == nullptr)
    {
        _console.WarningLog("TOML", "    [-] No graphics settings found in toml, using defaults", nullptr);
        return;
    }
    _console.Log("TOML", "    [-] Loading graphics settings from toml");
    toml::table table = *tbl["Graphics"].as_table();
    vsync = table["vsync"].value_or(true);
    borderlessWindow = table["borderlessWindow"].value_or(false);
    fullscreenPrimary = table["fullscreenPrimary"].value_or(false);
}

void GraphicsSettings::saveToml(toml::table &tbl)
{
    _console.Log("TOML", "    [-] Saving graphics settings to toml");
    toml::table table;
    table.insert("vsync", vsync);
    table.insert("borderlessWindow", borderlessWindow);
    table.insert("fullscreenPrimary", fullscreenPrimary);
    tbl.insert("Graphics", table);
}

bool GraphicsSettings::isEqual(const GraphicsSettings &other) const
{
    return vsync == other.vsync &&
           borderlessWindow == other.borderlessWindow &&
           fullscreenPrimary == other.fullscreenPrimary;
}

void GraphicsSettings::drawImGui()
{
    ImGui::Text("Graphics Settings");
    ImGui::SameLine();
    ImGui::Text("(APPLY and RESTART is required to take effect)");
    ImGui::Separator();
    if (ImGui::BeginTable("table_columns_flags_checkboxes", 2, ImGuiTableFlags_None))
    {
        // ImGui::TableSetupColumn("Column1", ImGuiTableColumnFlags_WidthStretch, 0.5f);
        // // Second column: Remaining space (will automatically be 50% in this case)
        // ImGui::TableSetupColumn("Column2", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("VSync");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##vsync", &vsync);
        ImGui::SameLine();
        Helper::HelpMarker("With Vsync disabled, the game will run as fast as possible, which may lead to high CPU/GPU usage. With Vsync enabled, the game will run at the monitor's refresh rate, which may lead to lower CPU/GPU usage and increased input lag.");
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Fullscreen");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##fullscreenPrimary", &fullscreenPrimary);

        if (fullscreenPrimary)
        {
            ImGui::Indent();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Borderless Window");
            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox("##borderless", &borderlessWindow);
            ImGui::SameLine();

            if (borderlessWindow)
            {
                Helper::HelpMarker("( Fullscreen Borderless Window selected )");
            }
            else
            {
                Helper::HelpMarker("( Fullscreen (not windowed) selected )");
            }
            ImGui::Unindent();
        }

        ImGui::EndTable();
    }
}

ConsoleSettings::ConsoleSettings()
{
}
void ConsoleSettings::loadToml(toml::table &tbl)
{
    if (tbl["Console"].as_table() == nullptr)
    {
        _console.WarningLog("TOML", "    [-] No key bindings found in toml, using defaults", nullptr);
        return;
    }
    toml::table table = *tbl["Console"].as_table();
    _console.Log("TOML", "    [-] Loading console settings from toml");
    Helper::loadTomlColor(table, "WarningColor", WarningColor);
    Helper::loadTomlColor(table, "ErrorColor", ErrorColor);
    Helper::loadTomlColor(table, "LogColor", LogColor);
    Helper::loadTomlColor(table, "DebugColor", DebugColor);
    Helper::loadTomlColor(table, "MainConsoleBgColor", MainConsoleBgColor);
}
void ConsoleSettings::saveToml(toml::table &tbl)
{
    toml::table table;
    _console.Log("TOML", "    [-] Saving console settings to toml");
    table.insert("WarningColor", Helper::ImVec4ToToml(WarningColor));
    table.insert("ErrorColor", Helper::ImVec4ToToml(ErrorColor));
    table.insert("LogColor", Helper::ImVec4ToToml(LogColor));
    table.insert("DebugColor", Helper::ImVec4ToToml(DebugColor));
    table.insert("MainConsoleBgColor", Helper::ImVec4ToToml(MainConsoleBgColor));
    tbl.insert("Console", table);
}
bool ConsoleSettings::isEqual(const ConsoleSettings &other) const
{
    return WarningColor.x == other.WarningColor.x &&
           WarningColor.y == other.WarningColor.y &&
           WarningColor.z == other.WarningColor.z &&
           WarningColor.w == other.WarningColor.w &&
           ErrorColor.x == other.ErrorColor.x &&
           ErrorColor.y == other.ErrorColor.y &&
           ErrorColor.z == other.ErrorColor.z &&
           ErrorColor.w == other.ErrorColor.w &&
           LogColor.x == other.LogColor.x &&
           LogColor.y == other.LogColor.y &&
           LogColor.z == other.LogColor.z &&
           LogColor.w == other.LogColor.w &&
           DebugColor.x == other.DebugColor.x &&
           DebugColor.y == other.DebugColor.y &&
           DebugColor.z == other.DebugColor.z &&
           DebugColor.w == other.DebugColor.w &&
           MainConsoleBgColor.x == other.MainConsoleBgColor.x &&
           MainConsoleBgColor.y == other.MainConsoleBgColor.y &&
           MainConsoleBgColor.z == other.MainConsoleBgColor.z &&
           MainConsoleBgColor.w == other.MainConsoleBgColor.w;
}

void ConsoleSettings::drawImGui()
{
    ImGui::Text("Console Colors");
    ImGui::SameLine();
    ImGui::Text(" - (be sure to save - restart may be required to take effect)");
    ImGui::Separator();
    static ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;
    if (ImGui::BeginTable("table_columns_flags_checkboxes", 2, ImGuiTableFlags_None))
    {
        ImGui::TableSetupColumn("Column1", ImGuiTableColumnFlags_WidthStretch, 0.5f);
        // Second column: Remaining space (will automatically be 50% in this case)
        ImGui::TableSetupColumn("Column2", ImGuiTableColumnFlags_WidthStretch);

        Helper::Color2Column("Warning Color", WarningColor, base_flags);
        Helper::Color2Column("Error Color", ErrorColor, base_flags);
        Helper::Color2Column("Log Color", LogColor, base_flags);
        Helper::Color2Column("Debug Color", DebugColor, base_flags);
        Helper::Color2Column("Main Console Background Color", MainConsoleBgColor, base_flags);
        ImGui::EndTable();
    }
}

KeyBindSettings::KeyBindSettings() {}
void KeyBindSettings::loadToml(toml::table &tbl)
{
    if (tbl["KeyBindings"].as_table() == nullptr)
    {
        _console.WarningLog("TOML", "    [-] No key bindings found in toml, using defaults", nullptr);
        return;
    }
    toml::table table = *tbl["KeyBindings"].as_table();
    _console.Log("TOML", "    [-] Loading key bindings from toml");
    Helper::loadTomlKeybind(table, "ToggleUiKey", "F2", toggleUiKey);
    Helper::loadTomlKeybind(table, "ToggleStatsKey", "F3", toggleStatsKey);
    Helper::loadTomlKeybind(table, "ToggleConsoleKey", "GraveAccent", toggleConsoleKey);
    Helper::loadTomlKeybind(table, "ToggleSettingsKey", "Escape", toggleSettingsKey);
}
void KeyBindSettings::saveToml(toml::table &tbl)
{
    toml::table table;
    _console.Log("TOML", "    [-] Saving key bindings to toml");
    table.insert("ToggleUiKey", ImGui::GetKeyName(toggleUiKey));
    table.insert("ToggleStatsKey", ImGui::GetKeyName(toggleStatsKey));
    table.insert("ToggleConsoleKey", ImGui::GetKeyName(toggleConsoleKey));
    table.insert("ToggleSettingsKey", ImGui::GetKeyName(toggleSettingsKey));
    tbl.insert("KeyBindings", table);
}

bool KeyBindSettings::isEqual(const KeyBindSettings &other) const
{
    return toggleUiKey == other.toggleUiKey &&
           toggleStatsKey == other.toggleStatsKey &&
           toggleConsoleKey == other.toggleConsoleKey &&
           toggleSettingsKey == other.toggleSettingsKey;
}
void KeyBindSettings::drawImGui()
{
    ImGui::Text("Key Bindings");
    ImGui::SameLine();
    ImGui::Text("(be sure to save - restart may be required to take effect)");
    if (ImGui::BeginTable("table_columns_flags_checkboxes", 2, ImGuiTableFlags_None))
    {
        // PushStyleCompact();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));

        Helper::KeyBind("Toggle UI Key", toggleUiKey, ImGuiKey_F2);
        Helper::KeyBind("Toggle Stats Key", toggleStatsKey, ImGuiKey_F3);
        Helper::KeyBind("Toggle Console Key", toggleConsoleKey, ImGuiKey_GraveAccent);
        Helper::KeyBind("Toggle Settings Key", toggleSettingsKey, ImGuiKey_Escape);

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndTable();
        // PopStyleCompact();
    }
}

GameSettings::GameSettings()
{
    // Initialize default settings
}
void GameSettings::loadDefaults(std::string filename)
{
    _console.Log("TOML", "[*] Loading game settings from toml");
    settingsTable = toml::parse_file(filename);
    consoleSettings.loadToml(settingsTable);
    keyBindSettings.loadToml(settingsTable);
    graphicsSettings.loadToml(settingsTable);
}

void GameSettings::saveChanges(std::string filename)
{
    _console.Log("TOML", "[*] Saving game settings to toml");

    toml::table newTable;
    consoleSettings.saveToml(newTable);
    keyBindSettings.saveToml(newTable);
    graphicsSettings.saveToml(newTable);

    std::ofstream file(filename);
    if (file.is_open())
    {
        file << newTable;
        file.close();
    }
    else
    {
        std::string errorMsg = "Could not open file for writing: " + filename;
        _console.ErrorLog("TOML", errorMsg.c_str());
        // std::cerr << "Could not open file for writing: " << filename << std::endl;
    }
}

bool GameSettings::isEqual(const GameSettings &other) const
{
    return consoleSettings.isEqual(other.consoleSettings) &&
           keyBindSettings.isEqual(other.keyBindSettings) &&
           graphicsSettings.isEqual(other.graphicsSettings);
}
