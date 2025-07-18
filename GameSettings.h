#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include "imgui.h"

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

    ImVec4 WarningColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
    ImVec4 ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 LogColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 DebugColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 MainConsoleBgColor = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
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
    }

    ConsoleSettings consoleSettings;

private:
    toml::table settingsTable;
};