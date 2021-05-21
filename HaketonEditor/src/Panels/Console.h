#pragma once
#include "Haketon/Core/Core.h"
#include "imgui/imgui.h"
#include "spdlog/sinks/base_sink.h"

namespace Haketon
{
    
    // TODO: Add Log Categories like Unreal
    struct Console
    {
        char InputBuf[256];
        ImVector<char*> Items;
        ImVector<const char*> Commands;
        ImVector<char*> History;
        int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
        ImGuiTextFilter Filter;
        bool AutoScroll;
        bool ScrollToBottom;

        Console();

        ~Console();

        // Portable helpers
        static int Stricmp(const char* s1, const char* s2);

        static int Strnicmp(const char* s1, const char* s2, int n);

        static char* Strdup(const char* s);

        static void Strtrim(char* s);

        void ClearLog();

        void AddLog(const char* fmt, ...) IM_FMTARGS(2);

        void Draw(const char* title, bool* p_open);

        void ExecCommand(const char* command_line);

        // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
        static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

        int TextEditCallback(ImGuiInputTextCallbackData* data);

        int32_t LogSinkIndex = -1;
    };

    template<typename Mutex>
        class MySink : public spdlog::sinks::base_sink <Mutex>
    {
    public:

        Console* console;

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

            if(console)
            {
                console->AddLog(fmt::to_string(formatted).c_str());
            }
        }

        void flush_() override
        {
            
        }

       

    };
}
