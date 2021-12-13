#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "scheduler.hpp"

#include "debug.hpp"
#include "gsc.hpp"
#include "json.hpp"
#include "scripting.hpp"
#include "game/scripting/execution.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>
#include <utils/compression.hpp>

#include <exception/minidump.hpp>

namespace exception
{
    namespace
    {
        thread_local struct
        {
            DWORD code = 0;
            PVOID address = nullptr;
        } exception_data;

        void show_mouse_cursor()
        {
            while (ShowCursor(TRUE) < 0);
        }

        void display_error_dialog()
        {
            std::string error_str = utils::string::va("Fatal error (0x%08X) at 0x%p.\n"
                                                      "A minidump has been written.\n\n",
                                                      exception_data.code, exception_data.address);

            error_str += "Make sure to update your graphics card drivers and install operating system updates!";

            utils::thread::suspend_other_threads();
            show_mouse_cursor();
            MessageBoxA(nullptr, error_str.data(), "Plutonium T6 ERROR", MB_ICONERROR);
            TerminateProcess(GetCurrentProcess(), exception_data.code);
        }

        void reset_state()
        {
            display_error_dialog();
        }

        __declspec(naked) void reset_state_stub()
        {
            __asm
            {
                sub esp, 0x10
                or esp, 0x8
                jmp reset_state
            }
        }

        std::string get_timestamp()
        {
            tm ltime{};
            char timestamp[MAX_PATH] = {0};
            const auto time = _time64(nullptr);

            _localtime64_s(&ltime, &time);
            strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

            return timestamp;
        }

        std::string generate_crash_info(const LPEXCEPTION_POINTERS exceptioninfo)
        {
            std::string info{};
            const auto line = [&info](const std::string& text)
            {
                info.append(text);
                info.append("\r\n");
            };

            line("Plutonium T6 Crash Dump");
            line("");
            line("Timestamp: "s + get_timestamp());
            line(utils::string::va("Exception: 0x%08X", exceptioninfo->ExceptionRecord->ExceptionCode));
            line(utils::string::va("Address: 0x%lX", exceptioninfo->ExceptionRecord->ExceptionAddress));

#pragma warning(push)
#pragma warning(disable: 4996)
            OSVERSIONINFOEXA version_info;
            ZeroMemory(&version_info, sizeof(version_info));
            version_info.dwOSVersionInfoSize = sizeof(version_info);
            GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&version_info));
#pragma warning(pop)

            line(utils::string::va("OS Version: %u.%u", version_info.dwMajorVersion, version_info.dwMinorVersion));

            return info;
        }

        std::string generate_gsc_dump()
        {
            std::string info{};
            const auto line = [&info](const std::string& text)
            {
                info.append(text);
                info.append("\r\n");
            };

            line("Call stack trace:");
            line(debug::get_call_stack(true));

            return info;
        }

        void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo)
        {
            const std::string crash_name = utils::string::va("minidumps/plutonium-t6-crash-%s.zip",
                                                             get_timestamp().data());

            utils::compression::zip::archive zip_file{};
            zip_file.add("crash.dmp", create_minidump(exceptioninfo));
            zip_file.add("info.txt", generate_crash_info(exceptioninfo));
            zip_file.add("gsc_vm_dump.txt", generate_gsc_dump());
            zip_file.write(crash_name, "Plutonium T6 Crash Dump");
        }

        bool is_harmless_error(const LPEXCEPTION_POINTERS exceptioninfo)
        {
            const auto code = exceptioninfo->ExceptionRecord->ExceptionCode;
            return code == STATUS_INTEGER_OVERFLOW || code == STATUS_FLOAT_OVERFLOW || code == STATUS_SINGLE_STEP;
        }

        LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS exceptioninfo)
        {
            if (is_harmless_error(exceptioninfo))
            {
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            write_minidump(exceptioninfo);

            exception_data.code = exceptioninfo->ExceptionRecord->ExceptionCode;
            exception_data.address = exceptioninfo->ExceptionRecord->ExceptionAddress;
            exceptioninfo->ContextRecord->Eip = reinterpret_cast<DWORD>(&reset_state_stub);

            return EXCEPTION_CONTINUE_EXECUTION;
        }

        LPTOP_LEVEL_EXCEPTION_FILTER WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER)
        {
            // Don't register anything here...
            return &exception_filter;
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            if (!strstr(GetCommandLineA(), "--gsc-debug"))
            {
                return;
            }

            SetUnhandledExceptionFilter(exception_filter);
            utils::hook::jump(reinterpret_cast<uintptr_t>(&SetUnhandledExceptionFilter), set_unhandled_exception_filter_stub);
        }
    };
}

REGISTER_COMPONENT(exception::component)
