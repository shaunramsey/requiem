#pragma once

#include "imgui.h"
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
    

}