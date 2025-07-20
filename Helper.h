#pragma once

#include "imgui.h"
#include <toml++/toml.hpp>

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

// a series of helper functions designed to make creating items in the two-column format of so many settings more easy
namespace Helper
{

    void Color2Column(const char *label, ImVec4 &color, ImGuiColorEditFlags flags = 0)
    {
        ImGui::TableNextColumn();
        ImGui::Text(label);
        ImGui::TableNextColumn();
        ImGui::ColorEdit4(label, (float *)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | flags);
    }

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

    void loadTomlColor(toml::table &table, const char *name, ImVec4 &color)
    {
        color = Helper::tomlToImVec4(table, name, color);
    }

    void loadTomlKeybind(toml::table &table, const char *name, const char *default_value, ImGuiKey &key, std::string &keyName)
    {
        if (default_value == nullptr)
        {
            keyName = (std::string)table[name].value_or("");
            if (keyName == "")
            {
                throw std::runtime_error("Keybind " + (std::string)name + " is required - has no default value -  and is not set in the config toml");
            }
            return;
        }

        keyName = (std::string)table[name].value_or(default_value);
        key = ImGuiKeyMap[keyName];
    }
}
