#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include "imgui.h"
#include "Helper.h"

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

class ConsoleSettings
{
public:
    ConsoleSettings()
    {
    }

    void loadToml(toml::table &tbl)
    {
        if (tbl["Console"].as_table() == nullptr)
        {
            std::cout << "  [-] No console settings found in toml, using defaults" << std::endl;
            return;
        }
        toml::table table = *tbl["Console"].as_table();
        std::cout << "  [-] Loading console settings from toml" << std::endl;
        Helper::loadTomlColor(table, "WarningColor", WarningColor);
        Helper::loadTomlColor(table, "ErrorColor", ErrorColor);
        Helper::loadTomlColor(table, "LogColor", LogColor);
        Helper::loadTomlColor(table, "DebugColor", DebugColor);
        Helper::loadTomlColor(table, "MainConsoleBgColor", MainConsoleBgColor);
    }

    void saveToml(toml::table &tbl)
    {
    }
    void drawImGui()
    {
        ImGui::Text("Console Colors");
        ImGui::SameLine();
        ImGui::Text(" - (be sure to save - restart may be required to take effect)");
        ImGui::Separator();
        static ImGuiColorEditFlags base_flags = ImGuiColorEditFlags_None;
        if (ImGui::BeginTable("table_columns_flags_checkboxes", 2, ImGuiTableFlags_None))
        {
            Helper::Color2Column("Warning Color", WarningColor, base_flags);
            Helper::Color2Column("Error Color", ErrorColor, base_flags);
            Helper::Color2Column("Log Color", LogColor, base_flags);
            Helper::Color2Column("Debug Color", DebugColor, base_flags);
            Helper::Color2Column("Main Console Background Color", MainConsoleBgColor, base_flags);
            ImGui::EndTable();
        }
    }

    ImVec4 WarningColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 LogColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 DebugColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 MainConsoleBgColor = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
};

class KeyBindSettings
{
public:
    KeyBindSettings() {}
    void loadToml(toml::table &tbl)
    {
        toml::table table = *tbl["KeyBindings"].as_table();
        std::cout << "  [-] Loading key bindings from toml" << std::endl;
        Helper::loadTomlKeybind(table, "ToggleUiKey", "F2", toggleUiKey);
        Helper::loadTomlKeybind(table, "ToggleStatsKey", "F3", toggleStatsKey);
        Helper::loadTomlKeybind(table, "ToggleConsoleKey", "GraveAccent", toggleConsoleKey);
        Helper::loadTomlKeybind(table, "ToggleSettingsKey", "Escape", toggleSettingsKey);
    }
    void saveToml(toml::table &tbl)
    {
        toml::table table;
        std::cout << "  [-] Saving key bindings to toml" << std::endl;
        table.insert("ToggleUiKey", ImGui::GetKeyName(toggleUiKey));
        table.insert("ToggleStatsKey", ImGui::GetKeyName(toggleStatsKey));
        table.insert("ToggleConsoleKey", ImGui::GetKeyName(toggleConsoleKey));
        table.insert("ToggleSettingsKey", ImGui::GetKeyName(toggleSettingsKey));
        tbl.insert("KeyBindings", table);
    }
    void drawImGui()
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
    std::string toggleUiKeyName = "F2";
    std::string toggleStatsKeyName = "F3";
    std::string toggleConsoleKeyName = "GraveAccent";
    std::string toggleSettingsKeyName = "Escape";
    ImGuiKey toggleUiKey = ImGuiKey_F2;
    ImGuiKey toggleStatsKey = ImGuiKey_F3;
    ImGuiKey toggleConsoleKey = ImGuiKey_GraveAccent; // the ` key
    ImGuiKey toggleSettingsKey = ImGuiKey_Escape;
};

class GameSettings
{
public:
    GameSettings()
    {
        // Initialize default settings
    }

    void loadDefaults(std::string filename = "main.toml")
    {
        std::cout << "[*] Loading game settings from toml" << std::endl;

        settingsTable = toml::parse_file(filename);
        consoleSettings.loadToml(settingsTable);
        keyBindSettings.loadToml(settingsTable);
    }

    void saveChanges(std::string filename = "main.toml")
    {
        std::cout << "[*] Saving game settings to toml" << std::endl;

        toml::table newTable;
        consoleSettings.saveToml(newTable);
        keyBindSettings.saveToml(newTable);

        std::ofstream file(filename);
        if (file.is_open())
        {
            file << newTable;
            file.close();
        }
        else
        {
            std::cerr << "Could not open file for writing: " << filename << std::endl;
        }
    }

    ConsoleSettings consoleSettings;
    KeyBindSettings keyBindSettings;

private:
    toml::table settingsTable;
};
