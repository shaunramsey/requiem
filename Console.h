#pragma once
// console.h
#include "imgui.h"

#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdarg>
#include <toml++/toml.hpp>
#include <chrono>
#include "externs.h"

namespace Ramsey
{

    void pushNoRepeat(const std::string &s, std::vector<std::string> &vec);

    class ColorString
    {
    public:
        ColorString(std::string s = "", ImVec4 v = {1.0, 1.0, 1.0, 1.0});
        std::string myString;
        ImVec4 color;
        toml::array saveToml() const;
        void loadToml(const toml::array &arr);
    };

    class ColorTimedString : public ColorString
    {
    public:
        ColorTimedString(std::string s = "", ImVec4 v = {1.0, 1.0, 1.0, 1.0}, std::chrono::seconds d = std::chrono::seconds(10), std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now())
            : ColorString(s, v), time(t), duration(d) {}
        std::chrono::time_point<std::chrono::system_clock> time;
        std::chrono::seconds duration;
    };

    class Console
    {

    private:
        std::vector<ColorString> log_history;
        std::vector<std::string> command_history;
        std::vector<ColorTimedString> timed_log;

        int current_index;
        const std::chrono::time_zone *new_york_tz = std::chrono::locate_zone("America/New_York");

    public:
        void loadToml(toml::table &tbl);
        void saveToml(toml::table &tbl);
        static std::string formatString(const char *fstring, ...);

        Console();
        void addLogHistory(ImVec4 color, const char *environment_desc, const char *prepend, const char *fstring, va_list args_list);
        void DebugLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr);
        void WarningLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr);
        void ErrorLog(const char *environment_desc, const char *fstring, va_list args_list = nullptr);
        void Log(const char *environment_desc, const char *fstring, va_list args_list = nullptr);
        int executeCommand(std::string s);
        static int ConsoleCommandCallback(ImGuiInputTextCallbackData *data);
        int ActualConsoleCommandCallback(ImGuiInputTextCallbackData *data);
        void drawImGui(bool *p_open = NULL);
        void drawConsole(ImVec2 size, bool show = false);
        void drawToast(bool show = true);
    };
}