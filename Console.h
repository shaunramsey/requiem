#pragma once
// console.h
#include "imgui.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cstdarg>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

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
         template <class Archive>
        void serialize(Archive &archive)
        {
            archive(cereal::make_nvp("logstring", myString), CEREAL_NVP(color.x), CEREAL_NVP(color.y), CEREAL_NVP(color.z), CEREAL_NVP(color.w));
        }
    };

    void pushNoRepeat(const std::string &s, std::vector<std::string> &vec)
    {
        if (vec.size() == 0 || vec.back() != s)
        {
            vec.push_back(s);
        }
    }

    class Console
    {

    private:
        std::vector<ColorString> log_history;
        std::vector<std::string> command_history;
        int current_index;
        const std::chrono::time_zone *new_york_tz = std::chrono::locate_zone("America/New_York");

    public:
        template <class Archive>
        void serialize(Archive &archive)
        {
            archive(CEREAL_NVP(command_history), CEREAL_NVP(log_history));
        }

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

        Console() : new_york_tz(std::chrono::locate_zone("America/New_York"))
        {
            log_history.push_back(ColorString(""));
            command_history.push_back("");
            current_index = 1;
        }

        void addLogHistory(ImVec4 color, const char *environment_desc, const char *prepend, const char *fstring, va_list args_list)
        {
            std::string inputString = formatString(fstring, args_list);
            const auto nynow = std::chrono::zoned_time(new_york_tz, std::chrono::system_clock::now());
            std::string now = "[";
            now += std::format("{:%d-%m-%Y %H:%M:%OS}", nynow) + "] [" + environment_desc + "] ";
            log_history.push_back(ColorString(now + std::string(prepend) + inputString, color));
        }

        void DebugLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr)
        {
            addLogHistory({0.0f, 0.9f, 0.0f, 1.0f}, environment_desc, "  DEBUG: ", fstring, args_list);
        }

        void WarningLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr)
        {
            addLogHistory({1.0f, 1.0f, 0.0f, 1.0f}, environment_desc, "WARNING: ", fstring, args_list);
        }

        void ErrorLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr)
        {
            addLogHistory({1.0f, 0.0f, 0.0f, 1.0f}, environment_desc, "  ERROR: ", fstring, args_list);
        }

        void Log(const char *environment_desc, const char *fstring, va_list args_list = nullptr)
        {
            addLogHistory({1.0f, 1.0f, 1.0f, 1.0f}, environment_desc, "    LOG: ", fstring, args_list);
        }

        int executeCommand(std::string s)
        {
            for (int i = 0; i < s.size(); i++)
            {
                s[i] = (char)tolower(s[i]);
            }
            if (s.size() == 0)
                return 0; // do nothing
            std::string debugLogMessage = " Executing Command: " + s;
            Log("executeCommand", debugLogMessage.c_str());
            if (s[0] == '!')
            { // repeat command
                int cmd_index = atoi(s.substr(1).c_str());
                if (cmd_index > 0 && cmd_index < command_history.size())
                {
                    executeCommand(command_history[cmd_index]);
                    return 0;
                }
                else
                {
                    pushNoRepeat(s, command_history);
                    log_history.push_back(ColorString("> " + s + " - COMMAND NOT FOUND", {1.0, 0.0, 0.0, 1.0}));
                    return 0;
                }
            }
            else if (s == "help")
            {
                pushNoRepeat(s, command_history);
                log_history.push_back("> " + s + " - available commands are: help, clear, history");
                log_history.push_back(ColorString("> clear - clears the console"));
                log_history.push_back(ColorString("> history - shows command history"));
                log_history.push_back(ColorString("> !<number> - repeat command number from history"));
            }
            else if (s == "clear" || s == "cls")
            {
                pushNoRepeat("clear", command_history);
                log_history.clear();
            }
            else if (s == "history")
            {
                pushNoRepeat("history", command_history);
                log_history.push_back("> " + s + " - command history:");
                for (int i = 1; i < command_history.size(); i++)
                {
                    log_history.push_back(ColorString("[" + std::to_string(i) + "] " + command_history[i], {0.8f, 0.8f, 0.8f, 1.0f}));
                }
            }
            else if (s == "save")
            {
                pushNoRepeat(s, command_history);
                cereal::JSONOutputArchive oarchive(std::cout);
                oarchive(*this);
            }
            else
            {
                pushNoRepeat(s, command_history);
                log_history.push_back(ColorString("> " + s + " - COMMAND NOT FOUND", {1.0, 0.0, 0.0, 1.0}));
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
                        current_index = (int)command_history.size() - 1;
                    }
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, command_history[current_index].c_str());
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    current_index++;
                    if (current_index > command_history.size() - 1)
                    {
                        current_index = 0;
                    }
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, command_history[current_index].c_str());
                }
            }
            return 0;
        }

        // void debugPrint(const char *fstring, ...)
        // {
        //     std::string final_string = formatString(fstring);
        //     log_history.push_back(ColorString(final_string, {0.7f, 0.9f, 0.9f, 1.0f}));
        // }

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

            float grey_value = 0.2f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(grey_value, grey_value, grey_value, 0.9f));
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
            for (int i = 0; i < log_history.size(); i++)
            {
                // debugPrint("imguiDisplay: " + std::to_string(log_history[i].color.x));
                ImGui::PushStyleColor(ImGuiCol_Text, log_history[i].color);
                ImGui::TextWrapped(log_history[i].myString.c_str());
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
                std::string debugCmd = "> ";
                debugCmd += commandBuffer;

                if (commandBuffer[0])
                { // if the first character is a null character
                    executeCommand(commandBuffer);
                    current_index = (int)command_history.size();
                    commandBuffer[0] = '\0';
                }
            }
            if ((ImGui::IsWindowFocused() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) && !ImGui::IsAnyItemActive())
                ImGui::SetKeyboardFocusHere(-1);
            // ImGui::SetKeyboardFocusHere(-1);
            //  ImGui::SetItemDefaultFocus();

            // ImGui::End();
            // ImGui::EndChild();
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
            ImGui::PopStyleColor();
            ImGui::End();
        }
    };

}