#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include "imgui.h"
#include "utils.h"
#include "Helper.h"

std::map<std::string, ImGuiKey> ImGuiKeyMap = {
    {"Tab", ImGuiKey_Tab},
    {"LeftArrow", ImGuiKey_LeftArrow},
    {"RightArrow", ImGuiKey_RightArrow},
    {"UpArrow", ImGuiKey_UpArrow},
    {"DownArrow", ImGuiKey_DownArrow},
    {"PageUp", ImGuiKey_PageUp},
    {"PageDown", ImGuiKey_PageDown},
    {"Home", ImGuiKey_Home},
    {"End", ImGuiKey_End},
    {"Insert", ImGuiKey_Insert},
    {"Delete", ImGuiKey_Delete},
    {"Backspace", ImGuiKey_Backspace},
    {"Space", ImGuiKey_Space},
    {"Enter", ImGuiKey_Enter},
    {"Escape", ImGuiKey_Escape},
    {"LeftCtrl", ImGuiKey_LeftCtrl},
    {"LeftShift", ImGuiKey_LeftShift},
    {"LeftAlt", ImGuiKey_LeftAlt},
    {"LeftSuper", ImGuiKey_LeftSuper},
    {"RightCtrl", ImGuiKey_RightCtrl},
    {"RightShift", ImGuiKey_RightShift},
    {"RightAlt", ImGuiKey_RightAlt},
    {"RightSuper", ImGuiKey_RightSuper},
    {"Menu", ImGuiKey_Menu},
    {"0", ImGuiKey_0},
    {"1", ImGuiKey_1},
    {"2", ImGuiKey_2},
    {"3", ImGuiKey_3},
    {"4", ImGuiKey_4},
    {"5", ImGuiKey_5},
    {"6", ImGuiKey_6},
    {"7", ImGuiKey_7},
    {"8", ImGuiKey_8},
    {"9", ImGuiKey_9},
    {"A", ImGuiKey_A},
    {"B", ImGuiKey_B},
    {"C", ImGuiKey_C},
    {"D", ImGuiKey_D},
    {"E", ImGuiKey_E},
    {"F", ImGuiKey_F},
    {"G", ImGuiKey_G},
    {"H", ImGuiKey_H},
    {"I", ImGuiKey_I},
    {"J", ImGuiKey_J},
    {"K", ImGuiKey_K},
    {"L", ImGuiKey_L},
    {"M", ImGuiKey_M},
    {"N", ImGuiKey_N},
    {"O", ImGuiKey_O},
    {"P", ImGuiKey_P},
    {"Q", ImGuiKey_Q},
    {"R", ImGuiKey_R},
    {"S", ImGuiKey_S},
    {"T", ImGuiKey_T},
    {"U", ImGuiKey_U},
    {"V", ImGuiKey_V},
    {"W", ImGuiKey_W},
    {"X", ImGuiKey_X},
    {"Y", ImGuiKey_Y},
    {"Z", ImGuiKey_Z},
    {"F1", ImGuiKey_F1},
    {"F2", ImGuiKey_F2},
    {"F3", ImGuiKey_F3},
    {"F4", ImGuiKey_F4},
    {"F5", ImGuiKey_F5},
    {"F6", ImGuiKey_F6},
    {"F7", ImGuiKey_F7},
    {"F8", ImGuiKey_F8},
    {"F9", ImGuiKey_F9},
    {"F10", ImGuiKey_F10},
    {"F11", ImGuiKey_F11},
    {"F12", ImGuiKey_F12},
    {"F13", ImGuiKey_F13},
    {"F14", ImGuiKey_F14},
    {"F15", ImGuiKey_F15},
    {"F16", ImGuiKey_F16},
    {"F17", ImGuiKey_F17},
    {"F18", ImGuiKey_F18},
    {"F19", ImGuiKey_F19},
    {"F20", ImGuiKey_F20},
    {"F21", ImGuiKey_F21},
    {"F22", ImGuiKey_F22},
    {"F23", ImGuiKey_F23},
    {"F24", ImGuiKey_F24},
    {"Apostrophe", ImGuiKey_Apostrophe},
    {"Comma", ImGuiKey_Comma},
    {"Minus", ImGuiKey_Minus},
    {"Period", ImGuiKey_Period},
    {"Slash", ImGuiKey_Slash},
    {"Semicolon", ImGuiKey_Semicolon},
    {"Equal", ImGuiKey_Equal},
    {"LeftBracket", ImGuiKey_LeftBracket},
    {"Backslash", ImGuiKey_Backslash},
    {"RightBracket", ImGuiKey_RightBracket},
    {"GraveAccent", ImGuiKey_GraveAccent},
    {"CapsLock", ImGuiKey_CapsLock},
    {"ScrollLock", ImGuiKey_ScrollLock},
    {"NumLock", ImGuiKey_NumLock},
    {"PrintScreen", ImGuiKey_PrintScreen},
    {"Pause", ImGuiKey_Pause},
    {"Keypad0", ImGuiKey_Keypad0},
    {"Keypad1", ImGuiKey_Keypad1},
    {"Keypad2", ImGuiKey_Keypad2},
    {"Keypad3", ImGuiKey_Keypad3},
    {"Keypad4", ImGuiKey_Keypad4},
    {"Keypad5", ImGuiKey_Keypad5},
    {"Keypad6", ImGuiKey_Keypad6},
    {"Keypad7", ImGuiKey_Keypad7},
    {"Keypad8", ImGuiKey_Keypad8},
    {"Keypad9", ImGuiKey_Keypad9},
    {"KeypadDecimal", ImGuiKey_KeypadDecimal},
    {"KeypadDivide", ImGuiKey_KeypadDivide},
    {"KeypadMultiply", ImGuiKey_KeypadMultiply},
    {"KeypadSubtract", ImGuiKey_KeypadSubtract},
    {"KeypadAdd", ImGuiKey_KeypadAdd},
    {"KeypadEnter", ImGuiKey_KeypadEnter},
    {"KeypadEqual", ImGuiKey_KeypadEqual},
    {"AppBack", ImGuiKey_AppBack},
    {"AppForward", ImGuiKey_AppForward}};

ImVec4 tomlToImVec4(const toml::table &tbl, std::string key, ImVec4 default_value = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    auto arr = tbl[key].as_array();
    if (arr && arr->size() >= 4)
    {
        return ImVec4(arr->at(0).value_or(default_value.x),
                      arr->at(1).value_or(default_value.y),
                      arr->at(2).value_or(default_value.z),
                      arr->at(3).value_or(default_value.w));
    }
    return default_value;
}

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
        toml::table table = *tbl["Console"].as_table();
        std::cout << "  [-] Loading console settings from toml" << std::endl;
        WarningColor = tomlToImVec4(table, "WarningColor", WarningColor);
        ErrorColor = tomlToImVec4(table, "ErrorColor", ErrorColor);
        LogColor = tomlToImVec4(table, "LogColor", LogColor);
        DebugColor = tomlToImVec4(table, "DebugColor", DebugColor);
        MainConsoleBgColor = tomlToImVec4(table, "MainConsoleBgColor", MainConsoleBgColor);
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

    ImVec4 WarningColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
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
        toggleUiKeyName = (std::string)table["ToggleUiKey"].value_or("F2");
        toggleStatsKeyName = (std::string)table["ToggleStatsKey"].value_or("F3");
        toggleConsoleKeyName = (std::string)table["ToggleConsoleKey"].value_or("GraveAccent");
        toggleSettingsKeyName = (std::string)table["ToggleSettingsKey"].value_or("Escape");
        toggleUiKey = ImGuiKeyMap[toggleUiKeyName];
        toggleStatsKey = ImGuiKeyMap[toggleStatsKeyName];
        toggleConsoleKey = ImGuiKeyMap[toggleConsoleKeyName];
        toggleSettingsKey = ImGuiKeyMap[toggleSettingsKeyName];
        std::cout << "    [-] ToggleUiKey: " << toggleUiKeyName << std::endl;
        std::cout << "    [-] ToggleStatsKey: " << toggleStatsKeyName << std::endl;
        std::cout << "    [-] ToggleConsoleKey: " << toggleConsoleKeyName << std::endl;
        std::cout << "    [-] ToggleSettingsKey: " << toggleSettingsKeyName << std::endl;
    }
    void drawImGui()
    {
        ImGui::Text("Key Bindings");
        ImGui::SameLine();
        ImGui::Text("(be sure to save - restart may be required to take effect)");
        if (ImGui::BeginTable("table_columns_flags_checkboxes", 2, ImGuiTableFlags_None))
        {
            PushStyleCompact();
            ImGui::TableNextColumn();
            ImGui::Text("Toggle UI Key:");
            ImGui::TableNextColumn();
            ImGui::Text("%s", toggleUiKeyName.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("Toggle Stats Key:");
            ImGui::TableNextColumn();
            ImGui::Text("%s", toggleStatsKeyName.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("Toggle Console Key:");
            ImGui::TableNextColumn();
            ImGui::Text("%s", toggleConsoleKeyName.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("Toggle Settings Key:");
            ImGui::TableNextColumn();
            ImGui::Text("%s", toggleSettingsKeyName.c_str());

            ImGui::EndTable();
            PopStyleCompact();
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

    ConsoleSettings consoleSettings;
    KeyBindSettings keyBindSettings;

private:
    toml::table settingsTable;
};
