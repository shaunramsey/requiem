#pragma once
// console.h
#include "imgui.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cstdarg>

namespace Ramsey
{

    class ColorString
    {
    public:
        ColorString(std::string s, ImVec4 v = {1.0, 1.0, 1.0, 1.0})
        {
            myString = s;
            color = v;
        }
        std::string myString;
        ImVec4 color;
    };

    class Console
    {

    private:
        std::vector<ColorString> history;
        int current_index;

    public:
        static std::string formatString(const char *fstring, ...)
        {
            va_list args;
            char *format = (char *)(fstring);
            va_start(args, fstring);
            std::string final_string = "";
            while (*format != '\0')
            {
                if (*format == '%')
                {
                    ++format;
                    if (*format == 'd')
                    {
                        int i = va_arg(args, int);
                        final_string += std::to_string(i);
                    }
                    else if (*format == 'c')
                    {
                        char c = (char)va_arg(args, int); // char promotes to int
                        final_string += c;
                    }
                    else if (*format == 'f')
                    {
                        double d = va_arg(args, double);
                        final_string += std::to_string(d);
                    }
                    else if (*format == 's')
                    {
                        char *s = va_arg(args, char *);
                        final_string += s;
                    }
                    else if (*format == '%')
                    {
                        final_string += '%';
                    }
                }
                else
                {
                    final_string += *format;
                }
                ++format;
            }
            va_end(args);
            return final_string;
        }

        Console()
        {
            history.push_back(ColorString(""));
            current_index = 1;
        }

        int executeCommand(std::string s)
        {
            for (int i = 0; i < s.size(); i++)
            {
                s[i] = (char)tolower(s[i]);
            }
            if (s == "help")
            {
                history.push_back("> " + s);
            }
            else
            {
                history.push_back("> " + s);
                history.push_back(ColorString("  - COMMAND NOT FOUND", {1.0, 0.0, 0.0, 1.0}));
            }
            return 0;
        }

        static int ConsoleCommandCallback(ImGuiInputTextCallbackData *data)
        {
            // debugPrint("ConsoleCommandCallback");
            return ((Console *)(data->UserData))->ActualConsoleCommandCallback(data);
        }

        int ActualConsoleCommandCallback(ImGuiInputTextCallbackData *data)
        {
            // debugPrint("conosle command was used");
            if (data->UserData)
            {
            }
            if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
            {
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    // debugPrint("uparrow callbackhistory");
                    current_index--;
                    if (current_index < 0)
                    {
                        current_index = (int)history.size() - 1;
                    }
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history[current_index].myString.c_str());
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    current_index++;
                    if (current_index > history.size() - 1)
                    {
                        current_index = 0;
                    }
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history[current_index].myString.c_str());
                }
            }
            return 0;
        }

        void debugPrint(const char *fstring, ...)
        {
            std::string final_string = formatString(fstring);
            history.push_back(ColorString(final_string, {0.7f, 0.9f, 0.9f, 1.0f}));
        }

        void draw(bool *p_open = NULL)
        {
            static int guess_size = 0;
            ImGuiViewport *imguiViewport = ImGui::GetMainViewport();
            ImVec2 size = imguiViewport->WorkSize;
            ImVec2 pos = imguiViewport->WorkPos;
            pos.y += size[1];

            size[1] = (float)(size[1] * 0.7f);
            size[1] -= guess_size;

            ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0, 1.0f));

            float grey_value = 0.3f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(grey_value, grey_value, grey_value, 0.5f));
            const static ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            if (!ImGui::Begin("Console", p_open, winFlags))
            {
                ImGui::PopStyleColor();
                ImGui::End();
                return;
            }
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar);
            // ImGui::TextWrapped("> Work Size is %d %d", imguiViewport->WorkSize.x, imguiViewport->WorkSize.y);
            ImGui::TextWrapped("> Hello Console!");
            for (int i = 0; i < history.size(); i++)
            {
                // debugPrint("imguiDisplay: " + std::to_string(history[i].color.x));
                ImGui::PushStyleColor(ImGuiCol_Text, history[i].color);
                ImGui::TextWrapped(history[i].myString.c_str());
                ImGui::PopStyleColor();
            }
            // ImGui::SetScrollY(ImGui::GetScrollMaxY());
            // ImGui::End();

            pos[1] = size[1];
            size[1] = (float)guess_size;
            // ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            // ImGui::SetNextWindowPos(pos);
            // ImGui::Begin("##You Don't Know", NULL, winFlags);
            // ImGui::SetNextChild
            // ImGui::BeginChild("##You odn't know", size, 0, winFlags);
            ImGui::SetScrollHereY(1.0f);
            if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::EndChild();
            ImGui::Separator();

            static char commandBuffer[1024];
            // inputtextwithhint
            // escape clears all
            ImGui::PushItemWidth(size[0] - 20);
            ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

            // ImGui::SetKeyboardFocusHere();
            // ImGui::SetItemDefaultFocus();
            if (ImGui::InputTextWithHint("##Commands", "Type help for help", commandBuffer, 1023, inputTextFlags, &ConsoleCommandCallback, (void *)this))
            {

                debugPrint(commandBuffer);
                if (commandBuffer[0])
                { // if the first character is a null character
                    executeCommand(commandBuffer);
                    current_index = (int)history.size();
                    commandBuffer[0] = '\0';
                }
                ImGui::SetKeyboardFocusHere(-1);
            }
            // ImGui::SetItemDefaultFocus();

            // ImGui::End();
            // ImGui::EndChild();
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
            ImGui::PopStyleColor();
            ImGui::End();
        }
    };

}