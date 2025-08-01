#include "Console.h"

#include "imgui.h"

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdarg>
#include <toml++/toml.hpp>
#include <chrono>
#include "externs.h"
#include "GameSettings.h"

namespace Ramsey
{

    ColorString::ColorString(std::string s, ImVec4 v)
    {
        myString = s;
        color = v;
    }

    toml::array ColorString::saveToml() const
    {
        toml::array arr;
        arr.push_back(myString);
        arr.push_back(color.x);
        arr.push_back(color.y);
        arr.push_back(color.z);
        arr.push_back(color.w);
        return arr;
    }
    void ColorString::loadToml(const toml::array &arr)
    {
        if (arr.size() >= 5)
        {
            myString = arr[0].value<std::string>().value_or("");
            color.x = arr[1].value<float>().value_or(1.0f);
            color.y = arr[2].value<float>().value_or(1.0f);
            color.z = arr[3].value<float>().value_or(1.0f);
            color.w = arr[4].value<float>().value_or(1.0f);
        }
    }

    void pushNoRepeat(const std::string &s, std::vector<std::string> &vec)
    {
        if (vec.size() == 0 || vec.back() != s)
        {
            vec.push_back(s);
        }
    }

    void Console::loadToml(toml::table &tbl)
    {
        log_history.clear();
        log_history.push_back(ColorString(""));
        command_history.clear();
        command_history.push_back("");
        current_index = 1;
        if (auto *cs_array = tbl["log_history"].as_array())
        {
            for (const auto &elem : *cs_array)
            {
                if (!elem.is_array())
                    continue;
                ColorString obj;
                obj.loadToml(*elem.as_array());
                log_history.push_back(obj);
            }
        }
        if (auto *cmd_array = tbl["command_history"].as_array())
        {
            for (const auto &elem : *cmd_array)
            {
                if (!elem.is_string())
                    continue;
                command_history.push_back(elem.value<std::string>().value_or(""));
            }
        }
    }
    // toml::value -- values
    // toml::array -- arrays
    // toml::table -- tables
    void Console::saveToml(toml::table &tbl)
    {
        toml::array command_array;
        for (size_t i = 1; i < command_history.size(); ++i)
        {
            command_array.push_back(command_history[i]);
        }
        tbl.insert("command_history", command_array);
        toml::array log_array;
        for (size_t i = 1; i < log_history.size(); ++i)
        {
            log_array.push_back(log_history[i].saveToml());
        }
        tbl.insert("log_history", log_array);
    }

    std::string Console::formatString(const char *fstring, va_list args)
    {
        /// va_list args;
        char *format = (char *)(fstring);
        /// va_start(args, fstring);
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
        /// va_end(args);
        return final_string;
    }

    Console::Console() : new_york_tz(std::chrono::locate_zone("America/New_York"))
    {
        log_history.push_back(ColorString(""));
        command_history.push_back("");
        current_index = 1;
    }

    void Console::addLogHistory(ImVec4 color, const char *environment_desc, const char *prepend, const char *fstring, va_list args_list)
    {
        std::string inputString = formatString(fstring, args_list);
        const auto nynow = std::chrono::zoned_time(new_york_tz, std::chrono::system_clock::now());
        std::string padding = "          ";
        if (environment_desc && strlen(environment_desc) < 10)
        {
            padding = std::string(10 - strlen(environment_desc), ' ');
        }
        std::string now = "[";
        now += std::format("{:%d-%m-%Y %H:%M:%OS}", nynow) + "] [" + padding + environment_desc + "] ";
        log_history.push_back(ColorString(now + std::string(prepend) + inputString, color));
        timed_log.push_back(ColorTimedString(now + std::string(prepend) + inputString, color));
    }

    void Console::DebugLog(const char *environment_desc, const char *fstring, ...)
    {
        va_list args;
        va_start(args, fstring);
        addLogHistory(gameSettings.consoleSettings.DebugColor, environment_desc, "[  DEBUG]: ", fstring, args);
        va_end(args);
    }

    void Console::WarningLog(const char *environment_desc, const char *fstring, ...)
    {
        va_list args;
        va_start(args, fstring);
        addLogHistory(gameSettings.consoleSettings.WarningColor, environment_desc, "[WARNING]: ", fstring, args);
        va_end(args);
    }

    void Console::ErrorLog(const char *environment_desc, const char *fstring, ...)
    {
        va_list args;
        va_start(args, fstring);
        addLogHistory(gameSettings.consoleSettings.ErrorColor, environment_desc, "[  ERROR]: ", fstring, args);
        va_end(args);
    }

    void Console::Log(const char *environment_desc, const char *fstring, ...)
    {
        va_list args;
        va_start(args, fstring);
        addLogHistory(gameSettings.consoleSettings.LogColor, environment_desc, "[    LOG]: ", fstring, args);
        va_end(args);
    }

    int Console::executeCommand(std::string s)
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
        else if (s.substr(0, 4) == "save")
        {
            pushNoRepeat(s, command_history);
            std::string filename = "log.toml";
            if (s.size() > 5)
            {
                filename = s.substr(5);
            }

            toml::table tbl;
            saveToml(tbl);
            std::ofstream ofs(filename);
            ofs << tbl; // put that toml right on out

            ofs.close();
        }
        else if (s.substr(0, 4) == "load")
        {
            pushNoRepeat(s, command_history);
            std::string filename = "log.toml";
            if (s.size() > 5)
            {
                filename = s.substr(5);
            }

            auto tbl = toml::parse_file(filename);
            loadToml(tbl);
        }
        else
        {
            pushNoRepeat(s, command_history);
            log_history.push_back(ColorString("> " + s + " - COMMAND NOT FOUND", {1.0, 0.0, 0.0, 1.0}));
        }
        return 0;
    }

    int Console::ConsoleCommandCallback(ImGuiInputTextCallbackData *data)
    {
        // debugPrint("ConsoleCommandCallback");
        return ((Console *)(data->UserData))->ActualConsoleCommandCallback(data);
    }

    int Console::ActualConsoleCommandCallback(ImGuiInputTextCallbackData *data)
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

    void Console::drawConsole(ImVec2 size, bool show)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, gameSettings.consoleSettings.MainConsoleBgColor);
        const static ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (!ImGui::Begin("Console", &show, winFlags))
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

        // pos[1] = size[1];
        // size[1] = 0.0f;;

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

    void Console::drawToast(bool show)
    {
        if (!show)
            return;
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        const static ImGuiWindowFlags toastFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
        if (!ImGui::Begin("ConsoleToast", nullptr, toastFlags))
        {
            ImGui::PopStyleColor();
            ImGui::End();
            return;
        }
        for (int i = 0; i < timed_log.size(); i++)
        {
            auto now = std::chrono::system_clock::now();
            if (timed_log[i].time + timed_log[i].duration < now)
            {
                // remove this log entry
                timed_log.erase(timed_log.begin() + i);
                --i; // adjust index since we removed an element
                continue;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, timed_log[i].color);
            ImGui::TextWrapped(timed_log[i].myString.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }

    void Console::drawImGui(bool *p_open)
    {

        ImGuiViewport *imguiViewport = ImGui::GetMainViewport();
        ImVec2 size = imguiViewport->WorkSize;
        ImVec2 pos = imguiViewport->WorkPos;
        pos.y += size[1];
        size[1] = (float)(size[1] * 0.7f);

        if (p_open && *p_open == true)
        {
            ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0, 1.0f));
            drawConsole(size);
        }

        size = imguiViewport->WorkSize;
        size[1] = 0.0f;
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0, 1.0f));
        bool showToast = true;
        if (p_open && *p_open == true)
        {
            showToast = false;
        }
        drawToast(showToast);
    }

} // namespace Ramsey