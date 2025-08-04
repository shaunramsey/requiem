#pragma once

#include "imgui.h"
#include <toml++/toml.hpp>

extern std::map<std::string, ImGuiKey> ImGuiKeyMap;

// a series of helper functions designed to make creating items in the two-column format of so many settings more easy
namespace Helper
{

    // IM GUI HELPERS

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    // In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
    void HelpMarker(const char *desc);
    void Color2Column(const char *label, ImVec4 &color, const ImVec4 &comparisonColor, ImGuiColorEditFlags flags = 0);
    void Checkbox2Column(const char *label, const char *helpmarkerText, bool &value, const bool &comparisonValue);
    void KeyBind(const char *name, ImGuiKey &in_key, const ImGuiKey &comparison_key, ImGuiKey default_key);
    // TOML HELPERS
    ImVec4 tomlToImVec4(const toml::table &tbl, std::string key, ImVec4 default_value = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImVec4 tomlArrayToImVec4(const toml::array &arr, const ImVec4 &default_value = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    toml::array ImVec4ToToml(const ImVec4 &color);
    void loadTomlColor(toml::table &table, const char *name, ImVec4 &color);
    void loadTomlKeybind(toml::table &table, const char *name, const char *default_value, ImGuiKey &key);
}
