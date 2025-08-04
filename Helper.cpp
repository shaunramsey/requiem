#pragma once

#include "imgui.h"
#include <toml++/toml.hpp>
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

// a series of helper functions designed to make creating items in the two-column format of so many settings more easy
namespace Helper
{

    // IM GUI HELPERS

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    // In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
    void HelpMarker(const char *desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void Checkbox2Column(const char *label, const char *helpmarkerText, bool &value, const bool &comparisonValue)
    {
        ImGui::TableNextColumn();
        bool pop = false;
        if (value != comparisonValue)
        {
            pop = true;
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }

        ImGui::Text("%s:", label);

        ImGui::TableNextColumn();
        std::string checkboxId = "##" + std::string(label);
        if (ImGui::Checkbox(checkboxId.c_str(), &value))
        {
            // Handle checkbox change if needed
        }
        if (helpmarkerText && helpmarkerText[0] != '\0')
        {
            ImGui::SameLine();
            Helper::HelpMarker(helpmarkerText);
        }

        if (pop)
        {
            ImGui::PopStyleColor(3); // Pop both CheckMark and Text styles
            ImGui::PopStyleVar();    // Pop the FrameBorderSize style
        }
    }

    void Color2Column(const char *label, ImVec4 &color, const ImVec4 &comparisonColor, ImGuiColorEditFlags flags)
    {

        bool pop = false;
        if (color.x != comparisonColor.x ||
            color.y != comparisonColor.y ||
            color.z != comparisonColor.z ||
            color.w != comparisonColor.w)
        {
            pop = true;
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
        }

        ImGui::TableNextColumn();
        ImGui::Text(label);
        ImGui::SameLine();
        HelpMarker("Click on the color square to open a color picker.\n CTRL+click on individual component to input value.\n");
        ImGui::TableNextColumn();
        ImGui::ColorEdit4(label, (float *)&color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel | flags);
        if (pop)
        {
            ImGui::PopStyleColor();
        }
    }

    void KeyBind(const char *name, ImGuiKey &in_key, const ImGuiKey &comparison_key, ImGuiKey default_key)
    {
        static ImGuiKey lastKeyPress = in_key;
        ImGui::TableNextColumn();
        if (in_key != comparison_key)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
            ImGui::Text("%s:", name);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("%s:", name);
        }
        ImGui::TableNextColumn();

        std::string buttonName = "Current Key: [";
        buttonName += ImGui::GetKeyName(in_key);
        buttonName += "]##" + std::string(name);
        std::string popupName = buttonName + "popup";
        if (in_key != comparison_key)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
        }
        if (ImGui::Button(buttonName.c_str()))
        {
            ImGui::SetNextWindowFocus();
            lastKeyPress = in_key;
            ImGui::OpenPopup(popupName.c_str());
        }
        if (in_key != comparison_key)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::BeginPopup(popupName.c_str()))
        {
            ImGuiIO &io = ImGui::GetIO();

            float mousey = io.MousePos.y - ImGui::GetWindowPos().y;
            bool allow_change = mousey < 100.0f;
            ImGui::Text("You are here to change the keybind for %s", name);
            ImGui::Text("You may %sedit the key now", allow_change ? "" : "NOT ");
            ImGui::Separator();
            ImGui::Text("Current Keybind: %s", ImGui::GetKeyName(in_key));
            ImGui::Text("Default Keybind: %s", ImGui::GetKeyName(default_key));

            ImGui::Text("Keys down:");
            ImGui::SameLine();

            if (allow_change)
            {

                for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
                {
                    if (!ImGui::IsKeyDown(key))
                        continue;
                    // if (key < ImGuiKey_MouseLeft)
                    lastKeyPress = key;
                    // ImGui::Text("\"%s\"", ImGui::GetKeyName(key));
                }
                ImGui::Text("\"%s\"", ImGui::GetKeyName(lastKeyPress));
            }
            else
            {
                ImGui::Text("\"%s\"", ImGui::GetKeyName(lastKeyPress));
            }
            if (!allow_change)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("Key Is Set - Move mouse here to enable key capture");
            }
            // debug for positions and super keys
            //  ImVec2 window_pos = ImGui::GetWindowPos();
            //  ImGui::Text("%f %f", io.MousePos.x - window_pos.x, io.MousePos.y - window_pos.y);

            // ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");

            ImGui::Separator();
            bool pop = false;
            if (in_key != lastKeyPress)
            {
                pop = true;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.4f, 0.0f, 1.0f));
            }
            if (ImGui::Button("Accept and Close"))
            {
                in_key = lastKeyPress;
                ImGui::CloseCurrentPopup();
            }
            if (pop)
            {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel and Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // TOML HELPERS
    toml::array ImVec4ToToml(const ImVec4 &color)
    {
        toml::array arr;
        arr.push_back(color.x);
        arr.push_back(color.y);
        arr.push_back(color.z);
        arr.push_back(color.w);
        return arr;
    }

    ImVec4 tomlArrayToImVec4(const toml::array &arr, const ImVec4 &default_value)
    {
        if (arr.size() >= 4)
        {
            return ImVec4(arr[0].value_or(default_value.x),
                          arr[1].value_or(default_value.y),
                          arr[2].value_or(default_value.z),
                          arr[3].value_or(default_value.w));
        }
        return default_value;
    }

    ImVec4 tomlToImVec4(const toml::table &tbl, std::string key, ImVec4 default_value)
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

    void loadTomlKeybind(toml::table &table, const char *name, const char *default_value, ImGuiKey &key)
    {
        if (default_value == nullptr)
        {
            std::string keyName = (std::string)table[name].value_or("");
            if (keyName == "")
            {
                throw std::runtime_error("Keybind " + (std::string)name + " is required - has no default value -  and is not set in the config toml");
            }
            return;
        }

        std::string keyName = (std::string)table[name].value_or(default_value);
        key = ImGuiKeyMap[keyName];
    }
}
